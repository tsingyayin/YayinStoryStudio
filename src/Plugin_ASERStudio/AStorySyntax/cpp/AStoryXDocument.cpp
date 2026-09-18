#include <QtCore/qmap.h>
#include <QtCore/qtimer.h>
#include <QtGui/qtextdocument.h>
#include <Editor/TextEdit.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include "AStorySyntax/AStoryXControllerParseData.h"
#include "AStorySyntax/AStoryXDocument.h"
#include "AStorySyntax/AStoryXLineData.h"
#include "AStorySyntax/private/AStoryXControllerParseData_p.h"

namespace ASERStudio::AStorySyntax {
	class AStoryXDocumentPrivate {
		friend class AStoryXDocument;
	protected:
		AStoryXDocument* q;
		QString FilePath;
		YSSCore::Editor::TextEdit* HostEdit = nullptr;
		QStringList Lines; // only used in manual mode.
		QList<qint32> LineUserStates;  // only used in manual mode.
		AStoryXDocument::WorkMode Mode = AStoryXDocument::WorkMode::SyntaxHighlighter;
		AStoryXRule CurrentRule;
		bool EnableDiagnostic = false;
		QList<AStoryXControllerParseData> ParseDataCache;   // only used in manual mode.
		QList<AStoryXDiagnosticData> GlobalDiagnostics;
		AStoryXDiagnosticData RuleNotSelectedDiagnostic;

		// 语法高亮模式下，每一行的解析数据是挂在对应文本块上的一个 AStoryXLineData，
		// 行被增删时不需要任何人搬运它，因此下面的取用都要求 HostEdit 有效。
		AStoryXLineData* findLineData(qint32 lineNumber) const {
			if (HostEdit == nullptr || lineNumber < 0) {
				return nullptr;
			}
			return HostEdit->getBlockData<AStoryXLineData>(lineNumber);
		}

		AStoryXLineData* ensureLineData(qint32 lineNumber) {
			AStoryXLineData* lineData = findLineData(lineNumber);
			if (lineData != nullptr || HostEdit == nullptr || lineNumber < 0) {
				return lineData;
			}
			lineData = new AStoryXLineData();
			HostEdit->setBlockData<AStoryXLineData>(lineNumber, lineData);
			return lineData;
		}

		void clearLineData() {
			if (HostEdit != nullptr) {
				HostEdit->removeAllBlockData<AStoryXLineData>();
			}
		}

		// 取得该行可写的解析数据。语法高亮模式下数据挂在文本块上，手动模式下存在本类的列表里。
		AStoryXControllerParseData& mutableParseData(qint32 lineNumber) {
			if (Mode == AStoryXDocument::WorkMode::SyntaxHighlighter) {
				AStoryXLineData* lineData = ensureLineData(lineNumber);
				if (lineData != nullptr) {
					return lineData->ParseData;
				}
			}
			else {
				if (lineNumber >= ParseDataCache.size()) {
					for (qint32 i = ParseDataCache.size(); i <= lineNumber; ++i) {
						ParseDataCache.append(AStoryXControllerParseData());
					}
				}
				return ParseDataCache[lineNumber];
			}
			static AStoryXControllerParseData Dummy;
			return Dummy;
		}

		void onParsed(const AStoryXControllerParseData& data, qint32 lineNumber) {
			if (Mode == AStoryXDocument::WorkMode::SyntaxHighlighter) {
				AStoryXLineData* lineData = ensureLineData(lineNumber);
				if (lineData != nullptr) {
					lineData->ParseData = data;
				}
				return;
			}
			if (lineNumber >= ParseDataCache.size()) {
				for (qint32 i = ParseDataCache.size(); i <= lineNumber; ++i) {
					ParseDataCache.append(AStoryXControllerParseData());
				}
			}
			ParseDataCache[lineNumber] = data;
			//vgDebug << "Parsed line " << lineNumber << ": " << data;
		}

		void setUserState(qint32 lineNumber, qint32 state, QTextBlock* block = nullptr) {
			if (block && Mode == AStoryXDocument::WorkMode::SyntaxHighlighter) {
				block->setUserState(state);
			}
			else if (Mode == AStoryXDocument::WorkMode::Manual) {
				if (lineNumber >= 0 && lineNumber < LineUserStates.size()) {
					LineUserStates[lineNumber] = state;
				}
				else {
					for (qint32 i = LineUserStates.size(); i <= lineNumber; ++i) {
						LineUserStates.append(0);
					}
					LineUserStates[lineNumber] = state;
				}
			}
		}

