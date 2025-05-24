#include "../ConfigWidget.h"
#include <QtCore/qfile.h>
#include "../../Utility/JsonConfig.h"
#include "../../Utility/FileUtility.h"
#include "../../General/TranslationHost.h"
#include <QtWidgets>
#include "../private/ConfigWidget_p.h"
#include "../../Utility/PathMacro.h"

namespace YSSCore::__Private__ {

	ConfigWidgetPrivate::ConfigWidgetPrivate(QWidget* self) {
		this->self = self;
		this->Layout = new QVBoxLayout(self);
		self->setLayout(Layout);
	}
	ConfigWidgetPrivate::~ConfigWidgetPrivate() {
		saveConfig();
		if (Config != nullptr) {
			delete Config;
		}
	}

	void ConfigWidgetPrivate::loadCWJson(const QString& json) {
		if (Config != nullptr) {
			delete Config;
		}
		YSSCore::Utility::JsonConfig cwJson;
		cwJson.parse(json);
		TargetConfigPath = YSSPathMacro(cwJson.getString("target"));
		if (!Settings.isEmpty()) {
			for (QWidget* w : Settings) {
				w->deleteLater();
			}
		}
		Settings.clear();
		QList<YSSCore::Utility::JsonConfig> widget = cwJson.getArray("widget");
		Settings = spawnWidget(widget);
		for (QWidget* w : Settings) {
			w->setParent(self);
			Layout->addWidget(w);
		}
		this->initConfig();
	}
	void ConfigWidgetPrivate::initConfig() {
		QString config = YSSCore::Utility::FileUtility::readAll(TargetConfigPath);
		if (Config != nullptr) {
			delete Config;
		}
		Config = new YSSCore::Utility::JsonConfig;
		Config->parse(config);
		if (TargetConfigPath.isEmpty() || config.isEmpty()) {
			spawnConfig();
		}
		else {
			syncConfig();
		}
		self->setWindowTitle(YSSI18N(Config->getString("windowTitle")));
		self->setWindowIcon(QIcon(Config->getString("windowIcon")));
	}
	void ConfigWidgetPrivate::syncConfig() {
		for (QComboBox* obj : ComboBoxDefault.keys()) {
			QString data = Config->getString(obj->objectName());
			for (int i = 0; i < obj->count(); i++) {
				if (obj->itemData(i) == data) {
					obj->setCurrentIndex(i);
					break;
				}
			}
		}
		for (QRadioButton* obj : RadioButtonDefault.keys()) {
			bool selected = Config->getBool(obj->objectName());
			if (obj->isChecked() == selected) {
				continue;
			}
			else {
				obj->toggle();
			}
		}
		for (QLineEdit* obj : LineEditDefault.keys()) {
			QString data = Config->getString(obj->objectName());
			obj->setText(data);
		}
	}
	void ConfigWidgetPrivate::resetConfig() {
		if (Config != nullptr) {
			delete Config;
		}
		Config = new YSSCore::Utility::JsonConfig;
		spawnConfig();
	}
	void ConfigWidgetPrivate::spawnConfig() {
		for (QComboBox* obj : ComboBoxDefault.keys()) {
			QString data = ComboBoxDefault[obj];
			for (int i = 0; i < obj->count(); i++) {
				if (obj->itemData(i) == data) {
					obj->setCurrentIndex(i);
					break;
				}
			}
			Config->setString(obj->objectName(), data);
		}
		for (QRadioButton* obj : RadioButtonDefault.keys()) {
			bool selected = RadioButtonDefault[obj];
			if (obj->isChecked() == selected) {
				continue;
			}
			else {
				obj->toggle();
			}
			Config->setBool(obj->objectName(), RadioButtonDefault[obj]);
		}
		for (QLineEdit* obj : LineEditDefault.keys()) {
			QString data = LineEditDefault[obj];
			obj->setText(data);
			Config->setString(obj->objectName(), LineEditDefault[obj]);
		}
	}
	void ConfigWidgetPrivate::saveConfig() {
		if (Config == nullptr || TargetConfigPath.isEmpty()) {
			return;
		}
		QString config = Config->toString();
		if (config.isEmpty()) {
			return;
		}
		YSSCore::Utility::FileUtility::saveAll(TargetConfigPath, config);
	}

