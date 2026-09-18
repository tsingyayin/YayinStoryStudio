#include <QtCore/qfileinfo.h>
#include <QtWidgets/qheaderview.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qscrollbar.h>
#include <Editor/FileEditWidget.h>
#include <Editor/FileServerManager.h>
#include <Editor/SyntaxHighlighter.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <Utility/FileUtility.h>
#include "Editor/MainEditor/DocumentMessageTracer.h"
#include "Editor/MainEditor/MessageCenter.h"
#include "Editor/MainEditor/MessageViewer.h"

namespace YSS::Editor {
	MessageViewerVFS::MessageViewerVFS(YSSCore::Editor::EditorPlugin* plugin) :
		YSSCore::Editor::FileServer("YSS Built-in Document Message Viewer", "cn.yxgeneral.yss_builtin.messageViewerVFS", plugin) {
		setEditorType(YSSCore::Editor::FileServer::BuiltInEditor);
		setSupportedFileExts({ "YSS.MainEditor.MessageViewer" });
		setAsVitrualFileServer(true);
		setPreferredOrientation(YSSCore::Editor::FileServer::Horizontal_Narrow);
		setListAsTool(true);
		setToolNickname("i18n:YSS::editor.messageViewer.title");
	}

	YSSCore::Editor::FileEditWidget* MessageViewerVFS::onCreateFileEditWidget() {
		return new MessageViewer();
	}

	MessageViewer::MessageViewer(QWidget* parent) :YSSCore::Editor::FileEditWidget(parent) {
		MessageTable = new QTableWidget(this);
		MessageTable->setColumnCount(5); // code, message, file, line, column
		MessageTable->setHorizontalHeaderLabels({
				VITRL("YSS::editor.messageViewer.code"),
				VITRL("YSS::editor.messageViewer.message"),
				VITRL("YSS::editor.messageViewer.file"),
				VITRL("YSS::editor.messageViewer.line"),
				VITRL("YSS::editor.messageViewer.column")
			});
		Layout = new QVBoxLayout(this);
		Layout->setContentsMargins(0, 0, 0, 0);
		Layout->addWidget(MessageTable);
		MessageTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
		MessageTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
		MessageTable->setColumnWidth(0, 100);
		MessageTable->setColumnWidth(3, 100);
		MessageTable->setColumnWidth(4, 100);
		MessageTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
		connect(MessageTable, &QTableWidget::cellClicked, this, &MessageViewer::onCellClicked);
		connect(MessageCenter::getInstance(), &MessageCenter::currentEditChanged, this, &MessageViewer::changeCurrentFile);
		connect(MessageCenter::getInstance(), &MessageCenter::messageChangedForLine, this, &MessageViewer::onMessageChangedForLine);
		connect(MessageCenter::getInstance(), &MessageCenter::tracerLineChanged, this, &MessageViewer::onTracerLineChanged);
		connect(MessageCenter::getInstance(), &MessageCenter::tracerDestroyed, this, &MessageViewer::onTracerDestroyed);
		changeCurrentFile(MessageCenter::getInstance()->getCurrentEdit());
	}

	void MessageViewer::changeCurrentFile(YSSCore::Editor::TextEdit* edit) {
		if (CurrentEdit == edit) {
			return;
		}
		CurrentEdit = edit;
		MessageTable->clearContents();
		MessageTable->setRowCount(0);
		syncMessageRows();
	}

	void MessageViewer::onCellClicked(int row, int column) {
		QString filePath = MessageTable->item(row, 2)->toolTip();
		qint32 lineNumber = MessageTable->item(row, 3)->text().toInt() - 1;
		qint32 columnNumber = MessageTable->item(row, 4)->text().toInt();
		//vgDebug << "Redirection requested to " << filePath << ":" << lineNumber << ":" << columnNumber;
		emit YSSCore::Editor::FileServerManager::getInstance()->focusOnFile(filePath, lineNumber, columnNumber);
	}

	void MessageViewer::onMessageChangedForLine(qint32 lineNumber) {
		if (!CurrentEdit || lineNumber < 0) {
			return;
		}
		QString filePath = CurrentEdit->getFilePath();
		// search all line == lineNumber, remove it
		for (int i = 0; i < MessageTable->rowCount(); ++i) {
			if (MessageTable->item(i, 2)->toolTip() == filePath && MessageTable->item(i, 3)->text().toInt() == lineNumber + 1) {
				MessageTable->removeRow(i);
				--i;
			}
		}
		insertMessagesForLine(lineNumber);
	}

