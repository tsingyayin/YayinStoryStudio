#include "Editor/MainEditor/StackWidgetArea.h"
#include "Editor/MainEditor/private/StackWidgetArea_p.h"
#include <QtWidgets/qsplitter.h>
#include "Editor/GlobalValue.h"
#include <General/YSSProject.h>
#include <Editor/TextEdit.h>
namespace YSS::Editor {
	class StackWidgetAreaPrivate {
		friend class StackWidgetArea;
	protected:
		StackWidgetTagArea* TagArea;
		QFrame* ContentArea = nullptr;
		QMap<QString, YSSCore::Editor::FileEditWidget*> WidgetMap;
		QVBoxLayout* Layout;
		MessageViewer* MsgViewer;
		DefaultStackWidgetCentralArea* CentralArea;
	};

	StackWidgetArea::StackWidgetArea(QWidget* parent) :QFrame(parent) {
		d = new StackWidgetAreaPrivate;
		d->TagArea = new StackWidgetTagArea(this);
		d->TagArea->setFixedHeight(50);
		d->CentralArea = new DefaultStackWidgetCentralArea(this);
		d->ContentArea = d->CentralArea;
		d->Layout = new QVBoxLayout(this);
		//d->Layout->setContentsMargins(0, 0, 0, 0);
		d->Layout->addWidget(d->TagArea);
		d->Layout->addWidget(d->ContentArea);
		QFrame* line = new QFrame(this);
		line->setFrameShape(QFrame::HLine);
		line->setFrameShadow(QFrame::Sunken);
		d->Layout->addWidget(line);
		d->MsgViewer = new MessageViewer(this);
		d->MsgViewer->setFixedHeight(260);
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
	}

	void StackWidgetArea::addWidget(YSSCore::Editor::FileEditWidget* widget) {
		QString filePath = widget->getFilePath();
		if (d->WidgetMap.contains(filePath)) {
			setCurrentWidget(filePath);
			widget->deleteLater();
			return;
		}
		widget->setParent(this);
		d->WidgetMap[filePath] = widget;
		d->TagArea->addStackLabel(filePath);
		YSSCore::Editor::TextEdit* textEdit = qobject_cast<YSSCore::Editor::TextEdit*>(widget);
		if (textEdit) {
			textEdit->setHoverArea(this);
		}
		GlobalValue::getCurrentProject()->addEditorOpenedFile(filePath);
		setCurrentWidget(filePath);
	}

	StackWidgetArea::~StackWidgetArea() {
		delete d;
	}

	void StackWidgetArea::closeWidget(YSSCore::Editor::FileEditWidget* widget) {
		closeWidget(widget->getFilePath());
	}

	void StackWidgetArea::closeWidget(const QString& filePath) {
		if (!d->WidgetMap.contains(filePath)) {
			return;
		}
		YSSCore::Editor::FileEditWidget* widget = d->WidgetMap[filePath];
		bool okToClose = widget->closeFile();
		if (okToClose) {
			d->WidgetMap.remove(filePath);
			d->TagArea->removeStackLabel(filePath); // this function handle re-choice if the closed widget is current one
			GlobalValue::getCurrentProject()->removeEditorOpenedFile(filePath);
			widget->deleteLater();
		}
		
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
		if (!d->WidgetMap.contains(filePath)) {
			return;
		}
		YSSCore::Editor::FileEditWidget* widget = d->WidgetMap[filePath];
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
		if (!d->WidgetMap.contains(filePath)) {
			return;
		}
		YSSCore::Editor::FileEditWidget* widget = d->WidgetMap[filePath];
		setCurrentWidget(widget);
		widget->cursorToPosition(lineNumber, column);
	}

	YSSCore::Editor::FileEditWidget* StackWidgetArea::getCurrentWidget() const {
		if (d->ContentArea == d->CentralArea) {
			return nullptr;
		}
		return (YSSCore::Editor::FileEditWidget*)d->ContentArea;
	}

	QList<YSSCore::Editor::FileEditWidget*> StackWidgetArea::getAllWidgets() const {
		return d->WidgetMap.values();
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