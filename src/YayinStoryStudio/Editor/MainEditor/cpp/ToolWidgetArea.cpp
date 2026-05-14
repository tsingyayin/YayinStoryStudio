#include "Editor/MainEditor/ToolWidgetArea.h"
#include "Editor/MainEditor/private/StackComponents_p.h"
#include "Editor/GlobalValue.h"
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <Editor/ToolWidgetManager.h>
namespace YSS::Editor {
	class ToolWidgetAreaPrivate {
		friend class ToolWidgetArea;
	protected:
		StackTagWidget* TagArea;
		QWidget* ContentArea = nullptr;
		QVBoxLayout* Layout;
		DefaultStackWidgetCentralArea* CentralArea;
	};

	ToolWidgetArea::ToolWidgetArea(QWidget* parent) : QFrame(parent) {
		d = new ToolWidgetAreaPrivate;
		d->Layout = new QVBoxLayout(this);
		d->Layout->setSpacing(0);
		d->TagArea = new StackTagWidget(this);
		d->TagArea->setToolWidgetMode(true);
		d->TagArea->setFixedHeight(32);
		d->CentralArea = new DefaultStackWidgetCentralArea(this);
		d->CentralArea->setText(VITR("YSS::editor.toolWidgetArea.noToolWidget"));
		d->ContentArea = d->CentralArea;

		d->Layout->addWidget(d->CentralArea);
		d->Layout->addWidget(d->TagArea);

		this->setMinimumHeight(200);
		connect(d->TagArea, &StackTagWidget::switchToFile, this, [this](const QString& widgetID) {
			setCurrentWidget(widgetID);
			});

		connect(d->TagArea, &StackTagWidget::closeFile, this, [this](const QString& widgetID) {
			closeWidget(widgetID);
			});

		connect(d->TagArea, &StackTagWidget::closeAllRequested, this, [this]() {
			closeAll();
			});
	}

	void ToolWidgetArea::addWidget(const QString& widgetID) {
		if (containsWidget(widgetID)) {
			setCurrentWidget(widgetID);
			return;
		}
		QWidget* widget = YSSTWM->getToolWidget(widgetID);
		if (widget == nullptr) {
			vgError << "Failed to create tool widget with ID " << widgetID;
			return;
		}
		connect(widget, &QWidget::destroyed, this, [this, widgetID]() {
			d->TagArea->removeStackLabel(widgetID);
			emit closed(widgetID);
			});
		d->TagArea->addStackLabel(widgetID, widget->windowTitle());
		widget->setParent(this);
		setCurrentWidget(widgetID);
	}

	void ToolWidgetArea::closeAll() {
		for (auto widget : YSSTWM->getAllOpenToolWidgets()) {
			widget->close();
		}
	}

	void ToolWidgetArea::closeWidget(const QString& widgetID) {
		QWidget* widget = YSSTWM->getToolWidget(widgetID);
		if (widget) {
			widget->close();
		}
	}

	bool ToolWidgetArea::containsWidget(const QString& widgetID) const {
		return d->TagArea->containsStackLabel(widgetID);
	}

	void ToolWidgetArea::setCurrentWidget(const QString& widgetID) {
		if (widgetID.isEmpty()) {
			d->ContentArea->hide();
			d->Layout->removeWidget(d->ContentArea);
			d->ContentArea = d->CentralArea;
			d->Layout->insertWidget(0, d->ContentArea);
			d->ContentArea->show();
		}
		if (widgetID.isEmpty()) {
			return;
		}
		QWidget* widget = YSSTWM->getToolWidget(widgetID);
		if (widget == nullptr) {
			vgError << "Failed to get tool widget with ID " << widgetID;
			return;
		}
		if (d->ContentArea != widget) {
			d->ContentArea->hide();
			d->Layout->removeWidget(d->ContentArea);
			d->ContentArea = widget;
			d->Layout->insertWidget(0, d->ContentArea);
			d->ContentArea->show();
		}
		emit currentWidgetChanged(widgetID);
		d->TagArea->setCurrentStackLabel(widgetID);
	}
}