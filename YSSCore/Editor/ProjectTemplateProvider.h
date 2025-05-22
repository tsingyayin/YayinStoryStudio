#pragma once
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtWidgets/QFrame>
#include <QtGui/QIcon>
#include "../Macro.h"

namespace YSSCore::Editor {
	class YSSCoreAPI ProjectTemplateInitWidget :public QFrame {
		Q_OBJECT;
	signals:
		void lastProjectPathChanged(const QString& path);
		void projectPrepared(const QString& path);
	};

	class ProjectTemplateProviderPrivate;

	class YSSCoreAPI ProjectTemplateProvider {
		friend class ProjectTemplateProviderPrivate;
	public:
		ProjectTemplateProvider();
		~ProjectTemplateProvider();
		QIcon getTemplateIcon();
		QString getTemplateID();
		QString getTemplateName();
		QString getTemplateDescription();
		QStringList getTemplateTags();
		ProjectTemplateInitWidget* projectInitWidget();
	private:
		ProjectTemplateProviderPrivate* d; 
	};
}