#include "../ThemeManager.h"
#include <QtGui/qcolor.h>
#include <QtCore/qfile.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include "Utility/JsonConfig.h"
#include "Widgets/StyleSheetTemplate.h"
#include "Utility/FileUtility.h"
#include "General/Log.h"
#include "General/VIApplication.h"
#include "General/Plugin.h"
#include <QtWidgets/qapplication.h>
#include <QtCore/qtimer.h>

namespace Visindigo::Widgets {
	class ThemeManagerPrivate {
		friend class ThemeManager;
	protected:
		QString ConfigPath;
		Visindigo::Utility::JsonConfig* Config = nullptr;
		Visindigo::Utility::JsonConfig* DefaultColorScheme = nullptr;
		QMap<QString, StyleSheetTemplate> Templates;
		QMap<QString, Visindigo::Utility::JsonConfig*> ColorSchemes;
		QMap<QString, Visindigo::General::Plugin*> PluginMapForSchemes;
		QMap<QString, Visindigo::General::Plugin*> PluginMapForTemplates;
		QList<ColorfulWidget*> RegisteredWidgets;
		QStringList ColorSchemePriority;
		QStringList StyleTemplatePriority;
		QString CurrentThemeID;
		StyleSheetTemplate MergedTemplate;
		Visindigo::Utility::JsonConfig* MergedColorScheme = nullptr;
		QMap<QString, QColor> PreviousColorMap;
		QMap<QString, QColor> ActiveColorMap;
		QMap<QString, QColor> AnimatingColorMap;
		float AnimationDurationMS = -1;
		float AnimationFrameRate = 60;
		float currentAnimationStep = 0;
		bool isInAnimation = false;
		bool AutoAdjustToSystem = true;
		static ThemeManager* Instance;
		static QMap<QString, ThemeManager::ThemeID> UserThemeIDEnumMap;
		QTimer* animationTimer = nullptr;
		void startThemeChangeAnimation();
	};

	void ThemeManagerPrivate::startThemeChangeAnimation() {
		if (!animationTimer) {
			animationTimer = new QTimer();
			QObject::connect(animationTimer, &QTimer::timeout, [this]() {
				currentAnimationStep += 1000.0 / AnimationFrameRate;
				float progress = currentAnimationStep / AnimationDurationMS;
				if (progress >= 1.0f) {
					// End animation
					isInAnimation = false;
					animationTimer->stop();
					for (ColorfulWidget* widget : RegisteredWidgets) {
						widget->onThemeChanged();
					}
					Instance->programThemeChanged(CurrentThemeID);
					return;
				}
				// Update colors
				QStringList keys = ActiveColorMap.keys();
				for (auto key : keys) {
					QColor startColor = PreviousColorMap.value(key, QColor("#ED1C24")); // Fallback to red
					QColor endColor = ActiveColorMap.value(key, QColor("#ED1C24")); // Fallback to red
					int r = startColor.red() + (endColor.red() - startColor.red()) * progress;
					int g = startColor.green() + (endColor.green() - startColor.green()) * progress;
					int b = startColor.blue() + (endColor.blue() - startColor.blue()) * progress;
					int a = startColor.alpha() + (endColor.alpha() - startColor.alpha()) * progress;
					AnimatingColorMap.insert(key, QColor(r, g, b, a));
				}
				for (ColorfulWidget* widget : RegisteredWidgets) {
					widget->onThemeChanged();
				}
				});
		}
		isInAnimation = true;
		currentAnimationStep = 0;
		animationTimer->start(1000.0 / AnimationFrameRate);
	}
	QMap<QString, ThemeManager::ThemeID> ThemeManagerPrivate::UserThemeIDEnumMap = QMap<QString, ThemeManager::ThemeID>();
	
	ThemeManager* ThemeManagerPrivate::Instance = nullptr;

