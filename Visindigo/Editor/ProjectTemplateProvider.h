#pragma once
#include <QtWidgets/qframe.h>
#include "EditorPluginModule.h"
// Forward declarations
class QString;
namespace Visindigo::Editor {
	class ProjectTemplateProviderPrivate;
}
// Main
namespace Visindigo::Editor {
	class VisindigoAPI ProjectTemplateInitWidget :public QFrame {
		Q_OBJECT;
	signals:
		void projectPrepared(QString projectPath);
		void closed();
	public:
		ProjectTemplateInitWidget(QWidget* parent = nullptr);
		virtual void closeEvent(QCloseEvent* event) override;
	};

	class VisindigoAPI ProjectTemplateProvider :public EditorPluginModule {
		friend class ProjectTemplateProviderPrivate;
	public:
		ProjectTemplateProvider(const QString& name, const QString& id, EditorPlugin* plugin);
		virtual ~ProjectTemplateProvider();
		QString getTemplateIconPath();
		void setTemplateIconPath(const QString& iconPath);
		QString getTemplateID();
		void setTemplateID(const QString& id);
		QString getTemplateName();
		void setTemplateName(const QString& name);
		QString getTemplateDescription();
		void setTemplateDescription(const QString& description);
		QStringList getTemplateTags();
		void setTemplateTags(const QStringList& tags);
		virtual ProjectTemplateInitWidget* projectInitWidget() = 0;
	private:
		ProjectTemplateProviderPrivate* d;
	};
}