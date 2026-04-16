#include "AStorySyntax/AStoryXController.h"
#include <QtCore/qmap.h>
#include "AStorySyntax/AStoryXControllerParseData.h"
#include "AStorySyntax/private/AStoryXControllerParseData_p.h"
#include "AStorySyntax/AStoryXDiagnosticData.h"
#include <General/TranslationHost.h>
#include "AStorySyntax/AStoryXValueMeta.h"
#include <General/Log.h>
namespace ASERStudio::AStorySyntax {
	class AStoryXControllerPrivate {
		friend class AStoryXController;
	public:
		bool IsValid = false;
		AStoryXController::ControllerType Type;
		QString Header; //StartSign
		QString RequiredParameterName;
		AStoryXValueMeta RequiredParameterValue;
		QString RequiredParameterSeparater;
	private:
		QStringList OptionalParameterNames;
		QMap<QString, QString> OptionalParameterPrefix;
		QMap<QString, AStoryXValueMeta> OptionalParameterValue;
		bool prefixChanged = true;
		bool Monotonicity = false;
	public:
		QStringList getOptionalParameterPrefixes() {
			QStringList prefixes;
			for (auto key : OptionalParameterNames) {
				prefixes.append(OptionalParameterPrefix[key]);
			}
			return prefixes;
		}
		QStringList getOptionalParameterNames() {
			return OptionalParameterNames;
		}
		void setOptionalParameter(const QString& name, const QString& prefix, AStoryXValueMeta value) {
			OptionalParameterNames.append(name);
			OptionalParameterPrefix[name] = prefix;
			OptionalParameterValue[name] = value;
			prefixChanged = true;
		}
		bool isMonotonicity(){
			if (not prefixChanged) {
				return Monotonicity;
			}
			QStringList usedPrefix;
			for (auto prefix : OptionalParameterPrefix.values()) {
				if (usedPrefix.contains(prefix)) {
					Monotonicity = false;
					return false;
				}
				else {
					usedPrefix.append(prefix);
				}
			}
			Monotonicity = true;
			return true;
		}
		void parseNonmonotonicity(QMap<QString, QString> protectTMP, QMap<QString, QString> protectedVAR,
			QString& str, qint32 cursorPosition, bool diagnostic, qint32 lineIndex, AStoryXControllerParseData* data) {
			int outputIndexOffset = Header.length();
			QList<qint32> prefixIndexes;
			QStringList prefixes = getOptionalParameterPrefixes();
			QStringList names = getOptionalParameterNames();
			// scan all prefix indexes, if not found, index is -1.
			int index = 0;
			for (auto prefix : prefixes) {
				if (index >= str.length()) {
					prefixIndexes.append(-1);
					continue;
				}
				index = str.indexOf(prefix, index);
				if (index != -1) {
					prefixIndexes.append(index);
					index += prefix.length();
				}
				else {
					prefixIndexes.append(-1);
				}
			}
			// remove the prefix which is not found, and keep the order of prefixes.
			QStringList usedPrefix;
			QStringList usedNames;
			QList<qint32> usedPrefixIndexes;
			for (int i = 0; i < prefixIndexes.size(); i++){
				if (prefixIndexes[i] != -1) {
					usedPrefix.append(prefixes[i]);
					usedNames.append(names[i]);
					usedPrefixIndexes.append(prefixIndexes[i]);
				}
			}
			// check whether optional start at the beginning, if so, it's a error of missing required parameter.
			if (diagnostic && usedPrefixIndexes.size() > 0 && usedPrefixIndexes.first() == 0) {
				AStoryXDiagnosticData diagnosticData = AStoryXDiagnosticData(
					VITR("ASERStudio::diagnostic.missingRequiredParameter.message").arg(
						VITR(QString("ASERStudio::controller.%1").arg(QMetaEnum::fromType<AStoryXController::ControllerType>().valueToKey(Type)))
					).arg(RequiredParameterName),
					lineIndex, outputIndexOffset, AStoryXDiagnosticData::DiagnosticType::MissingRequiredParameter,
					VITR("ASERStudio::diagnostic.missingRequiredParameter.fixAdvice")
				);
				data->d->Diagnostics.append(diagnosticData);
			}
			auto requiredParameter = AStoryXParameter();
			QString content;
			// first ensure required. 
			if (usedPrefixIndexes.size()>0){
				content = str.left(usedPrefixIndexes.first());
			}
			else {
				content = str;
			}
			qint32 rawRequiredLength = content.length();
			//vgDebug << protectTMP;
			for(auto key : protectTMP.keys()) {
				content = content.replace(key, protectTMP[key]);
			}
			for(auto key : protectedVAR.keys()) {
				content = content.replace(key, protectedVAR[key]);
			}
			qint32 deltaLength = content.length() - rawRequiredLength;
			requiredParameter.d->setParameter(RequiredParameterName, "", content, RequiredParameterValue, outputIndexOffset, RequiredParameterSeparater);
			data->d->RequiredParameter = requiredParameter;
			if (usedPrefixIndexes.size() == 0) {
				if (cursorPosition > outputIndexOffset) {
					data->d->cursorInWhichParameter = RequiredParameterName;
				}
				return;
			}

			QList<qint32> fixedPrefixIndexes;
			for (int i = 0; i < usedPrefixIndexes.size(); i++) {
				qint32 prefixLength = usedPrefix[i].length();
				qint32 left = usedPrefixIndexes[i] + prefixLength;
				qint32 right = i == usedPrefixIndexes.size() - 1 ? str.length() : usedPrefixIndexes[i + 1];
				QString content = str.mid(left, right - left);
				qint32 rawContentLength = content.length();
				for (auto key : protectTMP.keys()) {
					content =	 content.replace(key, protectTMP[key]);
				}
				for (auto key : protectedVAR.keys()) {
					content = content.replace(key, protectedVAR[key]);
				}
				auto optionalParameter = AStoryXParameter();
				optionalParameter.d->setParameter(
					usedNames[i], usedPrefix[i], content, 
					OptionalParameterValue[usedNames[i]], 
					outputIndexOffset + usedPrefixIndexes[i] + deltaLength);
				fixedPrefixIndexes.append(usedPrefixIndexes[i] + deltaLength + outputIndexOffset);
				deltaLength += content.length() - rawContentLength;
				data->d->OptionalParameters.append(optionalParameter);
			}
			if (cursorPosition > 0) {
				if (cursorPosition < outputIndexOffset) {
					data->d->cursorInWhichParameter = ""; // even not in required parameter.
					return;
				}
				bool inOptionalParameter = false;
				for (int i = 0; i < fixedPrefixIndexes.size(); i++) {
					if (cursorPosition > fixedPrefixIndexes[i]){
						data->d->cursorInWhichParameter = data->d->OptionalParameters[i].getName();
						inOptionalParameter = true;
						break;
					}
				}
				if (not inOptionalParameter) {
					data->d->cursorInWhichParameter = RequiredParameterName;
				}
			}
		}
		void parseMonotonicity(QMap<QString, QString> protectTMP, QMap<QString, QString> protectedVAR,
			QString& str, qint32 cursorPosition, bool diagnostic, qint32 lineIndex, AStoryXControllerParseData* data) {
			int outputIndexOffset = Header.length();
			QList<qint32> prefixIndexes;
			QStringList prefixes = getOptionalParameterPrefixes();
			QStringList prefixNames = getOptionalParameterNames();
			for (auto prefix : prefixes) {
				prefixIndexes.append(str.indexOf(prefix));
			}
			QList<std::tuple<qint32, QString, QString>> avaliablePrefixIndexes; // index, prefix, name
			for(int i = 0; i < prefixes.size(); i++) {
				if (prefixIndexes[i] != -1) {
					avaliablePrefixIndexes.append(std::tuple<qint32, QString, QString>(prefixIndexes[i], prefixes[i], prefixNames[i]));
				}
			}
			std::sort(avaliablePrefixIndexes.begin(), avaliablePrefixIndexes.end(), [](const std::tuple<qint32, QString, QString>& a, const std::tuple<qint32, QString, QString>& b) {
				return std::get<0>(a) < std::get<0>(b);
				});
			prefixIndexes.clear();
			prefixes.clear();
			prefixNames.clear();
			for(auto pair : avaliablePrefixIndexes) {
				prefixIndexes.append(std::get<0>(pair));
				prefixes.append(std::get<1>(pair));
				prefixNames.append(std::get<2>(pair));
			}
			if (diagnostic && prefixIndexes.size() > 0 && prefixIndexes.first() == 0) {
				AStoryXDiagnosticData diagnosticData = AStoryXDiagnosticData(
					VITR("ASERStudio::diagnostic.missingRequiredParameter.message").arg(
						VITR(QString("ASERStudio::controller.%1").arg(QMetaEnum::fromType<AStoryXController::ControllerType>().valueToKey(Type)))
					).arg(RequiredParameterName),
					lineIndex, outputIndexOffset, AStoryXDiagnosticData::DiagnosticType::MissingRequiredParameter,
					VITR("ASERStudio::diagnostic.missingRequiredParameter.fixAdvice")
				);
				data->d->Diagnostics.append(diagnosticData);
			}
			auto requiredParameter = AStoryXParameter();
			QString content;
			if (prefixIndexes.size() > 0) {
				content = str.left(prefixIndexes.first());
			}
			else {
				content = str;
			}
			qint32 rawRequiredLength = content.length();
			for (auto key : protectTMP.keys()) {
				content = content.replace(key, protectTMP[key]);
			}
			for (auto key : protectedVAR.keys()) {
				content = content.replace(key, protectedVAR[key]);
			}
			qint32 deltaLength = content.length() - rawRequiredLength;
			requiredParameter.d->setParameter(
				RequiredParameterName, "", content,
				RequiredParameterValue, outputIndexOffset, RequiredParameterSeparater
			);
			data->d->RequiredParameter = requiredParameter;
			if (prefixIndexes.size() == 0) {
				if (cursorPosition > outputIndexOffset) {
					data->d->cursorInWhichParameter = RequiredParameterName;
				}
				return;
			}

			QList<qint32> fixedPrefixIndexes;
			for (int i = 0; i < prefixIndexes.size(); i++) {
				qint32 prefixLength = prefixes[i].length();
				qint32 left = prefixIndexes[i] + prefixLength;
				qint32 right = i == prefixIndexes.size() - 1 ? str.length() : prefixIndexes[i + 1];
				QString content = str.mid(left, right - left);
				qint32 rawContentLength = content.length();
				for (auto key : protectTMP.keys()) {
					content = content.replace(key, protectTMP[key]);
				}
				for (auto key : protectedVAR.keys()) {
					content = content.replace(key, protectedVAR[key]);
				}
				auto optionalParameter = AStoryXParameter();
				optionalParameter.d->setParameter(
					prefixNames[i], prefixes[i], content,
					OptionalParameterValue[prefixNames[i]],
					outputIndexOffset + prefixIndexes[i] + deltaLength
				);
				fixedPrefixIndexes.append(prefixIndexes[i] + deltaLength + outputIndexOffset);
				deltaLength += content.length() - rawContentLength;
				data->d->OptionalParameters.append(optionalParameter);
			};
			if (cursorPosition > 0) {
				if (cursorPosition < outputIndexOffset) {
					data->d->cursorInWhichParameter = ""; // even not in required parameter.
					return;
				}
				bool inOptionalParameter = false;
				for (int i = 0; i < fixedPrefixIndexes.size(); i++) {
					if (cursorPosition > fixedPrefixIndexes[i] ) {
						data->d->cursorInWhichParameter = data->d->OptionalParameters[i].getName();
						inOptionalParameter = true;
						break;
					}
				}
				if (not inOptionalParameter) {
					data->d->cursorInWhichParameter = RequiredParameterName;
				}
			}
		}
	};