	/*!
		\class Visindigo::Widgets::ThemeManager
		\inheaderfile Widgets/ThemeManager.h
		\brief ThemeManager提供了比QPalette更强大的主题管理功能。
		\inmodule Visindigo
		\since Visindigo 0.13.0

		\note 这是一个从旧版Visindigo继承下来的功能（VIWidgets/VIDynamicStyleSheet），但已经重新实现，因此行为和接口与旧版
		有所不同。建议用户参考本类的文档和示例代码，以了解如何使用新版本。

		Qt的主题管理功能主要通过QPalette实现，这是个非常好的设计，但它主要假定应用程序
		的主题是由程序全权决定的，而忽略了用户对主题的个性化需求。而ThemeManager则提供了
		复杂的主题管理功能，允许用户动态使用配色方案（Color Scheme）和样式模板（Style Template）。
		且允许通过配置加载顺序来组合多个配色方案和样式模板，从而实现更复杂的主题效果。

		\section1 基本概念 - 配色方案 (Color Scheme)
		ThemeManager的主题管理功能主要基于两个概念：配色方案（Color Scheme）和样式模板（Style Template）。

		\warning 请注意，ThemeManager里面的Color Scheme和Color Theme是两个拼读相近（中文含义也相近）的概念，但它们是不同的东西。
		Color Scheme是配色方案，一个方案里面可以包括多个Color Theme（色彩主题）。配色方案提供的若干个色彩主题（Color Theme）
		中只有一个会处于激活状态，ThemeManager会根据当前激活的色彩主题来获取颜色值。

		配色方案是一组预定义的颜色集合，定义了应用程序中各种UI元素的颜色属性，如背景色、前景色、按钮色等。
		配色方案以JSON格式存储，结构如下：
		\badcode
		{
			"SchemeID": "YSSDefault",
			"SchemeName": "Yayin Story Studio Default Color Scheme",
			"Themes": {
				"Dark":{
					"Background_Light": "#3A3A3A",
					...
				},
				"...": { ... }
			}
		}
		\endcode
		即，配色方案包含一个SchemeID（主题ID），一个SchemeName（主题名称），以及一个Themes对象（色彩主题），
		这Theme对象内包含了多个色彩主题，每个色彩主题对应一组颜色键值对。主题的名称必须是ThemeManager::ThemeID枚举
		中的一个值（如"Dark"、"Light"等），也可以是用户自定义的主题名称。但需要通过ThemeManager::registerUserThemeID
		方法注册才能正确识别。

		\section1 基本概念 - 样式模板 (Style Template)
		样式模板（Style Template）是若干组预定义的样式表（Style Sheet）的集合，这些样式表定义了应用程序中各种UI组件的外观和风格。

		它为Qt Style Sheet 提供了两个重要的扩展：变量代换和命名空间。

		变量代换允许在样式表中使用颜色主题中的颜色键作为变量，从而实现样式表与颜色主题的动态绑定。变量代换也允许
		使用此样式表的QWidget组件动态带入自己的property属性值作为变量，从而实现更灵活的样式定义。例如在下面
		的这个例子里，border-radius属性的值是通过${borderRadius}变量代换实现的，这个变量的值由使用此样式表的QWidget
		组件的borderRadius属性决定。
		\badcode
			!TemplateName: Default_Template
			!TemplateID: YSS
			------
			!Namespace: General
			@Default
			QWidget{
				border: 1px solid $(Background_Light);
				border-radius: ${borderRadius}px;
			}
		\endcode
		样式模板文件以.vst扩展名存储，它总是遵照这样的格式，即首先使用模板名称、模板ID以及六条横线分隔符作为文件开头
		然后使用!Namespace定义命名空间，接着使用@StyleSheetName定义样式组名称，最后是样式组内容。

		!Namespace是可选的，如果不定义，则样式表命名空间等同于模板ID。但样式组名称必须定义，用于引用这个样式组。
		一个样式组内可以有多个样式表，它们会被一同应用到使用此样式模板的组件上。
		\badcode
			!TemplateName: Default_Template
			!TemplateID: YSS
			------
			!Namespace: General
			@Default
			QWidget{
				border: 1px solid $(Background_Light);
				border-radius: ${borderRadius}px;
			}
			QPushButton{
				background-color: $(Button_Background);
				color: $(Button_Foreground);
			}
			@someOtherStyle
			...
		\endcode

		一个样式模板文件可以包含多个命名空间，每个命名空间内可以包含多个样式组：
		\badcode
			!TemplateName: Default_Template
			!TemplateID: YSS
			------
			!Namespace: General
			@Default
			...
			@someOtherStyle
			...

			!Namespace: Special
			@SpecialButton
			...
		\endcode

		要使用某个样式表，必须通过ThemeManager::getTemplate方法，传入样式表的完整键名，格式为 Namespace::StyleSheetName，
		例如 "General::Default"。请注意，模板ID不影响键名，模板ID只是在程序内部区分不同模板文件而已。

		除此之外，当样式组的第一行是以#开头时，表示这一行是注释行，会被忽略掉。

		\section1 加载与应用
		ThemeManager在初始化时会从配置文件中读取上次保存的配色方案、样式模板优先级列表和主题并进行加载。
		然后在恰当时机根据优先级列表合并所有配色方案和样式模板，生成一个最终的配色方案和样式模板，并应用到应用程序中。

		有关ThemeManager默认配置文件的位置，请参考Visindigo::General::VIApplication::EnvKey。

		如果各插件需要向程序注册配色方案或样式模板，使用函数ThemeManager::pluginRegisterColorScheme和
		ThemeManager::pluginRegisterStyleTemplate即可。注意它不提供卸载方法（是设计缺陷，但暂时不打算更改）。
		
		如果需要在程序运行时重载从文件系统中读取的配色方案和样式模板，可以使用函数loadAndRefresh。
		它默认在读取后重新合并（即调用mergeAndApply方法）并触发信号programThemeChanged。如果不需要自动合并和应用，可以传入
		false参数。

		如果插件需要在程序运行时动态更改颜色方案或样式模板，则重新调用对应的注册函数即可，
		但之后还需要手动调用mergeAndApply方法重新合并和应用主题。

		但如果只是需要更改配色方案，则使用函数changeColorTheme即可。

		要将ThemeManager的结果应用到应用程序中，可以在setStyleSheet时使用ThemeManager::getTemplate方法获取样式表字符串，
		例如
		\code
			Label->setStyleSheet(ThemeManager::getInstance()->getTemplate("General::Default", Label));
		\badcode
		当然，这太长了，因此我们提供了宏VISTMGT来简化这个过程，使用方法如下：
		\code
			Label->setStyleSheet(VISTMGT("General::Default", Label));
		\endcode
		这里的Label参数是可选的，如果提供了这个参数，ThemeManager会尝试从Label的property属性中获取变量值进行代换。

		当然也有更简化的做法，宏applyVIStyleTemplate看上去似乎为QWidget增加了一个成员函数，即：
		\code
			Label->applyVIStyleTemplate("General::Default");
		\endcode
		这个函数等同于调用QWidget::setStyleSheet(VISTMGT(key, this))。

		此外，如果你确定一个样式表不需要任何变量代换（包括颜色主题变量和组件property变量），则可以使用
		ThemeManager::getRawTemplate方法获取原始样式表字符串，例如
		\code
			Label->setStyleSheet(ThemeManager::getInstance()->getRawTemplate("General::Default"));
			Label->setStyleSheet(VISTMGRT("General::Default"));
		\endcode
		使用原始样式表字符串可以略微提升性能，因为省去了变量代换的过程。

		\section1 配色方案和样式模板优先级
		ThemeManager允许用户通过配置配色方案和样式模板的加载优先级来控制最终的主题效果。使用函数
		setColorSchemePriority和setStyleTemplatePriority可以设置优先级列表，列表中靠后的主题或模板会覆盖
		前面的主题或模板的同名属性，并最终生成一个合并后的主题或模板。

		\section1 工作顺序
		ThemeManager的工作顺序如下：
		\list
		\li 1. 在程序主插件启用前，ThemeManager会被初始化，并加载配置文件中的配色方案和样式模板优先级列表。
		同时将文件系统中的配色方案和样式模板加载到内存中。
		\li 2. 主程序插件应在启用时，向ThemeManager注册默认的配色方案和样式模板。
		\li 3. 各插件应在构造函数中，向ThemeManager注册自己的配色方案和样式模板。
		请务必注意主程序插件和插件的注册差别，如果插件的注册时间延后，会错过合并和应用主题的时机，从而导致主题不完整。
		\li 4. 在所有插件启用前，ThemeManager会调用mergeAndApply方法，合并所有注册的配色方案和样式模板，并应用到应用程序中。

		ThemeManager在主题变更时的工作顺序如下：
		\list
		\li 1. 如果启用了自动适应系统主题（通过autoAdjustThemeToSystem方法），则在系统主题变化时准备执行后续步骤。
		或者用户调用changeColorTheme方法，准备执行后续步骤。
		\li 2. 如果未启用渐变动画，则直接触发programThemeChanged信号，结束。如果启用渐变动画，则进入后续步骤。
		\li 3. 计算当前主题和目标主题之间的颜色差值，并启动一个定时器以设定的帧率周期性地更新主题颜色。
		\li 4. 在动画期间，ThemeManager会周期性地调用所有启用主题变化动画支持的ColorfulWidget组件的onThemeChanged方法，
		并传入当前动画帧的主题ID（始终为ThemeID::__InAnimation__）。组件应在此方法中重新设置样式表。
		\li 5. 动画结束后，触发programThemeChanged信号，表示主题变化完成。
		\endlist

		\section1 响应主题变化
		用户可以自行决定是否相应主题变化，以及主题变化后的处理方式（例如是立即重设样式表，还是要求用户重启程序）。

		如果不需要主题变化时的颜色渐变动画效果，可以直接连接programThemeChanged信号，并在槽函数中处理主题变化，
		这个信号会在主题变化时（如果有动画则会在动画结束后）被触发。

		但如果用户希望在切换主题时有颜色渐变动画效果，则需要扩展ColorfulWidget接口，并实现onThemeChanged方法。
		这个方法会在主题变化时的每一个动画帧调用。动画渐变过程处理是对用户封闭的，这个函数里只需要正常
		设置样式表即可，由ThemeManager内部变更实际的赋值。

		因此需要注意的是，ColorfulWidget的接口在动画期间的调用频率基本等价于设置的动画帧率，因此其中不应该有任何耗时操作，否则会影响动画流畅度。

		ColorfulWidget需要通过函数Visindigo::Widgets::ColorfulWidget::setEnable()方法启用主题变化动画支持。未
		启用支持的ColorfulWidget组件永远不会被调用onThemeChanged方法。
	*/

