#include "Editor/MainEditor/StackWidgetArea.h"
#include "Editor/MainEditor/private/StackWidgetArea_p.h"
#include <QtWidgets/qsplitter.h>
#include "Editor/GlobalValue.h"
#include <General/YSSProject.h>
#include <Editor/TextEdit.h>
#include <General/Log.h>
#include <Editor/FileServerManager.h>
#include <QtCore/qfileinfo.h>
namespace YSS::Editor {
	class StackWidgetAreaPrivate {
		friend class StackWidgetArea;
	protected:
		StackWidgetTagArea* TagArea;
		QFrame* ContentArea = nullptr;
		QVBoxLayout* Layout;
		MessageViewer* MsgViewer;
		DefaultStackWidgetCentralArea* CentralArea;
	};

	StackWidgetArea::StackWidgetArea(QWidget* parent) :QFrame(parent) {
		d = new StackWidgetAreaPrivate;
		d->Layout = new QVBoxLayout(this);
		d->TagArea = new StackWidgetTagArea(this);
		d->TagArea->setFixedHeight(40);
		d->Layout->addWidget(d->TagArea);
		d->CentralArea = new DefaultStackWidgetCentralArea(this);
		d->ContentArea = d->CentralArea;
		//d->Layout->setContentsMargins(0, 0, 0, 0);
		d->Layout->addWidget(d->ContentArea);
		d->MsgViewer = new MessageViewer(this);
		d->MsgViewer->setFixedHeight(220);
		d->Layout->addWidget(d->MsgViewer);
		connect(d->TagArea, &StackWidgetTagArea::switchToFile, this, [this](const QString& filePath) {
			setCurrentWidget(filePath);
			});
		connect(d->MsgViewer, &MessageViewer::redirectionRequired, this, [this](const QString& filePath, qint32 lineNumber, qint32 column) {
			setCurrentWidget(filePath, lineNumber, column);
			});
		connect(d->TagArea, &StackWidgetTagArea::closeFile, this, [this](const QString& filePath) {
			closeWidget(filePath);
			});
		connect(d->TagArea, &StackWidgetTagArea::renameRequested, this, [this](const QString& oldPath) {
			emit renameRequested(oldPath);
			});
	}

	void StackWidgetArea::addWidget(YSSCore::Editor::FileEditWidget* widget) {
		QString filePath = widget->getFilePath();
		vgDebug << filePath;
		if (d->TagArea->containsStackLabel(filePath)) {
			setCurrentWidget(filePath);
			return;
		}
		widget->setParent(this);
		d->TagArea->addStackLabel(filePath);
		connect(widget, &YSSCore::Editor::FileEditWidget::fileChanged, d->TagArea, &StackWidgetTagArea::setFileChanged);
		connect(widget, &YSSCore::Editor::FileEditWidget::fileChangeCanceled, d->TagArea, &StackWidgetTagArea::cancelFileChanged);
		connect(widget, &YSSCore::Editor::FileEditWidget::fileSaved, d->TagArea, &StackWidgetTagArea::cancelFileChanged);
		connect(widget, &YSSCore::Editor::FileEditWidget::fileClosed, this, [this, widget]() {
			d->TagArea->removeStackLabel(widget->getFilePath()); // this function handle re-choice if the closed widget is current one
			GlobalValue::getCurrentProject()->removeEditorOpenedFile(widget->getFilePath());
			});
		connect(widget, &YSSCore::Editor::FileEditWidget::fileRenamed, this, [this](const QString& oldPath, const QString& newPath) {
			d->TagArea->changeStackLabel(oldPath, newPath);
			GlobalValue::getCurrentProject()->removeEditorOpenedFile(oldPath);
			GlobalValue::getCurrentProject()->addEditorOpenedFile(newPath);
			});
		YSSCore::Editor::TextEdit* textEdit = qobject_cast<YSSCore::Editor::TextEdit*>(widget);
		if (textEdit) {
			textEdit->setHoverArea(this);
		}
		GlobalValue::getCurrentProject()->addEditorOpenedFile(filePath);
		setCurrentWidget(filePath);
	}

	StackWidgetArea::~StackWidgetArea() {
		for(auto widget: YSSFSM->getAllFileEditWidgets()) {
			widget->setParent(nullptr);
			widget->close();
		}
		delete d;
	}

