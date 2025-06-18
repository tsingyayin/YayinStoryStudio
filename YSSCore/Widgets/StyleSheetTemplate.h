#pragma once
#include "../Macro.h"

class QString;
class QWidget;

namespace YSSCore::Utility {
	class JsonConfig;
}
namespace YSSCore::Widgets {
	class StyleSheetTemplatePrivate;
	class YSSCoreAPI StyleSheetTemplate {
	public:
		StyleSheetTemplate();
		~StyleSheetTemplate();
		StyleSheetTemplate(const StyleSheetTemplate& other);
		StyleSheetTemplate(StyleSheetTemplate&& other) noexcept; 
		StyleSheetTemplate& operator=(const StyleSheetTemplate& other);
		StyleSheetTemplate& operator=(StyleSheetTemplate&& other);
		void parse(QString& templateStr);
		QString toString();
		QString getRawStyleSheet(const QString& key);
		QString getStyleSheet(const QString& key, YSSCore::Utility::JsonConfig* config = nullptr, QWidget* getter = nullptr);
		void setStyleSheetTemplate(const QString& key, const QString& styleSheet);
		void setTemplateName(const QString& name);
		QString getTemplateName();
		void setTemplateID(const QString& name);
		QString getTemplateID();
	private:
		StyleSheetTemplatePrivate* d;
	};
}