	/*!
		\enum Visindigo::Widgets::ThemeManager::ThemeID
		\since Visindigo 0.13.0
		\value __InAnimation__ 动画过渡中的临时主题ID，用于表示主题正在变化中。（启用动画过渡时使用）。
		虽然这是个特殊值，但它可以正常和字符串相互转换。用户不应该占用这个名称作为自定义主题ID。
		\value Unknown 未知主题
		\value Light 明亮主题
		\value Dark 黑暗主题
		\value White 白色主题（更浅的明亮主题）
		\value Black 黑色主题（更暗的黑暗主题）
		\value __UserMin__ 用户自定义主题ID的起始值。这个值及其之后的值保留给用户自定义主题ID使用。它不能
		直接作为主题ID使用。因此不能和字符串相互转换。从枚举转换到字符串以及从字符串转换到枚举都会变成Unknown。
		ThemeID枚举定义了ThemeManager支持的预定义主题ID。

		用户也可以通过ThemeManager::registerUserThemeID方法注册自定义主题ID。
	*/

	/*!
		\fn Visindigo::Widgets::ThemeManager::systemThemeChanged(Qt::ColorScheme newScheme)
		\since Visindigo 0.13.0
		当系统主题发生变化时触发此信号。参数newScheme表示新的系统主题。
		这个信号等同于QApplication::styleHints()->colorSchemeChanged信号。

		如果ThemeManager设置为自动适应系统主题（通过autoAdjustThemeToSystem方法），
		则稍后programThemeChanged信号也会被触发，表示程序主题也发生了变化。
	*/

