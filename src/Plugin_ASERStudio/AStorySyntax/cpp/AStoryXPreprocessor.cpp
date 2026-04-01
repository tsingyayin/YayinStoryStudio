#include "AStorySyntax/AStoryXPreprocessor.h"
#include "AStorySyntax/AStoryXDiagnosticData.h"
#include "AStorySyntax/AStoryXControllerParseData.h"
#include "AStorySyntax/private/AStoryXControllerParseData_p.h"
#include <QtCore/qstringlist.h>
namespace ASERStudio::AStorySyntax {
	class AStoryXPreprocessorPrivate {
		friend class AStoryXPreprocessor;
	protected:
		static const QStringList PreprocessorHeaders;
		void onUseRule(const QString& str, qint32 cursorPosition, AStoryXControllerParseData* data, bool diagnostic, qint32 lineIndex);
		void onAliases(const QString& str, qint32 cursorPosition, AStoryXControllerParseData* data, bool diagnostic, qint32 lineIndex);
		void onEndAliases(const QString& str, qint32 cursorPosition, AStoryXControllerParseData* data, bool diagnostic, qint32 lineIndex);
		void onBlock(const QString& str, qint32 cursorPosition, AStoryXControllerParseData* data, bool diagnostic, qint32 lineIndex);
		void onEndBlock(const QString& str, qint32 cursorPosition, AStoryXControllerParseData* data, bool diagnostic, qint32 lineIndex);
		void onUse(const QString& str, qint32 cursorPosition, AStoryXControllerParseData* data, bool diagnostic, qint32 lineIndex);
	};

	const QStringList AStoryXPreprocessorPrivate::PreprocessorHeaders = {
		"#useRule", "#aliases", "#endaliases", "#block", "#endblock", "#use"
	};

	void AStoryXPreprocessorPrivate::onUseRule(const QString& str, qint32 cursorPosition, 
		AStoryXControllerParseData* data, bool diagnostic, qint32 lineIndex) {
		QString content = str.mid(QString("#useRule:").length()).trimmed();
		data->d->RequiredParameter = "useRule";
		data->d->OptionalParameterNames.append("ruleName");
		data->d->OptionalParameters.append(content);
		data->d->RequiredParameterStringIndex = 0;
		if (cursorPosition>QString("#useRule:").length() && cursorPosition <= str.length()) {
			data->d->cursorInWhichParameter = "ruleName";
			data->d->OptionalParameterStringIndex.append(QString("#useRule:").length());
		}
		else {
			data->d->cursorInWhichParameter = "useRule";
		}
	}

	void AStoryXPreprocessorPrivate::onAliases(const QString& str, qint32 cursorPosition, 
		AStoryXControllerParseData* data, bool diagnostic, qint32 lineIndex) {
		data->d->RequiredParameter = "aliases";
		data->d->RequiredParameterStringIndex = 0;
		if (cursorPosition >= 0 && cursorPosition <= str.length()) {
			data->d->cursorInWhichParameter = "aliases";
		}
		else {
			data->d->cursorInWhichParameter = "aliases";
		}
	}

	void AStoryXPreprocessorPrivate::onEndAliases(const QString& str, qint32 cursorPosition, 
		AStoryXControllerParseData* data, bool diagnostic, qint32 lineIndex) {
		data->d->RequiredParameter = "endaliases";
		data->d->RequiredParameterStringIndex = 0;
		if (cursorPosition >= 0 && cursorPosition <= str.length()) {
			data->d->cursorInWhichParameter = "endaliases";
		}
		else {
			data->d->cursorInWhichParameter = "endaliases";
		}
	}