		qint32 getUserState(qint32 currentLine, qint32 targetLine, QTextBlock* block = nullptr) {
			if (block && Mode == AStoryXDocument::WorkMode::SyntaxHighlighter) {
				qint32 delta = currentLine - targetLine;
				if (delta == 0) {
					return block->userState();
				}
				else if (delta > 0) {
					QTextBlock previousBlock = block->previous();
					while (previousBlock.isValid() && delta > 0) {
						if (previousBlock.userState() != 0) {
							return previousBlock.userState();
						}
						previousBlock = previousBlock.previous();
						--delta;
					}
				}
				else {
					QTextBlock nextBlock = block->next();
					while (nextBlock.isValid() && delta < 0) {
						if (nextBlock.userState() != 0) {
							return nextBlock.userState();
						}
						nextBlock = nextBlock.next();
						++delta;
					}
				}
			}
			else if (Mode == AStoryXDocument::WorkMode::Manual) {
				if (targetLine >= 0 && targetLine < LineUserStates.size()) {
					return LineUserStates[targetLine];
				}
				else {
					return 0;
				}
			}
			return 0;
		}

		void onGeneralParse(qint32 lineNumber, const QString& text, QTextBlock* block = nullptr) {
			if (text.startsWith("#useRule:")) {
				QString ruleName = text.mid(QString("#useRule:").length()).trimmed();
				if (AStoryXRule::hasRule(ruleName)) {
					auto rule = AStoryXRule::getRule(ruleName);
					if (rule) {
						//vgDebugF << rule->getName() << d->CurrentRule.getName();
						if (rule->getName() != CurrentRule.getName()) {
							CurrentRule = *rule;
							GlobalDiagnostics.removeAll(RuleNotSelectedDiagnostic);
							emit q->currentRuleChanged();
						}
					}
				}
				else {
					CurrentRule = AStoryXRule();
					AStoryXControllerParseData& data = mutableParseData(lineNumber);
					if (data.isValid()) {
						data.d->ControllerType = AStoryXController::ControllerType::Preprocessor;
						AStoryXParameter parameter;
						parameter.d->setParameter("type", "", "useRule", AStoryXValueMeta("preprocessor.type", AStoryXValueMeta::String),
							1);
						data.d->RequiredParameter = parameter;
						AStoryXParameter optionalParameter;
						parameter.d->setParameter("ruleName", "", ruleName, AStoryXValueMeta("useRule.ruleName", AStoryXValueMeta::String),
							QString("#useRule:").length());
						data.d->OptionalParameters.append(optionalParameter);
						AStoryXDiagnosticData diagnosticData = AStoryXDiagnosticData(
							VITRL("ASERStudio::diagnostic.noSuchRule.message"),
							lineNumber, QString("#useRule:").length(), AStoryXDiagnosticData::DiagnosticType::NoSuchRule,
							VITRL("ASERStudio::diagnostic.noSuchRule.fixAdvice")
						);
					}
				}
			}
			if (CurrentRule.isValid()) {
				AStoryXControllerParseData data = CurrentRule.parseAStoryX(text, -1, true, lineNumber);
				if (data.getControllerType() == AStoryXController::ControllerType::Preprocessor) {
					if (data.getRequiredParameter().getContent() == "aliases") {
						setUserState(lineNumber, 0x4241, block);
					}
					else if (data.getRequiredParameter().getContent() == "endaliases") {
						setUserState(lineNumber, 0x4242, block);
					}
					onParsed(data, lineNumber);
					//emit q->parseDataUpdated(lineNumber);
				}
				else {
					if (getUserState(lineNumber, lineNumber - 1, block) == 0x4241) {
						setUserState(lineNumber, 0x4241, block);
						// here can do sth to parse alias context.
						// sth like data = d->CurrentRule.parseAliasContext(text, lineNumber);
						data = AStoryXRule::parseAliases(text, -1, true, lineNumber);
						onParsed(data, lineNumber);
						//emit q->parseDataUpdated(lineNumber);
					}
					else {
						setUserState(lineNumber, 0, block);
						onParsed(data, lineNumber);
						//emit q->parseDataUpdated(lineNumber);
					}
				}
			}
			else {
				if (not GlobalDiagnostics.contains(RuleNotSelectedDiagnostic)) {
					GlobalDiagnostics.append(RuleNotSelectedDiagnostic);
					//emit q->parseDataUpdated(-1);
				}
			}
		}
	};

