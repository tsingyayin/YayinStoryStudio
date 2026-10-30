#include <QtCore/qmetaobject.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qset.h>
#include <QtCore/qstringlist.h>
#include <QtGui/qcolor.h>
#include <QtGui/qfont.h>
#include <Editor/ColorThemeProvider.h>
#include <Editor/VirtualFilePath.h>
#include <General/Log.h>
#include <Utility/ColorTool.h>
#include <Widgets/ThemeManager.h>
#include "AStorySyntax/AStoryXControllerParseData.h"
#include "AStorySyntax/AStoryXDocument.h"
#include "AStorySyntax/AStoryXRule.h"
#include "AStorySyntax/AStoryXValueMeta.h"
#include "YSS/InlayColorProvider.h"
#include "YSS/InlayPositionProvider.h"
#include "YSS/LangServer_AStoryX.h"
#include "YSS/LS_AStoryXSyntaxHighlighter.h"
namespace ASERStudio::YSS {
	static bool isControllerStyleKey(const QString& key) {
		static QSet<QString> controllerNames;
		if (controllerNames.isEmpty()) {
			QMetaEnum meta = QMetaEnum::fromType<ASERStudio::AStorySyntax::AStoryXController::ControllerType>();
			for (int i = 0; i < meta.keyCount(); ++i) {
				controllerNames.insert(QString::fromUtf8(meta.key(i)));
			}
		}
		return controllerNames.contains(key);
	}
	class LS_AStoryXSyntaxHighlighterPrivate {
		friend class LS_AStoryXSyntaxHighlighter;
	protected:
		QString FilePath;
		AStorySyntax::AStoryXDocument* Document = nullptr;
		QMap<QString, YSSCore::Editor::StyleData> CurrentTheme;
		QColor InlayHintColor;
	};
	LS_AStoryXSyntaxHighlighter::LS_AStoryXSyntaxHighlighter(YSSCore::Editor::TextEdit* parent) : SyntaxHighlighter(parent) {
		d = new LS_AStoryXSyntaxHighlighterPrivate();
		d->FilePath = parent->getFilePath();
		ASERStudio::AStorySyntax::AStoryXDocument* doc = new ASERStudio::AStorySyntax::AStoryXDocument(parent);
		doc->setEnableDiagnostic(true);
		AStoryXLanguageServer::setAStoryXDocument(parent->getFilePath(), doc);
		d->Document = AStoryXLanguageServer::getAStoryXDocument(d->FilePath);
		// 触发本次变化的那一趟格式化已经把该行之后的内容用新规则解析过了，
		// 因此只需要重做该行及其之前的部分，不必整篇再解析一遍。
		connect(d->Document, &AStorySyntax::AStoryXDocument::currentRuleChanged, this,
			[this](qint32 lineNumber) { this->rehighlightRange_s(0, lineNumber + 1); });
		// 规则是异步加载的，文档可能先于规则完成着色；规则就绪后整篇重来一遍。
		connect(d->Document, &AStorySyntax::AStoryXDocument::rulesReloaded, this,
			[this]() { this->rehighlightProgressive_s(); });
		connect(parent, &YSSCore::Editor::FileEditWidget::fileRenamed, this,
			[this](const YSSCore::General::YSSURI& oldUri, const YSSCore::General::YSSURI& newUri) {
				const QString oldPath = oldUri.getLocalPath();
				const QString newPath = newUri.getLocalPath();
				AStoryXLanguageServer::renameAStoryXDocument(oldPath, newPath);
				d->FilePath = newPath;
			});
	}
	LS_AStoryXSyntaxHighlighter::~LS_AStoryXSyntaxHighlighter() {
		AStoryXLanguageServer::deleteAStoryXDocument(d->FilePath, d->Document);
		delete d;
	}
	void LS_AStoryXSyntaxHighlighter::onBlockChanged(const QString& text, int blockNumber) {
		if (!d->Document) return;
		d->Document->onSyntaxHighlighter(currentBlock(), text);
		if (text.isEmpty()) return;
		setFormatWithColorKey(0, text.length(), "PlainText");
		const auto& parseData = d->Document->getParseData(blockNumber);
		if (parseData.getControllerType() == ASERStudio::AStorySyntax::AStoryXController::ControllerType::Comment) {
			setFormatWithColorKey(0, text.length(), "Comment");
			return;
		}
		//vgDebug << "Highlighting line " << blockNumber << " with controller type " << parseData.getControllerType();
		const QString controllerKey = ASERStudio::AStorySyntax::AStoryXController::controllerTypeToString(parseData.getControllerType());
		setFormatWithColorKey(0, parseData.getStartSign().length(), controllerKey);
		const auto& required = parseData.getRequiredParameter();
		if (parseData.getControllerType() != ASERStudio::AStorySyntax::AStoryXController::ControllerType::Dialog) {
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
				requiredType = "Vector";
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
			case ASERStudio::AStorySyntax::AStoryXValueMeta::MacroParameter:
				requiredType = "MacroParameter";
				break;
			}
			setFormatWithColorKey(required.getIndex(), required.getContent().size(), requiredType);
		}
		else {
			const auto& dialogRequired = parseData.getRequiredParameter();
			const auto& content = dialogRequired.getContent();
			qint32 tabIndex = content.indexOf("\t");
			if (d->CurrentTheme.contains("Dialog")) {
				setFormatWithColorKey(dialogRequired.getIndex(), tabIndex, "Dialog");
				setFormatWithColorKey(dialogRequired.getIndex() + tabIndex, content.size() - tabIndex, "String");
			}
			else {
				setFormatWithColorKey(dialogRequired.getIndex(), content.size(), "String");
			}
		}

