#include "AStorySyntax/AStoryXControllerParseData.h"
#include "AStorySyntax/private/AStoryXControllerParseData_p.h"
namespace ASERStudio::AStorySyntax {
	/*!
		\class ASERStudio::AStorySyntax::AStoryXParameter
		\brief AStoryXParameter记录AStoryXController解析AStoryX时得到的参数信息。
		\since ASERStudio 2.0
		\inmodule ASERStudio

		这个类是由AStoryXController解析AStoryX时得到的参数信息的载体。它包含了参数名称、前缀、分隔符、内容、值等信息。

		这个类是只读的，一旦AStoryXController创建它，就不能再更改。
	*/

	/*!
		\since ASERStudio 2.0
		构造函数
	*/
	AStoryXParameter::AStoryXParameter() {
		d = new AStoryXParameterPrivate();
	}

	/*!
		\since ASERStudio 2.0
		析构函数
	*/
	AStoryXParameter::~AStoryXParameter() {
		delete d;
	}

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXParameter::AStoryXParameter(const AStoryXParameter& other)
		\since ASERStudio 2.0
		复制构造函数
	*/

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXParameter::AStoryXParameter(AStoryXParameter&& other) noexcept
		\since ASERStudio 2.0
		移动构造函数
	*/

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXParameter::operator=(const AStoryXParameter& other)
		\since ASERStudio 2.0
		复制赋值运算符
	*/

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXParameter::operator=(AStoryXParameter&& other) noexcept
		\since ASERStudio 2.0
		移动赋值运算符
	*/

	VIMoveable_Impl(AStoryXParameter);
	VICopyable_Impl(AStoryXParameter);

	/*
		\since ASERStudio 2.0
		获取参数名称。
	*/
	QString AStoryXParameter::getName() const {
		return d->Name;
	}

	/*
		\since ASERStudio 2.0
		获取参数前缀。
	*/
	QString AStoryXParameter::getPrefix() const {
		return d->Prefix;
	}

	/*
		\since ASERStudio 2.0
		获取参数分隔符。
	*/
	QString AStoryXParameter::getSeparater() const {
		return d->Separater;
	}

	/*
		\since ASERStudio 2.0
		获取参数内容。
	*/
	QString AStoryXParameter::getContent() const {
		return d->Content;
	}

	/*
		\since ASERStudio 2.0
		获取参数值元。
	*/
	AStoryXValueMeta AStoryXParameter::getValue() const {
		return d->Value;
	}

	/*
		\since ASERStudio 2.0
		获取参数在原文中的位置索引。
	*/
	qint32 AStoryXParameter::getIndex() const {
		return d->Index;
	}

	/*
		\since ASERStudio 2.0
		判断参数是否有效。这个有效指的是它是否包含有效信息。
		如果不包含有效信息，访问这个对象没有意义。
	*/
	bool AStoryXParameter::isValid() const {
		return d->Valid;
	}

	/*
		\since ASERStudio 2.0
		将参数信息转换为字符串形式，主要用于调试和诊断输出。
	*/
	QString AStoryXParameter::toString() const {
		if (!isValid()) {
			return "Invalid Parameter";
		}
		return QString("Parameter(Name: %1, Prefix: %2, Separater: %3, Content: %4, ValueType: %5, Index: %6)")
			.arg(getName())
			.arg(getPrefix())
			.arg(getSeparater())
			.arg(getContent())
			.arg(getValue().getType())
			.arg(getIndex());
	}

