#include "../FileTemplateProvider.h"

namespace YSSCore::Editor {
	class FileTemplateInitWidgetPrivate
	{
		friend class FileTemplateInitWidget;
	protected:
		QString InitFolder;
	};

	FileTemplateInitWidget::FileTemplateInitWidget(const QString& initFolder,QWidget* parent)
		: QFrame(parent)
	{
		d = new FileTemplateInitWidgetPrivate;
		d->InitFolder = initFolder;
	}

	FileTemplateInitWidget::~FileTemplateInitWidget()
	{
		delete d;
	}

	QString FileTemplateInitWidget::getInitFolder()
	{
		return d->InitFolder;
	}

	class FileTemplateProviderPrivate
	{
		friend class FileTemplateProvider;
	protected:
		QString TemplateIconPath;
		QString TemplateID;
		QString TemplateName;
		QString TemplateDescription;
		QStringList TemplateTags;
	};
	FileTemplateProvider::FileTemplateProvider(const QString& name, const QString& id, EditorPlugin* plugin):
		EditorPluginModule(name, id, plugin)
	{
		d = new FileTemplateProviderPrivate;
	}
	FileTemplateProvider::~FileTemplateProvider()
	{
		delete d;
	}
	QString FileTemplateProvider::getTemplateIconPath()
	{
		return d->TemplateIconPath;
	}
	void FileTemplateProvider::setTemplateIconPath(const QString& iconPath)
	{
		d->TemplateIconPath= iconPath;
	}
	QString FileTemplateProvider::getTemplateID()
	{
		return d->TemplateID;
	}
	void FileTemplateProvider::setTemplateID(const QString& id)
	{
		d->TemplateID = id;
	}
	QString FileTemplateProvider::getTemplateName()
	{
		return d->TemplateName;
	}
	void FileTemplateProvider::setTemplateName(const QString& name)
	{
		d->TemplateName = name;
	}
	QString FileTemplateProvider::getTemplateDescription()
	{
		return d->TemplateDescription;
	}
	void FileTemplateProvider::setTemplateDescription(const QString& description)
	{
		d->TemplateDescription = description;
	}
	QStringList FileTemplateProvider::getTemplateTags()
	{
		return d->TemplateTags;
	}
	void FileTemplateProvider::setTemplateTags(const QStringList& tags)
	{
		d->TemplateTags = tags;
	}
}