	/*!
		\class ASERStudio::AStorySyntax::AStoryXController
		\brief AStoryXController是对AStoryX控制器和解析的一体化封装。
		\since ASERStudio 2.0
		\inmodule ASERStudio

		AStoryXController类封装了AStoryX控制器的定义和解析功能。
		它可以通过解析JSON配置来设置控制器的类型、头部信息、必需参数和可选参数等属性。
		同时，它还提供了一个方法来解析AStoryX字符串，并返回解析结果。

		它可同时用于对控制器的定义以及语法解析。有关解析结果的详细信息，以及“独一性”的具体解释，
		请参阅AStoryXControllerParseData类。

		值得指出的是，AStoryXController并不是AStoryXControllerParseData的唯一来源，另请参见
		AStoryXPreprocessor类。

		\note 这类目前是只读的，且只能从JSON配置中构造。未来可能会增加更多的构造方式和修改方法，但目前不保证。

		\section1 兼容语义
		ASE-Ramake和ASERStudio在部分概念的称呼上存在差异（但它们本质是同一个东西，
		也保证行为一致）。
		有关“独一性”的两种称呼问题，请参阅AStoryXControllerParseData类的相关说明。
		此类提供了等同函数isMonotonicity和isAdvanced，用户可以根据自己的习惯选择使用哪个函数。

		此外，ASE-Ramake将控制器行首标识符直接称作“Header”，而ASERStudio将其称作“StartSign”。
		ASERStudio 为这两种称呼提供了等同函数getStartSign和getHeader，用户可以根据自己的习惯选择使用哪个函数。
	*/