	/*!
		\class ASERStudio::AStorySyntax::AStoryXDocument
		\brief AStoryXDocument是对整个AStoryX文档的封装，包含了对文档内容的解析和诊断信息的存储.
		\since ASERStudio 2.0
		\inmodule ASERStudio

		AStoryXDocument是对整个AStoryX文档的封装，包含了对文档内容的解析和诊断信息的存储。

		你可以在你的SyntaxHighlighter中调用AStoryXDocument的onSyntaxHighlighter方法来解析每一行的内容，并获取相应的诊断信息

		请注意，语法高亮模式下这个类需要宿主编辑器：构造时把YSSCore::Editor::TextEdit对象传进来，
		它会把每一行的解析数据挂在对应的文本块上，因此行被插入、删除或合并时数据会随文本块自动调整，
		调用方不需要关心行号的搬运。不传宿主编辑器时，这个类只能在手动模式下使用。

		请注意，即使设置了QSyntaxHighlighter，AStoryXDocument也不会自动解析文档内容。它只用于在必须情况下
		触发你设置的QSyntaxHighlighter的QSyntaxHighlighter::rehighlight方法刷新全文档信息。
		如果你需要完全动态更新解析结果，就需要在你的SyntaxHighlighter的highlightBlock方法
		中调用AStoryXDocument的onSyntaxHighlighter方法来触发解析过程。

		值得一提的是，大部分与QTextDocument相关的类的生命周期都被设计为受到QTextDocument控制，
		例如QTextBlock、QSyntaxHighlighter等。但此类没有遵循这个设计原则，它的生命周期完全由用户控制，
		以便用户在QTextDocument的生命周期之外也能使用它来存储和管理解析结果和诊断信息。
		需要注意的是，语法高亮模式下的行数据是挂在宿主编辑器的文档上的，因此宿主编辑器销毁后
		这些数据也会随之消失，此时只有手动模式下的数据仍然可用。

		但如果你确定你的AStoryXDocument的生命周期完全可以由QTextDocument控制，那么你只需要在
		你的SyntaxHighlighter析构时顺手析构此类即可。

		此外，这个类没有专为复用设计，因此一般也建议在QTextDocument销毁时将其销毁，而且也不要尝试
		重新设置QTextDocument和QSyntaxHighlighter来复用此类，因为这可能会导致一些意想不到的问题，例如诊断信息无法正确更新等。
	*/

	/*!
		\class ASERStudio::AStorySyntax::AStoryXLineData
		\brief 挂在AStoryX文档某一行上的解析数据。
		\since ASERStudio 2.2
		\inmodule ASERStudio

		AStoryXDocument在语法高亮模式下会把每一行解析出的AStoryXControllerParseData存放在这里，
		并把它挂在对应的文本块上。这样当行被插入、删除或合并时，数据会随文本块一起调整，
		不需要额外的搬运逻辑。
	*/

	/*!
		\since ASERStudio 2.0
		\a edit 宿主编辑器。语法高亮模式下必须传入，手动模式下可以不传。

		构造函数。
	*/
	AStoryXDocument::AStoryXDocument(YSSCore::Editor::TextEdit* edit) :d(new AStoryXDocumentPrivate()) {
		d->q = this;
		d->HostEdit = edit;
		d->RuleNotSelectedDiagnostic = AStoryXDiagnosticData(
			VITRL("ASERStudio::diagnostic.ruleNotSelected.message"),
			-1, 0, AStoryXDiagnosticData::DiagnosticType::RuleNotSelected,
			VITRL("ASERStudio::diagnostic.ruleNotSelected.fixAdvice")
		);
	}

	/*!
		\since ASERStudio 2.0
		析构函数。
	*/
	AStoryXDocument::~AStoryXDocument() {
		delete d;
	}

	/*!
		\since ASERStudio 2.0
		在SyntaxHighlighter中调用此方法来解析每一行的内容，并获取相应的诊断信息。
		对于\a currentBlock ，可以用QSyntaxHighlighter::currentBlock()获取。
		\a text 则为QSyntaxHighlighter::highlightBlock方法的参数，直接传递即可。

		调用这个函数时，会自动将模式切换到SyntaxHighlighter模式，并清除之前在Manual模式下存储的行内容和用户状态。
	*/
	void AStoryXDocument::onSyntaxHighlighter(QTextBlock currentBlock, const QString& text) {
		if (d->Mode != AStoryXDocument::WorkMode::SyntaxHighlighter) {
			d->Mode = AStoryXDocument::WorkMode::SyntaxHighlighter;
			d->Lines.clear();
			d->LineUserStates.clear();
			d->GlobalDiagnostics.clear();
			d->ParseDataCache.clear();
			d->clearLineData();
		}
		d->onGeneralParse(currentBlock.blockNumber(), text, &currentBlock);
	}

