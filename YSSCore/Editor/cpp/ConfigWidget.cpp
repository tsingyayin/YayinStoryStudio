#include "../ConfigWidget.h"
#include <QtCore/qfile.h>
#include "../../Utility/JsonConfig.h"
#include "../../Utility/FileUtility.h"
#include "../../General/TranslationHost.h"
#include <QtWidgets>
#include "../private/ConfigWidget_p.h"
#include "../../Utility/PathMacro.h"
namespace YSSCore::__Private__ {

	ConfigWidgetPrivate::ConfigWidgetPrivate() {
		
	}
	ConfigWidgetPrivate::~ConfigWidgetPrivate() {
		saveConfig();
		if (Config != nullptr) {
			delete Config;
		}
	}
	QString ConfigWidgetPrivate::getTr(QString raw) {
		if (raw.startsWith("i18n:")) {
			QStringList keys = raw.mid(5).split("::");
			if (keys.length() != 2) {
				return raw;
			}
			return YSSTR(keys[0], keys[1]);
		}
		return raw;
	}

	void ConfigWidgetPrivate::loadConfig() {
		QString config = YSSCore::Utility::FileUtility::readAll(TargetConfigPath);
		if (Config != nullptr) {
			delete Config;
		}
		Config = new YSSCore::Utility::JsonConfig;
		Config->parse(config);
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

	void ConfigWidgetPrivate::backToDefaultConfig() {
		for (QComboBox* box : ComboBoxDefault.keys()) {
			QString data = ComboBoxDefault[box];
			for (int i = 0; i < box->count(); i++) {
				if (box->itemData(i) == data) {
					box->setCurrentIndex(i);
					break;
				}
			}
		}
		for (QRadioButton* box : RadioButtonDefault.keys()) {
			bool selected = RadioButtonDefault[box];
			if (box->isChecked() == selected) {
				continue;
			}
			else {
				box->toggle();
			}
		}
	}
	QList<QWidget*> ConfigWidgetPrivate::spawnWidget(QList<YSSCore::Utility::JsonConfig>& config) {
		QList<QWidget*> rtn;
		for (YSSCore::Utility::JsonConfig widget : config) {
			rtn.append(widgetSpawner(widget));
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
		YSSCore::Utility::JsonConfig selfConfig = config.getObject("data");
		QWidget* target = widgetRouter(type, node, selfConfig);
		if (target != nullptr) {
			target->setParent(self);
			Layout->addWidget(target);
		}
		QStringList keys = config.keys("children");
		QList<YSSCore::Utility::JsonConfig> childrenConfig = config.getArray("children");
		for (YSSCore::Utility::JsonConfig child : childrenConfig) {
			QWidget* childWidget = widgetSpawner(child, node);
			childWidget->setParent(self);
			Layout->addWidget(childWidget);
		}
		return self;
	}
	QWidget* ConfigWidgetPrivate::widgetRouter(const QString& type, const QString& node, YSSCore::Utility::JsonConfig& config) {
		QWidget* rtn = nullptr;
		if (type == "Frame") {
			rtn = widget_QFrame(node, config);
		}
		else if (type == "ComboBox") {
			rtn = widget_ComboBox(node, config);
		}
		else if (type == "RadioButton") {
			rtn = widget_RadioButton(node, config);
		}
		return rtn;
	}
	QWidget* ConfigWidgetPrivate::widget_QFrame(const QString& node, YSSCore::Utility::JsonConfig& config) {
		QWidget* frame = new QWidget();
		QLabel* IconLabel = new QLabel(frame); // no impl yet
		QLabel* TitleLabel = new QLabel(frame);
		QLabel* TextLabel = new QLabel(frame);
		QGridLayout* Layout = new QGridLayout(frame);
		frame->setLayout(Layout);
		TitleLabel->setText(getTr(config.getString("title")));
		TextLabel->setText(getTr(config.getString("text")));
		Layout->addWidget(IconLabel, 0, 0, 2, 2);
		Layout->addWidget(TitleLabel, 0, 1, 1, 1);
		Layout->addWidget(TextLabel, 1, 1, 1, 1);
		return frame;
	}
	QWidget* ConfigWidgetPrivate::widget_ComboBox(const QString& node, YSSCore::Utility::JsonConfig& config) {
		QWidget* frame = new QWidget();
		QLabel* IconLabel = new QLabel(frame); // no impl yet
		QLabel* TitleLabel = new QLabel(frame);
		QLabel* TextLabel = new QLabel(frame);
		QComboBox* ComboBox = new QComboBox(frame);
		ComboBox->setObjectName(node);
		connect(ComboBox, &QComboBox::currentIndexChanged, this, &ConfigWidgetPrivate::onComboBoxIndexChanged);
		QGridLayout* Layout = new QGridLayout(frame);
		frame->setLayout(Layout);
		TitleLabel->setText(getTr(config.getString("title")));
		TextLabel->setText(getTr(config.getString("text")));
		Layout->addWidget(IconLabel, 0, 0, 2, 2);
		Layout->addWidget(TitleLabel, 0, 1, 1, 1);
		Layout->addWidget(TextLabel, 1, 1, 1, 1);
		Layout->addWidget(ComboBox, 0, 2, 2, 1);
		QList<YSSCore::Utility::JsonConfig> items = config.getArray("comboBox");
		for (YSSCore::Utility::JsonConfig item : items) {
			QString combo_data = item.getString("data");
			QString combo_key = getTr(item.getString("key"));
			ComboBox->addItem(combo_key, combo_data);
		}
		QString data = Config->getString(node);
		for (int i = 0; i < ComboBox->count(); i++) {
			if (ComboBox->itemData(i) == data) {
				ComboBox->setCurrentIndex(i);
				break;
			}
		}
		QString defaultValue = config.getString("default");
		ComboBoxDefault.insert(ComboBox, defaultValue);
		return frame;
	}
	QWidget* ConfigWidgetPrivate::widget_RadioButton(const QString& node, YSSCore::Utility::JsonConfig& config) {
		QWidget* frame = new QWidget();
		QLabel* IconLabel = new QLabel(frame); // no impl yet
		QLabel* TitleLabel = new QLabel(frame);
		QLabel* TextLabel = new QLabel(frame);
		QRadioButton* RadioButton = new QRadioButton(frame);
		RadioButton->setObjectName(node);
		connect(RadioButton, &QRadioButton::toggled, this, &ConfigWidgetPrivate::onRadioButtonChanged);
		QGridLayout* Layout = new QGridLayout(frame);
		frame->setLayout(Layout);
		TitleLabel->setText(getTr(config.getString("title")));
		TextLabel->setText(getTr(config.getString("text")));
		Layout->addWidget(IconLabel, 0, 0, 2, 2);
		Layout->addWidget(TitleLabel, 0, 1, 1, 1);
		Layout->addWidget(TextLabel, 1, 1, 1, 1);
		Layout->addWidget(RadioButton, 0, 2, 2, 1);

		bool defaultValue = config.getBool("default");
		RadioButtonDefault.insert(RadioButton, defaultValue);
		return frame;
	}
	void ConfigWidgetPrivate::onComboBoxIndexChanged(int index) {
		QObject* obj = sender();
		QString node = obj->objectName();
		QComboBox* comboBox = static_cast<QComboBox*>(obj);
		QString data = comboBox->itemData(index).toString();
		Config->setString(node, data);
	}
	void ConfigWidgetPrivate::onRadioButtonChanged(bool checked) {
		QObject* obj = sender();
		QString node = obj->objectName();
		Config->setBool(node, checked);
	}
	
}
namespace YSSCore::Editor {
	ConfigWidget::ConfigWidget(QWidget* parent) :QFrame(parent) {
		d = new YSSCore::__Private__::ConfigWidgetPrivate;
		d->Layout = new QVBoxLayout(this);
		this->setLayout(d->Layout);
	}
	ConfigWidget::~ConfigWidget() {
		delete d;
	}
	bool ConfigWidget::setConfig(const QString& jsonPath) {
		QString jsonStr = YSSCore::Utility::FileUtility::readAll(jsonPath);
		YSSCore::Utility::JsonConfig config;
		qDebug() << jsonStr;
		config.parse(jsonStr);
		d->TargetConfigPath = YSSPathMacro(config.getString("target"));
		qDebug() << d->TargetConfigPath;
		d->loadConfig();
		QList<YSSCore::Utility::JsonConfig> widget = config.getArray("widget");
		QList<QWidget*> widgets = d->spawnWidget(widget);
		for (QWidget* w : widgets) {
			w->setParent(this);
			d->Layout->addWidget(w);
			w->show();
		}
		return true;
	}
}