	/*!
		\since ASERStudio 2.0
		构造函数
	*/
	AStoryXController::AStoryXController() {
		d = new AStoryXControllerPrivate();
	}

	/*!
		\since ASERStudio 2.0
		析构函数
	*/
	AStoryXController::~AStoryXController() {
		delete d;
	}

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXController::AStoryXController(const AStoryXController& other)
		\since ASERStudio 2.0
		复制构造函数
	*/

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXController::AStoryXController(AStoryXController&& other) noexcept
		\since ASERStudio 2.0
		移动构造函数
	*/

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXController::operator=(const AStoryXController& other)
		\since ASERStudio 2.0
		复制赋值运算符
	*/

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXController::operator=(AStoryXController&& other) noexcept
		\since ASERStudio 2.0
		移动赋值运算符
	*/
	VIMoveable_Impl(AStoryXController);
	VICopyable_Impl(AStoryXController);

	/*
		\since ASERStudio 2.0
		解析JSON配置以设置控制器属性。
	*/
	bool AStoryXController::parseRule(const Visindigo::Utility::JsonConfig& config, const Visindigo::Utility::JsonConfig& meta) {
		if (config.isEmpty()) {
			return false;
		}
		QString key = config.getString("Key");
		if (key.isEmpty()) {
			return false;
		}
		d->Type = (AStoryXController::ControllerType)QMetaEnum::fromType<AStoryXController::ControllerType>().keyToValue(key.toStdString().c_str());
		if (d->Type == AStoryXController::ControllerType::Dialog) { // Dialog controller does not have header.
			d->Header = "";
		}
		else {
			d->Header = config.getString("Value.header");
			if (d->Header.isEmpty()) {
				return false;
			}
		}
		d->RequiredParameterName = config.getString("Value.requiredParametersName");
		d->RequiredParameterValue.setMetaData(key + "." + d->RequiredParameterName, meta);
		d->RequiredParameterSeparater = config.getString("Value.requiredParametersSeparater");
		bool ok = true;
		for(auto item: config.getArray("Value.optionalParameters")) {
			QString name = item.getString("parameterName");
			if (name.isEmpty()) {
				ok = false;
			}
			QString prefix = item.getString("prefix");
			if (prefix.isEmpty()) {
				ok = false;
			}
			AStoryXValueMeta parameterValue;
			//vgDebug << "Parsing optional parameter:" << name << " with prefix:" << prefix;
			parameterValue.setMetaData(key + "." + name, meta);
			d->setOptionalParameter(name, prefix, parameterValue);
		}
		d->IsValid = ok;
		return ok;
	}
	