	/*!
		\fn Visindigo::Widgets::ThemeManager::programThemeChanged(const QString& themeID)
		\since Visindigo 0.13.0
		当程序主题发生变化时触发此信号。参数themeID表示新的程序主题ID。

		如果ThemeManager设置为自动适应系统主题（通过autoAdjustThemeToSystem方法），
		则此信号可能会在systemThemeChanged信号之后被触发，表示程序主题也发生了变化。
	*/

	/*!
		\since Visindigo 0.13.0
		将ThemeID枚举值转换为字符串表示形式。
	*/
	QString ThemeManager::themeIDToString(ThemeID id) {
		switch (id) {
		case ThemeID::__InAnimation__:
			return "__InAnimation__";
		case ThemeID::Unknown:
			return "Unknown";
		case ThemeID::Light:
			return "Light";
		case ThemeID::Dark:
			return "Dark";
		case ThemeID::White:
			return "White";
		case ThemeID::Black:
			return "Black";
		default:
			for (auto it = ThemeManagerPrivate::UserThemeIDEnumMap.begin(); it != ThemeManagerPrivate::UserThemeIDEnumMap.end(); ++it) {
				if (it.value() == id) {
					return it.key();
				}
			}
			return "Unknown";
		}
	}

	/*!
		\since Visindigo 0.13.0
		将字符串表示形式转换为ThemeID枚举值。
	*/
	ThemeManager::ThemeID ThemeManager::stringToThemeID(const QString& str) {
		if (str == "__InAnimation__") {
			return ThemeID::__InAnimation__;
		}
		else if (str == "Unknown") {
			return ThemeID::Unknown;
		}
		else if (str == "Light") {
			return ThemeID::Light;
		}
		else if (str == "Dark") {
			return ThemeID::Dark;
		}
		else if (str == "White") {
			return ThemeID::White;
		}
		else if (str == "Black") {
			return ThemeID::Black;
		}
		else {
			if (ThemeManagerPrivate::UserThemeIDEnumMap.contains(str)) {
				return ThemeManagerPrivate::UserThemeIDEnumMap[str];
			}
			else {
				return ThemeID::Unknown;
			}
		}
	}

	/*!
		\since Visindigo 0.13.0
		注册用户自定义的主题ID。
	*/
	void ThemeManager::registerUserThemeID(const QString& id, ThemeID themeID) {
		if (themeID <= ThemeID::__UserMin__) {
			vgWarningF << "Cannot register user theme ID:" << id << "with reserved ThemeID value:" << (int)themeID;
			return;
		}
		if (id.isEmpty()) {
			vgWarningF << "Cannot register user theme ID: empty string.";
			return;
		}
		if (id == "__InAnimation__" || id == "Unknown" || id == "__UserMin__") {
			vgErrorF << "Cannot register user theme ID:" << id << "with reserved name.";
			return;
		}
		if (ThemeManagerPrivate::UserThemeIDEnumMap.contains(id)) {
			vgWarningF << "User theme ID:" << id << "is already registered, overwriting.";
		}
		ThemeManagerPrivate::UserThemeIDEnumMap[id] = themeID;
	}


	/*!
		\since Visindigo 0.13.0
		ThemeManager类的构造函数。
	*/
	ThemeManager::ThemeManager(QObject* parent) : QObject(parent) {
		d = new ThemeManagerPrivate();
		d->DefaultColorScheme = new Visindigo::Utility::JsonConfig();
		d->DefaultColorScheme->parse(Visindigo::Utility::FileUtility::readAll(":/Visindigo/compiled/default/defaultTheme.json"));
		//d->ColorSchemes["#Default"] = d->DefaultColorScheme;
		d->ConfigPath = VIApp->getEnvConfig(Visindigo::General::VIApplication::EnvKey::ThemeFolderPath).toString();
		d->Config = new Visindigo::Utility::JsonConfig();
		if (!Visindigo::Utility::FileUtility::isFileExist(d->ConfigPath + "/config.json")) {
			vgWarningF << "Theme config file not found at path:" << d->ConfigPath + "/config.json" << ", creating default config.";
			d->Config->setArray("Schemes", QStringList({"#Default"}));
			d->Config->setArray("Templates", QStringList());
			d->Config->setString("Theme", "Dark");
			vgDebugF << d->Config->toString();
			Visindigo::Utility::FileUtility::saveAll(d->ConfigPath + "/config.json", d->Config->toString());
		}
		d->Config->parse(Visindigo::Utility::FileUtility::readAll(d->ConfigPath + "/config.json"));
		if (d->Config->contains("Schemes")) {
			QStringList schemes = d->Config->getStringList("Schemes");
			setColorSchemePriority(schemes);
		}
		if (d->Config->contains("Templates")) {
			QStringList templates = d->Config->getStringList("Templates");
			setStyleTemplatePriority(templates);
		}
		if (d->Config->contains("Theme")) {
			d->CurrentThemeID = d->Config->getString("Theme");
		}
		else {
			d->CurrentThemeID = "Dark";
		}
		loadAndRefresh(false); // not merge for constructor
		connect(qApp->styleHints(), &QStyleHints::colorSchemeChanged, this, [this](Qt::ColorScheme newScheme) {
			emit systemThemeChanged(newScheme);
			if (d->AutoAdjustToSystem) {
				QString targetThemeID = themeIDToString((ThemeID)(newScheme));
				if (targetThemeID != d->CurrentThemeID) {
					changeColorTheme(targetThemeID);
				}
			}
			});
		vgSuccessF << "Success!";
	}