	void MessageViewer::onTracerLineChanged(DocumentMessageTracer* tracer, qint32 oldLine, qint32 newLine) {
		if (tracer == nullptr || newLine < 0) {
			return;
		}
		for (int i = 0; i < MessageTable->rowCount(); ++i) {
			if (rowTracer(i) == reinterpret_cast<quintptr>(tracer)) {
				QTableWidgetItem* lineItem = MessageTable->item(i, 3);
				if (lineItem != nullptr) {
					lineItem->setText(QString::number(newLine + 1));
				}
			}
		}
	}

	void MessageViewer::removeRowsForTracer(DocumentMessageTracer* tracer) {
		if (tracer == nullptr) {
			return;
		}
		for (int i = MessageTable->rowCount() - 1; i >= 0; --i) {
			if (rowTracer(i) == reinterpret_cast<quintptr>(tracer)) {
				MessageTable->removeRow(i);
			}
		}
	}

	void MessageViewer::onTracerDestroyed(DocumentMessageTracer* tracer) {
		removeRowsForTracer(tracer);
	}

	quintptr MessageViewer::rowTracer(int row) const {
		QTableWidgetItem* item = MessageTable->item(row, 3);
		if (item == nullptr) {
			return 0;
		}
		return item->data(Qt::UserRole).value<quintptr>();
	}

	void MessageViewer::setRowTracer(int row, DocumentMessageTracer* tracer) {
		QTableWidgetItem* item = MessageTable->item(row, 3);
		if (item == nullptr) {
			return;
		}
		item->setData(Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(tracer)));
	}

	void MessageViewer::insertMessagesForLine(qint32 lineNumber) {
		if (!CurrentEdit || lineNumber < 0) {
			return;
		}
		auto msgList = CurrentEdit->getMessages(lineNumber);
		if (msgList.isEmpty()) {
			return;
		}
		QString filePath = CurrentEdit->getFilePath();
		DocumentMessageTracer* tracer = MessageCenter::getInstance()->getTracer(lineNumber);
		for (auto msg : msgList) {
			int row = MessageTable->rowCount();
			MessageTable->insertRow(row);
			auto msgCode = new QLabel();
			msgCode->setText(QString("<a href=\"%1\">%2</a>").arg(msg.getHelpUrl().toString()).arg(msg.getCode()));
			msgCode->setOpenExternalLinks(true);
			msgCode->setAlignment(Qt::AlignCenter);
			MessageTable->setCellWidget(row, 0, msgCode);
			auto messageItem = new QTableWidgetItem(msg.getMessage());
			messageItem->setToolTip(msg.getFixAdvice());
			MessageTable->setItem(row, 1, messageItem);
			auto filePathItem = new QTableWidgetItem(QFileInfo(filePath).fileName());
			filePathItem->setToolTip(filePath);
			MessageTable->setItem(row, 2, filePathItem);
			auto lineItem = new QTableWidgetItem(QString::number(msg.getLineNumber() + 1));
			lineItem->setTextAlignment(Qt::AlignCenter);
			MessageTable->setItem(row, 3, lineItem);
			auto columnItem = new QTableWidgetItem(QString::number(msg.getColumnNumber()));
			columnItem->setTextAlignment(Qt::AlignCenter);
			MessageTable->setItem(row, 4, columnItem);
			setRowTracer(row, tracer);
		}
		MessageTable->sortByColumn(3, Qt::AscendingOrder);
	}

	// 整个列表重建一次：按层里现有的追踪器补行、删掉追踪器已经不在的行。换文件或面板刚打开时用。
	void MessageViewer::syncMessageRows() {
		if (!CurrentEdit) {
			return;
		}
		const QList<DocumentMessageTracer*> tracers = MessageCenter::getInstance()->getTracers();
		for (int i = MessageTable->rowCount() - 1; i >= 0; --i) {
			quintptr rowTracerValue = rowTracer(i);
			bool alive = false;
			for (DocumentMessageTracer* tracer : tracers) {
				if (reinterpret_cast<quintptr>(tracer) == rowTracerValue) {
					alive = true;
					break;
				}
			}
			if (not alive) {
				MessageTable->removeRow(i);
			}
		}
		for (DocumentMessageTracer* tracer : tracers) {
			qint32 lineNumber = tracer->getBlockNumber();
			bool found = false;
			for (int i = 0; i < MessageTable->rowCount(); ++i) {
				if (rowTracer(i) == reinterpret_cast<quintptr>(tracer)) {
					QTableWidgetItem* lineItem = MessageTable->item(i, 3);
					if (lineItem != nullptr) {
						lineItem->setText(QString::number(lineNumber + 1));
					}
					found = true;
				}
			}
			if (not found) {
				insertMessagesForLine(lineNumber);
			}
		}
	}

	bool MessageViewer::onVirtualOpen(const QString& ext, const QString& fileName, const QString& param) {
		if (ext == "YSS.MainEditor.MessageViewer") {
			return true;
		}
		return false;
	}
}