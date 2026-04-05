#include "AStorySyntax/AStoryXRule.h"
#include "AStorySyntax/AStoryXValueMeta.h"
#include "AStorySyntax/AStoryXPreprocessor.h"
#include "AStorySyntax/AStoryXController.h"
#include "AStorySyntax/AStoryXControllerParseData.h"
#include "AStorySyntax/private/AStoryXControllerParseData_p.h"
#include <Utility/JsonConfig.h>
#include <Utility/FileUtility.h>
#include <QtCore/qjsondocument.h>
#include <General/Log.h>
namespace ASERStudio::AStorySyntax {
	class AStoryXRulePrivate {
		friend class AStoryXRule;
	public:
		bool isValid = false;
		QString Version;
		QString Name;
		QMap<AStoryXController::ControllerType, AStoryXController> Controllers;
		AStoryXPreprocessor Preprocessor;
		Visindigo::Utility::JsonConfig MetaConfig;
		static QMap<QString, AStoryXRule*> RegisteredRules;
	public:
		void loadMetaJson() {
			QString jsonStr = Visindigo::Utility::FileUtility::readAll(":/resource/cn.yxgeneral.aserstudio/astoryxMeta/" + Version.replace(".", "_") + ".json");
			if (jsonStr.isEmpty()) {
				jsonStr = Visindigo::Utility::FileUtility::readAll(":/resource/cn.yxgeneral.aserstudio/astoryxMeta/default.json");
				return;
			}
			MetaConfig.parse(jsonStr);
		}
	};

	QMap<QString, AStoryXRule*> AStoryXRulePrivate::RegisteredRules;

	/*!
		\class ASERStudio::AStorySyntax::AStoryXRule
		\brief AStoryXRule是对整个AStoryX语法规则的封装，包含了对不同版本的AStoryX语法规则的解析和存储。
		\since ASERStudio 2.0
		\inmodule ASERStudio
	*/

	/*!
		\since ASERStudio 2.0
		构造函数。
		\a version AStoryX语法规则的版本号，例如"1.0"、"2.0"等。
		这影响对AStoryX语法规则元数据的读取，如果ASERStudio不支持对应版本的规则，将会使用默认规则。
		默认规则永远是最新规则，不保证向下兼容。
	*/
	AStoryXRule::AStoryXRule(const QString& version) {
		d = new AStoryXRulePrivate;
		d->Version = version;
		d->loadMetaJson();
	}

	/*!
		\since ASERStudio 2.0
		析构函数。
	*/
	AStoryXRule::~AStoryXRule() {
		delete d;
		d = nullptr;
	}

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXRule::operator=(AStoryXRule&& other) noexcept
		\since ASERStudio 2.0
		移动赋值运算符
	*/

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXRule::operator=(const AStoryXRule& other)
		\since ASERStudio 2.0
		复制赋值运算符
	*/

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXRule::AStoryXRule(const AStoryXRule& other)
		\since ASERStudio 2.0
		复制构造函数
	*/

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXRule::AStoryXRule(AStoryXRule&& other) noexcept
		\since ASERStudio 2.0
		移动构造函数
	*/
	VIMoveable_Impl(AStoryXRule);
	VICopyable_Impl(AStoryXRule);
	
	/*!
		\since ASERStudio 2.0
		返回AStoryX语法规则的版本号。
	*/
	QString AStoryXRule::getVersion() const {
		return d->Version;
	}

	/*!
		\since ASERStudio 2.0
		设置AStoryX语法规则的名称。
		\a name AStoryX语法规则的名称。按ASE-Remake的要求，它必须
		为这个规则文件的主文件名，否则将引起AStoryX的解析错误。
		例如，如果规则文件名为"astoryx_v1_0.json"，则名称必须设置为"astoryx_v1_0"。
	*/
	void AStoryXRule::setName(const QString& name) {
		d->Name = name;
	}

	/*!
		\since ASERStudio 2.0
		返回AStoryX语法规则的名称。
	*/
	QString AStoryXRule::getName() const {
		return d->Name;
	}
	/*!
		\since ASERStudio 2.0
		解析JSON字符串以设置AStoryX语法规则。
		\a str 要解析的JSON字符串，必须符合ASERStudio的AStoryX语法规则JSON格式。
	*/
	bool AStoryXRule::parseJson(const QString& str) {
		Visindigo::Utility::JsonConfig config;
		QJsonParseError error = config.parse(str);
		if (error.error != QJsonParseError::NoError) {
			return false;
		}
		bool ok = true;
		for(auto item: config.getArray("rules")) {
			AStoryXController controller;
			if (not controller.parseRule(item, d->MetaConfig)) {
				ok = false;
			}
			d->Controllers.insert(controller.getControllerType(), controller);
			//vgDebug << controller;
		}
		d->isValid = ok;
		return ok;
	}