	/*!
		\since ASERStudio 2.0
		解析AStoryX字符串并返回解析结果。
		\a str 要解析的AStoryX字符串
		\a cursorPosition 可选的光标位置参数，默认为-1。如果提供此参数，结果中将包含光标所在的参数名称，
		可以用于编辑器中的参数提示等功能。
		\a diagnostic 可选的诊断模式参数，默认为false。如果设置为true，解析器将返回更详细的诊断信息，
		包括解析错误的位置、类型和建议的修复方法等。这对于开发者调试和改进AStoryX语法非常有帮助。
		\a lineIndex 可选的行索引参数，默认为-1。如果提供此参数，则诊断结果会包含行索引信息，否则为-1。

		这个函数既可用于在编辑器中解析并进行语法提示，也可用于在运行时解析AStoryX字符串并执行相应的控制器行为。
		但显然前者会比较重，因此建议根据实际需要传递参数，并非总进行诊断。

		此外，除非你有特殊需要（比如在特定场景下只尝试按这个控制器的规则解析，而不考虑其他控制器），
		否则你通常不需要直接调用这个函数。针对任意文本的解析，
		应该使用ASERStudio::AStorySyntax::AStoryXRule::parseAStoryX函数，该函数会根据所有控制器的规则来解析文本，并返回最合适的解析结果。
	*/
	AStoryXControllerParseData AStoryXController::parseAStoryX(const QString& str, qint32 cursorPosition, bool diagnostic, qint32 lineIndex) {
		if (not d->IsValid) {
			return AStoryXControllerParseData();
		}
		AStoryXControllerParseData result;
		static QRegularExpression protectTMP("&\\{[\\d\\D]*?\\}");
		QRegularExpressionMatchIterator protectTMPIt = protectTMP.globalMatch(str);
		QMap<QString, QString> protectedStrs;
		quint32 i = 0;
		QString ptStr = str;
		static QString protectedStrTemplate = "▲★▲TMP%1▲★▲";
		for(auto match: protectTMPIt) {
			QString content = match.captured();
			ptStr.replace(content, protectedStrTemplate.arg(i));
			protectedStrs[protectedStrTemplate.arg(i)] = content;
			i++;
		}
		static QRegularExpression protectRef("\\$\\([\\d\\D]*?\\)");
		QRegularExpressionMatchIterator protectRefIt = protectRef.globalMatch(ptStr);
		i = 0;
		QMap<QString, QString> protectedRefStrs;
		static QString protectedRefTemplate = "▲★▲REF%1▲★▲";
		for(auto match: protectRefIt) {
			QString content = match.captured();
			ptStr.replace(content, protectedRefTemplate.arg(i));
			protectedRefStrs[protectedRefTemplate.arg(i)] = content;
			result.d->referenceVariables.append(content.mid(2, content.length() - 3));
			i++;
		}
		ptStr = ptStr.mid(d->Header.length());
		result.d->StartSign = d->Header;
		result.d->ControllerType = d->Type;
		result.d->DiagnosticAvailable = diagnostic;
		//vgDebug << d->isMonotonicity();
		if (d->isMonotonicity()) {
			d->parseMonotonicity(protectedStrs, protectedRefStrs, ptStr, cursorPosition, diagnostic, lineIndex, &result);
		}
		else {
			//非单调性解析
			d->parseNonmonotonicity(protectedStrs, protectedRefStrs, ptStr, cursorPosition, diagnostic, lineIndex, &result);
		}
		AStoryXParameter requiredParameter = result.getRequiredParameter();
		if (d->Type == AStoryXController::ControllerType::Dialog) {
			QString content = requiredParameter.getContent();
			if (content.contains(" ") && diagnostic) {
				AStoryXDiagnosticData diagnosticData = AStoryXDiagnosticData(
					VITR("ASERStudio::diagnostic.useTabInsteadSpace_dialog.message"),
					lineIndex, requiredParameter.getIndex() + content.indexOf(" ")
					, AStoryXDiagnosticData::DiagnosticType::UseTabInsteadSpace,
					VITR("ASERStudio::diagnostic.useTabInsteadSpace_dialog.fixAdvice")
				);
				result.d->Diagnostics.append(diagnosticData);
			}
		}
		auto reqResult = requiredParameter.getValue().isTypeMatching(requiredParameter.getContent());
		switch (reqResult) {
		case AStoryXDiagnosticData::Undefined:
			break;
		case AStoryXDiagnosticData::ParameterTypeMismatch: {
			if (diagnostic) {
				AStoryXDiagnosticData diagnosticData = AStoryXDiagnosticData(
					VITR("ASERStudio::diagnostic.parameterTypeMismatch.message")
					.arg(d->RequiredParameterName)
					.arg(AStoryXValueMeta::typeToString(AStoryXValueMeta::guessType(requiredParameter.getContent())))
					.arg(d->RequiredParameterValue.getTypeString()),
					lineIndex, requiredParameter.getIndex(), AStoryXDiagnosticData::DiagnosticType::ParameterTypeMismatch,
					VITR("ASERStudio::diagnostic.parameterTypeMismatch.fixAdvice").arg(d->RequiredParameterValue.getTypeString())
				);
				result.d->Diagnostics.append(diagnosticData);
			}
			break;
		}
		case AStoryXDiagnosticData::ParameterFormatError: {
			if (diagnostic) {
				QString msg = VITR("ASERStudio::diagnostic.parameterFormatError.message").arg(requiredParameter.getName());
				if (requiredParameter.getValue().getType() == AStoryXValueMeta::Type::Vector) {
					msg += VITR("ASERStudio::diagnostic.parameterFormatError.allowedDimension").
						arg(requiredParameter.getValue().getVectorCheckDimension());
				}
				else if (requiredParameter.getValue().getType() == AStoryXValueMeta::Type::String) {
					QString check = requiredParameter.getValue().getStringCheckRegex();
					if (not check.isEmpty()) {
						msg += VITR("ASERStudio::diagnostic.parameterFormatError.allowedFormat").arg(check);
					}
				}
				AStoryXDiagnosticData diagnosticData = AStoryXDiagnosticData(
					msg,
					lineIndex, requiredParameter.getIndex(), AStoryXDiagnosticData::DiagnosticType::ParameterFormatError,
					VITR("ASERStudio::diagnostic.parameterFormatError.fixAdvice")
				);
				result.d->Diagnostics.append(diagnosticData);
			}
			break;
		}
		case AStoryXDiagnosticData::ParameterOutOfRange: {
			QString msg = VITR("ASERStudio::diagnostic.parameterOutOfRange.message").arg(requiredParameter.getName());
			if (requiredParameter.getValue().getType() == AStoryXValueMeta::Type::Integer) {
				QPair<qint64, qint64> range = requiredParameter.getValue().getIntegerCheckRange();
				if (range.first != std::numeric_limits<qint64>::min() && range.second != std::numeric_limits<qint64>::max()) {
					msg += VITR("ASERStudio::diagnostic.parameterOutOfRange.allowedRange").
						arg(range.first).arg(range.second);
				}
				else if (range.first == std::numeric_limits<qint64>::min() && range.second != std::numeric_limits<qint64>::max()) {
					msg += VITR("ASERStudio::diagnostic.parameterOutOfRange.allowedMax").
						arg(range.second);
				}
				else if (range.first != std::numeric_limits<qint64>::min() && range.second == std::numeric_limits<qint64>::max()) {
					msg += VITR("ASERStudio::diagnostic.parameterOutOfRange.allowedMin").
						arg(range.first);
				}
			}
			else if (requiredParameter.getValue().getType() == AStoryXValueMeta::Type::Float) {
				QPair<double, double> range = requiredParameter.getValue().getFloatCheckRange();
				if (range.first != -std::numeric_limits<double>::infinity() && range.second != std::numeric_limits<double>::infinity()) {
					msg += VITR("ASERStudio::diagnostic.parameterOutOfRange.allowedRange").
						arg(range.first).arg(range.second);
				}
				else if (range.first == -std::numeric_limits<double>::infinity() && range.second != std::numeric_limits<double>::infinity()) {
					msg += VITR("ASERStudio::diagnostic.parameterOutOfRange.allowedMax").
						arg(range.second);
				}
				else if (range.first != -std::numeric_limits<double>::infinity() && range.second == std::numeric_limits<double>::infinity()) {
					msg += VITR("ASERStudio::diagnostic.parameterOutOfRange.allowedMin").
						arg(range.first);
				}
			}
			else if (requiredParameter.getValue().getType() == AStoryXValueMeta::Type::Enum) {
				QStringList allowedValues = requiredParameter.getValue().getEnumCheckList();
				if (!allowedValues.isEmpty()) {
					msg += VITR("ASERStudio::diagnostic.parameterOutOfRange.allowedEnums").arg(allowedValues.join(", "));
				}
			}
			if (diagnostic) {
				AStoryXDiagnosticData diagnosticData = AStoryXDiagnosticData(
					msg,
					lineIndex, requiredParameter.getIndex(), AStoryXDiagnosticData::DiagnosticType::ParameterOutOfRange,
					VITR("ASERStudio::diagnostic.parameterOutOfRange.fixAdvice")
				);
				result.d->Diagnostics.append(diagnosticData);
			}
			break;
		}
		}
		
		for (auto optional : result.getOptionalParameters()) {
			auto optResult = optional.getValue().isTypeMatching(optional.getContent());
			switch (optResult) {
			case AStoryXDiagnosticData::Undefined:
				break;
			case AStoryXDiagnosticData::ParameterTypeMismatch: {
				if (diagnostic) {
					AStoryXDiagnosticData diagnosticData = AStoryXDiagnosticData(
						VITR("ASERStudio::diagnostic.parameterTypeMismatch.message")
						.arg(optional.getName())
						.arg(AStoryXValueMeta::typeToString(AStoryXValueMeta::guessType(optional.getContent())))
						.arg(optional.getValue().getTypeString()),
						lineIndex, optional.getIndex(), AStoryXDiagnosticData::DiagnosticType::ParameterTypeMismatch,
						VITR("ASERStudio::diagnostic.parameterTypeMismatch.fixAdvice").arg(optional.getValue().getTypeString())
					);
					result.d->Diagnostics.append(diagnosticData);
				}
				break;
			}
			case AStoryXDiagnosticData::ParameterFormatError: {
				QString msg = VITR("ASERStudio::diagnostic.parameterFormatError.message").arg(optional.getName());
				if (optional.getValue().getType() == AStoryXValueMeta::Type::Vector) {
					msg += VITR("ASERStudio::diagnostic.parameterFormatError.allowedDimension").
						arg(optional.getValue().getVectorCheckDimension());
				}
				else if (optional.getValue().getType() == AStoryXValueMeta::Type::String) {
					QString check = optional.getValue().getStringCheckRegex();
					if (not check.isEmpty()) {
						msg += VITR("ASERStudio::diagnostic.parameterFormatError.allowedFormat").arg(check);
					}
				}
				if (diagnostic) {
					AStoryXDiagnosticData diagnosticData = AStoryXDiagnosticData(
						msg,
						lineIndex, optional.getIndex(), AStoryXDiagnosticData::DiagnosticType::ParameterFormatError,
						VITR("ASERStudio::diagnostic.parameterFormatError.fixAdvice")
					);
					result.d->Diagnostics.append(diagnosticData);
				}
				break;
			}
			case AStoryXDiagnosticData::ParameterOutOfRange: {
				QString msg = VITR("ASERStudio::diagnostic.parameterOutOfRange.message").arg(optional.getName());
				if (optional.getValue().getType() == AStoryXValueMeta::Type::Integer) {
					QPair<qint64, qint64> range = optional.getValue().getIntegerCheckRange();
					if (range.first != std::numeric_limits<qint64>::min() && range.second != std::numeric_limits<qint64>::max()) {
						msg += VITR("ASERStudio::diagnostic.parameterOutOfRange.allowedRange").
							arg(range.first).arg(range.second);
					}
					else if (range.first == std::numeric_limits<qint64>::min() && range.second != std::numeric_limits<qint64>::max()) {
						msg += VITR("ASERStudio::diagnostic.parameterOutOfRange.allowedMax").
							arg(range.second);
					}
					else if (range.first != std::numeric_limits<qint64>::min() && range.second == std::numeric_limits<qint64>::max()) {
						msg += VITR("ASERStudio::diagnostic.parameterOutOfRange.allowedMin").
							arg(range.first);
					}
				}
				else if (optional.getValue().getType() == AStoryXValueMeta::Type::Float) {
					QPair<double, double> range = optional.getValue().getFloatCheckRange();
					if (range.first != -std::numeric_limits<double>::infinity() && range.second != std::numeric_limits<double>::infinity()) {
						msg += VITR("ASERStudio::diagnostic.parameterOutOfRange.allowedRange").
							arg(range.first).arg(range.second);
					}
					else if (range.first == -std::numeric_limits<double>::infinity() && range.second != std::numeric_limits<double>::infinity()) {
						msg += VITR("ASERStudio::diagnostic.parameterOutOfRange.allowedMax").
							arg(range.second);
					}
					else if (range.first != -std::numeric_limits<double>::infinity() && range.second == std::numeric_limits<double>::infinity()) {
						msg += VITR("ASERStudio::diagnostic.parameterOutOfRange.allowedMin").
							arg(range.first);
					}
				}
				else if (optional.getValue().getType() == AStoryXValueMeta::Type::Enum) {
					QStringList allowedValues = optional.getValue().getEnumCheckList();
					if (!allowedValues.isEmpty()) {
						msg += VITR("ASERStudio::diagnostic.parameterOutOfRange.allowedEnums").arg(allowedValues.join(", "));
					}
				}
				if (diagnostic) {
					AStoryXDiagnosticData diagnosticData = AStoryXDiagnosticData(
						msg,
						lineIndex, optional.getIndex(), AStoryXDiagnosticData::DiagnosticType::ParameterOutOfRange,
						VITR("ASERStudio::diagnostic.parameterOutOfRange.fixAdvice")
					);
					result.d->Diagnostics.append(diagnosticData);
				}
				break;
			}
			}
		}
		return result;
	}

