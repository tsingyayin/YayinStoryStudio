#include "../ConfigWidget.h"
#include <QtCore/qfile.h>
#include "../../Utility/JsonConfig.h"
#include "../../Utility/FileUtility.h"
#include "../../General/TranslationHost.h"
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qradiobutton.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qframe.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qfiledialog.h>
#include "../private/ConfigWidget_p.h"
#include "../../Utility/PathMacro.h"
#include "../../Widgets/MultiLabel.h"
#include "../../General/Log.h"
#include "../ThemeManager.h"

namespace Visindigo::__Private__ {
	ConfigWidgetPrivate::ConfigWidgetPrivate(Visindigo::Widgets::ConfigWidget* self) {
		this->self = self;
		this->Layout = new QVBoxLayout(self);
		self->setLayout(Layout);
		self->setStyleSheet(VISTMGT("YSS::ConfigWidget", self));
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
		Visindigo::Utility::JsonConfig cwJson;
		cwJson.parse(json);
		TargetConfigPath = YSSPathMacro(cwJson.getString("target"));
		if (!Settings.isEmpty()) {
			for (QWidget* w : Settings) {
				w->deleteLater();
			}
		}
		Settings.clear();
		QList<Visindigo::Utility::JsonConfig> widget = cwJson.getArray("widget");
		Settings = spawnWidget(widget);
		for (QWidget* w : Settings) {
			w->setParent(self);
			//w->setStyleSheet("QWidget{border:1px solid black}");
			Layout->addWidget(w);
		}
		self->setStyleSheet(VISTMGT("YSS::ConfigWidget", self));
		this->initConfig();
	}
	void ConfigWidgetPrivate::initConfig() {
		QString config = Visindigo::Utility::FileUtility::readAll(TargetConfigPath);
		if (Config != nullptr) {
			delete Config;
		}
		Config = new Visindigo::Utility::JsonConfig;
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
		Config = new Visindigo::Utility::JsonConfig;
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
		Visindigo::Utility::FileUtility::saveAll(TargetConfigPath, config);
	}

