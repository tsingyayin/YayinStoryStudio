#include "YSS/LS_AStoryXSyntaxHighlighter.h"
#include "AStorySyntax/AStoryXDocument.h"
#include "YSS/LangServer_AStoryX.h"
#include "AStorySyntax/AStoryXControllerParseData.h"
#include <General/Log.h>
#include <Widgets/ThemeManager.h>
#include <Utility/ColorTool.h>
#include "AStorySyntax/AStoryXValueMeta.h"

namespace ASERStudio::YSS {
	class LS_AStoryXSyntaxHighlighterPrivate {
		friend class LS_AStoryXSyntaxHighlighter;
	protected:
		QString FilePath;
		AStorySyntax::AStoryXDocument* Document = nullptr;
	};
	LS_AStoryXSyntaxHighlighter::LS_AStoryXSyntaxHighlighter(YSSCore::Editor::TextEdit* parent) : SyntaxHighlighter(parent) {
		d = new LS_AStoryXSyntaxHighlighterPrivate();
		d->FilePath = parent->getFilePath();
		ASERStudio::AStorySyntax::AStoryXDocument* doc = new ASERStudio::AStorySyntax::AStoryXDocument();
		doc->setEnableDiagnostic(true);
		doc->setSyntaxHighlighter(this);
		doc->setTextDocument(parent->getDocument());
		AStoryXLanguageServer::setAStoryXDocument(parent->getFilePath(), doc);
		d->Document = AStoryXLanguageServer::getAStoryXDocument(d->FilePath);
		connect(d->Document, &AStorySyntax::AStoryXDocument::currentRuleChanged, this,
			&LS_AStoryXSyntaxHighlighter::rehighlight_s);
	}
	LS_AStoryXSyntaxHighlighter::~LS_AStoryXSyntaxHighlighter() {
		AStoryXLanguageServer::deleteAStoryXDocument(d->FilePath);
		delete d;
	}
	void LS_AStoryXSyntaxHighlighter::onBlockChanged(const QString& text, int blockNumber) {
		if (!d->Document) return;
		d->Document->onSyntaxHighlighter(currentBlock(), text);
		if (text.isEmpty()) return;
		setFormatWithColorKey(0, text.length(), "PlainText");
		auto parseData = d->Document->getParseData(blockNumber);
		if (parseData.getControllerType() == ASERStudio::AStorySyntax::AStoryXController::ControllerType::Comment) {
			setFormatWithColorKey(0, text.length(), "Comment");
			return;
		}
		//vgDebug << "Highlighting line " << blockNumber << " with controller type " << parseData.getControllerType();
		setFormatWithColorKey(0, parseData.getStartSign().length(), "Keyword");
		auto required = parseData.getRequiredParameter();
		QString requiredType = "PlainText";
		switch (required.getValue().getType()) {
		case ASERStudio::AStorySyntax::AStoryXValueMeta::String:
			requiredType = "String";
			break;
		case ASERStudio::AStorySyntax::AStoryXValueMeta::Number:
		case ASERStudio::AStorySyntax::AStoryXValueMeta::Integer:
		case ASERStudio::AStorySyntax::AStoryXValueMeta::Float:
		case ASERStudio::AStorySyntax::AStoryXValueMeta::Bool:
			requiredType = "Number";
			break;
		case ASERStudio::AStorySyntax::AStoryXValueMeta::Enum:
			requiredType = "Enum";
			break;
		case ASERStudio::AStorySyntax::AStoryXValueMeta::Vector:
			requiredType = "Class";
			break;
		case ASERStudio::AStorySyntax::AStoryXValueMeta::None:
			requiredType = "PlainText";
			break;
		case ASERStudio::AStorySyntax::AStoryXValueMeta::Function:
			requiredType = "Function";
			break;
		case ASERStudio::AStorySyntax::AStoryXValueMeta::Parameter:
			requiredType = "Parameter";
			break;
		case ASERStudio::AStorySyntax::AStoryXValueMeta::Keyword:
			requiredType = "Keyword";
			break;
		case ASERStudio::AStorySyntax::AStoryXValueMeta::Macro:
			requiredType = "Macro";
			break;
		}
		setFormatWithColorKey(required.getIndex(), required.getContent().size(), requiredType);

		for (auto optional : parseData.getOptionalParameters()) {
			setFormatWithColorKey(optional.getIndex(), optional.getPrefix().size(), "PlainText");
			ASERStudio::AStorySyntax::AStoryXValueMeta value = optional.getValue();
			QString type = "PlainText";
			switch (value.getType()) {
			case ASERStudio::AStorySyntax::AStoryXValueMeta::String:
				type = "String";
				break;
			case ASERStudio::AStorySyntax::AStoryXValueMeta::Number:
			case ASERStudio::AStorySyntax::AStoryXValueMeta::Integer:
			case ASERStudio::AStorySyntax::AStoryXValueMeta::Float:
			case ASERStudio::AStorySyntax::AStoryXValueMeta::Bool:
				type = "Number";
				break;
			case ASERStudio::AStorySyntax::AStoryXValueMeta::Enum:
				type = "Enum";
				break;
			case ASERStudio::AStorySyntax::AStoryXValueMeta::Vector:
				type = "Class";
				break;
			case ASERStudio::AStorySyntax::AStoryXValueMeta::None:
				type = "PlainText";
				break;
			case ASERStudio::AStorySyntax::AStoryXValueMeta::Function:
				type = "Function";
				break;
			case ASERStudio::AStorySyntax::AStoryXValueMeta::Parameter:
				type = "Parameter";
				break;
			case ASERStudio::AStorySyntax::AStoryXValueMeta::Keyword:
				type = "Keyword";
				break;
			case ASERStudio::AStorySyntax::AStoryXValueMeta::Macro:
				type = "Macro";
				break;
			}
			setFormatWithColorKey(optional.getIndex() + optional.getPrefix().size(), optional.getContent().size(), type);
		}

		static QRegularExpression protectTMP("&\\{[\\d\\D]*?\\}");
		QRegularExpressionMatchIterator protectTMPIt = protectTMP.globalMatch(text);
		while (protectTMPIt.hasNext()) {
			QRegularExpressionMatch match = protectTMPIt.next();
			setFormatWithColorKey(match.capturedStart(), match.capturedLength(), "SpecialString");
		}

		static QRegularExpression variableTMP("\\$\\([\\d\\D]*?\\)");
		QRegularExpressionMatchIterator variableTMPIt = variableTMP.globalMatch(text);
		while (variableTMPIt.hasNext()) {
			QRegularExpressionMatch match = variableTMPIt.next();
			setFormatWithColorKey(match.capturedStart(), match.capturedLength(), "Parameter");
		}

		QList<ASERStudio::AStorySyntax::AStoryXDiagnosticData> diagnostics = d->Document->getDiagnostics(blockNumber);
		for (auto diagnostic : diagnostics) {
			//vgDebug << diagnostic;
			DiagnosticTransform(diagnostic);
		}
	}

