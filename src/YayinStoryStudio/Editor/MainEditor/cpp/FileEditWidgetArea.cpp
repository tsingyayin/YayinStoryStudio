#include "Editor/MainEditor/FileEditWidgetArea.h"
#include "Editor/MainEditor/private/StackComponents_p.h"
#include <QtWidgets/qsplitter.h>
#include "Editor/GlobalValue.h"
#include <General/YSSProject.h>
#include <Editor/TextEdit.h>
#include <General/Log.h>
#include <Editor/FileServerManager.h>
#include <QtCore/qfileinfo.h>
#include <General/TranslationHost.h>
namespace YSS::Editor {
	class FileEditWidgetAreaPrivate {
		friend class FileEditWidgetArea;
	protected:
		StackTagWidget* TagArea;
		QFrame* ContentArea = nullptr;
		QVBoxLayout* Layout;
		MessageViewer* MsgViewer;
		DefaultStackWidgetCentralArea* CentralArea;
	};

	FileEditWidgetArea::FileEditWidgetArea(QWidget* parent) :QFrame(parent) {
		d = new FileEditWidgetAreaPrivate;
		d->Layout = new QVBoxLayout(this);
		d->TagArea = new StackTagWidget(this);
		d->TagArea->setFixedHeight(32);
		d->Layout->addWidget(d->TagArea);
		d->CentralArea = new DefaultStackWidgetCentralArea(this);
		d->CentralArea->setText(VITR("YSS::editor.stackWidgetArea.noFileOpened"));
		d->ContentArea = d->CentralArea;
		d->Layout->setSpacing(1);
		//d->Layout->setContentsMargins(0, 0, 0, 0);
		d->Layout->addWidget(d->ContentArea);
		d->MsgViewer = new MessageViewer(this);
		d->MsgViewer->setFixedHeight(260);
		d->Layout->addWidget(d->MsgViewer);
		connect(d->TagArea, &StackTagWidget::switchToFile, this, [this](const QString& filePath) {
			setCurrentWidget(filePath);
			});
		connect(d->TagArea, &StackTagWidget::closeFile, this, [this](const QString& filePath) {
			closeWidget(filePath);
			});
		connect(d->TagArea, &StackTagWidget::renameRequested, this, [this](const QString& oldPath) {
			emit renameRequested(oldPath);
			});
		connect(d->TagArea, &StackTagWidget::saveAsRequested, this, [this](const QString& filePath) {
			emit saveAsRequested(filePath);
			});
		connect(d->TagArea, &StackTagWidget::closeAllRequested, this, [this]() {
			closeAll();
			});
		connect(d->TagArea, &StackTagWidget::closeSavedRequested, this, [this]() {
			closeSaved();
			});
	}

	void FileEditWidgetArea::addWidget(YSSCore::Editor::FileEditWidget* widget) {
		QString filePath = widget->getFilePath();
		vgDebug << filePath;
		if (d->TagArea->containsStackLabel(filePath)) {
			setCurrentWidget(filePath);
			return;
		}
		widget->setParent(this);
		d->TagArea->addStackLabel(filePath);
		connect(widget, &YSSCore::Editor::FileEditWidget::fileChanged, d->TagArea, &StackTagWidget::setFileChanged);
		connect(widget, &YSSCore::Editor::FileEditWidget::fileChangeCanceled, d->TagArea, &StackTagWidget::cancelFileChanged);
		connect(widget, &YSSCore::Editor::FileEditWidget::fileSaved, d->TagArea, &StackTagWidget::cancelFileChanged);
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

	FileEditWidgetArea::~FileEditWidgetArea() {
		for(auto widget: YSSFSM->getAllFileEditWidgets()) {
			widget->setParent(nullptr);
			widget->closeFile();
		}
		delete d;
	}

	void FileEditWidgetArea::closeAll(bool autoGiveup) {
		for(auto widget: YSSFSM->getAllFileEditWidgets()) {
			vgDebug << "close " << widget->getFilePath();
			widget->closeFile(autoGiveup);
		}
	}

	void FileEditWidgetArea::closeSaved() {
		for(auto widget: YSSFSM->getAllFileEditWidgets()) {
			QString filePath = widget->getFilePath();
			if (not widget->isFileChanged() and not d->TagArea->isStackLabelPinned(filePath)) {
				vgDebug << "close " << filePath;
				widget->closeFile();
			}
		}
	}
	void FileEditWidgetArea::closeWidget(YSSCore::Editor::FileEditWidget* widget) {
		closeWidget(widget->getFilePath());
	}

	void FileEditWidgetArea::closeWidget(const QString& filePath) {
		QFileInfo fileInfo(filePath);
		QString absPath = fileInfo.absoluteFilePath();
		if (not YSSFSM->getAllOpenedFilePaths().contains(absPath)) {
			return;
		}
		YSSCore::Editor::FileEditWidget* widget = YSSFSM->getFileEditWidget(absPath);
		if (not widget) {
			return;
		}
		widget->closeFile();
	}

	void FileEditWidgetArea::setCurrentWidget(YSSCore::Editor::FileEditWidget* widget) {
		setCurrentWidget(widget->getFilePath());
	}

	void FileEditWidgetArea::setCurrentWidget(const QString& filePath) {
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

	void FileEditWidgetArea::setCurrentWidget(YSSCore::Editor::FileEditWidget* widget, qint32 lineNumber, qint32 column) {
		setCurrentWidget(widget->getFilePath(), lineNumber, column);
	}

	void FileEditWidgetArea::setCurrentWidget(const QString& filePath, qint32 lineNumber, qint32 column) {
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

	YSSCore::Editor::FileEditWidget* FileEditWidgetArea::getCurrentWidget() const {
		if (d->ContentArea == d->CentralArea) {
			return nullptr;
		}
		return (YSSCore::Editor::FileEditWidget*)d->ContentArea;
	}

	void FileEditWidgetArea::setMessageViewerEnable(bool enable) {
		if (enable) {
			d->MsgViewer->show();
		}
		else {
			d->MsgViewer->hide();
		}
	}

	void FileEditWidgetArea::resizeEvent(QResizeEvent* event) {
		QFrame::resizeEvent(event);
		if (d->ContentArea) {
			//d->ContentArea->setFixedHeight(this->height() - d->TagArea->height() - (d->MsgViewer->isVisible() ? d->MsgViewer->height() : 0));
		}
		//d->CentralArea->setFixedHeight(this->height() - d->TagArea->height() - (d->MsgViewer->isVisible() ? d->MsgViewer->height() : 0));
	}
}