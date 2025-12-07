#pragma once
#include "../Macro.h"
#include <QtWidgets/qframe.h>
#include <QtCore/qstringlist.h>
#include <General/PluginModule.h>
// Forward declarations
namespace YSSCore::Editor {
	class FileTemplateInitWidgetPrivate;
	class FileTemplateProviderPrivate;
	class EditorPlugin;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI FileTemplateInitWidget :public QFrame {
		Q_OBJECT;
	signals:
		void filePrepared(QString filePath);
		void closed();
	public:
		FileTemplateInitWidget(const QString& initFolder, QWidget* parent);
		virtual ~FileTemplateInitWidget();
		QString getInitFolder();
	private:
		FileTemplateInitWidgetPrivate* d;
	};

	class YSSCoreAPI FileTemplateProvider :public Visindigo::General::PluginModule {
		friend class FileTemplateProviderPrivate;
		Q_OBJECT;
	public:
		FileTemplateProvider(const QString& name, const QString& id, EditorPlugin* plugin);
		virtual ~FileTemplateProvider();
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
		virtual FileTemplateInitWidget* fileInitWidget(const QString& initPath) = 0;
	private:
		FileTemplateProviderPrivate* d;
	};
}