	/*!
		\since Visindigo 0.13.0
		ThemeManager类的析构函数。没有任何情况需要手动调用此函数。
	*/
	ThemeManager::~ThemeManager() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		获取ThemeManager的单例实例。
	*/
	ThemeManager* ThemeManager::getInstance() {
		if (ThemeManagerPrivate::Instance == nullptr) {
			ThemeManagerPrivate::Instance = new ThemeManager();
		}
		return ThemeManagerPrivate::Instance;
	}

	/*!
		\since Visindigo 0.13.0
		插件注册配色方案。\a plugin参数是注册此配色方案的插件实例，\a jsonStr参数是配色方案的JSON字符串。
		任何不符合规范的配色方案注册请求都会被拒绝并返回false。
	*/
	bool ThemeManager::pluginRegisterColorScheme(Visindigo::General::Plugin* plugin, const QString& jsonStr) {
		Visindigo::Utility::JsonConfig* json = new Visindigo::Utility::JsonConfig();
		if (json->parse(jsonStr).error == QJsonParseError::NoError) {
			if (!json->contains("SchemeID")) {
				vgWarningF << "Color scheme from plugin:" << plugin->getPluginName() << "does not contain SchemeID.";
				delete json;
				return false;
			}
			if (!json->contains("Themes")) {
				vgWarningF << "Color scheme from plugin:" << plugin->getPluginName() << "does not contain Themes.";
				delete json;
				return false;
			}
			if (!json->contains("SchemeName")) {
				vgWarningF << "Color scheme from plugin:" << plugin->getPluginName() << "does not contain SchemeName.";
				delete json;
				return false;
			}
			QString schemeID = json->getString("SchemeID");
			if (d->ColorSchemes.contains(schemeID)) {
				delete d->ColorSchemes[schemeID];
			}
			d->ColorSchemes[schemeID] = json;
			d->PluginMapForSchemes[schemeID] = plugin;
			return true;
		}
		else {
			vgWarningF << "Failed to parse color scheme from plugin:" << plugin->getPluginName();
			delete json;
			return false;
		}
	}

	/*!
		\since Visindigo 0.13.0
		插件注册样式模板。\a plugin参数是注册此样式模板的插件实例，\a vstStr参数是样式模板的字符串内容。
		任何不符合规范的样式模板注册请求都会被拒绝并返回false。
	*/
	bool ThemeManager::pluginRegisterStyleTemplate(Visindigo::General::Plugin* plugin, QString vstStr) {
		StyleSheetTemplate templateSS;
		if (templateSS.parse(vstStr)) {
			d->Templates[templateSS.getTemplateID()] = templateSS;
			d->PluginMapForTemplates[templateSS.getTemplateID()] = plugin;
			return true;
		}
		else {
			vgWarningF << "Failed to parse style template from plugin:" << plugin->getPluginName();
			return false;
		}
	}

	/*!
		\since Visindigo 0.13.0
		检查指定ID的配色方案是否由插件注册。
	*/
	bool ThemeManager::isColorSchemeFromPlugin(const QString& ID) {
		return d->PluginMapForSchemes.contains(ID);
	}

	/*!
		\since Visindigo 0.13.0
		检查指定ID的样式模板是否由插件注册。
	*/
	bool ThemeManager::isStyleTemplateFromPlugin(const QString& ID) {
		return d->PluginMapForTemplates.contains(ID);
	}