	QList<QWidget*> ConfigWidgetPrivate::spawnWidget(QList<YSSCore::Utility::JsonConfig>& config) {
		QList<QWidget*> rtn;
		for (YSSCore::Utility::JsonConfig widget : config) {
			QWidget* w = widgetSpawner(widget);
			if (w != nullptr) {
				rtn.append(w);
			}
		}
		return rtn;
	}
	QWidget* ConfigWidgetPrivate::widgetSpawner(YSSCore::Utility::JsonConfig& config, const QString& parentPath) {
		QString type = config.getString("type");
		QString node = config.getString("node");
		if (parentPath != "") {
			node = parentPath + "." + node;
		}
		QWidget* self = new QWidget();
		QVBoxLayout* Layout = new QVBoxLayout(self);
		self->setLayout(Layout);
		QWidget* SettingFrame = new QWidget(self);
		Layout->addWidget(SettingFrame);
		QGridLayout* SettingLayout = new QGridLayout(SettingFrame);
		SettingFrame->setLayout(SettingLayout);
		QLabel* TitleLabel = new QLabel(SettingFrame);
		TitleLabel->setText(YSSI18N(config.getString("title")));
		QLabel* TextLabel = new QLabel(SettingFrame);
		TextLabel->setText(YSSI18N(config.getString("text")));
		QLabel* IconLabel = new QLabel(SettingFrame);
		IconLabel->setPixmap(QPixmap(config.getString("icon")));
		SettingLayout->addWidget(IconLabel, 0, 0, 2, 1);
		SettingLayout->addWidget(TitleLabel, 0, 1, 1, 1);
		SettingLayout->addWidget(TextLabel, 1, 1, 1, 1);
		if (config.contains("data")) {
			YSSCore::Utility::JsonConfig selfConfig = config.getObject("data");
			QWidget* target = widgetRouter(type, node, selfConfig);
			if (target != nullptr) {
				target->setParent(SettingFrame);
				SettingLayout->addWidget(target, 0, 2, 2, 1);
			}
			else {
				self->deleteLater();
				return nullptr;
			}
		}
		if (config.contains("children")) {
			QStringList keys = config.keys("children");
			QList<YSSCore::Utility::JsonConfig> childrenConfig = config.getArray("children");
			for (YSSCore::Utility::JsonConfig child : childrenConfig) {
				QWidget* childWidget = widgetSpawner(child, node);
				childWidget->setParent(self);
				Layout->addWidget(childWidget);
			}
		}
		return self;
	}
	QWidget* ConfigWidgetPrivate::widgetRouter(const QString& type, const QString& node, YSSCore::Utility::JsonConfig& config) {
		QWidget* rtn = nullptr;
		if (type == "ComboBox") {
			rtn = widget_ComboBox(node, config);
		}
		else if (type == "RadioButton") {
			rtn = widget_RadioButton(node, config);
		}
		return rtn;
	}
	QWidget* ConfigWidgetPrivate::widget_QFrame(const QString& node, YSSCore::Utility::JsonConfig& config) {
		return nullptr; // delete this function later
	}
	QWidget* ConfigWidgetPrivate::widget_ComboBox(const QString& node, YSSCore::Utility::JsonConfig& config) {
		QComboBox* ComboBox = new QComboBox();
		ComboBox->setObjectName(node);
		connect(ComboBox, &QComboBox::currentIndexChanged, this, &ConfigWidgetPrivate::onComboBoxIndexChanged);
		QList<YSSCore::Utility::JsonConfig> items = config.getArray("comboBox");
		for (YSSCore::Utility::JsonConfig item : items) {
			QString combo_data = item.getString("data");
			QString combo_key = YSSI18N(item.getString("key"));
			ComboBox->addItem(combo_key, combo_data);
		}
		QString defaultValue = config.getString("default");
		ComboBoxDefault.insert(ComboBox, defaultValue);
		return ComboBox;
	}
	QWidget* ConfigWidgetPrivate::widget_RadioButton(const QString& node, YSSCore::Utility::JsonConfig& config) {
		QRadioButton* RadioButton = new QRadioButton();
		RadioButton->setObjectName(node);
		connect(RadioButton, &QRadioButton::toggled, this, &ConfigWidgetPrivate::onRadioButtonChanged);
		bool defaultValue = config.getBool("default");
		RadioButtonDefault.insert(RadioButton, defaultValue);
		return RadioButton;
	}
	void ConfigWidgetPrivate::onComboBoxIndexChanged(int index) {
		QObject* obj = sender();
		QString node = obj->objectName();
		QComboBox* comboBox = static_cast<QComboBox*>(obj);
		QString data = comboBox->itemData(index).toString();
		if (Config != nullptr) {
			Config->setString(node, data);
		}
	}
	void ConfigWidgetPrivate::onRadioButtonChanged(bool checked) {
		QObject* obj = sender();
		QString node = obj->objectName();
		if (Config != nullptr) {
			Config->setBool(node, checked);
		}
	}
	
}
namespace YSSCore::Editor {
	ConfigWidget::ConfigWidget(QWidget* parent) :QFrame(parent) {
		d = new YSSCore::__Private__::ConfigWidgetPrivate(this);
	}
	ConfigWidget::~ConfigWidget() {
		delete d;
	}
	void ConfigWidget::loadCWJson(const QString& json) {
		d->loadCWJson(json);
	}
}