	/*!
		\since ASERStudio 2.0
		return 控制器的类型。
	*/
	AStoryXController::ControllerType AStoryXController::getControllerType() {
		return d->Type;
	}

	/*!
		\since ASERStudio 2.0
		return 用字符串表示的控制器类型。
	*/
	QString AStoryXController::getControllerTypeString() {
		return controllerTypeToString(d->Type);
	}

	/*!
		\since ASERStudio 2.0
		return 控制器行首标识符（即ASE-Ramake中的Header）。ASERStudio中也称其为StartSign。
	*/
	QString AStoryXController::getStartSign() {
		return d->Header;
	}

	/*!
		\since ASERStudio 2.0
		return 控制器行首标识符（即ASE-Ramake中的Header）。ASERStudio中也称其为StartSign。
	*/
	QString AStoryXController::getHeader() {
		return d->Header;
	}

	/*!
		\since ASERStudio 2.0
		return 必需参数的名称。
	*/
	QString AStoryXController::getRequiredParameterName() {
		return d->RequiredParameterName;
	}

	/*!
		\since ASERStudio 2.0
		return 必需参数的内部分隔符。
	*/
	QString AStoryXController::getRequiredParameterSeparater() {
		return d->RequiredParameterSeparater;
	}

	/*!
		\since ASERStudio 2.0
		return 可选参数的名称列表。
	*/
	QStringList AStoryXController::getOptionalParameterNames() {
		return d->getOptionalParameterNames();
	}

