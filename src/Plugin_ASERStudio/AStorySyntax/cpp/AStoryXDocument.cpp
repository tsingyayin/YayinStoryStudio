#include "AStorySyntax/AStoryXDocument.h"
#include "AStorySyntax/private/AStoryXDocument_p.h"
#include "AStorySyntax/AStoryXControllerParseData.h"
#include "AStorySyntax/private/AStoryXControllerParseData_p.h"
#include <QtCore/qtimer.h>
#include <QtGui/qtextdocument.h>
#include <General/TranslationHost.h>
#include <General/Log.h>

namespace ASERStudio::AStorySyntax {
	AStoryXDocumentNotifier::AStoryXDocumentNotifier(QObject* parent) :QSyntaxHighlighter(parent) {

	}
	AStoryXDocumentNotifier::~AStoryXDocumentNotifier() {

	}
	void AStoryXDocumentNotifier::highlightBlock(const QString& text) {
		emit blockChanged(text);
	}
	QTextBlock AStoryXDocumentNotifier::getCurrentBlock() const {
		return currentBlock();
	}

	class AStoryXDocumentPrivate {
		friend class AStoryXDocument;
	protected:
		QTextDocument* TextDocument = nullptr;
		QSyntaxHighlighter* Highlighter = nullptr;
		QString FilePath;
		AStoryXRule CurrentRule;
		bool EnableDiagnostic = false;
		QMap<qint32, AStoryXControllerParseData> ParseDataCache;
		QList<AStoryXDiagnosticData> GlobalDiagnostics;
		AStoryXDiagnosticData RuleNotSelectedDiagnostic;
		void onParsed(const AStoryXControllerParseData& data, qint32 lineNumber) {
			ParseDataCache[lineNumber] = data;
			//vgDebug << "Parsed line " << lineNumber << ": " << data;
		}
	};

	/*!
		\class ASERStudio::AStorySyntax::AStoryXDocument
		\brief AStoryXDocument是对整个AStoryX文档的封装，包含了对文档内容的解析和诊断信息的存储。
		\since ASERStudio 2.0
		\inmodule ASERStudio

		AStoryXDocument是对整个AStoryX文档的封装，包含了对文档内容的解析和诊断信息的存储。
		
		你可以在你的SyntaxHighlighter中调用AStoryXDocument的onSyntaxHighlighter方法来解析每一行的内容，并获取相应的诊断信息

		请注意，这个类获取信息依赖于通过setTextDocument方法设置的QTextDocument，
		以及通过setSyntaxHighlighter方法设置的QSyntaxHighlighter。确保在使用这些功能之前正确设置这些组件。
		这个类通过QTextDocument获取静态信息，通过QSyntaxHighlighter感知文档的动态变化，
		从而实时刷新解析结果。

		请注意，即使设置了QSyntaxHighlighter，AStoryXDocument也不会自动解析文档内容。它只用于在必须情况下
		触发你设置的QSyntaxHighlighter的QSyntaxHighlighter::rehighlight方法刷新全文档信息。
		如果你需要完全动态更新解析结果，就需要在你的SyntaxHighlighter的highlightBlock方法
		中调用AStoryXDocument的onSyntaxHighlighter方法来触发解析过程。

		值得一提的是，大部分与QTextDocument相关的类的生命周期都被设计为受到QTextDocument控制，
		例如QTextBlock、QSyntaxHighlighter等。但此类没有遵循这个设计原则，它的生命周期完全由用户控制，
		以便用户在QTextDocument的生命周期之外也能使用它来存储和管理解析结果和诊断信息。

		但如果你确定你的AStoryXDocument的生命周期完全可以由QTextDocument控制，那么你只需要在
		你的SyntaxHighlighter析构时顺手析构此类即可。

		此外，这个类没有专为复用设计，因此一般也建议在QTextDocument销毁时将其销毁，而且也不要尝试
		重新设置QTextDocument和QSyntaxHighlighter来复用此类，因为这可能会导致一些意想不到的问题，例如诊断信息无法正确更新等。
	*/

	/*!
		\since ASERStudio 2.0
		构造函数。
	*/
	AStoryXDocument::AStoryXDocument() :d(new AStoryXDocumentPrivate()) {
		d->RuleNotSelectedDiagnostic = AStoryXDiagnosticData(
			VITR("ASERStudio::diagnostic.ruleNotSelected.message"),
			-1, 0, AStoryXDiagnosticData::DiagnosticType::RuleNotSelected,
			VITR("ASERStudio::diagnostic.ruleNotSelected.fixAdvice")
		);
	}