	/*!
		\since Visindigo 0.13.0
		从文件系统重新加载配色方案和样式模板，并可选择是否自动合并和应用。
		这不包括重新加载插件注册的配色方案和样式模板，它们需要插件自行重新注册。
		这也不会更改当前的配色方案和样式模板优先级列表。
	*/
	void ThemeManager::loadAndRefresh(bool autoMergeAndApply) {
		QMap<QString, StyleSheetTemplate> pluginTemplates = QMap<QString, StyleSheetTemplate>();
		QMap<QString, Visindigo::Utility::JsonConfig*> pluginSchemes = QMap<QString, Visindigo::Utility::JsonConfig*>();
		for (auto json : d->ColorSchemes.keys()){
			if (!isColorSchemeFromPlugin(json) && d->ColorSchemes[json] != d->DefaultColorScheme) {
				delete d->ColorSchemes[json];
			}
			else {
				pluginSchemes[json] = d->ColorSchemes[json];
			}
		}
		d->ColorSchemes = pluginSchemes;
		d->ColorSchemes["#Default"] = d->DefaultColorScheme;
		for (auto temp : d->Templates.keys()) {
			if (!isStyleTemplateFromPlugin(temp)) {
				d->Templates.remove(temp);
			}
			else {
				pluginTemplates[temp] = d->Templates[temp];
			}
		}
		d->Templates = pluginTemplates;

		QStringList templates = Visindigo::Utility::FileUtility::fileFilter(d->ConfigPath, { "*.vst" });
		QStringList schemes = Visindigo::Utility::FileUtility::fileFilter(d->ConfigPath, { "*.json" });

		for(auto name : templates) {
			QString all = Visindigo::Utility::FileUtility::readAll(name);
			StyleSheetTemplate templateSS;
			if (templateSS.parse(all)) {
				vgSuccessF << "Loaded style template at path:" << name;
				d->Templates[templateSS.getTemplateID()] = templateSS;
			}
			else {
				vgWarningF << "Failed to parse style template at path:" << name;
			}
		}

		for (auto name : schemes) {
			if (name == d->ConfigPath + "/config.json") {
				continue; // skip config file
			}
			QString all = Visindigo::Utility::FileUtility::readAll(name);
			Visindigo::Utility::JsonConfig* json = new Visindigo::Utility::JsonConfig();
			if (json->parse(all).error == QJsonParseError::NoError) {
				if (!json->contains("SchemeID")) {
					vgWarningF << "Color scheme at path:" << name << "does not contain SchemeID.";
					delete json;
					continue;
				}
				if (!json->contains("Themes")) {
					vgWarningF << "Color scheme at path:" << name << "does not contain Themes.";
					delete json;
					continue;
				}
				if (!json->contains("SchemeName")) {
					vgWarningF << "Color scheme at path:" << name << "does not contain SchemeName.";
					delete json;
					continue;
				}
				vgSuccessF << "Loaded color scheme at path:" << name;
				d->ColorSchemes[json->getString("SchemeID")] = json;
			}
			else {
				vgWarningF << "Failed to parse color scheme at path:" << name;
				delete json;
			}
		}
		if (autoMergeAndApply){
			mergeAndApply();
		}
	}

	/*!
		\since Visindigo 0.13.0
		将指定ID的配色方案添加到优先级列表中。
		如果指定ID的配色方案已在优先级列表中，则返回false。
	*/
	bool ThemeManager::addColorSchemeToPriority(const QString& ID) {
		if (d->ColorSchemePriority.contains(ID)) {
			vgWarningF << "Color scheme" << ID << "already added.";
			return false;
		}
		d->ColorSchemePriority.append(ID);
		return true;
	}

	/*!
		\since Visindigo 0.13.0
		将指定ID的样式模板添加到优先级列表中。
		如果指定ID的样式模板已在优先级列表中，则返回false。
	*/
	bool ThemeManager::addStyleTemplateToPriority(const QString& ID) {
		if (d->StyleTemplatePriority.contains(ID)) {
			vgWarningF << "Style template" << ID << "already added.";
			return false;
		}
		d->StyleTemplatePriority.append(ID);
		return true;
	}

	/*!
		\since Visindigo 0.13.0
		将指定ID的配色方案从优先级列表中移除。
		如果指定ID的配色方案不存在或不在优先级列表中，则返回false。
	*/
	bool ThemeManager::removeColorSchemeFromPriority(const QString& ID) {
		return d->ColorSchemePriority.removeAll(ID) > 0;
	}

	/*!
		\since Visindigo 0.13.0
		将指定ID的样式模板从优先级列表中移除。
		如果指定ID的样式模板不存在或不在优先级列表中，则返回false。
	*/
	bool ThemeManager::removeStyleTemplateFromPriority(const QString& ID) {
		return d->StyleTemplatePriority.removeAll(ID) > 0;
	}

	/*!
		\since Visindigo 0.13.0
		清空配色方案优先级列表。
	*/
	void ThemeManager::clearColorSchemePriority() {
		d->ColorSchemePriority.clear();
	}

	/*!
		\since Visindigo 0.13.0
		清空样式模板优先级列表。
	*/
	void ThemeManager::clearStyleTemplatePriority() {
		d->StyleTemplatePriority.clear();
	}

	/*!
		\since Visindigo 0.13.0
		设置配色方案优先级列表。
	*/
	void ThemeManager::setColorSchemePriority(const QStringList& schemeIDList) {
		d->ColorSchemePriority = schemeIDList;
	}

	/*!
		\since Visindigo 0.13.0
		设置样式模板优先级列表。
	*/
	void ThemeManager::setStyleTemplatePriority(const QStringList& templateIDList) {
		d->StyleTemplatePriority = templateIDList;
	}

	/*!
		\since Visindigo 0.13.0
		设置动画时间，单位为毫秒。
	*/
	void ThemeManager::setAnimationDuration(int ms) {
		d->AnimationDurationMS = ms;
	}

	/*!
		\since Visindigo 0.13.0
		设置动画帧率，单位为每秒帧数。
	*/
	void ThemeManager::setAnimationFrameRate(int fps) {
		d->AnimationFrameRate = fps;
	}

