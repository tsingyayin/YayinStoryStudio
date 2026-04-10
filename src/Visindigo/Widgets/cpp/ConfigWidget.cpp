#include "../ConfigWidget.h"
#include <QtCore/qfile.h>
#include "../../Utility/JsonConfig.h"
#include "../../Utility/FileUtility.h"
#include "../../General/TranslationHost.h"
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qradiobutton.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qframe.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qfiledialog.h>
#include "../private/ConfigWidget_p.h"
#include "../../Widgets/MultiLabel.h"
#include "../../General/Log.h"
#include "../ThemeManager.h"
#include <QtWidgets/qtextedit.h>
#include <QtGui/qcolor.h>
#include <QtWidgets/qcolordialog.h>
#include "General/Placeholder.h"
namespace Visindigo::__Private__ {
	ConfigWidgetPrivate::ConfigWidgetPrivate(Visindigo::Widgets::ConfigWidget* self) {
		this->self = self;
		this->Layout = new QVBoxLayout(self);
		self->setLayout(Layout);
		self->setStyleSheet(VISTMGT("YSS::ConfigWidget", self));
	}

	ConfigWidgetPrivate::~ConfigWidgetPrivate() {
		saveConfig();
	}

	void ConfigWidgetPrivate::loadCWJson(const QString& json) {
		Visindigo::Utility::JsonConfig cwJson;
		QJsonParseError error = cwJson.parse(json);
		if (error.error != QJsonParseError::NoError) {
			return;
		}
		TargetConfigPath = VIPlaceholder(cwJson.getString("target"));
		TargetConfigNode = cwJson.getString("targetNode");
		if (!SettingsWidget.isEmpty()) {
			for (QWidget* w : SettingsWidget) {
				w->deleteLater();
			}
		}
		SettingsWidget.clear();
		QList<Visindigo::Utility::JsonConfig> widget = cwJson.getArray("widget");
		SettingsWidget = spawnWidget(widget);
		for (QWidget* w : SettingsWidget) {
			w->setParent(self);
			Layout->addWidget(w);
		}
		if (ResetButton) {
			ResetButton->deleteLater();
			ResetButton = nullptr;
		}
		if (SaveButton) {
			SaveButton->deleteLater();
			SaveButton = nullptr;
		}
		if (ButtonLayout) {
			ButtonLayout->deleteLater();
			ButtonLayout = nullptr;
		}
		ResetButton = new QPushButton(VITR("Visindigo::general.reset"), self);
		SaveButton = new QPushButton(VITR("Visindigo::general.save"), self);
		connect(ResetButton, &QPushButton::clicked, self, [this]() {
			this->resetConfig();
			emit self->reseted();
			});
		connect(SaveButton, &QPushButton::clicked, self, [this]() {
			this->saveConfig();
			emit self->saved();
			});
		ButtonLayout = new QHBoxLayout(self);
		ButtonLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
		ButtonLayout->addWidget(ResetButton);
		ButtonLayout->addWidget(SaveButton);
		Layout->addLayout(ButtonLayout);
		IndependentMode = cwJson.getBool("independent");
		if (not IndependentMode) {
			ResetButton->setVisible(false);
			SaveButton->setVisible(false);
		}
		else {
			ResetButton->setVisible(true);
			SaveButton->setVisible(true);
		}
		self->setStyleSheet(VISTMGT("YSS::ConfigWidget", self));
		self->setWindowTitle(VI18N(cwJson.getString("windowTitle")));
		self->setWindowIcon(QIcon(cwJson.getString("windowIcon")));
		this->initConfig();
	}

	void ConfigWidgetPrivate::initConfig() {
		QString config = Visindigo::Utility::FileUtility::readAll(TargetConfigPath);
		Config = Visindigo::Utility::JsonConfig();
		if (!config.isEmpty()) {
			Config.parse(config);
		}
		if (TargetConfigNode != "") {
			Config = Visindigo::Utility::JsonConfig(Config.getObject(TargetConfigNode));
		}
		spawnConfig();
		syncConfig();
	}