	/*!
		\class ASERStudio::AStorySyntax::AStoryXControllerParseData
		\brief AStoryXControllerParseData记录由AStoryXController解析AStoryX时得到的数据。
		\since ASERStudio 2.0
		\inmodule ASERStudio

		这个类是由AStoryXController解析AStoryX时得到的数据的载体。它包含了控制器类型、必选参数、可选参数等信息。

		这个类是只读的，一旦AStoryXController创建它，就不能再更改。

		值得指出的是，由于解析时的设置，getDiagnostics可能不会包含诊断信息。
		只有在isDiagnosticAvailable返回true，且getDiagnostics为空时，才保证没有语法错误。
		其他注意事项请参见AStoryXController::parseAStoryX的说明。

		\section1 无序的可选参数
		由于AStoryX相对AStory，允许在其满足称作“独一性”前缀定义的时候（即所有可选参数的前缀都不相同），
		可选参数可以以任意顺序出现。因此，AStoryXControllerParseData中的OptionalParameters和
		OptionalParameterStringIndex的顺序可能与定义顺序不同，用户不该假定它们是按照定义顺序排列的。
		用户应该根据OptionalParameterNames中的名称来获取对应参数的实际含义。

		\note AStoryX的”独一性“定义在ASE-Remake和ASERStudio中保持相同行为，但实际称谓略有不同。
		在代码实现中，ASE-Ramake将其称作”高级参数“（Advanced），而ASERStudio将其称作
		”单调参数“（Monotonic）。但它们的行为是完全相同的，只是叫法不同而已。

		\section1 单调性产生的解析行为
		例如，如果有一个控制器定义：
		\list
		\li 行首标识符：EG
		\li 必选参数内部分隔符：空格
		\li 必选参数：param1
		\li 可选参数：opt1（前缀/），opt2（前缀/），opt3（前缀/）
		\endlist
		那么对于以下AStoryX字符串

		\badcode
		EGp1 p2/rp1//rp3
		\endcode

		其中，必须参数param1的值为"p1 p2"（解析结果不自动切分，可根据返回值手动切分），
		而由于可选参数前缀有所重复（这个例子甚至完全相同），因此不满足单调性要求，所以只能按照定义顺序解析，
		于是可选参数opt1的值为"rp1"，opt3的值为"rp3"，而opt2位置的前缀之后没有值，因此opt2没有值。
		如果我们删掉opt2位置的前缀斜杠，即：

		\badcode
		EGp1 p2/rp1/rp3
		\endcode

		当中间缺少一个斜杠后，则可选参数op1的值为"rp1"，op2的值为"rp3"，而因为第三个斜杠缺失，
		因此opt3没有值。

		但假如控制器定义满足单调性要求（即前缀各不相同），则对于控制器：
		\list
		\li 行首标识符：EG2
		\li 必选参数内部分隔符：&
		\li 必选参数：param1
		\li 可选参数：opt1（前缀-@opt1:），opt2（前缀-@opt2:），opt3（前缀-@opt3:）
		\endlist

		则对于AStoryX字符串
		\badcode
		EG2p1&p2-@opt2:rp2-@opt1:rp1-@opt3:rp3
		\endcode
		其中，必选参数param1的值为"p1&p2"，而由于满足单调性要求，所以可选参数的解析不受定义顺序限制，
		因此opt1的值为"rp1"，opt2的值为"rp2"，opt3的值为"rp3"，而其实际书写顺序则是opt2、opt1、opt3。

		\section1 预处理器的特殊情况
		由于AStoryX相对AStory增加了预处理器机制，因此这个类返回的结果严格意义上不总是
		代表AStoryXController。如果parseAStoryX时发现是预处理器（即控制器类型是Preprocessor），
		则如下看待返回结果：

		\list
		\li ControllerType恒为Preprocessor
		\li RequiredParameter为预处理器的类型（不带#）
		\li OptionalParameters为预处理器的参数列表
		\endlist

		譬如，对于AStroyX的预处理器
		\badcode
		#useRule:BaseRule
		\endcode
		那么返回的ControllerType为Preprocessor，RequiredParameter为"useRule"，OptionalParameters为["BaseRule"]。

		再例如，对于AStroyX的预处理器
		\badcode
		#block 文本块1(参数1, 参数2)
		...
		#endblock
		\endcode
		那么文本块开头的返回结果为ControllerType为Preprocessor，RequiredParameter为"block"，OptionalParameters为["文本块1", "参数1", "参数2"]。
		文本块结束则返回ControllerType为Preprocessor，RequiredParameter为"endblock"，OptionalParameters为[]。
	*/

	/*!
		\since ASERStudio 2.0
		构造函数。
	*/
	AStoryXControllerParseData::AStoryXControllerParseData() {
		d = new AStoryXControllerParseDataPrivate();
	}

	/*!
		\fn AStoryXControllerParseData::AStoryXControllerParseData(AStoryXControllerParseData&& other)
		\since ASERStudio 2.0
		移动构造函数。
	*/

	/*!
		\fn AStoryXControllerParseData::AStoryXControllerParseData(const AStoryXControllerParseData& other)
		\since ASERStudio 2.0
		复制构造函数。
	*/

	/*!
		\fn AStoryXControllerParseData& AStoryXControllerParseData::operator=(AStoryXControllerParseData&& other)
		\since ASERStudio 2.0
		移动赋值运算符。
	*/

	/*!
		\fn AStoryXControllerParseData& AStoryXControllerParseData::operator=(const AStoryXControllerParseData& other)
		\since ASERStudio 2.0
		复制赋值运算符。
	*/
	VIMoveable_Impl(AStoryXControllerParseData);
	VICopyable_Impl(AStoryXControllerParseData);

	/*!
		\since ASERStudio 2.0
		析构函数。
	*/
	AStoryXControllerParseData::~AStoryXControllerParseData() {
		delete d;
	}

	/*!
		\since ASERStudio 2.0
		返回控制器类型。
	*/
	AStoryXController::ControllerType AStoryXControllerParseData::getControllerType() const {
		return d->ControllerType;
	}

	/*!
		\since ASERStudio 2.0
		返回行首标识符。对于预处理器来说，这个行首标识符永远是#。
	*/
	QString AStoryXControllerParseData::getStartSign() const {
		return d->StartSign;
	}

	/*!
		\since ASERStudio 2.0
		返回必选参数的解析结果
	*/
	AStoryXParameter AStoryXControllerParseData::getRequiredParameter() const {
		return d->RequiredParameter;
	}