	/*!
		\since Visindigo 0.13.0
		获得当前动画时间，单位为毫秒。
	*/
	int ThemeManager::getAnimationDuration() {
		return d->AnimationDurationMS;
	}

	/*!
		\since Visindigo 0.13.0
		获得当前动画帧率，单位为每秒帧数。
	*/
	int ThemeManager::getAnimationFrameRate() {
		return d->AnimationFrameRate;
	}

	/*!
		\since Visindigo 0.13.0
		合并并应用当前的配色方案和样式模板优先级列表。
		请注意，如果先前设置的优先级列表中包含不存在的配色方案或样式模板ID，
		只有在调用此方法时才会检测到并发出警告，并同时被自动丢弃。
		它们不会阻止合并和应用过程。

		此函数执行后也会自动将当前的优先级列表和主题保存到配置文件中。
	*/
	void ThemeManager::mergeAndApply() {
		// Merge color schemes
		if (d->MergedColorScheme) {
			delete d->MergedColorScheme;
		}
		d->MergedColorScheme = new Visindigo::Utility::JsonConfig();
		d->MergedColorScheme->setString("SchemeID", "#MergedScheme");
		d->MergedColorScheme->setString("SchemeName", "Final Merged Color Scheme");
		QStringList finalSchemes;
		for (auto schemeID : d->ColorSchemePriority) {
			if (!d->ColorSchemes.contains(schemeID)) {
				vgWarningF << "Color scheme" << schemeID << "not found in ColorSchemes map. Skipping.";
				continue;
			}
			Visindigo::Utility::JsonConfig* scheme = d->ColorSchemes[schemeID];
			if (!scheme) {
				vgErrorF << "Color scheme" << schemeID << "is null. Skipping.\nBut this should not happen!";
			}
			QStringList themes = scheme->keys("Themes");
			for (auto themeID : themes) {
				QStringList colorNames = scheme->keys("Themes." + themeID);
				for (auto colorName : colorNames) {
					d->MergedColorScheme->setString("Themes." + themeID + "." + colorName, scheme->getString("Themes." + themeID + "." + colorName));
				}
			}
			finalSchemes.append(schemeID);
		}
		d->ColorSchemePriority = finalSchemes;
		// Merge style templates
		d->MergedTemplate = StyleSheetTemplate();
		d->MergedTemplate.setTemplateID("#MergedTemplate");
		d->MergedTemplate.setTemplateName("Final Merged Template");
		QStringList finalTemplates;
		for (auto templateID : d->StyleTemplatePriority) {
			if (!d->Templates.contains(templateID)) {
				vgWarningF << "Style template" << templateID << "not found in Templates map. Skipping.";
				continue;
			}
			StyleSheetTemplate templateSS = d->Templates[templateID];
			d->MergedTemplate.merge(templateSS);
			finalTemplates.append(templateID);
		}
		d->StyleTemplatePriority = finalTemplates;
		d->Config->setStringList("Schemes", d->ColorSchemePriority);
		d->Config->setStringList("Templates", d->StyleTemplatePriority);
		d->Config->setString("Theme", d->CurrentThemeID);
		Visindigo::Utility::FileUtility::saveAll(d->ConfigPath + "/config.json", d->Config->toString());
		this->changeColorTheme(d->CurrentThemeID);
	}

	/*!
		\since Visindigo 0.13.0
		获取所有已加载的配色方案ID列表。
	*/
	QStringList ThemeManager::getAllColorSchemes() {
		return d->ColorSchemes.keys();
	}

	/*!
		\since Visindigo 0.13.0
		获取当前配色方案优先级列表。
	*/
	QStringList ThemeManager::getAvailableColorSchemes() {
		return d->ColorSchemePriority;
	}

	/*!
		\since Visindigo 0.13.0
		获取所有已加载的样式模板ID列表。
	*/
	QStringList ThemeManager::getAllStyleTemplates() {
		return d->Templates.keys();
	}

	/*!
		\since Visindigo 0.13.0
		获取当前样式模板优先级列表。
	*/
	QStringList ThemeManager::getAvailableStyleTemplates() {
		return d->StyleTemplatePriority;
	}

	/*!
		\since Visindigo 0.13.0
		获取合并后的配色方案中的所有色彩主题ID列表。
	*/
	QStringList ThemeManager::getColorThemes() {
		return d->MergedColorScheme->keys();
	}

	/*!
		\since Visindigo 0.13.0
		获取合并后的样式模板中的所有命名空间列表。
	*/
	QStringList ThemeManager::getStyleNamespaces() {
		return d->MergedTemplate.getNamespaces();
	}