	void LS_AStoryXSyntaxHighlighter::setFormatWithColorKey(int start, int count, const QString& colorKey_F, const QString& colorKey_B, const QString& colorKey_U) {
		QColor color_F = VISTM->getColor(colorKey_F);
		//vgDebug << Visindigo::Utility::ColorTool::toColorString(VISTM->getColor(colorKey_F));
		QTextCharFormat format;;
		format.setForeground(color_F);
		if (!colorKey_B.isEmpty()) {
			QColor color_B = VISTM->getColor(colorKey_B);
			format.setBackground(color_B);
		}
		if (!colorKey_U.isEmpty()) {
			QColor color_U = VISTM->getColor(colorKey_U);
			format.setUnderlineColor(color_U);
			format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
		}
		setFormat(start, count, format);
	}

	void LS_AStoryXSyntaxHighlighter::DiagnosticTransform(const ASERStudio::AStorySyntax::AStoryXDiagnosticData& diagnostic) {
		int column = diagnostic.getColumn();
		int length = 1;
		if (column < 0) {
			return;
		}
		if (diagnostic.getType() >= ASERStudio::AStorySyntax::AStoryXDiagnosticData::UnknownWarning) {
			createWarningMessage(diagnostic.getMessage(), column, length, 
				QString::number((qint32)diagnostic.getType(), 16).toUpper(),
				QUrl("http://prts.site/aserstudio-astoryx-diagnostic.html#" + QString::number((qint32)diagnostic.getType(), 16).toLower()),
				diagnostic.getFixAdvice()
				);
		}
		else if (diagnostic.getType() >= ASERStudio::AStorySyntax::AStoryXDiagnosticData::UnknownError) {
			createErrorMessage(diagnostic.getMessage(), column, length, 
				QString::number((qint32)diagnostic.getType(), 16).toUpper(),
				QUrl("http://prts.site/aserstudio-astoryx-diagnostic.html#" + QString::number((qint32)diagnostic.getType(), 16).toLower()),
				diagnostic.getFixAdvice()
				);
		}
		else {
			createInfoMessage(diagnostic.getMessage(), column, length, 
				QString::number((qint32)diagnostic.getType(), 16).toUpper(),
				QUrl("http://prts.site/aserstudio-astoryx-diagnostic.html#" + QString::number((qint32)diagnostic.getType(), 16).toLower()),
				diagnostic.getFixAdvice()
				);
		}
	}
}