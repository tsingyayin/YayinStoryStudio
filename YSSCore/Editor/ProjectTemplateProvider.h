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
		void projectPrepared();
	public:
		ProjectTemplateInitWidget(QWidget* parent = nullptr);
	};

	class EditorPlugin;
	class ProjectTemplateProviderPrivate;

	class YSSCoreAPI ProjectTemplateProvider {
		friend class ProjectTemplateProviderPrivate;
	public:
		ProjectTemplateProvider(EditorPlugin* plugin);
		virtual ~ProjectTemplateProvider();
		QIcon getTemplateIcon();
		void setTemplateIcon(const QIcon& icon);
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