	void AStoryXPreprocessorPrivate::onBlock(const QString& str, qint32 cursorPosition, 
		AStoryXControllerParseData* data, bool diagnostic, qint32 lineIndex) {
		QString context = str.mid(QString("#block ").length()).trimmed();
		//#block blockName(param1, param2, ...)
		qint32 leftParenIndex = context.indexOf('(');
		qint32 rightParenIndex = context.lastIndexOf(')');
		data->d->RequiredParameter = "block";
		data->d->RequiredParameterStringIndex = 0;
		QString blockName = context.mid(0, leftParenIndex).trimmed();
		data->d->OptionalParameterNames.append("blockName");
		QString params = context.mid(leftParenIndex + 1, rightParenIndex - leftParenIndex - 1).trimmed();
		QStringList paramsList = params.split(',');
		int index = 0;
		int paramStartIndex = leftParenIndex + 1;
		for (const QString& param : paramsList) {
			data->d->OptionalParameterNames.append(QString("param%1").arg(index));
			data->d->OptionalParameters.append(param.trimmed());
			data->d->OptionalParameterStringIndex.append(paramStartIndex);
			paramStartIndex += param.length() + 1; // +1 for the comma
			index++;
		}
		if (cursorPosition >= 0) {
			if (cursorPosition <= QString("#block ").length()) {
				data->d->cursorInWhichParameter = "block";
				return;
			}
			else {
				for (int i = 0; i < data->d->OptionalParameterStringIndex.size(); i++) {
					if (cursorPosition > data->d->OptionalParameterStringIndex[i]) {
						data->d->cursorInWhichParameter = data->d->OptionalParameterNames[i + 1]; // +1 because OptionalParameterNames[0] is blockName
						break;
					}
				}
			}
		}
	}

	void AStoryXPreprocessorPrivate::onEndBlock(const QString& str, qint32 cursorPosition, 
		AStoryXControllerParseData* data, bool diagnostic, qint32 lineIndex) {
		data->d->RequiredParameter = "endblock";
		data->d->RequiredParameterStringIndex = 0;
		if (cursorPosition >= 0 && cursorPosition <= str.length()) {
			data->d->cursorInWhichParameter = "endblock";
		}
		else {
			data->d->cursorInWhichParameter = "endblock";
		}
	}

	void AStoryXPreprocessorPrivate::onUse(const QString& str, qint32 cursorPosition, AStoryXControllerParseData* data, bool diagnostic, qint32 lineIndex) {
		//#use blockName(param1, param2, ...)
		QString context = str.mid(QString("#use ").length()).trimmed();
		qint32 leftParenIndex = context.indexOf('(');
		qint32 rightParenIndex = context.lastIndexOf(')');
		data->d->RequiredParameter = "use";
		data->d->RequiredParameterStringIndex = 0;
		QString blockName = context.mid(0, leftParenIndex).trimmed();
		data->d->OptionalParameterNames.append("blockName");
		QString params = context.mid(leftParenIndex + 1, rightParenIndex - leftParenIndex - 1).trimmed();
		QStringList paramsList = params.split(',');
		int index = 0;
		int paramStartIndex = leftParenIndex + 1;
		for (const QString& param : paramsList) {
			data->d->OptionalParameterNames.append(QString("param%1").arg(index));
			data->d->OptionalParameters.append(param.trimmed());
			data->d->OptionalParameterStringIndex.append(paramStartIndex);
			paramStartIndex += param.length() + 1; // +1 for the comma
			index++;
		}
		if (cursorPosition >= 0) {
			if (cursorPosition <= QString("#use ").length()) {
				data->d->cursorInWhichParameter = "use";
				return;
			}
			else {
				for (int i = 0; i < data->d->OptionalParameterStringIndex.size(); i++) {
					if (cursorPosition > data->d->OptionalParameterStringIndex[i]) {
						data->d->cursorInWhichParameter = data->d->OptionalParameterNames[i + 1]; // +1 because OptionalParameterNames[0] is blockName
						break;
					}
				}
			}
		}
	}