	void ConfigWidgetPrivate::syncConfig() {
		for (QComboBox* obj : ComboBoxDefault.keys()) {
			if (not Config.contains(obj->objectName())) {
				continue;
			}
			QString data = Config.getString(obj->objectName());
			for (int i = 0; i < obj->count(); i++) {
				if (obj->itemData(i) == data) {
					obj->setCurrentIndex(i);
					break;
				}
			}
		}
		for (QRadioButton* obj : RadioButtonDefault.keys()) {
			if (not Config.contains(obj->objectName())) {
				continue;
			}
			bool selected = Config.getBool(obj->objectName());
			if (obj->isChecked() == selected) {
				continue;
			}
			else {
				obj->toggle();
			}
		}
		for (QLineEdit* obj : LineEditDefault.keys()) {
			if (not Config.contains(obj->objectName())) {
				continue;
			}
			QString data = Config.getString(obj->objectName());
			obj->setText(data);
		}
		for (QTextEdit* obj : TextEditDefault.keys()) {
			if (not Config.contains(obj->objectName())) {
				continue;
			}
			QString data = Config.getString(obj->objectName());
			obj->setText(data);
		}
	}

	void ConfigWidgetPrivate::resetConfig() {
		Config.clear();
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
			if (not Config.contains(obj->objectName())) {
				Config.setString(obj->objectName(), data);
			}
		}
		for (QRadioButton* obj : RadioButtonDefault.keys()) {
			bool selected = RadioButtonDefault[obj];
			if (obj->isChecked() == selected) {
				continue;
			}
			else {
				obj->toggle();
			}
			if (not Config.contains(obj->objectName())) {
				Config.setBool(obj->objectName(), RadioButtonDefault[obj]);
			}
		}
		for (QLineEdit* obj : LineEditDefault.keys()) {
			QString data = LineEditDefault[obj];
			obj->setText(data);
			if (not Config.contains(obj->objectName())) {
				Config.setString(obj->objectName(), LineEditDefault[obj]);
			}
		}
		for (QTextEdit* obj : TextEditDefault.keys()) {
			QString data = TextEditDefault[obj];
			obj->setText(data);
			if (not Config.contains(obj->objectName())) {
				Config.setString(obj->objectName(), TextEditDefault[obj]);
			}
		}
	}

	void ConfigWidgetPrivate::saveConfig() {
		if (TargetConfigPath.isEmpty()) {
			return;
		}
		if (TargetConfigNode.isEmpty()) {
			QString config = Config.toString();
			if (config.isEmpty()) {
				return;
			}
			Visindigo::Utility::FileUtility::saveAll(TargetConfigPath, config);
		}
		else {
			Visindigo::Utility::JsonConfig rawConfig;
			rawConfig.parse(Visindigo::Utility::FileUtility::readAll(TargetConfigPath));
			rawConfig.setObject(TargetConfigNode, Config);
			QString config = rawConfig.toString();
			if (config.isEmpty()) {
				return;
			}
			Visindigo::Utility::FileUtility::saveAll(TargetConfigPath, config);
		}
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
		MultiLabel->setTitle(VI18N(config.getString("title")));
		MultiLabel->setDescription(VI18N(config.getString("text")));
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
			QWidget* target = widgetRouter(type, node, selfConfig, config.getBool("readOnly"));
			if (target != nullptr) {
				target->setMinimumWidth(200);
				target->setParent(SettingFrame);
				SettingLayout->addWidget(target);
			}
			else {
				self->deleteLater();
				return nullptr;
			}
		}
		QFrame* childLine = new QFrame(self);
		childLine->setFrameShape(QFrame::HLine);
		childLine->setFrameShadow(QFrame::Sunken);
		Layout->addWidget(childLine);

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

	QWidget* ConfigWidgetPrivate::widgetRouter(const QString& type, const QString& node, Visindigo::Utility::JsonConfig& config, bool readOnly) {
		QWidget* rtn = nullptr;
		vgDebugF << node << type;
		if (type == "Frame") {
			rtn = widget_QFrame(node, config, readOnly);
		}
		else if (type == "ComboBox") {
			rtn = widget_ComboBox(node, config, readOnly);
		}
		else if (type == "RadioButton") {
			rtn = widget_RadioButton(node, config, readOnly);
		}
		else if (type == "LineEdit") {
			rtn = widget_LineEdit(node, config, readOnly);
		}
		else if (type == "TextEdit") {
			rtn = widget_TextEdit(node, config, readOnly);
		}
		else if (type == "ColorDialog") {
			// Design of colordialog is not acceptable yet, so disabled for now
			//rtn = widget_ColorDialog(node, config, readOnly);
		}
		return rtn;
	}

	QWidget* ConfigWidgetPrivate::widget_QFrame(const QString& node, Visindigo::Utility::JsonConfig& config, bool readOnly) {
		// TODO: implement a shrink-expand frame
		return nullptr; 
	}

	QWidget* ConfigWidgetPrivate::widget_ComboBox(const QString& node, Visindigo::Utility::JsonConfig& config, bool readOnly) {
		QComboBox* ComboBox = new QComboBox();
		ComboBox->setObjectName(node);
		connect(ComboBox, &QComboBox::currentIndexChanged, this, &ConfigWidgetPrivate::onComboBoxIndexChanged);
		QList<Visindigo::Utility::JsonConfig> items = config.getArray("comboBox");
		for (Visindigo::Utility::JsonConfig item : items) {
			QString combo_data = item.getString("data");
			QString combo_key = VI18N(item.getString("key"));
			ComboBox->addItem(combo_key, combo_data);
		}
		QString defaultValue = config.getString("default");
		ComboBoxDefault.insert(ComboBox, defaultValue);
		if (readOnly) {
			ComboBox->setEnabled(false);
		}
		return ComboBox;
	}

	QWidget* ConfigWidgetPrivate::widget_RadioButton(const QString& node, Visindigo::Utility::JsonConfig& config, bool readOnly) {
		QRadioButton* RadioButton = new QRadioButton();
		RadioButton->setObjectName(node);
		connect(RadioButton, &QRadioButton::toggled, this, &ConfigWidgetPrivate::onRadioButtonChanged);
		bool defaultValue = config.getBool("default");
		RadioButtonDefault.insert(RadioButton, defaultValue);
		if (readOnly) {
			RadioButton->setEnabled(false);
		}
		return RadioButton;
	}

	QWidget* ConfigWidgetPrivate::widget_LineEdit(const QString& node, Visindigo::Utility::JsonConfig& config, bool readOnly) {
		QLineEdit* LineEdit = new QLineEdit();
		LineEdit->setObjectName(node);
		QString defaultValue = config.getString("default");
		connect(LineEdit, &QLineEdit::textChanged, this, &ConfigWidgetPrivate::onLineEditTextChanged);
		if (config.contains("isFolder") || config.contains("isFile")) {
			LineEdit->setText(VIPlaceholder(defaultValue));
			LineEditDefault.insert(LineEdit, VIPlaceholder(defaultValue));
			QFrame* container = new QFrame();
			QHBoxLayout* layout = new QHBoxLayout(container);
			LineEdit->setParent(container);
			layout->addWidget(LineEdit);
			QPushButton* selectButton = new QPushButton(container);
			selectButton->setText(VITR("Visindigo::general.preview"));
			layout->addWidget(selectButton);
			connect(selectButton, &QPushButton::clicked, [=]() {
				QString folder = QFileDialog::getExistingDirectory(container,
					VITR("Visindigo::general.selectFolder"),
					VIPlaceholder(defaultValue),
					QFileDialog::ShowDirsOnly
					);
				if (!folder.isEmpty()) {
					LineEdit->setText(folder);
				}
				});
			if (readOnly) {
				selectButton->setEnabled(false);
				LineEdit->setEnabled(false);
			}
			return container;
		}
		else {
			LineEditDefault.insert(LineEdit, defaultValue);
			LineEdit->setText(defaultValue);
			if (readOnly) {
				LineEdit->setEnabled(false);
			}
			return LineEdit;
		}
	}

	QWidget* ConfigWidgetPrivate::widget_TextEdit(const QString& node, Visindigo::Utility::JsonConfig& config, bool readOnly) {
		QTextEdit* TextEdit = new QTextEdit();
		TextEdit->setObjectName(node);
		QString defaultValue = config.getString("default");
		connect(TextEdit, &QTextEdit::textChanged, this, &ConfigWidgetPrivate::onTextEditTextChanged);
		TextEditDefault.insert(TextEdit, defaultValue);
		TextEdit->setText(defaultValue);
		if (readOnly) {
			TextEdit->setEnabled(false);
		}
		return TextEdit;
	}

	QWidget* ConfigWidgetPrivate::widget_ColorDialog(const QString& node, Visindigo::Utility::JsonConfig& config, bool readOnly) {
		QPushButton* ColorButton = new QPushButton();
		ColorButton->setObjectName(node);
		QString defaultValue = config.getString("default");
		QColor defaultColor(defaultValue);
		ColorButton->setStyleSheet(QString("QPushButton#%1{background-color: %1}").
			arg(ColorButton->objectName()).arg(defaultColor.name(QColor::HexArgb)));
		ColorButton->setText(defaultValue);
		connect(ColorButton, &QPushButton::clicked, [=]() {
			QColor clr = QColorDialog::getColor(defaultColor, self);
			if (clr.isValid()) {
				ColorButton->setStyleSheet(QString("QPushButton#%1{background-color: %1}").
					arg(ColorButton->objectName()).arg(clr.name(QColor::HexArgb)));
				onColorChanged(clr);
			}
			});
		if (readOnly) {
			ColorButton->setEnabled(false);
		}
		return ColorButton;
	}

	void ConfigWidgetPrivate::onComboBoxIndexChanged(int index) {
		QObject* obj = sender();
		QString node = obj->objectName();
		QComboBox* comboBox = static_cast<QComboBox*>(obj);
		QString data = comboBox->itemData(index).toString();
		Config.setString(node, data);
		emit self->comboBoxIndexChanged(node, index, data);
	}

	void ConfigWidgetPrivate::onRadioButtonChanged(bool checked) {
		QObject* obj = sender();
		QString node = obj->objectName();
		Config.setBool(node, checked);
		emit self->radioButtonChanged(node, checked);
	}

	void ConfigWidgetPrivate::onLineEditTextChanged(QString str) {
		QObject* obj = sender();
		QString node = obj->objectName();
		Config.setString(node, str);
		emit self->lineEditTextChanged(node, str);
	}

	void ConfigWidgetPrivate::onTextEditTextChanged() {
		QObject* obj = sender();
		QString node = obj->objectName();
		QTextEdit* textEdit = static_cast<QTextEdit*>(obj);
		QString str = textEdit->toPlainText();
		Config.setString(node, str);
		emit self->textEditTextChanged(node, str);
	}
	
	void ConfigWidgetPrivate::onColorChanged(const QColor& clr) {
		QObject* obj = sender();
		QString node = obj->objectName();
		Config.setString(node, clr.name(QColor::HexArgb));
		emit self->colorChanged(node, clr);
	}
}

