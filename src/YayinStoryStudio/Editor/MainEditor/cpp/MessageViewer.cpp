#include "Editor/MainEditor/MessageViewer.h"
#include "Editor/MainEditor/MainWin.h"
#include "Editor/MainEditor/FileEditWidgetArea.h"
#include <QtCore/qfileinfo.h>
#include <Editor/DocumentMessageManager.h>
#include <General/TranslationHost.h>
#include <Editor/SyntaxHighlighter.h>
#include <General/Log.h>
#include <QtWidgets/qheaderview.h>
#include <QtWidgets/qscrollbar.h>
#include <Utility/FileUtility.h>
#include <Editor/FileEditWidget.h>
#include <Editor/FileServerManager.h>
#include <QtWidgets/qlabel.h>

namespace YSS::Editor {
	MessageViewer::MessageViewer(QWidget* parent) :QFrame(parent) {
		MessageTable = new QTableWidget(this);
		MessageTable->setColumnCount(5); // code, message, file, line, column
		MessageTable->setHorizontalHeaderLabels({
				VITR("YSS::editor.messageViewer.code"),
				VITR("YSS::editor.messageViewer.message"),
				VITR("YSS::editor.messageViewer.file"),
				VITR("YSS::editor.messageViewer.line"),
				VITR("YSS::editor.messageViewer.column")
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
		connect(YSSCore::Editor::DocumentMessageManager::getInstance(),
			&YSSCore::Editor::DocumentMessageManager::messageChanged, this, &MessageViewer::onMessageChanged);
		connect(YSSCore::Editor::DocumentMessageManager::getInstance(),
			&YSSCore::Editor::DocumentMessageManager::messageChangedForLine, this, &MessageViewer::onMessageChangedForLine);
		connect(MessageTable, &QTableWidget::cellClicked, this, &MessageViewer::onCellClicked);
		connect(MainWin::getInstance()->getFileEditWidgetArea(), &FileEditWidgetArea::currentFileChanged, this, &MessageViewer::changeCurrentFile);
		changeCurrentFile(MainWin::getInstance()->getFileEditWidgetArea()->getCurrentWidgetFilePath());
	}

	void MessageViewer::changeCurrentFile(const QString& filePath) {
		if (filePath.isEmpty()) {
			MessageTable->clearContents();
			MessageTable->setRowCount(0);
			CurrentFilePath = "";
			return;
		}
		if (CurrentFilePath == filePath) {
			return;
		}
		CurrentFilePath = filePath;
		MessageTable->clearContents();
		MessageTable->setRowCount(0);
		onMessageChanged(filePath);
	}

	void MessageViewer::onCellClicked(int row, int column) {
		QString filePath = MessageTable->item(row, 2)->toolTip();
		qint32 lineNumber = MessageTable->item(row, 3)->text().toInt() - 1;
		qint32 columnNumber = MessageTable->item(row, 4)->text().toInt();
		//vgDebug << "Redirection requested to " << filePath << ":" << lineNumber << ":" << columnNumber;
		emit YSSCore::Editor::FileServerManager::getInstance()->focusOnFile(filePath, lineNumber, columnNumber);
	}

	void MessageViewer::onMessageChanged(const QString& filePath) {
		//vgDebug << "Message changed for file: " << filePath;
		if (filePath == CurrentFilePath) {
			MessageTable->clearContents();
			MessageTable->setRowCount(0);
			auto messages = YSSCore::Editor::DocumentMessageManager::getInstance()->getAllMessages(filePath);
			qint32 totalLines = 0;
			for (auto msgList : messages.values()) {
				totalLines += msgList.size();
			}
			MessageTable->setRowCount(totalLines);
			qint32 row = 0;
			for (auto msgList : messages.values()) {
				for (auto msg : msgList) {
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
					row++;
				}
			}
		}
	}

	void MessageViewer::onMessageChangedForLine(const QString& filePath, qint32 lineNumber) {
		// search all line == lineNumber, remove it
		//vgDebug << filePath << CurrentFilePath;
		if (filePath != CurrentFilePath) {
			return;
		}
		for (int i = 0; i < MessageTable->rowCount(); ++i) {
			if (MessageTable->item(i, 2)->toolTip() == filePath && MessageTable->item(i, 3)->text().toInt() == lineNumber + 1) {
				MessageTable->removeRow(i);
				//vgDebug << "Removed message for line " << lineNumber << " at row " << i;
				--i;
			}
		}
		auto msgList = YSSCore::Editor::DocumentMessageManager::getInstance()->getMessages(filePath, lineNumber);
		//vgDebug << "Line " << lineNumber << " has " << msgList.size() << " messages.";
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
		}
		if (msgList.size() != 0) {
			MessageTable->sortByColumn(3, Qt::AscendingOrder);
		}
	}
}