	QList<QWidget*> ConfigWidgetPrivate::spawnWidget(QList<Visindigo::Utility::JsonConfig>& config) {
		QList<QWidget*> rtn;
		for (Visindigo::Utility::JsonConfig widget : config) {
			QWidget* w = widgetSpawner(widget);
			if (w != nullptr) {
				rtn.append(w);
			}
		}
		return rtn;
	}
	QWidget* ConfigWidgetPrivate::widgetSpawner(Visindigo::Utility::JsonConfig& config, const QString& parentPath) {
		QString type = config.getString("type");
		QString node = config.getString("node");
		if (parentPath != "") {
			node = parentPath + "." + node;
		}
		QFrame* self = new QFrame();
		QVBoxLayout* Layout = new QVBoxLayout(self);
		self->setLayout(Layout);
		QFrame* SettingFrame = new QFrame(self);
		Layout->addWidget(SettingFrame);
		Layout->setContentsMargins(0, 0, 0, 0);
		QHBoxLayout* SettingLayout = new QHBoxLayout(SettingFrame);
		SettingFrame->setLayout(SettingLayout);
		SettingLayout->setContentsMargins(0, 0, 0, 0);

		Visindigo::Widgets::MultiLabel* MultiLabel = new Visindigo::Widgets::MultiLabel(SettingFrame);
		MultiLabel->setTitle(YSSI18N(config.getString("title")));
		MultiLabel->setDescription(YSSI18N(config.getString("text")));
		QString iconPath = config.getString("icon");
		if (!iconPath.isEmpty()) {
			MultiLabel->setPixmapPath(iconPath);
		}
		MultiLabel->setAlignment(Qt::AlignLeft);
		//MultiLabel->setFixedHeight(80);
		SettingLayout->addWidget(MultiLabel);
		SettingLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Minimum));
		if (config.contains("data")) {
			Visindigo::Utility::JsonConfig selfConfig = config.getObject("data");
			QWidget* target = widgetRouter(type, node, selfConfig);
			target->setMinimumWidth(200);
			if (target != nullptr) {
				target->setParent(SettingFrame);
				SettingLayout->addWidget(target);
			}
			else {
				self->deleteLater();
				return nullptr;
			}
		}
		if (config.contains("children")) {
			QStringList keys = config.keys("children");
			QList<Visindigo::Utility::JsonConfig> childrenConfig = config.getArray("children");
			for (Visindigo::Utility::JsonConfig child : childrenConfig) {
				QWidget* childWidget = widgetSpawner(child, node);
				childWidget->setParent(self);
				Layout->addWidget(childWidget);
			}
		}
		return self;
	}
	QWidget* ConfigWidgetPrivate::widgetRouter(const QString& type, const QString& node, Visindigo::Utility::JsonConfig& config) {
		QWidget* rtn = nullptr;
		vgDebugF << node << type;
		if (type == "ComboBox") {
			rtn = widget_ComboBox(node, config);
		}
		else if (type == "RadioButton") {
			rtn = widget_RadioButton(node, config);
		}
		else if (type == "LineEdit") {
			rtn = widget_LineEdit(node, config);
		}
		return rtn;
	}
	QWidget* ConfigWidgetPrivate::widget_QFrame(const QString& node, Visindigo::Utility::JsonConfig& config) {
		return nullptr; // delete this function later
	}
	QWidget* ConfigWidgetPrivate::widget_ComboBox(const QString& node, Visindigo::Utility::JsonConfig& config) {
		QComboBox* ComboBox = new QComboBox();
		ComboBox->setObjectName(node);
		connect(ComboBox, &QComboBox::currentIndexChanged, this, &ConfigWidgetPrivate::onComboBoxIndexChanged);
		QList<Visindigo::Utility::JsonConfig> items = config.getArray("comboBox");
		for (Visindigo::Utility::JsonConfig item : items) {
			QString combo_data = item.getString("data");
			QString combo_key = YSSI18N(item.getString("key"));
			ComboBox->addItem(combo_key, combo_data);
		}
		QString defaultValue = config.getString("default");
		ComboBoxDefault.insert(ComboBox, defaultValue);
		return ComboBox;
	}
	QWidget* ConfigWidgetPrivate::widget_RadioButton(const QString& node, Visindigo::Utility::JsonConfig& config) {
		QRadioButton* RadioButton = new QRadioButton();
		RadioButton->setObjectName(node);
		connect(RadioButton, &QRadioButton::toggled, this, &ConfigWidgetPrivate::onRadioButtonChanged);
		bool defaultValue = config.getBool("default");
		RadioButtonDefault.insert(RadioButton, defaultValue);
		return RadioButton;
	}
	QWidget* ConfigWidgetPrivate::widget_LineEdit(const QString& node, Visindigo::Utility::JsonConfig& config) {
		QLineEdit* LineEdit = new QLineEdit();
		LineEdit->setObjectName(node);
		QString defaultValue = config.getString("default");
		connect(LineEdit, &QLineEdit::textChanged, this, &ConfigWidgetPrivate::onLineEditTextChanged);
		if (config.contains("isFolder") || config.contains("isFile")) {
			vgDebugF << YSSPathMacro(defaultValue);
			LineEdit->setText(YSSPathMacro(defaultValue));
			LineEditDefault.insert(LineEdit, YSSPathMacro(defaultValue));
			QFrame* container = new QFrame();
			QHBoxLayout* layout = new QHBoxLayout(container);
			LineEdit->setParent(container);
			layout->addWidget(LineEdit);
			QPushButton* selectButton = new QPushButton(container);
			selectButton->setText(YSSTR("Visindigo::general.preview"));
			layout->addWidget(selectButton);
			connect(selectButton, &QPushButton::clicked, [=]() {
				QString folder = QFileDialog::getExistingDirectory(container,
					YSSTR("Visindigo::general.selectFolder"),
					YSSPathMacro(defaultValue),
					QFileDialog::ShowDirsOnly
					);
				if (!folder.isEmpty()) {
					LineEdit->setText(folder);
				}
				});
			return container;
		}
		else {
			LineEditDefault.insert(LineEdit, defaultValue);
			LineEdit->setText(defaultValue);
			return LineEdit;
		}
	}
	void ConfigWidgetPrivate::onComboBoxIndexChanged(int index) {
		QObject* obj = sender();
		QString node = obj->objectName();
		QComboBox* comboBox = static_cast<QComboBox*>(obj);
		QString data = comboBox->itemData(index).toString();
		if (Config != nullptr) {
			Config->setString(node, data);
		}
		emit self->comboBoxIndexChanged(node, index, data);
	}
	void ConfigWidgetPrivate::onRadioButtonChanged(bool checked) {
		QObject* obj = sender();
		QString node = obj->objectName();
		if (Config != nullptr) {
			Config->setBool(node, checked);
		}
		emit self->radioButtonChanged(node, checked);
	}
	void ConfigWidgetPrivate::onLineEditTextChanged(QString str) {
		QObject* obj = sender();
		QString node = obj->objectName();
		if (Config != nullptr) {
			Config->setString(node, str);
		}
		emit self->lineEditTextChanged(node, str);
	}
}
namespace Visindigo::Widgets {
	/*!
		\class Visindigo::Widgets::ConfigWidget
		\brief 此类从CWJson创建配置窗口。
		\since Yayin Story Studio 0.13.0
		\inmodule Visindigo

		此类提供一种便捷的配置文件操作窗口创建方式，使用一种被约定为“CWJSON”的Json格式来描述配置窗口的内容。并且
		允许窗口和一个配置文件相绑定，将结果同步到配置文件中。

		CWJson的顶层格式如下：
		\badcode
			{
				"target": "$(ProgramPath)/resource/test_config.json",
				"targetType": "json",
				"widget":[]
			}
		\endcode
		其中，target为目标配置文件的路径，targetType为目标配置文件的类型，widget为窗口的内容。
		\note 当前，targetType仅支持json格式，目前有支持YAML的计划，但具体实现时间未定。

		对于widget来说，其基本格式如下，我们以一个ComboBox类型的配置为例：
		\badcode
			{
				"node": "Theme",
				"type": "ComboBox",
				"icon": "",
				"title": "i18n:YSS::config.theme.title",
				"text": "i18n:YSS::config.theme.text",
				"data": {
					"comboBox": [
						{
							"data": "theme_dark",
							"key": "i18n:YSS::config.theme.dark"
						},
						{
							"data": "theme_light",
							"key": "i18n:YSS::config.theme.light"
						}
					],
					"default": "theme_dark"
				},
				"children": []
			}
		\endcode
		其中，node是要绑定到的配置项节点，子一级节点会自动继承父一级节点的名称。
		type是要创建的控件类型，icon是控件的图标，title是控件的标题，text是控件的描述。
		data是控件的具体数据，children是控件的子控件。

		对于type，当前支持的类型为ComboBox、RadioButton、Frame和LineEdit。其中Frame是
		专门用来作为容器的控件，没有输入功能。

		title和text支持YSS翻译系统，当其以“i18n:”开头时，会自动进行翻译，具体参见 \l Visindigo::General::TranslationHost

		不同的控件类型有不同的data格式，ComboBox的格式已经在上面给出，RadioButton的格式如下：
		\badcode
			"data": {
				"default": true
			}
		\endcode
		即只提供默认是否选中即可。而LineEdit的格式如下：
		\badcode
			"data": {
				"default": "test"
				"check": {
					"min": 1,
					"max": 10,
					"regex": "^[a-zA-Z0-9]+$"
				}
			}
		\endcode
		其中，check为检查项，min和max为最小和最大长度，regex为正则表达式。
		\note 目前，LineEdit的检查项仅支持最小和最大长度，正则表达式的检查项未实现。
	*/