namespace Visindigo::Widgets {
	/*!
		\class Visindigo::Widgets::ConfigWidget
		\brief 此类从CWJson创建配置窗口。
		\since Visindigo 0.13.0
		\inmodule Visindigo

		此类提供一种便捷的配置文件操作窗口创建方式，使用一种被约定为“CWJSON”的Json格式来描述配置窗口的内容。并且
		允许窗口和一个配置文件相绑定，将结果同步到配置文件中。

		CWJson的顶层格式如下：
		\badcode
			{
				"target": "$(ProgramPath)/resource/test_config.json",
				"targetType": "json",
				“targetNode": "someConfig.node",
				"widget":[]
			}
		\endcode
		其中，target为目标配置文件的路径，targetType为目标配置文件的类型，widget为窗口的内容。
		target键不存在的时候，则不和任何特定文件绑定，只在内存中保存配置数据。调用
		saveConfig()不产生任何效果，可以自行调用getConfig()将其另行处理。

		当targetNode被设置的时候，则只读取target中targetNode节点的数据进行初始化，并且只将targetNode节点的数据进行保存，其他节点的数据不受影响。

		这对于一份文件存储多个不同模块的配置时非常有用，可以让不同模块的配置窗口只关心自己对应的节点，而不需要担心其他节点的数据被覆盖。

		在有targetNode设置的情况下，存储数据时会先读取整个文件的数据，更新targetNode节点的数据后再写回文件，
		这个操作在单线程情况下是安全的（无论有多少个ConfigWidget准备写入这个Config文件），但
		多线程时可能会出现竞争条件，导致数据丢失或损坏，因此在多线程环境下使用时需要注意。

		\note 当前，targetType仅支持json格式，目前有支持YAML的计划，但具体实现时间未定。

		对于widget来说，其基本格式如下，我们以一个ComboBox类型的配置为例：
		\badcode
			{
				"node": "Theme",
				"type": "ComboBox",
				"icon": "",
				"title": "i18n:YSS::config.theme.title",
				"text": "i18n:YSS::config.theme.text",
				“readOnly": false,
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
		专门用来作为容器的控件，没有输入功能。因此Frame没有data项目。

		\note 实际上，内部逻辑是，只有检测到data项目时，再根据不同的type来初始化数据控件。因此当type不为
		Frame且漏写data时，其效果如同使用Frame，即没有输入功能，仅作为容器使用。

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
		\since Visindigo 0.13.0
		类的构造函数
	*/
	ConfigWidget::ConfigWidget(QWidget* parent) :QFrame(parent) {
		d = new Visindigo::__Private__::ConfigWidgetPrivate(this);
	}

