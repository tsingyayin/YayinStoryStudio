#pragma once
#include <QtCore>
#include <QtWidgets>
#include "../Macro.h"

namespace YSSCore::Editor {
	class ProjectTemplateInitWidget :public QFrame {
		Q_OBJECT;
	signals:
		void lastProjectPathChanged(const QString& path);
		void projectPrepared(const QString& path);
	};
	class ProjectTemplateProviderPrivate;
	class YSSCoreAPI ProjectTemplateProvider  {
		friend class ProjectTemplateProviderPrivate;
	public:
		ProjectTemplateProvider();
		~ProjectTemplateProvider();
		QIcon getTemplateIcon();
		QString getTemplateID();
		QString getTemplateName();
		QString getTemplateDescription();
		ProjectTemplateInitWidget* projectInitWidget();
	private:
		ProjectTemplateProviderPrivate* p;
	};
}