	/*!
		\since Visindigo 0.13.0
		更改当前的配色主题为指定的主题ID。
		如果指定的主题ID不存在于合并后的配色方案中，则返回false。
	*/
	bool ThemeManager::changeColorTheme(const QString& themeID) {
		vgDebugF << d->MergedColorScheme->toString();
		if (!d->MergedColorScheme->keys("Themes").contains(themeID)) {
			vgErrorF << "ThemeID" << themeID << "not found in merged color scheme. Change failed.";
			return false;
		}
		d->CurrentThemeID = themeID;
		d->PreviousColorMap = d->ActiveColorMap;
		d->ActiveColorMap = QMap<QString, QColor>();
		QStringList colorNames = d->MergedColorScheme->keys("Themes." + themeID);
		for (auto colorName : colorNames) {
			QString colorStr = d->MergedColorScheme->getString("Themes." + themeID + "." + colorName);
			d->ActiveColorMap[colorName] = QColor(colorStr);
		}
		if (d->PreviousColorMap.isEmpty()) {
			// First time setting theme, no animation
			for (auto widget : d->RegisteredWidgets) {
				widget->onThemeChanged();
			}
			emit programThemeChanged(themeID);
			return true;
		}
		if (d->AnimationDurationMS > 0) {
			// Start animation
			d->startThemeChangeAnimation();
		}
		else {
			for (auto widget : d->RegisteredWidgets) {
				widget->onThemeChanged();
			}
			emit programThemeChanged(themeID);
		}
		return true;
	}

	/*!
		\since Visindigo 0.13.0
		设置是否自动根据系统主题调整应用程序主题。
	*/
	void ThemeManager::autoAdjustThemeToSystem(bool autoAdjust) {
		d->AutoAdjustToSystem = autoAdjust;
	}

	/*!
		\since Visindigo 0.13.0
		获取当前的配色主题ID。
	*/
	QString ThemeManager::getCurrentColorTheme() {
		return d->CurrentThemeID;
	}

	/*!
		\since Visindigo 0.13.0
		根据指定的颜色键获取对应的QColor对象。
		如果当前没有设置主题或指定的键不存在，则返回红色（#ED1C24）作为错误指示。
	*/
	QColor ThemeManager::getColor(const QString& key) {
		if (d->CurrentThemeID.isEmpty()) {
			vgWarningF << "No current theme set.";
			return QColor("#ED1C24");
		}
		if (d->isInAnimation){
			if (d->AnimatingColorMap.contains(key)) {
				return d->AnimatingColorMap[key];
			}
			else {
				vgErrorF << "No such key" << key << "in animating color map.";
				return QColor("#ED1C24");
			}
		}
		else {
			if (d->ActiveColorMap.contains(key)) {
				return d->ActiveColorMap[key];
			}
			else {
				vgErrorF << "No such key" << key;
				return QColor("#ED1C24");
			}
		}
	}

	/*!
		\since Visindigo 0.13.0
		获取指定键的原始样式表字符串，不进行任何变量代换。
	*/
	QString ThemeManager::getRawTemplate(const QString& key) {
		return d->MergedTemplate.getRawStyleSheet(key);
	}

	/*!
		\since Visindigo 0.13.0
		获取指定键的样式表字符串，并进行变量代换。
		如果提供了\a getter参数，ThemeManager会尝试从该QWidget的property属性中获取变量值进行代换。
	*/
	QString ThemeManager::getTemplate(const QString& key, QWidget* getter) {
		//vgDebugF << "Getting template for key:" << key << "with getter:" << getter;
	    return d->MergedTemplate.getStyleSheet(key, getter);
	}

	/*!
		\since Visindigo 0.13.0
		注册一个ColorfulWidget组件，以便在主题变化动画期间接收onThemeChanged调用。
	*/
	void ThemeManager::registerColorfulWidget(Visindigo::Widgets::ColorfulWidget* widget) {
		if (!d->RegisteredWidgets.contains(widget)) {
			d->RegisteredWidgets.append(widget);
		}
	}

	/*!
		\since Visindigo 0.13.0
		注销一个ColorfulWidget组件，停止在主题变化动画期间接收onThemeChanged调用。
	*/
	void ThemeManager::unregisterColorfulWidget(Visindigo::Widgets::ColorfulWidget* widget) {
		d->RegisteredWidgets.removeAll(widget);
	}

	/*!
		\since Visindigo 0.13.0
		判断一个ColorfulWidget组件是否已注册。
	*/
	bool ThemeManager::isColorfulWidgetRegistered(Visindigo::Widgets::ColorfulWidget* widget) {
		return d->RegisteredWidgets.contains(widget);
	}

	/*!
		\macro VISTM
		\since Visindigo 0.13.0
		简化获取ThemeManager实例的过程，等同于Visindigo::Widgets::ThemeManager::getInstance()
	*/

	/*!
		\macro VISTMGT
		\since Visindigo 0.13.0
		简化获取样式表字符串的过程，等同于Visindigo::Widgets::ThemeManager::getInstance()->getTemplate
	*/

	/*!
		\macro VISTMGRT
		\since Visindigo 0.13.0
		简化获取原始样式表字符串的过程，等同于Visindigo::Widgets::ThemeManager::getInstance()->getRawTemplate
	*/

	void ColorfulWidget::setColorfulEnable(bool enable) {
		if (enable) {
			Visindigo::Widgets::ThemeManager::getInstance()->registerColorfulWidget(this);
		}
		else {
			Visindigo::Widgets::ThemeManager::getInstance()->unregisterColorfulWidget(this);
		}
	}

	bool ColorfulWidget::isColorfulEnabled() {
		return Visindigo::Widgets::ThemeManager::getInstance()->isColorfulWidgetRegistered(this);
	}

	ColorfulWidget::~ColorfulWidget() {
		Visindigo::Widgets::ThemeManager::getInstance()->unregisterColorfulWidget(this);
	}
}