	/*!
		\since Visindigo 0.13.0
		析构函数
	*/
	ConfigWidget::~ConfigWidget() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		\a json 为CWJson的内容
		加载CWJson
	*/
	void ConfigWidget::loadCWJson(const QString& json) {
		d->loadCWJson(json);
	}

	/*!
		\since Visindigo 0.13.0
		手动设置目标配置文件路径和节点信息。

		注意CWJson是只读的且只读一次，除非重新调用loadCWJson，否则
		调用此函数后CWJson中关于target和targetNode的设置将不再生效，但也不会被改写。
	*/
	void ConfigWidget::setTargetConfig(const QString& path, const QString& node, const QString& fileType) {
		d->TargetConfigPath = VIPlaceholder(path);
		d->TargetConfigNode = node;
		d->initConfig();
	}

	/*!
		\since Visindigo 0.13.0
		获取当下的配置数据，返回一个JsonConfig对象的指针，可以直接对其进行读写操作来获取或修改配置数据。
		返回的JsonConfig指针在此类的生命周期内始终有效，直到此类被销毁为止。
	*/
	Visindigo::Utility::JsonConfig* ConfigWidget::getConfig() {
		return &d->Config;
	}

	/*!
		\since Visindigo 0.13.0
		重置配置数据为默认值，默认值在CWJson中进行设置。
	*/
	void ConfigWidget::resetConfig() {
		d->resetConfig();
	}

