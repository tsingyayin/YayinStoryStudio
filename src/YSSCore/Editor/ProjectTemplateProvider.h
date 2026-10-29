#ifndef YSSCore_Editor_ProjectTemplateProvider_h
#define YSSCore_Editor_ProjectTemplateProvider_h
#include "YSSCoreCompileMacro.h"
#include <General/YSSURI.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qmap.h>
#include <QtCore/qstringlist.h>
#include <QtWidgets/qframe.h>
#include <General/PluginModule.h>
// Forward declarations
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace YSSCore::Editor {
	class ProjectTemplateInitWidgetPrivate;
	class ProjectTemplateProviderPrivate;
	class EditorPlugin;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI ProjectTemplateInitWidget :public QFrame {
		Q_OBJECT;
	signals:
		void projectPrepared();
	public:
		ProjectTemplateInitWidget(const YSSCore::General::YSSURI& targetUri, QWidget* parent = nullptr);
		virtual ~ProjectTemplateInitWidget();
		YSSCore::General::YSSURI getTargetUri() const;
		void setFileContent(const QString& inProjRelativePath, const QByteArray& content);
		QByteArray getFileContent(const QString& inProjRelativePath) const;
		QMap<QString, QByteArray> getAllFileContents() const;
		void setDefaultOpenFiles(const QStringList& inProjRelativePaths);
		QStringList getDefaultOpenFiles() const;
		void setYSSProjectContent(const Visindigo::Utility::JsonConfig& config);
		Visindigo::Utility::JsonConfig getYSSProjectContent() const;
	private:
		ProjectTemplateInitWidgetPrivate* d;
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
		virtual ProjectTemplateInitWidget* projectInitWidget(const YSSCore::General::YSSURI& targetUri) = 0;
	private:
		ProjectTemplateProviderPrivate* d;
	};
}
#endif // YSSCore_Editor_ProjectTemplateProvider_h
