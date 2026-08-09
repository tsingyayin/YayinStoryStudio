#ifndef YSSCore_Editor_ColorThemeProvider_h
#define YSSCore_Editor_ColorThemeProvider_h
#include "YSSCoreCompileMacro.h"
#include <QtCore/qobject.h>
#include <Utility/JsonConfig.h>
#include <QtGui/qcolor.h>
namespace YSSCore::Editor {
	class LangServer;
	class YSSCoreAPI StyleData {
		Q_GADGET;
	public:
		enum UnderlineType { // keep in sync with QTextCharFormat::UnderlineStyle!
			NoUnderline,
			SingleUnderline,
			DashUnderline,
			DotLine,
			DashDotLine,
			DashDotDotLine,
			WaveUnderline,
			SpellCheckUnderline
		};
		Q_ENUM(UnderlineType);
		enum IdentifierType {
			Undefined, __Overlay_Background__, __Overlay_Underline__, __LineNumber__,
			Normal, Keyword, String, SpecialString, Comment, Number, Operator, Function, Class, Variable, Parameter, Vector,
			Macro, MacroParameter, Meta, Preprocessor, Type, Constant, Label, Namespace, Template, Enum, Struct, Union,
		};
		Q_ENUM(IdentifierType);
	public:
		Q_PROPERTY(QString styleName READ getStyleName WRITE setStyleName);
		Q_PROPERTY(IdentifierType identifierType READ getIdentifierType WRITE setIdentifierType);
		Q_PROPERTY(QColor textColor READ getTextColor WRITE setTextColor);
		Q_PROPERTY(QColor bgColor READ getBgColor WRITE setBgColor);
		Q_PROPERTY(bool bgColorEnabled READ isBgColorEnabled WRITE setBgColorEnabled);
		Q_PROPERTY(QColor lineColor READ getLineColor WRITE setLineColor);
		Q_PROPERTY(UnderlineType underlineType READ getUnderlineType WRITE setUnderlineType);
		Q_PROPERTY(bool bold READ isBold WRITE setBold);
		Q_PROPERTY(bool italic READ isItalic WRITE setItalic);
	public:
		QString getStyleName() const;
		void setStyleName(const QString& name);
		IdentifierType getIdentifierType() const;
		void setIdentifierType(IdentifierType type);
		QColor getTextColor() const;
		void setTextColor(const QColor& color);
		QColor getBgColor() const;
		void setBgColor(const QColor& color);
		bool isBgColorEnabled() const;
		void setBgColorEnabled(bool enabled);
		QColor getLineColor() const;
		void setLineColor(const QColor& color);
		UnderlineType getUnderlineType() const;
		void setUnderlineType(UnderlineType type);
		bool isBold() const;
		void setBold(bool bold);
		bool isItalic() const;
		void setItalic(bool italic);
	private:
		QString styleName;
		IdentifierType identifierType = IdentifierType::Undefined;
		QColor textColor = QColor(0, 0, 0);
		QColor bgColor = QColor(255, 255, 255);
		bool bgColorEnabled = false;
		QColor lineColor = QColor(255, 0, 0);
		UnderlineType underlineType = UnderlineType::NoUnderline;
		bool bold = false;
		bool italic = false;
	};

	class ColorThemeProviderPrivate;
	class YSSCoreAPI ColorThemeProvider :public QObject {
		Q_OBJECT;
	public:
		ColorThemeProvider(LangServer* parent);
		~ColorThemeProvider();
	signals:
		void currentThemeChanged(const QString& themeName);
		void themeAdded(const QString& themeName);
		void themeRemoved(const QString& themeName);
		void themeModified(const QString& themeName);
	public:
		QStringList getSupportedThemes();
		QString getCurrentTheme();
		void createNewTheme(const QString& themeName, const QString& copyFromTheme);
		void removeTheme(const QString& themeName);
		void setCurrentTheme(const QString& themeName);
		void parseStaticThemeFrom(const QString& jsonStr);
		void parseUserThemeFrom(const QString& jsonStr);
		QString deriveUserThemeToJson(const QString& themeName);
		QMap<QString, StyleData> getThemeStyleData(const QString& themeName);
		void setThemeStyleData(const QString& themeName, const QMap<QString, StyleData>& styleData);
		bool isStaticTheme(const QString& themeName);
		void setTemplateTextPath(const QString& filePath);
		QString getTemplateTextPath();
		StyleData getStyleData(const QString& themeName, const QString& styleName);
		StyleData getCurrentThemeStyleData(const QString& styleName);
		QMap<QString, StyleData> getCurrentThemeStyleData();
	private:
		ColorThemeProviderPrivate* d;
	};
}
#endif //YSSCore_Editor_ColorThemeProvider_h
