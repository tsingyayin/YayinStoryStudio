#include "AStorySyntax/AStoryXDiagnosticData.h"

namespace ASERStudio::AStorySyntax {
	 class AStoryXDiagnosticDataPrivate {
		 friend class AStoryXDiagnosticData;
	 protected:
		 QString Message;
		 qint32 Line;
		 qint32 Column;
		 AStoryXDiagnosticData::DiagnosticType Type;
		 QString FixAdvice;
	 };

	 /*!
		\class ASERStudio::AStorySyntax::AStoryXDiagnosticData
		\brief AStoryXDiagnosticData记录AStoryX的诊断信息。
		\since ASERStudio 2.0
		\inmodule ASERStudio

		这个类是由AStoryXController解析AStoryX时得到的诊断信息的载体。它包含了诊断消息、位置、类型和修复建议等信息。

		这个类是只读的，一旦创建它，就不能再更改。
	 */

	 /*!
		\enum ASERStudio::AStorySyntax::AStoryXDiagnosticData::DiagnosticType
		\value Undefined 未定义的错误类型
		\value UnknownError 未知错误
		\value NoSuchController 没有这样的控制器
		\value MissingRequiredParameter 缺少必选参数
		\value InvalidParameterType 参数类型无效
		\value UnexpectedParameter 出现了意外的参数
		\value UnclosedSymbol 未闭合的符号，它泛指一切应该成对出现的符号（甚至包括XML标签）
		\value VariableNotDefined 变量未定义
		\value UseTabInsteadSpace 使用了空格但应该使用制表符
		\value UseSpaceInsteadTab 使用了制表符但应该使用空格
		\value UseEnglishPunctuation 使用了中文标点但应该使用英文标点
		\value UseChinesePunctuation 使用了英文标点但应该使用中文标点
		\value NoSuchBlock 找不到代码块
		\value BlockNameConflict 代码块名称冲突
		\value InstrumentRedundant 冗余的指令（重定义）
		\value NoSuchPreprocessor 没有这样的处理器
		\value UnknownWarning 未知警告
		\value NoSuchResource 没有这样的资源
		
		UnknownError开始的错误类型均为可能导致AStoryX根本无法运行的错误，
		而UnknownWarning开始的错误类型则是不影响运行但可能导致AStoryX行为不符合预期的警告。
		由于ASE-Remake的控制器识别逻辑里，会将所有匹配不到的控制器都默认视作
		Dialog控制器，因此实际上NoSuchController暂时是不会出现的，但为了完整性和未来可能的变化，还是保留了这个错误类型。
	 */

	 /*!
		\since ASERStudio 2.0
		默认构造函数，创建一个未定义的诊断信息。
	 */
	 AStoryXDiagnosticData::AStoryXDiagnosticData() {
		 d = new AStoryXDiagnosticDataPrivate;
	 }

	 /*!
		\since ASERStudio 2.0
		构造函数。
		\a message 诊断消息
		\a line 诊断位置的行号
		\a column 诊断位置的列号
		\a type 诊断类型，默认为Undefined
		\a fixAdvice 修复建议，默认为空字符串
	 */
	AStoryXDiagnosticData::AStoryXDiagnosticData(const QString& message, qint32 line, qint32 column, DiagnosticType type, const QString& fixAdvice) {
		d = new AStoryXDiagnosticDataPrivate;
		d->Message = message;
		d->Line = line;
		d->Column = column;
		d->Type = type;
		d->FixAdvice = fixAdvice;
	}

	/*!
		\since ASERStudio 2.0
		析构函数。
	*/
	AStoryXDiagnosticData::~AStoryXDiagnosticData() {
		delete d;
	}

	/*!
		\fn AStoryXDiagnosticData::AStoryXDiagnosticData(const AStoryXDiagnosticData& other)
		\since ASERStudio 2.0
		复制构造函数。
	*/

	/*!
		\fn AStoryXDiagnosticData::AStoryXDiagnosticData(AStoryXDiagnosticData&& other) noexcept
		\since ASERStudio 2.0
		移动构造函数。
	*/

	/*!
		\fn AStoryXDiagnosticData& AStoryXDiagnosticData::operator=(AStoryXDiagnosticData&& other)
		\since ASERStudio 2.0
		移动赋值运算符。
	*/

	/*!
		\fn AStoryXDiagnosticData& AStoryXDiagnosticData::operator=(const AStoryXDiagnosticData& other)
		\since ASERStudio 2.0
		复制赋值运算符。
	*/

	VICopyable_Impl(AStoryXDiagnosticData);
	VIMoveable_Impl(AStoryXDiagnosticData);

	/*!
		\since ASERStudio 2.0
		比较两个AStoryXDiagnosticData是否相等。相等的条件是它们的位置、类型都相同。
		消息和修复建议不在考虑范围内。
	*/
	bool AStoryXDiagnosticData::operator==(const AStoryXDiagnosticData& other) const {
		return d->Line == other.d->Line &&
			d->Column == other.d->Column &&
			d->Type == other.d->Type;
	}
	/*!
		\since ASERStudio 2.0
		返回诊断消息。
	*/
	QString AStoryXDiagnosticData::getMessage() const {
		return d->Message;
	}

	/*!
		\since ASERStudio 2.0
		返回诊断位置的行号。
	*/
	qint32 AStoryXDiagnosticData::getLine() const {
		return d->Line;
	}

	/*!
		\since ASERStudio 2.0
		返回诊断位置的列号。
	*/
	qint32 AStoryXDiagnosticData::getColumn() const {
		return d->Column;
	}

	/*!
		\since ASERStudio 2.0
		返回修复建议。
	*/
	QString AStoryXDiagnosticData::getFixAdvice() const {
		return d->FixAdvice;
	}

	/*!
		\since ASERStudio 2.0
		返回诊断类型。
	*/
	AStoryXDiagnosticData::DiagnosticType AStoryXDiagnosticData::getType() const {
		return d->Type;
	}
}