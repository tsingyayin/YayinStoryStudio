#ifndef YSSCore_Editor_FileTemplateProvider_h
#define YSSCore_Editor_FileTemplateProvider_h
#include "YSSCoreCompileMacro.h"
#include <General/YSSURI.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qstringlist.h>
#include <QtWidgets/qframe.h>
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
		void contentPrepared();
	public:
		FileTemplateInitWidget(const YSSCore::General::YSSURI& targetUri, QWidget* parent);
		virtual ~FileTemplateInitWidget();
		YSSCore::General::YSSURI getTargetUri() const;
		const QByteArray& getBinaryContent() const;
		void setBinaryContent(QByteArray& content);
		QString getTargetName() const;
		void setTargetName(const QString& name);
	private:
		FileTemplateInitWidgetPrivate* d;
	};

	class YSSCoreAPI FileTemplateProvider :public Visindigo::General::PluginModule {
		friend class FileTemplateProviderPrivate;
		Q_OBJECT;
	public:
		FileTemplateProvider(const QString& name, const QString& id, EditorPlugin* plugin);
		virtual ~FileTemplateProvider();
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
		QString getFileExt() const;
		void setFileExt(const QString& ext);
		virtual FileTemplateInitWidget* fileInitWidget(const YSSCore::General::YSSURI& targetUri);
	private:
		FileTemplateProviderPrivate* d;
	};
}

#endif // YSSCore_Editor_FileTemplateProvider_h