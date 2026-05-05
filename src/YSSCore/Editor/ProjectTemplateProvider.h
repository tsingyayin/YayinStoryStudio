#ifndef YSSCore_Editor_ProjectTemplateProvider_h
#define YSSCore_Editor_ProjectTemplateProvider_h
#include "YSSCoreCompileMacro.h"
#include <QtWidgets/qframe.h>
#include <General/PluginModule.h>
// Forward declarations
class QString;
namespace YSSCore::Editor {
	class ProjectTemplateProviderPrivate;
	class EditorPlugin;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI ProjectTemplateInitWidget :public QFrame {
		Q_OBJECT;
	signals:
		void projectPrepared(QString projectPath);
	public:
		ProjectTemplateInitWidget(QWidget* parent = nullptr);
	};

	class YSSCoreAPI ProjectTemplateProvider :public Visindigo::General::PluginModule {
		friend class ProjectTemplateProviderPrivate;
		Q_OBJECT;
	public:
		ProjectTemplateProvider(const QString& name, const QString& id, EditorPlugin* plugin);
		virtual ~ProjectTemplateProvider();
		QString getTemplateIconPath() const;
		void setTemplateIconPath(const QString& iconPath);
		QString getTemplateID() const;
		void setTemplateID(const QString& id);
		QString getTemplateName() const;
		void setTemplateName(const QString& name);
		QString getTemplateDescription() const;
		void setTemplateDescription(const QString& description);
		QStringList getTemplateTags() const;
		void setTemplateTags(const QStringList& tags);
		virtual ProjectTemplateInitWidget* projectInitWidget() = 0;
	private:
		ProjectTemplateProviderPrivate* d;
	};
}
#endif