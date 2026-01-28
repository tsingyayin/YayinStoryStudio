#pragma once
#include "../Macro.h"
#include <QtCore/qstringlist.h>
#include <QtGui/qcolor.h>

// Forward declarations
class QString;
class QWidget;
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::Widgets {
	class StyleSheetTemplatePrivate;
}
// Main
namespace Visindigo::Widgets {
	class VisindigoAPI StyleSheetTemplate {
	public:
		StyleSheetTemplate();
		~StyleSheetTemplate();
		VIMoveable(StyleSheetTemplate);
		VICopyable(StyleSheetTemplate);
		bool parse(QString& templateStr);
		QString toString();
		QString getRawStyleSheet(const QString& keyWithNamespace);
		QString getRawStyleSheet(const QString& namespaceStr, const QString& key);
		QString getStyleSheet(const QString& keyWithNamespace, QWidget* getter = nullptr);
		QStringList getNamespaces() const;
		void setStyleSheetTemplate(const QString& namespaceStr, const QString& key, const QString& styleSheet);
		void setStyleSheetTemplate(const QString& keyWithNamespace, const QString& styleSheet);
		QString getTemplateID() const;
		QString getTemplateName() const;
		void setTemplateID(const QString& id);
		void setTemplateName(const QString& name);
		void merge(const StyleSheetTemplate& other);
	private:
		StyleSheetTemplatePrivate* d;
	};
}