	/*!
		\since ASERStudio 2.0
		return 可选参数的前缀列表。前缀用于区分不同的可选参数，并且在解析AStoryX字符串时具有重要作用。
	*/
	QStringList AStoryXController::getOptionalParameterPrefixes() {
		return d->getOptionalParameterPrefixes();
	}

	/*!
		\since ASERStudio 2.0
		return 可选参数的值对象Map，每个值对象包含了该参数的类型、默认值、以及用于类型检查和转换的相关信息。
	*/
	QMap<QString, AStoryXValueMeta> AStoryXController::getOptionalParameterValues() {
		return d->OptionalParameterValue;
	}

	/*!
		\since ASERStudio 2.0
		return 必需参数的值对象，包含了该参数的类型、默认值、以及用于类型检查和转换的相关信息。
	*/
	AStoryXValueMeta AStoryXController::getRequiredParameterValue() {
		return d->RequiredParameterValue;
	}

	/*!
		\since ASERStudio 2.0
		 return 控制器的单调性（Monotonicity）。单调性是指控制器的可选参数前缀是否具有独一性，即是否存在两个或以上的可选参数前缀相同。
	*/
	bool AStoryXController::isMonotonicity() {
		return d->isMonotonicity();
	}

	/*!
		\since ASERStudio 2.0
		 return 控制器的高级性（Advanced）。和单调性等价，即控制器的可选参数前缀是否具有独一性。提供这个函数主要是为了兼容不同用户的习惯称呼。
	*/
	bool AStoryXController::isAdvanced() {
		return d->isMonotonicity();
	}

