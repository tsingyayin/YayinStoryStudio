#pragma once
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtWidgets/qframe.h>
#include <QtGui/qimage.h>
#include "../Macro.h"

namespace YSSCore::Editor {
	class YSSCoreAPI ProjectTemplateInitWidget :public QFrame {
		Q_OBJECT;
	signals:
		void projectPrepared(QString projectPath);
		void closed();
	public:
		ProjectTemplateInitWidget(QWidget* parent = nullptr);
		virtual void closeEvent(QCloseEvent* event) override;
	};

	class EditorPlugin;
	class ProjectTemplateProviderPrivate;

	class YSSCoreAPI ProjectTemplateProvider {
		friend class ProjectTemplateProviderPrivate;
	public:
		ProjectTemplateProvider(EditorPlugin* plugin);
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