	/*
		\since ASERStudio 2.0
		返回可选参数的解析结果列表。这个列表不包括那些没有被捕捉到的可选参数，
		因此它的长度可能小于可选参数的实际定义数量。
	*/
	QList<AStoryXParameter> AStoryXControllerParseData::getOptionalParameters() const {
		return d->OptionalParameters;
	}

	/*!
		\since ASERStudio 2.0
		返回给定光标位置所在的参数名称。如果光标位置不在任何参数内，则返回空字符串。
	*/
	QString AStoryXControllerParseData::getCursorInWhichParameter(qint32 cursorPosition) const {
		if (cursorPosition < 0) {
			return d->cursorInWhichParameter;
		}
		for (const auto& param : d->OptionalParameters) {
			if (param.getIndex() <= cursorPosition && cursorPosition <= param.getIndex() + param.getContent().length() + param.getPrefix().length()){
				return param.getName();
			}
		}
		if (d->OptionalParameters.size()!=0 && d->OptionalParameters.last().getIndex() + d->OptionalParameters.last().getContent().length() <= cursorPosition) {
			return d->OptionalParameters.last().getName();
		}
		if (d->RequiredParameter.isValid() && d->RequiredParameter.getIndex() <= cursorPosition && cursorPosition <= d->RequiredParameter.getIndex() + d->RequiredParameter.getContent().length()) {
			return d->RequiredParameter.getName();
		}
		return "";
	}

	/*!
		\since ASERStudio 2.0
		返回给定光标位置所在的参数解析结果。如果光标位置不在任何参数内，则返回一个无效的AStoryXParameter对象。
	*/
	AStoryXParameter AStoryXControllerParseData::getCursorParameter(qint32 cursorPosition) const {
		if (cursorPosition < 0) {
			for (const auto& param : d->OptionalParameters) {
				if (param.getName() == d->cursorInWhichParameter) {
					return param;
				}
			}
			if (d->RequiredParameter.isValid() && d->RequiredParameter.getName() == d->cursorInWhichParameter) {
				return d->RequiredParameter;
			}
			return AStoryXParameter();
		}
		for (const auto& param : d->OptionalParameters) {
			if (param.getIndex() <= cursorPosition && cursorPosition <= param.getIndex() + param.getContent().length() + param.getPrefix().length()) {
				return param;
			}
		}
		if (d->OptionalParameters.size()!=0 && d->OptionalParameters.last().getIndex() + d->OptionalParameters.last().getContent().length() <= cursorPosition) {
			return d->OptionalParameters.last();
		}
		if (d->RequiredParameter.isValid() && d->RequiredParameter.getIndex() <= cursorPosition && cursorPosition <= d->RequiredParameter.getIndex() + d->RequiredParameter.getContent().length()) {
			return d->RequiredParameter;
		}
		return AStoryXParameter();
	}
	/*!
		\since ASERStudio 2.0
		返回文本中引用的变量列表。	
	*/
	QStringList AStoryXControllerParseData::referenceVariables() const {
		return d->referenceVariables;
	}

	/*!
		\since ASERStudio 2.0
		解析是否启用了诊断。为false时，getDiagnostics的返回值不具有任何意义；
		为true时，getDiagnostics的返回值可能包含诊断信息，也可能不包含诊断信息（如果没有语法错误的话）。
	*/
	bool AStoryXControllerParseData::isDiagnosticAvailable() const {
		return d->DiagnosticAvailable;
	}

	/*!
		\since ASERStudio 2.0
		返回诊断信息列表。如果isDiagnosticAvailable返回false，则这个返回值不具有任何意义。
		如果isDiagnosticAvailable返回true，且这个返回值为空列表，则保证没有语法错误。
	*/
	QList<AStoryXDiagnosticData> AStoryXControllerParseData::getDiagnostics() const {
		return d->Diagnostics;
	}

	/*!
		\since ASERStudio 2.0
		返回这个解析结果的字符串表示。这个字符串表示不具有任何特定格式，仅供调试使用。
	*/
	QString AStoryXControllerParseData::toString() const {
		QString result = QString("ControllerType: %1\n").arg(d->ControllerType);
		result += QString("StartSign: %1\n").arg(d->StartSign);
		result += QString("RequiredParameter: %1\n").arg(d->RequiredParameter.isValid() ? d->RequiredParameter.toString() : "Invalid");
		for (const auto& param : d->OptionalParameters) {
			result += QString("OptionalParameter: %1\n").arg(param.isValid() ? param.toString() : "Invalid");
		}
		return result;
	}
	/*!
		\since ASERStudio 2.0
		返回这个解析结果是否有效。无效的解析结果代表它是创建即返回的，没有包含任何有效数据。
	*/
	bool AStoryXControllerParseData::isValid() const {
		return d->ControllerType != AStoryXController::ControllerType::Unknown;
	}
}