	/*!
		\since ASERStudio 2.0
		 return 控制器定义的有效性。一个有效的控制器定义应该至少包含一个非空的行首标识符（Header/StartSign）和一个必需参数名称。
	*/
	bool AStoryXController::isValid() {
		return d->IsValid;
	}

	/*!
		\since ASERStudio 2.0
		return 控制器定义的字符串表示形式，主要用于调试和日志记录等目的。通常会包含控制器的类型、行首标识符、必需参数名称和可选参数信息等内容。
	*/
	QString AStoryXController::toString() {
		QStringList optionalParams;
		for (auto name : getOptionalParameterNames()) {
			optionalParams.append(name + "(prefix: " + d->OptionalParameterPrefix[name] + ", type: " + QString::number(d->OptionalParameterValue[name].getType()) + ")");
		}
		return QString("Type: %1, Header: %2, RequiredParameterName: %3, OptionalParameters: [%4]")
			.arg(QMetaEnum::fromType<AStoryXController::ControllerType>().valueToKey(d->Type))
			.arg(d->Header)
			.arg(d->RequiredParameterName)
			.arg(optionalParams.join(", "));
	}

	/*!
		\since ASERStudio 2.0
		将ControllerType枚举值转换为字符串。
	*/
	QString AStoryXController::controllerTypeToString(ControllerType type) {
		return QMetaEnum::fromType<AStoryXController::ControllerType>().valueToKey(type);
	}

}
