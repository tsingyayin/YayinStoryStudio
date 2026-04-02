#include "AStorySyntax/AStoryXControllerParseData.h"
#include "AStorySyntax/private/AStoryXControllerParseData_p.h"
namespace ASERStudio::AStorySyntax {
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
		返回必选参数在AStoryX字符串中的位置（以字符串为单位）。
	*/
	qint32 AStoryXControllerParseData::getRequiredParameterStringIndex() const {
		return d->RequiredParameterStringIndex;
	}

	/*!
		\since ASERStudio 2.0
		返回可选参数在AStoryX字符串中的位置（以字符串为单位）。如果没有可选参数，则返回空列表。
	*/
	QList<qint32> AStoryXControllerParseData::getOptionalParameterStringIndexes() const {
		return d->OptionalParameterStringIndex;
	}

	/*!
		\since ASERStudio 2.0
		返回光标所在的参数。如果光标不在任何参数中，则返回空字符串。
		注意，这个返回混合了必选参数和可选参数，用户需要根据OptionalParameterNames来判断它是哪个参数。
	*/
	QString AStoryXControllerParseData::getCursorInWhichParameter(qint32 cursorPosition) const {
		if (cursorPosition >= 0) {
			qint32 firstNotEmpty = 0;
			for (qint32 index : d->OptionalParameterStringIndex) {
				if (index >= 0) {
					firstNotEmpty = index;
					break;
				}
			}
			if (cursorPosition<firstNotEmpty) {
				if (cursorPosition >= d->RequiredParameterStringIndex) {
					d->cursorInWhichParameter = d->RequiredParameter;
				}
			}
			else {
				for (qint32 index : d->OptionalParameterStringIndex) {
					if (cursorPosition >= index) {
						d->cursorInWhichParameter = d->OptionalParameterNames[d->OptionalParameterStringIndex.indexOf(index)];
					}
				}
			}
		}
		else {
			return d->cursorInWhichParameter;
		}
	}

	/*!
		\since ASERStudio 2.0
		返回必选参数的值。如果没有必选参数，则返回空字符串。
	*/
	QString AStoryXControllerParseData::getRequiredParameter() const {
		return d->RequiredParameter;
	}

	/*!
		\since ASERStudio 2.0
		返回可选参数的值列表。如果没有可选参数，则返回空列表。
	*/
	QStringList AStoryXControllerParseData::getOptionalParameters() const {
		return d->OptionalParameters;
	}

	/*!
		\since ASERStudio 2.0
		返回可选参数的名称列表。如果没有可选参数，则返回空列表。
	*/
	QStringList AStoryXControllerParseData::getOptionalParameterNames() const {
		return d->OptionalParameterNames;
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
		返回可选参数的数量。
	*/
	qint32 AStoryXControllerParseData::getOptionalParameterCount() const {
		return d->OptionalParameters.size();
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
		result += QString("IsValid: %1\n").arg(isValid());
		result += QString("RequiredParameter: %1\n").arg(d->RequiredParameter);
		result += QString("OptionalParameters: [%1]\n").arg(d->OptionalParameters.join(", "));
		result += QString("OptionalParameterNames: [%1]\n").arg(d->OptionalParameterNames.join(", "));
		result += QString("ReferenceVariables: [%1]\n").arg(d->referenceVariables.join(", "));
		result += QString("RequiredParameterStringIndex: %1\n").arg(d->RequiredParameterStringIndex);
		result += QString("DiagnosticAvailable: %1\n").arg(d->DiagnosticAvailable);
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