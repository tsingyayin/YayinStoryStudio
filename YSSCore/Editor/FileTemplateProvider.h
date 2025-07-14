#pragma once
#include <QtWidgets/qframe.h>
#include <QtCore/qstringlist.h>
#include "EditorPluginModule.h"
// Forward declarations
namespace YSSCore::Editor {
	class FileTemplateInitWidgetPrivate;
	class FileTemplateProviderPrivate;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI FileTemplateInitWidget :public QFrame {
		Q_OBJECT;
	signals:
		void filePrepared(QString filePath);
		void closed();
	public:
		FileTemplateInitWidget(QWidget* parent = nullptr);
	};

	class YSSCoreAPI FileTemplateProvider :public EditorPluginModule{
		friend class FileTemplateProviderPrivate;
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
		virtual FileTemplateInitWidget* fileInitWidget() = 0;
	private:
		FileTemplateProviderPrivate* d;
	};
}