	/*!
		\since ASERStudio 2.0
		返回指定控制器类型的头部字符串。与getStartSign等价。
		\a type 控制器类型，例如AStoryXController::ControllerType::Background、AStoryXController::ControllerType::Music等。
	*/
	QString AStoryXRule::getHeader(AStoryXController::ControllerType type) const {
		if (d->Controllers.contains(type)) {
			return d->Controllers[type].getHeader();
		}
		return "";
	}

	/*!
		\since ASERStudio 2.0
		返回指定控制器类型的起始标志字符串。与getHeader等价。
		\a type 控制器类型，例如AStoryXController::ControllerType::Background、AStoryXController::ControllerType::Music等。
	*/
	QString AStoryXRule::getStartSign(AStoryXController::ControllerType type) const {
		if (d->Controllers.contains(type)) {
			return d->Controllers[type].getStartSign();
		}
		return "";
	}

	/*!
		\since ASERStudio 2.0
		返回指定控制器类型的必需参数名称。
		\a type 控制器类型，例如AStoryXController::ControllerType::Background、AStoryXController::ControllerType::Music等。
	*/
	QString AStoryXRule::getRequiredParameterName(AStoryXController::ControllerType type) const {
		if (d->Controllers.contains(type)) {
			return d->Controllers[type].getRequiredParameterName();
		}
		return "";
	}

	/*!
		\since ASERStudio 2.0
		返回指定控制器类型的必需参数分隔符。
		\a type 控制器类型，例如AStoryXController::ControllerType::Background、AStoryXController::ControllerType::Music等。
	*/
	QString AStoryXRule::getRequiredParameterSeparater(AStoryXController::ControllerType type) const {
		if (d->Controllers.contains(type)) {
			return d->Controllers[type].getRequiredParameterSeparater();
		}
		return "";
	}

	/*!
		\since ASERStudio 2.0
		返回指定控制器类型的可选参数名称列表。
		\a type 控制器类型，例如AStoryXController::ControllerType::Background、AStoryXController::ControllerType::Music等。
	*/
	QStringList AStoryXRule::getOptionalParameterNames(AStoryXController::ControllerType type) const {
		if (d->Controllers.contains(type)) {
			return d->Controllers[type].getOptionalParameterNames();
		}
		return QStringList();
	}

	/*!
		\since ASERStudio 2.0
		返回指定控制器类型的可选参数前缀列表。
		\a type 控制器类型，例如AStoryXController::ControllerType::Background、AStoryXController::ControllerType::Music等。
	*/
	QStringList AStoryXRule::getOptionalParameterPrefixes(AStoryXController::ControllerType type) const {
		if (d->Controllers.contains(type)) {
			return d->Controllers[type].getOptionalParameterPrefixes();
		}
		return QStringList();
	}

	/*!
		\since ASERStudio 2.0
		返回指定控制器类型的可选参数值对象Map。
		\a type 控制器类型，例如AStoryXController::ControllerType::Background、AStoryXController::ControllerType::Music等。
	*/
	QMap<QString, AStoryXValueMeta> AStoryXRule::getOptionalParameterValues(AStoryXController::ControllerType type) const {
		if (d->Controllers.contains(type)) {
			return d->Controllers[type].getOptionalParameterValues();
		}
		return QMap<QString, AStoryXValueMeta>();
	}

	/*!
		\since ASERStudio 2.0
		返回全部可用的控制器列表。
	*/
	QList<AStoryXController> AStoryXRule::getAvailableControllers() const {
		return d->Controllers.values();
	}

	/*!
		\since ASERStudio 2.0
		返回全部支持的预处理器名称列表。
	*/
	QStringList AStoryXRule::getSupportedPreprocessors() const {
		return d->Preprocessor.getSupportedPreprocessors();
	}

	/*!
		\since ASERStudio 2.0
		返回指定控制器类型的必需参数值对象。
	*/
	AStoryXValueMeta AStoryXRule::getRequiredParameterValue(AStoryXController::ControllerType type) const {
		if (d->Controllers.contains(type)) {
			return d->Controllers[type].getRequiredParameterValue();
		}
		return AStoryXValueMeta();
	}

	/*!
		\since ASERStudio 2.0
		返回指定控制器类型是否具有单调性。与isAdvanced等价。
		\a type 控制器类型，例如AStoryXController::ControllerType::Background、AStoryXController::ControllerType::Music等。
	*/
	bool AStoryXRule::isMonotonicity(AStoryXController::ControllerType type) const {
		if (d->Controllers.contains(type)) {
			return d->Controllers[type].isMonotonicity();
		}
		return false;
	}