		for (const auto& optional : parseData.getOptionalParameters()) {
			setFormatWithColorKey(optional.getIndex(), optional.getPrefix().size(), "PlainText");
			const ASERStudio::AStorySyntax::AStoryXValueMeta& value = optional.getValue();
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
				type = "Vector";
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
			case ASERStudio::AStorySyntax::AStoryXValueMeta::MacroParameter:
				type = "MacroParameter";
				break;
			}
			setFormatWithColorKey(optional.getIndex() + optional.getPrefix().size(), optional.getContent().size(), type);
		}

		
		ASERStudio::AStorySyntax::AStoryXRule* rule = d->Document->getCurrentRule();
		if (rule != nullptr and not rule->isMonotonicity(parseData.getControllerType())) {
			for (const auto& optional : parseData.getOptionalParameters()) {
				if (not optional.isValid()) {
					continue;
				}
				createInlayHint(optional.getIndex() + optional.getPrefix().size(),
					optional.getName(), d->InlayHintColor);
			}
		}

		createColorInlayHints(text, parseData);
		createPositionInlayHints(text, parseData);

		// 这三类占位符都以 & 或 $ 开头；行里没有它们时直接跳过三趟正则扫描。
		if (text.contains(QLatin1Char('&')) || text.contains(QLatin1Char('$'))) {
			static QRegularExpression protectTMP("&\\{[\\d\\D]*?\\}");
			QRegularExpressionMatchIterator protectTMPIt = protectTMP.globalMatch(text);
			while (protectTMPIt.hasNext()) {
				QRegularExpressionMatch match = protectTMPIt.next();
				setFormatWithColorKey(match.capturedStart(), match.capturedLength(), "SpecialString");
			}

			static QRegularExpression variableTMP("\\$\\[[\\d\\D]*?\\]");
			QRegularExpressionMatchIterator variableTMPIt = variableTMP.globalMatch(text);
			while (variableTMPIt.hasNext()) {
				QRegularExpressionMatch match = variableTMPIt.next();
				setFormatWithColorKey(match.capturedStart(), match.capturedLength(), "Parameter");
			}

			static QRegularExpression blockParaTMP("\\$\\([\\d\\D]*?\\)");
			QRegularExpressionMatchIterator blockParaTMPIt = blockParaTMP.globalMatch(text);
			while (blockParaTMPIt.hasNext()) {
				QRegularExpressionMatch match = blockParaTMPIt.next();
				setFormatWithColorKey(match.capturedStart(), match.capturedLength(), "MacroParameter");
			}
		}

		QList<ASERStudio::AStorySyntax::AStoryXDiagnosticData> diagnostics = d->Document->getDiagnostics(blockNumber);
		for (auto diagnostic : diagnostics) {
			//vgDebug << diagnostic;
			DiagnosticTransform(diagnostic);
		}
	}

	void LS_AStoryXSyntaxHighlighter::addColorInlayHint(qint32 position, qint32 start, qint32 length,
		const QString& value, double scale, bool hasAlpha, qint32 components) {
		if (position < 1 or start < 0 or length <= 0) {
			return;
		}
		Visindigo::Utility::JsonConfig para;
		para.setString(InlayColorKeys::Kind,
			value.startsWith(QLatin1Char('#')) ? InlayColorKeys::KindHex : InlayColorKeys::KindVector);
		para.setInt(InlayColorKeys::Start, start);
		para.setInt(InlayColorKeys::Length, length);
		para.setString(InlayColorKeys::Value, value);
		para.setDouble(InlayColorKeys::Scale, scale);
		para.setBool(InlayColorKeys::Alpha, hasAlpha);
		para.setInt(InlayColorKeys::Components, components);
		YSSCore::Editor::VirtualFilePath path(InlayColorProviderID, value, para);
		if (not path.isValid()) {
			return;
		}
		YSSCore::Editor::InlayHint hint(position, QString(), QColor(), 8);
		hint.setMulti(path.toString());
		createInlayHint(hint);
	}

	void LS_AStoryXSyntaxHighlighter::addVectorColorInlayHint(
		const ASERStudio::AStorySyntax::AStoryXParameter& param, qint32 contentOffset, bool acceptByControllerType) {
		if (not param.isValid()) {
			return;
		}
		if (not acceptByControllerType
			and param.getName().compare(QStringLiteral("Color"), Qt::CaseInsensitive) != 0) {
			return;
		}
		if (param.getValue().getType() != ASERStudio::AStorySyntax::AStoryXValueMeta::Vector) {
			return;
		}
		const QString content = param.getContent();
		qint32 leading = 0;
		while (leading < content.size() and content.at(leading).isSpace()) {
			++leading;
		}
		qint32 trailing = content.size();
		while (trailing > leading and content.at(trailing - 1).isSpace()) {
			--trailing;
		}
		if (trailing <= leading) {
			return;
		}
		const QString value = content.mid(leading, trailing - leading);
		const QStringList parts = value.split(QLatin1Char(','));
		if (parts.size() != 3 and parts.size() != 4) {
			return;
		}
		double scale = 1.0;
		const QColor color = InlayColorProvider::colorFromString(value, &scale);
		if (not color.isValid()) {
			return;
		}
		const qint32 start = param.getIndex() + contentOffset + leading;
		const qint32 length = trailing - leading;
		addColorInlayHint(start + length, start, length, value, scale, parts.size() >= 4, parts.size());
	}

	void LS_AStoryXSyntaxHighlighter::createColorInlayHints(const QString& text,
		const ASERStudio::AStorySyntax::AStoryXControllerParseData& parseData) {
		// 剧本正文里的富文本颜色标签：<color=#RRGGBB> / <color=#AARRGGBB>
		static const QRegularExpression colorTagRegex(
			QStringLiteral(R"(<\s*color\s*=\s*(#[0-9a-fA-F]{6}|#[0-9a-fA-F]{8})\s*>)"),
			QRegularExpression::CaseInsensitiveOption);
		QRegularExpressionMatchIterator tagMatches = colorTagRegex.globalMatch(text);
		while (tagMatches.hasNext()) {
			const QRegularExpressionMatch match = tagMatches.next();
			const QString value = match.captured(1);
			if (not QColor(value).isValid()) {
				continue;
			}
			const bool hasAlpha = value.length() == 9;
			// 色块放在颜色值之后、标签的 '>' 之前，而不是放在整个标签之后。
			addColorInlayHint(match.capturedEnd(1), match.capturedStart(1), match.capturedLength(1),
				value, 1.0, hasAlpha, hasAlpha ? 4 : 3);
		}

		if (not parseData.isValid()) {
			return;
		}
		// 控制器参数中的颜色：目前是 ColorMask 的必需参数 Color，按 r,g,b[,a] 向量表示。
		addVectorColorInlayHint(parseData.getRequiredParameter(), 0,
			parseData.getControllerType() == ASERStudio::AStorySyntax::AStoryXController::ColorMask);
		for (const ASERStudio::AStorySyntax::AStoryXParameter& optional : parseData.getOptionalParameters()) {
			// 可选参数的 getIndex 指向前缀起点，内容从 prefix 之后开始。
			addVectorColorInlayHint(optional, optional.getPrefix().size(), false);
		}
	}

	void LS_AStoryXSyntaxHighlighter::createPositionInlayHints(const QString& text,
		const ASERStudio::AStorySyntax::AStoryXControllerParseData& parseData) {
		if (not parseData.isValid()) {
			return;
		}
		const auto controllerType = parseData.getControllerType();
		const bool isCharacter = controllerType == ASERStudio::AStorySyntax::AStoryXController::Character;
		const bool isBackground = controllerType == ASERStudio::AStorySyntax::AStoryXController::Background;
		if (not isCharacter and not isBackground) {
			return;
		}
		qint32 slot = 0;
		for (const ASERStudio::AStorySyntax::AStoryXParameter& optional : parseData.getOptionalParameters()) {
			if (optional.getName().compare(QStringLiteral("SlotIndex"), Qt::CaseInsensitive) != 0) {
				continue;
			}
			bool ok = false;
			const qint32 value = optional.getContent().trimmed().toInt(&ok);
			if (ok) {
				slot = value;
			}
			break;
		}
		const QList<double> defaults = InlayPositionProvider::getDefaultPosition(isCharacter, slot);
		const QList<ASERStudio::AStorySyntax::AStoryXParameter>& optionals = parseData.getOptionalParameters();
		for (qint32 paramIndex = 0; paramIndex < optionals.size(); ++paramIndex) {
			const ASERStudio::AStorySyntax::AStoryXParameter& optional = optionals[paramIndex];
			if (optional.getName().compare(QStringLiteral("StartPosition"), Qt::CaseInsensitive) != 0
				and optional.getName().compare(QStringLiteral("EndPosition"), Qt::CaseInsensitive) != 0) {
				continue;
			}
			if (optional.getValue().getType() != ASERStudio::AStorySyntax::AStoryXValueMeta::Vector) {
				continue;
			}
			// 直接按原文取得参数值区间，避免解析层还原占位符后长度与原文不一致。
			qint32 rawContentStart = optional.getIndex() + optional.getPrefix().length();
			qint32 rawContentEnd = text.length();
			for (qint32 nextIndex = paramIndex + 1; nextIndex < optionals.size(); ++nextIndex) {
				const qint32 nextPrefixIndex = optionals[nextIndex].getIndex();
				if (nextPrefixIndex > optional.getIndex()) {
					rawContentEnd = nextPrefixIndex;
					break;
				}
			}
			rawContentStart = qBound(0, rawContentStart, text.length());
			rawContentEnd = qBound(rawContentStart, rawContentEnd, text.length());
			const QString rawContent = text.mid(rawContentStart, rawContentEnd - rawContentStart);
			qint32 leading = 0;
			while (leading < rawContent.size() and rawContent.at(leading).isSpace()) {
				++leading;
			}
			qint32 trailing = rawContent.size();
			while (trailing > leading and rawContent.at(trailing - 1).isSpace()) {
				--trailing;
			}
			const QString value = rawContent.mid(leading, trailing - leading);
			const bool valid = InlayPositionProvider::parsePosition(value);
			qint32 valueStart = rawContentStart;
			qint32 valueLength = rawContent.size();
			if (valid) {
				valueStart += leading;
				valueLength = trailing - leading;
			}
			valueStart = qBound(0, valueStart, text.length());
			valueLength = qBound(0, valueLength, text.length() - valueStart);

			// 图标放在参数值之后，避免与参数名内联提示共用同一个插入位置。
			// 参数值为空时退到内容区间末尾；若连内容区间都没有，则放弃提示，
			// 否则会和参数名提示落在同一个插入点上。
			qint32 iconPosition = rawContentStart + trailing;
			if (trailing <= leading) {
				if (rawContentEnd <= rawContentStart) {
					continue;
				}
				iconPosition = rawContentEnd;
			}
			if (iconPosition < 1 or iconPosition > text.length()) {
				continue;
			}

			Visindigo::Utility::JsonConfig para;
			para.setString(InlayPositionKeys::Kind,
				isCharacter ? InlayPositionKeys::KindCharacter : InlayPositionKeys::KindBackground);
			para.setString(InlayPositionKeys::Parameter, optional.getName());
			para.setInt(InlayPositionKeys::Start, valueStart);
			para.setInt(InlayPositionKeys::Length, valueLength);
			para.setString(InlayPositionKeys::Value, value);
			if (d->InlayHintColor.isValid()) {
				para.setString(InlayPositionKeys::Color, d->InlayHintColor.name(QColor::HexArgb));
			}
			if (defaults.size() >= 3) {
				para.setDouble(InlayPositionKeys::DefaultX, defaults[0]);
				para.setDouble(InlayPositionKeys::DefaultY, defaults[1]);
				para.setDouble(InlayPositionKeys::DefaultZ, defaults[2]);
			}
			YSSCore::Editor::VirtualFilePath path(InlayPositionProviderID,
				isCharacter ? InlayPositionKeys::KindCharacter : InlayPositionKeys::KindBackground,
				para);
			if (not path.isValid()) {
				continue;
			}
			YSSCore::Editor::InlayHint hint(iconPosition, QString(), QColor(), 8);
			hint.setMulti(path.toString());
			createInlayHint(hint);
		}
	}

	void LS_AStoryXSyntaxHighlighter::onThemeChanged(const QMap<QString, YSSCore::Editor::StyleData>& theme) {
		d->CurrentTheme = theme;
		if (theme.contains("InlayHint")) {
			d->InlayHintColor = theme.value("InlayHint").getTextColor();
		}
	}

	void LS_AStoryXSyntaxHighlighter::onBlockRemoved(qint32 startBlockNumber, qint32 count) {
		d->Document->onLinesRemoved(startBlockNumber, count);
	}

	void LS_AStoryXSyntaxHighlighter::onBlockAdded(qint32 startBlockNumber, qint32 count) {
		d->Document->onLinesAdded(startBlockNumber, count);
	}

	void LS_AStoryXSyntaxHighlighter::setFormatWithColorKey(int start, int count, const QString& styleKey) {
		QString resolvedKey = styleKey;
		if (isControllerStyleKey(styleKey) && not d->CurrentTheme.contains(styleKey)) {
			resolvedKey = "Keyword";
		}
		YSSCore::Editor::StyleData styleData = d->CurrentTheme.value(resolvedKey, YSSCore::Editor::StyleData());
		QTextCharFormat format;
		format.setForeground(styleData.getTextColor());
		if (styleData.isBgColorEnabled()) {
			format.setBackground(styleData.getBgColor());
		}
		if (styleData.getUnderlineType() != YSSCore::Editor::StyleData::UnderlineType::NoUnderline) {
			format.setUnderlineColor(styleData.getLineColor());
			format.setUnderlineStyle(QTextCharFormat::UnderlineStyle(styleData.getUnderlineType()));
		}
		format.setFontWeight(styleData.isBold() ? QFont::Bold : QFont::Normal);
		format.setFontItalic(styleData.isItalic());
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