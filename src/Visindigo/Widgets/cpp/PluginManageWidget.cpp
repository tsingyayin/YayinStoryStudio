#include "Widgets/private/PluginManageWidget_p.h"
#include "Widgets/PluginManageWidget.h" 
#include "General/TranslationHost.h"
#include "General/Plugin.h"
#include "General/PluginManager.h"
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qlayout.h>
#include <QtWidgets/qscrollarea.h>
#include "General/VIApplication.h"
#include "Utility/FileUtility.h"
#include "General/Log.h"
#include <QtCore/qdir.h>
#include <QtWidgets/qscrollbar.h>

namespace Visindigo::__Private__ {
	Divider::Divider(const QString& title, const QString& description, QWidget* parent) : QFrame(parent)
	{
		TitleLabel = new QLabel(title, this);
		DescriptionLabel = new QLabel(description, this);
		DescriptionLabel->setWordWrap(true);
		Layout = new QVBoxLayout(this);
		Layout->addWidget(TitleLabel);
		if (not description.isEmpty()) {
			Layout->addWidget(DescriptionLabel);
		}
		auto line = new QFrame(this);
		line->setFrameShape(QFrame::HLine);
		line->setFrameShadow(QFrame::Sunken);
		Layout->addWidget(line);
	}

	Divider::~Divider() {
	}