	/*!
		\since ASERStudio 2.2
		在Manual模式下调用此方法来解析每一行的内容，并获取相应的诊断信息。
		\a lineIndex 为行号，从0开始；\a text 为该行的文本内容；\a cursorPosition 为光标在该行中的位置，如果不需要可以传递-1。

		调用这个函数时，会自动将模式切换到Manual模式。
	*/
	void AStoryXDocument::onManualParse(qint32 lineIndex, const QString& text, qint32 cursorPosition) {
		if (d->Mode != AStoryXDocument::WorkMode::Manual) {
			d->Mode = AStoryXDocument::WorkMode::Manual;
			d->LineUserStates.clear();
			d->GlobalDiagnostics.clear();
			d->ParseDataCache.clear();
		}
		if (lineIndex >= d->Lines.size()) {
			for (int i = d->Lines.size(); i <= lineIndex; ++i) {
				d->Lines.append("");
			}
			d->Lines[lineIndex] = text;
		}
		else {
			d->Lines[lineIndex] = text;
		}
		d->onGeneralParse(lineIndex, text);
	}

	/*!
		\since ASERStudio 2.2
		当行被添加时调用此方法，\a startLine 为添加的起始行号，\a count 为添加的行数。

		这个方法只在手动模式下需要调用：该模式下行内容和解析数据都由本类自己保存，需要在这里同步。
		语法高亮模式下解析数据挂在文本块上，会随文档自动调整，因此调用它不会有任何效果。
	*/
	void AStoryXDocument::onLinesAdded(qint32 startLine, qint32 count) {
		if (d->Mode != AStoryXDocument::WorkMode::Manual) {
			return;
		}
		if (startLine >= d->ParseDataCache.size()) {
			return;
		}
		for (qint32 i = 0; i < count; ++i) {
			d->Lines.insert(startLine, "");
			d->LineUserStates.insert(startLine, 0);
			d->ParseDataCache.insert(startLine, AStoryXControllerParseData());
		}
	}
	/*!
		\since ASERStudio 2.2
		当行被删除时调用此方法，\a startLine 为删除的起始行号，\a count 为删除的行数。

		这个方法只在手动模式下需要调用：该模式下行内容和解析数据都由本类自己保存，需要在这里同步。
		语法高亮模式下解析数据挂在文本块上，会随文档自动删除，因此调用它不会有任何效果。
	*/
	void AStoryXDocument::onLinesRemoved(qint32 startLine, qint32 count) {
		if (d->Mode != AStoryXDocument::WorkMode::Manual) {
			return;
		}
		for (qint32 i = 0; i < count; ++i) {
			if (startLine < d->Lines.size()) {
				d->Lines.removeAt(startLine);
			}
			if (startLine < d->LineUserStates.size()) {
				d->LineUserStates.removeAt(startLine);
			}
			if (startLine >= d->ParseDataCache.size()) {
				break;
			}
			d->ParseDataCache.remove(startLine);
		}
	}

	/*!
		\since ASERStudio 2.2
		获取文档的所有行，返回一个QStringList，每个元素对应一行的文本内容。

		这个函数只在手动模式下有效。如果是SyntaxHighlighter模式，则返回一个空的QStringList，
		因为在这种模式下，行内容是通过QTextDocument动态获取的，而不是预先存储在AStoryXDocument中的。

		要在SyntaxHighlighter下访问相关信息，你应该直接使用驱动了该AStoryXDocument的QTextDocument来获取行内容。
	*/
	QStringList AStoryXDocument::getLines() const {
		if (d->Mode == AStoryXDocument::WorkMode::Manual) {
			return d->Lines;
		}
		else {
			return QStringList();
		}
	}

	/*!
		\since ASERStudio 2.0
		获取指定行的诊断信息，返回一个AStoryXDiagnosticData的列表。
		如果\a lineNumber 为-1，则返回全局诊断信息；否则返回指定行的诊断信息。

		要获取全局诊断信息，也可以用getGlobalDiagnostics方法。

		全局诊断指的是某些不特定于具体某一行的信息，例如当前没有选定任何规则等。
		这些信息通常会影响整个文档的解析和行为，因此被归类为全局诊断。

		它不保证非空。如果没有任何诊断信息，则返回一个空的列表。
	*/
	QList<AStoryXDiagnosticData> AStoryXDocument::getDiagnostics(qint32 lineNumber) const {
		QList<AStoryXDiagnosticData> diagnostics;
		if (lineNumber == -1) {
			diagnostics.append(d->GlobalDiagnostics);
		}
		else {
			diagnostics.append(getParseData(lineNumber).getDiagnostics());
		}
		return diagnostics;
	}