	/*!
		\since ASERStudio 2.0
		返回指定控制器类型是否为高级控制器。与isMonotonicity等价。
		\a type 控制器类型，例如AStoryXController::ControllerType::Background、AStoryXController::ControllerType::Music等。
	*/
	bool AStoryXRule::isAdvanced(AStoryXController::ControllerType type) const {
		if (d->Controllers.contains(type)) {
			return d->Controllers[type].isAdvanced();
		}
		return false;
	}

	/*!
		\since ASERStudio 2.0
		\a str 要解析的AStoryX字符串，必须符合ASERStudio的AStoryX语法规则。
		\a cursorPosition 可选参数，表示光标位置，用于诊断和错误提示，默认为-1表示不使用光标位置。

		解析AStoryX字符串以获取控制器解析数据。

		这是不包含上下文处理的最完整的parseAStoryX解析函数，AStoryXController和AStoryXPreprocessor的解析函数都只能
		各自处理自己类型的AStoryX字符串，而这个函数会根据字符串内容自动判断应该使用哪个控制器或预处理器来解析，并返回解析结果。

		此外，这个函数能处理注释行（以"//"开头的行），并将其解析为AStoryXControllerParseData对象，ControllerType为Comment。
		RequiredParameter为注释的内容。
	*/
	AStoryXControllerParseData AStoryXRule::parseAStoryX(const QString& str, qint32 cursorPosition, bool diagnostic, qint32 lineIndex) {
		if (str.isEmpty()) {
			return AStoryXControllerParseData();
		}
		if (str.startsWith("//")) {
			auto rtn = AStoryXControllerParseData();
			rtn.d->ControllerType = AStoryXController::ControllerType::Comment;
			AStoryXParameter param;
			param.d->setParameter("comment", "", str.mid(2), AStoryXValueMeta("comment", AStoryXValueMeta::Type::Comment), 2);
			rtn.d->RequiredParameter = param;
			return rtn;
		}
		if (d->Preprocessor.isPreprocessor(str)) {
			return d->Preprocessor.parse(str, cursorPosition, diagnostic, lineIndex);
		}
		bool isNotDialog = true;
		for (auto cType : d->Controllers.keys()) {
			if (str.startsWith(d->Controllers[cType].getHeader()) && cType != AStoryXController::ControllerType::Dialog) {
				return d->Controllers[cType].parseAStoryX(str, cursorPosition, diagnostic, lineIndex);
			}
		}
		return d->Controllers[AStoryXController::ControllerType::Dialog].parseAStoryX(str, cursorPosition, diagnostic, lineIndex);
	}

	/*!
		\since ASERStudio 2.0
		返回AStoryX控制器的元数据，以JSON格式表示。
	*/
	Visindigo::Utility::JsonConfig AStoryXRule::getAStoryXControllerMetaData() const {
		return d->MetaConfig;
	}

	/*!
		\since ASERStudio 2.0
		返回AStoryX语法规则是否有效。
	*/
	bool AStoryXRule::isValid() const {
		return d->isValid;
	}
	/*!
		\since ASERStudio 2.0
		注册AStoryX语法规则。
		\a rule 要注册的AStoryXRule对象。注册后可以通过getRule函数获取该规则。
	*/
	void AStoryXRule::registerRule(const AStoryXRule& rule) {
		AStoryXRule* newRule = new AStoryXRule(rule);
		AStoryXRulePrivate::RegisteredRules.insert(newRule->getName(), newRule);
	}

	/*!
		\since ASERStudio 2.0
		获取已注册的AStoryX语法规则。
		\a name 要获取的AStoryXRule对象的名称。必须与注册时设置的名称一致。
		返回值：如果找到对应名称的规则，则返回指向该规则的指针；否则返回nullptr。
	*/
	AStoryXRule* AStoryXRule::getRule(const QString& name) {
		if (AStoryXRulePrivate::RegisteredRules.contains(name)) {
			return AStoryXRulePrivate::RegisteredRules[name];
		}
		return nullptr;
	}

	/*!
		\since ASERStudio 2.0
		检查是否存在已注册的AStoryX语法规则。
		\a name 要检查的AStoryXRule对象的名称。必须与注册时设置的名称一致。
		返回值：如果存在对应名称的规则，则返回true；否则返回false。
	*/
	bool AStoryXRule::hasRule(const QString& name) {
		return AStoryXRulePrivate::RegisteredRules.contains(name);
	}

	/*!
		\since ASERStudio 2.0
		获取所有已注册的AStoryX语法规则的名称列表。
		返回值：一个QStringList，包含所有已注册的AStoryXRule对象的名称。
	*/
	QStringList AStoryXRule::getAvailableRuleNames() {
		return AStoryXRulePrivate::RegisteredRules.keys();
	}
}