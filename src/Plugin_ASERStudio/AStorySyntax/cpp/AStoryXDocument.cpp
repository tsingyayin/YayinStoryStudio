#include "AStorySyntax/AStoryXDocument.h"
#include "AStorySyntax/private/AStoryXDocument_p.h"
#include "AStorySyntax/AStoryXControllerParseData.h"
#include "AStorySyntax/private/AStoryXControllerParseData_p.h"

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
		QString FilePath;
		AStoryXRule CurrentRule;
		bool EnableDiagnostic = false;
		QMap<qint32, AStoryXControllerParseData> ParseDataCache;
		QList<AStoryXDiagnosticData> GlobalDiagnostics;
		AStoryXDocumentNotifier* Notifier = nullptr;
		AStoryXDiagnosticData RuleNotSelectedDiagnostic;
		void onParsed(const AStoryXControllerParseData& data, qint32 lineNumber) {
			ParseDataCache[lineNumber] = data;
		}
	};

	AStoryXDocument::AStoryXDocument() :d(new AStoryXDocumentPrivate()) {
		d->Notifier = new AStoryXDocumentNotifier(this);
		d->RuleNotSelectedDiagnostic = AStoryXDiagnosticData(
			VITR("ASERStudio::diagnostic.ruleNotSelected.message"),
			-1, 0, AStoryXDiagnosticData::DiagnosticType::RuleNotSelected,
			VITR("ASERStudio::diagnostic.ruleNotSelected.fixAdvice")
		);
		connect(d->Notifier, &AStoryXDocumentNotifier::blockChanged, this, [this](const QString& text) {
			qint32 lineNumber = d->Notifier->getCurrentBlock().blockNumber();
			vgDebug << "Parsing line" << lineNumber << ":" << text;
			if (text.startsWith("#useRule:")) {
				QString ruleName = text.mid(QString("#useRule:").length()).trimmed();
				if (AStoryXRule::hasRule(ruleName)) {
					auto rule = AStoryXRule::getRule(ruleName);
					if (rule) {
						d->CurrentRule = *rule;
					}
					d->GlobalDiagnostics.removeAll(d->RuleNotSelectedDiagnostic);
				}
				else {
					d->CurrentRule = AStoryXRule();
					AStoryXControllerParseData& data = d->ParseDataCache[lineNumber];
					if (data.isValid()) {
						data.d->ControllerType = AStoryXController::ControllerType::Preprocessor;
						data.d->RequiredParameter = "useRule";
						data.d->OptionalParameterNames.append("ruleName");
						data.d->OptionalParameters.append(ruleName);
						data.d->RequiredParameterStringIndex = 0;
						data.d->OptionalParameterStringIndex.append(QString("#useRule:").length());
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
				d->onParsed(data, lineNumber);
				emit parseDataUpdated(lineNumber);
			}
			else {
				if (not d->GlobalDiagnostics.contains(d->RuleNotSelectedDiagnostic)) {
					d->GlobalDiagnostics.append(d->RuleNotSelectedDiagnostic);
					emit parseDataUpdated(-1);
				}
			}
		});
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
			d->Notifier->setDocument(doc);
		}
	}

	QTextDocument* AStoryXDocument::getTextDocument() const {
		return d->TextDocument;
	}

	void AStoryXDocument::setEnableDiagnostic(bool enable) {
		d->EnableDiagnostic = enable;
	}

	bool AStoryXDocument::isDiagnosticEnabled() const {
		return d->EnableDiagnostic;
	}


}