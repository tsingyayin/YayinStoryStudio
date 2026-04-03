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
	*/
	AStoryXDocument::AStoryXDocument() :d(new AStoryXDocumentPrivate()) {
		d->RuleNotSelectedDiagnostic = AStoryXDiagnosticData(
			VITR("ASERStudio::diagnostic.ruleNotSelected.message"),
			-1, 0, AStoryXDiagnosticData::DiagnosticType::RuleNotSelected,
			VITR("ASERStudio::diagnostic.ruleNotSelected.fixAdvice")
		);
	}

	void AStoryXDocument::onSyntaxHighlighter(QTextBlock currentBlock, const QString& text) {
		qint32 lineNumber = currentBlock.blockNumber();
		if (text.startsWith("#useRule:")) {
			QString ruleName = text.mid(QString("#useRule:").length()).trimmed();
			if (AStoryXRule::hasRule(ruleName)) {
				auto rule = AStoryXRule::getRule(ruleName);
				if (rule) {
					//vgDebugF << rule->getName() << d->CurrentRule.getName();
					if (rule->getName() != d->CurrentRule.getName()) {
						d->CurrentRule = *rule;
						d->GlobalDiagnostics.removeAll(d->RuleNotSelectedDiagnostic);
						QTimer::singleShot(1, this, [this](){
							this->refreshParseData();
							});
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

	AStoryXDocument::~AStoryXDocument() {
		delete d;
		d = nullptr;
	}

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

	QList<AStoryXDiagnosticData> AStoryXDocument::getAllDiagnostics() const {
		QList<AStoryXDiagnosticData> diagnostics;
		diagnostics.append(d->GlobalDiagnostics);
		for (auto data : d->ParseDataCache) {
			diagnostics.append(data.d->Diagnostics);
		}
		return diagnostics;
	}

	QList<AStoryXDiagnosticData> AStoryXDocument::getGlobalDiagnostics() const {
		return d->GlobalDiagnostics;
	}

	AStoryXControllerParseData AStoryXDocument::getParseData(qint32 lineNumber) const {
		if (d->ParseDataCache.contains(lineNumber)) {
			return d->ParseDataCache[lineNumber];
		}
		else {
			return AStoryXControllerParseData();
		}
	}

	QList<AStoryXControllerParseData> AStoryXDocument::getAllParseData() const {
		return d->ParseDataCache.values();
	}

	void AStoryXDocument::setTextDocument(QTextDocument* doc) {
		if (doc) {
			d->TextDocument = doc;
		}
	}

	QTextDocument* AStoryXDocument::getTextDocument() const {
		return d->TextDocument;
	}

	void AStoryXDocument::setEnableDiagnostic(bool enable) {
		d->EnableDiagnostic = enable;
	}

	void AStoryXDocument::setSyntaxHighlighter(QSyntaxHighlighter* highlighter) {
		d->Highlighter = highlighter;
	}

	void AStoryXDocument::refreshParseData() {
		if (d->Highlighter) {
			d->Highlighter->rehighlight();
		}
		emit parseDataAllUpdated();
	}

	bool AStoryXDocument::isDiagnosticEnabled() const {
		return d->EnableDiagnostic;
	}


}