	/*!
		\since Visindigo 0.13.0
		将当前配置数据同步到窗口中，更新窗口的显示状态。
	*/
	void ConfigWidget::syncConfig() {
		d->syncConfig();
	}

	/*!
		\since Visindigo 0.13.0
		将当前配置数据保存到目标配置文件中。

		如果目标文件路径未设置，则此函数不执行任何操作。
	*/
	void ConfigWidget::saveConfig() {
		d->saveConfig();
	}

	/*!
		\since Visindigo 0.13.0
		手动设置某个LineEdit的文本内容。
	*/
	void ConfigWidget::setLineEditText(const QString& node, const QString& text) {
		for (QLineEdit* obj : d->LineEditDefault.keys()) {
			if (obj->objectName() == node) {
				obj->setText(text);
				return;
			}
		}
	}

	/*!
		\since Visindigo 0.13.0
		手动设置某个ComboBox的选中项。
	*/
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

	/*!
		\since Visindigo 0.13.0
		手动设置某个RadioButton的选中状态。
	*/
	void ConfigWidget::setRadioButtonChecked(const QString& node, bool checked) {
		for (QRadioButton* obj : d->RadioButtonDefault.keys()) {
			if (obj->objectName() == node) {
				obj->setChecked(checked);
				return;
			}
		}
	}

	/*!
		\since Visindigo 0.13.0
		设置独立模式。独立模式下将显示重置和保存按钮，非独立模式下隐藏重置和保存按钮。
	*/
	void ConfigWidget::setIndependentMode(bool independent) {
		d->IndependentMode = independent;
		if (d->ResetButton != nullptr && d->SaveButton != nullptr) {
			d->ResetButton->setVisible(independent);
			d->SaveButton->setVisible(independent);
		}
	}

	bool ConfigWidget::isIndependentMode() const {
		return d->IndependentMode;
	}

}