	/*
		\since ASERStudio 2.0
		在SyntaxHighlighter中调用此方法来解析每一行的内容，并获取相应的诊断信息。
		对于\a currentBlock，可以用QSyntaxHighlighter::currentBlock()获取。
		\a text则为QSyntaxHighlighter::highlightBlock方法的参数，直接传递即可。
	*/
	void AStoryXDocument::onSyntaxHighlighter(QTextBlock currentBlock, const QString& text) {
		qint32 lineNumber = currentBlock.blockNumber();
		if (lineNumber == 0) {
			auto firstLine = AStoryXControllerParseData();
			firstLine.d->ControllerType = AStoryXController::ControllerType::Comment;
			AStoryXParameter param;
			param.d->setParameter("comment", "", text, AStoryXValueMeta("comment", AStoryXValueMeta::Type::Comment), 2);
			firstLine.d->RequiredParameter = param;
			d->onParsed(firstLine, lineNumber);
			return;
		}
		if (text.startsWith("#useRule:")) {
			QString ruleName = text.mid(QString("#useRule:").length()).trimmed();
			if (AStoryXRule::hasRule(ruleName)) {
				auto rule = AStoryXRule::getRule(ruleName);
				if (rule) {
					//vgDebugF << rule->getName() << d->CurrentRule.getName();
					if (rule->getName() != d->CurrentRule.getName()) {
						d->CurrentRule = *rule;
						d->GlobalDiagnostics.removeAll(d->RuleNotSelectedDiagnostic);
						emit currentRuleChanged();
					}
				}
			}
			else {
				d->CurrentRule = AStoryXRule();
				AStoryXControllerParseData& data = d->ParseDataCache[lineNumber];
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
						VITR("ASERStudio::diagnostic.noSuchRule.message"),
						lineNumber, QString("#useRule:").length(), AStoryXDiagnosticData::DiagnosticType::NoSuchRule,
						VITR("ASERStudio::diagnostic.noSuchRule.fixAdvice")
					);
				}
			}
		}
		if (d->CurrentRule.isValid()) {
			AStoryXControllerParseData data = d->CurrentRule.parseAStoryX(text, -1, true, lineNumber);
			if (data.getControllerType() == AStoryXController::ControllerType::Preprocessor) {
				if (data.getRequiredParameter().getContent() == "aliases") {
					currentBlock.setUserState(0x4241);
				}
				else if (data.getRequiredParameter().getContent() == "endaliases") {
					currentBlock.setUserState(0x4242);
				}
				d->onParsed(data, lineNumber);
				emit parseDataUpdated(lineNumber);
			}
			else {
				if (currentBlock.previous().isValid() && currentBlock.previous().userState() == 0x4241) {
					currentBlock.setUserState(0x4241);
					// here can do sth to parse alias context.
					// sth like data = d->CurrentRule.parseAliasContext(text, lineNumber);
					data = AStoryXControllerParseData(); // just for test now.
					data.d->ControllerType = AStoryXController::ControllerType::Comment;
					// just for test now.
					d->onParsed(data, lineNumber);
					emit parseDataUpdated(lineNumber);
				}
				else {
					currentBlock.setUserState(0);
					d->onParsed(data, lineNumber);
					emit parseDataUpdated(lineNumber);
				}
			}
		}
		else {
			if (not d->GlobalDiagnostics.contains(d->RuleNotSelectedDiagnostic)) {
				d->GlobalDiagnostics.append(d->RuleNotSelectedDiagnostic);
				emit parseDataUpdated(-1);
			}
		}

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
		获取文档的所有行内容，返回一个QStringList，每个元素对应一行文本。
		如果没有设置TextDocument，则返回一个空的QStringList。
	*/
	QStringList AStoryXDocument::getLines() const {
		if (d->TextDocument) {
			QStringList lines;
			for (int i = 0; i < d->TextDocument->blockCount(); i++) {
				lines.append(d->TextDocument->findBlockByNumber(i).text());
			}
			return lines;
		}
		else {
			return QStringList();
		}
	}

	/*!
		\since ASERStudio 2.0
		获取指定行的诊断信息，返回一个AStoryXDiagnosticData的列表。
		如果\a lineNumber为-1，则返回全局诊断信息；否则返回指定行的诊断信息。

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
			if (d->ParseDataCache.contains(lineNumber)) {
				diagnostics.append(d->ParseDataCache[lineNumber].d->Diagnostics);
			}
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
		for (auto data : d->ParseDataCache) {
			diagnostics.append(data.d->Diagnostics);
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
		如果\a lineNumber对应的行没有解析数据，则返回一个无效的AStoryXControllerParseData对象。
	*/
	AStoryXControllerParseData AStoryXDocument::getParseData(qint32 lineNumber) const {
		if (d->ParseDataCache.contains(lineNumber)) {
			return d->ParseDataCache[lineNumber];
		}
		else {
			return AStoryXControllerParseData();
		}
	}

	/*!
		\since ASERStudio 2.0
		获取文档的所有解析数据，返回一个AStoryXControllerParseData的列表，每个元素对应一行的解析数据。
		如果某行没有解析数据，则对应的元素将是一个无效的AStoryXControllerParseData对象。
	*/
	QList<AStoryXControllerParseData> AStoryXDocument::getAllParseData() const {
		return d->ParseDataCache.values();
	}
	
	/*!
		\since ASERStudio 2.0
		设置文档的文本内容，\a doc为一个QTextDocument对象。
	*/
	void AStoryXDocument::setTextDocument(QTextDocument* doc) {
		if (doc) {
			d->TextDocument = doc;
		}
	}

	/*!
		\since ASERStudio 2.0
		获取文档的文本内容，返回一个QTextDocument对象。
		如果没有设置TextDocument，则返回nullptr。
	*/
	QTextDocument* AStoryXDocument::getTextDocument() const {
		return d->TextDocument;
	}
	
	/*!
		\since ASERStudio 2.0
		设置是否启用诊断功能，\a enable为true表示启用，false表示禁用。

		在不启用诊断时，所有解析过程都不会产生任何诊断信息，有关诊断信息的函数
		将永远返回空的结果，例如getDiagnostics、getAllDiagnostics、getGlobalDiagnostics等。
	*/
	void AStoryXDocument::setEnableDiagnostic(bool enable) {
		d->EnableDiagnostic = enable;
	}

	/*!
		\since ASERStudio 2.0
		设置用于解析文档的语法高亮器，\a highlighter为一个QSyntaxHighlighter对象。
		这个高亮器将被用来在解析过程中感知文档的动态变化，并触发相应的解析更新。
	*/
	void AStoryXDocument::setSyntaxHighlighter(QSyntaxHighlighter* highlighter) {
		d->Highlighter = highlighter;
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
		判断是否启用诊断功能，返回一个布尔值，true表示启用，false表示禁用。
	*/
	bool AStoryXDocument::isDiagnosticEnabled() const {
		return d->EnableDiagnostic;
	}


}