	/*!
		\since ASERStudio 2.0
		获取文档的所有诊断信息，返回一个AStoryXDiagnosticData的列表，包括全局诊断和每行的诊断。

		它不保证非空。如果没有任何诊断信息，则返回一个空的列表。
	*/
	QList<AStoryXDiagnosticData> AStoryXDocument::getAllDiagnostics() const {
		QList<AStoryXDiagnosticData> diagnostics;
		diagnostics.append(d->GlobalDiagnostics);
		for (auto data : getAllParseData()) {
			diagnostics.append(data.getDiagnostics());
		}
		return diagnostics;
	}

	/*!
		\since ASERStudio 2.0
		获取全局诊断信息，返回一个AStoryXDiagnosticData的列表。
		全局诊断指的是某些不特定于具体某一行的信息，例如当前没有选定任何规则等。
		这些信息通常会影响整个文档的解析和行为，因此被归类为全局诊断。

		它不保证非空。如果没有任何全局诊断信息，则返回一个空的列表。
	*/
	QList<AStoryXDiagnosticData> AStoryXDocument::getGlobalDiagnostics() const {
		return d->GlobalDiagnostics;
	}

	/*!
		\since ASERStudio 2.0
		获取指定行的解析数据，返回一个AStoryXControllerParseData对象。
		如果\a lineNumber 对应的行没有解析数据，则返回一个无效的AStoryXControllerParseData对象。
	*/
	AStoryXControllerParseData AStoryXDocument::getParseData(qint32 lineNumber) const {
		if (lineNumber < 0) {
			return AStoryXControllerParseData();
		}
		if (d->Mode == AStoryXDocument::WorkMode::SyntaxHighlighter) {
			AStoryXLineData* lineData = d->findLineData(lineNumber);
			return lineData != nullptr ? lineData->ParseData : AStoryXControllerParseData();
		}
		if (lineNumber < d->ParseDataCache.size()) {
			return d->ParseDataCache[lineNumber];
		}
		return AStoryXControllerParseData();
	}

	/*!
		\since ASERStudio 2.0
		获取文档的所有解析数据，返回一个AStoryXControllerParseData的列表，每个元素对应一行的解析数据。
		如果某行没有解析数据，则对应的元素将是一个无效的AStoryXControllerParseData对象。
	*/
	QList<AStoryXControllerParseData> AStoryXDocument::getAllParseData() const {
		if (d->Mode == AStoryXDocument::WorkMode::Manual || d->HostEdit == nullptr) {
			return d->ParseDataCache;
		}
		QList<AStoryXControllerParseData> result;
		const QMap<qint32, AStoryXLineData*> allData = d->HostEdit->getAllBlockData<AStoryXLineData>();
		qint32 blockCount = d->HostEdit->getDocument()->blockCount();
		for (qint32 i = 0; i < blockCount; ++i) {
			auto it = allData.constFind(i);
			if (it == allData.constEnd()) {
				result.append(AStoryXControllerParseData());
			}
			else {
				result.append(it.value()->ParseData);
			}
		}
		return result;
	}

	/*!
		\since ASERStudio 2.0
		设置是否启用诊断功能，\a enable 为true表示启用，false表示禁用。

		在不启用诊断时，所有解析过程都不会产生任何诊断信息，有关诊断信息的函数
		将永远返回空的结果，例如getDiagnostics、getAllDiagnostics、getGlobalDiagnostics等。
	*/
	void AStoryXDocument::setEnableDiagnostic(bool enable) {
		d->EnableDiagnostic = enable;
	}

	/*!
		\since ASERStudio 2.0
		获取当前选定的规则，返回一个指向AStoryXRule对象的指针。

		请注意，请把这个指针视为弱指针，即用即弃。不要在外部
		持久化存储它，因为它随时可能因为选定的规则变更而失效。
	*/
	AStoryXRule* AStoryXDocument::getCurrentRule() const {
		return d->CurrentRule.isValid() ? &d->CurrentRule : nullptr;
	}

	/*!
		\since ASERStudio 2.0
		return 是否启用诊断功能，返回一个布尔值，true表示启用，false表示禁用。
	*/
	bool AStoryXDocument::isDiagnosticEnabled() const {
		return d->EnableDiagnostic;
	}
}