	/*!
	   \class ASERStudio::AStorySyntax::AStoryXPreprocessor
	   \brief AStoryXPreprocessor是一个预处理器，用于在解析AStoryX之前对AStoryX进行预处理。
	   \since ASERStudio 2.0
	   \inmodule ASERStudio

	   AStoryXPreprocessor包括AStoryX中所有以#开头的指令。

	   实际上，与其称之为预处理器，“上下文控制器“可能更为合适。不过由于ASE-Remake文档已经称之为预处理，
	   且#指令的语法与C语言预处理器指令类似，所以就沿用这个名字了。

	   与AStoryXController不同，AStoryXPreprocessor在每个AStoryXRule中只有一个实例，用于处理全部情况。
	   
	   有关AStoryXPreprocessor如何通过AStoryXControllerParseData存储解析结果的说明，请参见AStoryXControllerParseData的文档。
	*/

	/*!
	   \since ASERStudio 2.0
	   构造函数。
	*/
	AStoryXPreprocessor::AStoryXPreprocessor() {
		d = new AStoryXPreprocessorPrivate;
	}

	/*!
	   \since ASERStudio 2.0
	   析构函数。
	*/
	AStoryXPreprocessor::~AStoryXPreprocessor() {
		delete d;
		d = nullptr;
	}

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXPreprocessor::AStoryXPreprocessor(AStoryXPreprocessor&& other)
		\since ASERStudio 2.0
		移动构造函数。
	*/

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXPreprocessor::AStoryXPreprocessor(const AStoryXPreprocessor& other)
		\since ASERStudio 2.0
		复制构造函数。
	*/

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXPreprocessor& AStoryXPreprocessor::operator=(AStoryXPreprocessor&& other)
		\since ASERStudio 2.0
		移动赋值运算符。
	*/

	/*!
		\fn ASERStudio::AStorySyntax::AStoryXPreprocessor& AStoryXPreprocessor::operator=(const AStoryXPreprocessor& other)
		\since ASERStudio 2.0
		复制赋值运算符。
	*/

	VIMoveable_Impl(AStoryXPreprocessor);
	VICopyable_Impl(AStoryXPreprocessor);
	/*!
	   \since ASERStudio 2.0
	   判断给定的字符串是否是一个预处理指令。
	   \a str 待判断的字符串
	*/
	bool AStoryXPreprocessor::isPreprocessor(const QString& str) {
		for (const QString& header : AStoryXPreprocessorPrivate::PreprocessorHeaders) {
			if (str.startsWith(header)) {
				return true;
			}
		}
		return false;
	}

	/*!
	   \since ASERStudio 2.0
	   解析给定的字符串，如果它是一个预处理指令的话。
	   \a str 待解析的字符串
	   \a cursorPosition 光标位置，用于确定光标位于哪个参数中
	   \a diagnostic 是否启用诊断信息
	   \a lineIndex 行索引，仅在启用诊断信息时有效
	*/
	AStoryXControllerParseData AStoryXPreprocessor::parse(const QString& str, qint32 cursorPosition, bool diagnostic, qint32 lineIndex) {
		if (!isPreprocessor(str)) {
			return AStoryXControllerParseData();
		}
		AStoryXControllerParseData data;
		data.d->ControllerType = AStoryXController::Preprocessor;
		data.d->DiagnosticAvailable = diagnostic;
		if (str.startsWith("#useRule")) {
			d->onUseRule(str, cursorPosition, &data, diagnostic, lineIndex);
		}
		else if (str.startsWith("#aliases")) {
			d->onAliases(str, cursorPosition, &data, diagnostic, lineIndex);
		}
		else if (str.startsWith("#endaliases")) {
			d->onEndAliases(str, cursorPosition, &data, diagnostic, lineIndex);
		}
		else if (str.startsWith("#block")) {
			d->onBlock(str, cursorPosition, &data, diagnostic, lineIndex);
		}
		else if (str.startsWith("#endblock")) {
			d->onEndBlock(str, cursorPosition, &data, diagnostic, lineIndex);
		}
		else if (str.startsWith("#use")) {
			d->onUse(str, cursorPosition, &data, diagnostic, lineIndex);
		}
		return data;
	}
}