	void StackWidgetArea::closeAll() {
		for(auto widget: YSSFSM->getAllFileEditWidgets()) {
			vgDebug << "close " << widget->getFilePath();
			widget->close();
		}
	}
	void StackWidgetArea::closeWidget(YSSCore::Editor::FileEditWidget* widget) {
		closeWidget(widget->getFilePath());
	}

	void StackWidgetArea::closeWidget(const QString& filePath) {
		QFileInfo fileInfo(filePath);
		QString absPath = fileInfo.absoluteFilePath();
		if (not YSSFSM->getAllOpenedFilePaths().contains(absPath)) {
			return;
		}
		YSSCore::Editor::FileEditWidget* widget = YSSFSM->getFileEditWidget(absPath);
		if (not widget) {
			return;
		}
		widget->close();
	}

	void StackWidgetArea::setCurrentWidget(YSSCore::Editor::FileEditWidget* widget) {
		setCurrentWidget(widget->getFilePath());
	}

	void StackWidgetArea::setCurrentWidget(const QString& filePath) {
		if (filePath.isEmpty()) {
			d->ContentArea->hide();
			d->Layout->removeWidget(d->ContentArea);
			d->ContentArea = d->CentralArea;
			d->Layout->insertWidget(1, d->ContentArea);
			//d->ContentArea->setFixedHeight(this->height() - d->TagArea->height() - (d->MsgViewer->isVisible() ? d->MsgViewer->height() : 0));
			d->ContentArea->show();
		}
		QFileInfo fileInfo(filePath);
		QString absPath = fileInfo.absoluteFilePath();
		if (not YSSFSM->getAllOpenedFilePaths().contains(absPath)) {
			return;
		}
		YSSCore::Editor::FileEditWidget* widget = YSSFSM->getFileEditWidget(absPath);
		if (not widget) {
			return;
		}
		if (d->ContentArea != widget) {
			d->ContentArea->hide();
			d->Layout->removeWidget(d->ContentArea);
			d->ContentArea = widget;
			d->Layout->insertWidget(1, d->ContentArea);
			//d->ContentArea->setFixedHeight(this->height() - d->TagArea->height() - (d->MsgViewer->isVisible() ? d->MsgViewer->height() : 0));
			d->ContentArea->show();
		}
		GlobalValue::getCurrentProject()->setFocusedFile(filePath);
		d->MsgViewer->changeCurrentFile(filePath);
		d->TagArea->setCurrentStackLabel(filePath);
	}

	void StackWidgetArea::setCurrentWidget(YSSCore::Editor::FileEditWidget* widget, qint32 lineNumber, qint32 column) {
		setCurrentWidget(widget->getFilePath(), lineNumber, column);
	}

	void StackWidgetArea::setCurrentWidget(const QString& filePath, qint32 lineNumber, qint32 column) {
		if (filePath.isEmpty()) {
			d->ContentArea->hide();
			d->Layout->removeWidget(d->ContentArea);
			d->ContentArea = d->CentralArea;
			d->Layout->insertWidget(1, d->ContentArea);
			//d->ContentArea->setFixedHeight(this->height() - d->TagArea->height() - (d->MsgViewer->isVisible() ? d->MsgViewer->height() : 0));
			d->ContentArea->show();
		}
		QFileInfo fileInfo(filePath);
		QString absPath = fileInfo.absoluteFilePath();
		if (not YSSFSM->getAllOpenedFilePaths().contains(absPath)) {
			return;
		}
		YSSCore::Editor::FileEditWidget* widget = YSSFSM->getFileEditWidget(absPath);
		if (not widget) {
			return;
		}
		setCurrentWidget(widget);
		widget->cursorToPosition(lineNumber, column);
	}

	YSSCore::Editor::FileEditWidget* StackWidgetArea::getCurrentWidget() const {
		if (d->ContentArea == d->CentralArea) {
			return nullptr;
		}
		return (YSSCore::Editor::FileEditWidget*)d->ContentArea;
	}

	void StackWidgetArea::setMessageViewerEnable(bool enable) {
		if (enable) {
			d->MsgViewer->show();
		}
		else {
			d->MsgViewer->hide();
		}
	}

	void StackWidgetArea::resizeEvent(QResizeEvent* event) {
		QFrame::resizeEvent(event);
		if (d->ContentArea) {
			//d->ContentArea->setFixedHeight(this->height() - d->TagArea->height() - (d->MsgViewer->isVisible() ? d->MsgViewer->height() : 0));
		}
		//d->CentralArea->setFixedHeight(this->height() - d->TagArea->height() - (d->MsgViewer->isVisible() ? d->MsgViewer->height() : 0));
	}
}