	PluginInfoPanel::PluginInfoPanel(bool asdependency, QWidget* parent) : QFrame(parent)
	{
		this->setFixedHeight(100);
		NameLabel = new QLabel(this);
		IconLabel = new QLabel(this);
		AuthorLabel = new QLabel(this);
		VersionLabel = new QLabel(this);
		DateTimeLabel = new QLabel(this);
		DescriptionLabel = new QLabel(this);
		DescriptionLabel->setWordWrap(true);
		ActiveCheckBox = new QCheckBox(this);
		CheckBoxLabel = new QLabel(this);
		OpenFolderButton = new QPushButton(this);
		OpenFolderButton->setFixedWidth(140);
		OpenFolderButton->setText(VITR("Visindigo::general.openFolder"));
		OpenConfigButton = new QPushButton(this);
		OpenConfigButton->setFixedWidth(140);
		OpenConfigButton->setText(VITR("Visindigo::general.openConfig"));

		Layout = new QGridLayout(this);
		Layout->addWidget(IconLabel, 0, 0, 3, 1);
		Layout->addWidget(NameLabel, 0, 1);
		Layout->addWidget(VersionLabel, 1, 1);
		Layout->addWidget(DateTimeLabel, 1, 2);
		Layout->addWidget(AuthorLabel, 2, 1, 1, 2);
		Layout->addWidget(DescriptionLabel, 0, 3, 3, 1);
		Layout->addWidget(OpenFolderButton, 0, 4);
		Layout->addWidget(OpenConfigButton, 1, 4);
		auto checkLayout = new QHBoxLayout();
		checkLayout->addWidget(CheckBoxLabel);
		checkLayout->addWidget(ActiveCheckBox);
		Layout->addLayout(checkLayout, 2, 4);
		AsDependency = asdependency;
		if (AsDependency) {
			ActiveCheckBox->setVisible(false);
			CheckBoxLabel->setVisible(false);
		}

		connect(ActiveCheckBox, &QCheckBox::toggled, this, &PluginInfoPanel::onActiveStateChanged);
		connect(OpenFolderButton, &QPushButton::clicked, this, &PluginInfoPanel::onOpenFolderButtonClicked);
		connect(OpenConfigButton, &QPushButton::clicked, this, &PluginInfoPanel::onOpenConfigButtonClicked);
	}
	void PluginInfoPanel::setPlugin(Visindigo::General::Plugin* plugin) {
		if (not plugin) { return; }
		Plugin = plugin;
		NameLabel->setText(plugin->getPluginName());
		AuthorLabel->setText(plugin->getPluginAuthor().join(", "));
		VersionLabel->setText(plugin->getPluginVersion().toString());
		DescriptionLabel->setText(plugin->getPluginDescription());
		bool deactivate = Visindigo::General::PluginManager::getInstance()->isPluginDeactivate(plugin->getPluginID());
		CheckBoxLabel->setText(deactivate ? VITR("Visindigo::general.deactive") : VITR("Visindigo::general.active"));
		ActiveCheckBox->setChecked(not deactivate);
		vgDebug << ":/resource/" + plugin->getPluginID() + "/icon.png";
		QPixmap icon(":/resource/"+plugin->getPluginID()+"/icon.png");
		if (icon.isNull()) {
			IconLabel->setPixmap(QPixmap(":/resource/default_plugin_icon.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		}
		else {
			IconLabel->setPixmap(icon.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		}
		QWidget* configWidget = plugin->getConfigWidget();
		if (not configWidget) {
			OpenConfigButton->setEnabled(false);
		}
		else {
			OpenConfigButton->setEnabled(true);
		}
	}

	void PluginInfoPanel::resizeEvent(QResizeEvent* event) {
		QFrame::resizeEvent(event);
		IconLabel->setFixedWidth(IconLabel->height());
	}

	PluginInfoPanel::~PluginInfoPanel() {
	}

	void PluginInfoPanel::onActiveStateChanged(bool checked) {
		if (not Plugin) { return; }
		Visindigo::General::PluginManager::getInstance()->setPluginDeactivate(Plugin->getPluginID(), not checked);
		CheckBoxLabel->setText(checked ? VITR("Visindigo::general.active") : VITR("Visindigo::general.deactive"));
	}

	void PluginInfoPanel::onOpenFolderButtonClicked() {
		if (not Plugin) { return; }
		Visindigo::Utility::FileUtility::openExplorer(Plugin->getPluginFolder().absolutePath());
	}

	void PluginInfoPanel::onOpenConfigButtonClicked() {
		if (not Plugin) { return; }
		QWidget* configWidget = Plugin->getConfigWidget();
		if (not configWidget) { return; }
		configWidget->setWindowModality(Qt::ApplicationModal);
		configWidget->show();
	}

	DeactivatePluginInfoPanel::DeactivatePluginInfoPanel(QWidget* parent) : QFrame(parent)
	{
		this->setFixedHeight(60);
		IDLabel = new QLabel(this);
		RemoveButton = new QPushButton(this);
		RemoveButton->setText(VITR("Visindigo::general.remove"));
		Layout = new QHBoxLayout(this);
		Layout->addWidget(IDLabel);
		Layout->addWidget(RemoveButton);
		connect(RemoveButton, &QPushButton::clicked, this, [this]() {
			if (IDLabel->text().isEmpty()) { return; }
			Visindigo::General::PluginManager::getInstance()->setPluginDeactivate(IDLabel->text(), false);
			emit removePlugin(IDLabel->text());
			});
	}

	DeactivatePluginInfoPanel::~DeactivatePluginInfoPanel() {
	}

	void DeactivatePluginInfoPanel::setPluginID(const QString& pluginID) {
		IDLabel->setText(pluginID);
	}
}

namespace Visindigo::Widgets {
	class PluginManageWidgetPrivate {
		friend class PluginManageWidget;
	protected:
		QList<Visindigo::__Private__::Divider*> Dividers;
		QList<QLabel*> EmptyLabels;
		Visindigo::__Private__::PluginInfoPanel* MainPluginInfoPanel;
		QList<Visindigo::__Private__::PluginInfoPanel*> DependencyPluginInfoPanels;
		QList<Visindigo::__Private__::DeactivatePluginInfoPanel*> DeactivatePluginInfoPanels;
		QList<Visindigo::__Private__::PluginInfoPanel*> PluginInfoPanels;
		QScrollArea* ScrollArea;
		QWidget* ScrollAreaWidget;
		QVBoxLayout* ScrollAreaLayout;
	};

	PluginManageWidget::PluginManageWidget(QWidget* parent) : QFrame(parent), d(new PluginManageWidgetPrivate)
	{
		d->ScrollArea = new QScrollArea(this);
		d->ScrollAreaWidget = new QWidget(d->ScrollArea);
		d->ScrollAreaLayout = new QVBoxLayout(d->ScrollAreaWidget);
		d->ScrollAreaLayout->setAlignment(Qt::AlignTop);
		d->ScrollAreaWidget->setLayout(d->ScrollAreaLayout);
		d->ScrollArea->setWidget(d->ScrollAreaWidget);
		d->ScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		this->setMinimumSize(500, 600);
		refreshPluginList();
	}

	void PluginManageWidget::refreshPluginList() {
		for (int i = 0; i < d->Dividers.size(); i++) {
			d->ScrollAreaLayout->removeWidget(d->Dividers[i]);
			delete d->Dividers[i];
		}
		d->Dividers.clear();
		for(int i = 0; i < d->EmptyLabels.size(); i++) {
			d->ScrollAreaLayout->removeWidget(d->EmptyLabels[i]);
			delete d->EmptyLabels[i];
		}
		d->EmptyLabels.clear();
		for (int i = 0; i < d->DependencyPluginInfoPanels.size(); i++) {
			d->ScrollAreaLayout->removeWidget(d->DependencyPluginInfoPanels[i]);
			delete d->DependencyPluginInfoPanels[i];
		}
		d->DependencyPluginInfoPanels.clear();
		for (int i = 0; i < d->DeactivatePluginInfoPanels.size(); i++) {
			d->ScrollAreaLayout->removeWidget(d->DeactivatePluginInfoPanels[i]);
			delete d->DeactivatePluginInfoPanels[i];
		}
		d->DeactivatePluginInfoPanels.clear();
		for (int i = 0; i < d->PluginInfoPanels.size(); i++) {
			d->ScrollAreaLayout->removeWidget(d->PluginInfoPanels[i]);
			delete d->PluginInfoPanels[i];
		}
		d->PluginInfoPanels.clear();

		auto mainDivider = new __Private__::Divider(VITR("Visindigo::widgets.pluginManager.program"), VITR("Visindigo::widgets.pluginManager.programDesc"));
		d->Dividers.append(mainDivider);
		d->ScrollAreaLayout->addWidget(mainDivider);

		auto mainPanel = new __Private__::PluginInfoPanel(true, d->ScrollAreaWidget);
		mainPanel->setPlugin(VIApp->getMainPlugin());
		d->MainPluginInfoPanel = mainPanel;
		d->ScrollAreaLayout->addWidget(mainPanel);

		auto dependencyDivider = new __Private__::Divider(VITR("Visindigo::widgets.pluginManager.dependency"), VITR("Visindigo::widgets.pluginManager.dependencyDesc"));
		d->Dividers.append(dependencyDivider);
		d->ScrollAreaLayout->addWidget(dependencyDivider);

		bool hasDependency = VIApp->getDependencyPlugins().size() > 0;
		if (hasDependency) {	
			QList<General::Plugin*> dependencyPlugins = VIApp->getDependencyPlugins();
			for (int i = 0; i < dependencyPlugins.size(); i++) {
				__Private__::PluginInfoPanel* panel = new __Private__::PluginInfoPanel(true, d->ScrollAreaWidget);
				panel->setPlugin(dependencyPlugins[i]);
				d->DependencyPluginInfoPanels.append(panel);
				d->ScrollAreaLayout->addWidget(panel);
			}
		}
		else {
			auto emptyLabel = new QLabel(VITR("Visindigo::widgets.pluginManager.nothing"), d->ScrollAreaWidget);
			emptyLabel->setAlignment(Qt::AlignCenter);
			d->EmptyLabels.append(emptyLabel);
			d->ScrollAreaLayout->addWidget(emptyLabel);
		}

		auto pluginDivider = new __Private__::Divider(VITR("Visindigo::widgets.pluginManager.plugin"), VITR("Visindigo::widgets.pluginManager.pluginDesc"));
		d->Dividers.append(pluginDivider);
		d->ScrollAreaLayout->addWidget(pluginDivider);


		QList<General::Plugin*> plugins = General::PluginManager::getInstance()->getLoadedPlugins();
		if (plugins.size() > 0) {
			for (int i = 0; i < plugins.size(); i++) {
				__Private__::PluginInfoPanel* panel = new __Private__::PluginInfoPanel(false, d->ScrollAreaWidget);
				panel->setPlugin(plugins[i]);
				d->PluginInfoPanels.append(panel);
				d->ScrollAreaLayout->addWidget(panel);
			}
			
		}
		else {
			auto emptyLabel = new QLabel(VITR("Visindigo::widgets.pluginManager.nothing"), d->ScrollAreaWidget);
			emptyLabel->setAlignment(Qt::AlignCenter);
			d->EmptyLabels.append(emptyLabel);
			d->ScrollAreaLayout->addWidget(emptyLabel);
		}

		auto deactivateDivider = new __Private__::Divider(VITR("Visindigo::widgets.pluginManager.deactivated"), VITR("Visindigo::widgets.pluginManager.deactivatedDesc"));
		d->Dividers.append(deactivateDivider);
		d->ScrollAreaLayout->addWidget(deactivateDivider);

		QList<QString> deactivatedPlugins = General::PluginManager::getInstance()->getDeactivatedPluginIDList();
		if (deactivatedPlugins.size() > 0) {
			for (int i = 0; i < deactivatedPlugins.size(); i++) {
				__Private__::DeactivatePluginInfoPanel* panel = new __Private__::DeactivatePluginInfoPanel(d->ScrollAreaWidget);
				panel->setPluginID(deactivatedPlugins[i]);
				connect(panel, &__Private__::DeactivatePluginInfoPanel::removePlugin, this, [this](const QString& pluginID) {
					QObject* senderObj = sender();
					auto targetPanel = qobject_cast<__Private__::DeactivatePluginInfoPanel*>(senderObj);
					if (not targetPanel) { return; }
					d->ScrollAreaLayout->removeWidget(targetPanel);
					d->DeactivatePluginInfoPanels.removeOne(targetPanel);
					delete targetPanel;
					});
				d->DeactivatePluginInfoPanels.append(panel);
				d->ScrollAreaLayout->addWidget(panel);
			}
		}
		else {
			auto emptyLabel = new QLabel(VITR("Visindigo::widgets.pluginManager.nothing"), d->ScrollAreaWidget);
			emptyLabel->setAlignment(Qt::AlignCenter);
			d->EmptyLabels.append(emptyLabel);
			d->ScrollAreaLayout->addWidget(emptyLabel);
		}
		
		d->ScrollAreaWidget->setFixedHeight(d->ScrollAreaLayout->sizeHint().height());
		d->ScrollAreaWidget->setFixedWidth(this->width() - d->ScrollArea->verticalScrollBar()->width());
		d->ScrollArea->setGeometry(0, 0, this->width(), this->height());
		
	}

	PluginManageWidget::~PluginManageWidget()
	{
		delete d;
	}

	void PluginManageWidget::resizeEvent(QResizeEvent* event) {
		QFrame::resizeEvent(event);
		d->ScrollArea->setGeometry(0, 0, this->width(), this->height());
		d->ScrollAreaWidget->setFixedWidth(this->width());
	}
	
}