	/*!
		\a parent 父窗口
		\since Yayin Story Studio 0.13.0
		类的构造函数
	*/
	ConfigWidget::ConfigWidget(QWidget* parent) :QFrame(parent) {
		d = new Visindigo::__Private__::ConfigWidgetPrivate(this);
	}

	/*!
		\since Yayin Story Studio 0.13.0
		析构函数
	*/
	ConfigWidget::~ConfigWidget() {
		delete d;
	}

	/*!
		\since Yayin Story Studio 0.13.0
		\a json 为CWJson的内容
		加载CWJson
	*/
	void ConfigWidget::loadCWJson(const QString& json) {
		d->loadCWJson(json);
	}

	Visindigo::Utility::JsonConfig* ConfigWidget::getConfig() {
		return d->Config;
	}

	void ConfigWidget::resetConfig() {
		d->resetConfig();
	}

	void ConfigWidget::saveConfig() {
		d->saveConfig();
	}

	void ConfigWidget::setLineEditText(const QString& node, const QString& text) {
		for (QLineEdit* obj : d->LineEditDefault.keys()) {
			if (obj->objectName() == node) {
				obj->setText(text);
				return;
			}
		}
	}

	void ConfigWidget::setComboBoxIndex(const QString& node, int index) {
		for (QComboBox* obj : d->ComboBoxDefault.keys()) {
			if (obj->objectName() == node) {
				if (index >= 0 && index < obj->count()) {
					obj->setCurrentIndex(index);
				}
				return;
			}
		}
	}

	void ConfigWidget::setRadioButtonChecked(const QString& node, bool checked) {
		for (QRadioButton* obj : d->RadioButtonDefault.keys()) {
			if (obj->objectName() == node) {
				obj->setChecked(checked);
				return;
			}
		}
	}
}