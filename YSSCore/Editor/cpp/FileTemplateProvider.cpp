#include "../FileTemplateProvider.h"

namespace YSSCore::Editor {
	FileTemplateInitWidget::FileTemplateInitWidget(QWidget* parent)
		: QFrame(parent)
	{
	}
	class FileTemplateProviderPrivate
	{
	public:
		QIcon TemplateIcon;
		QString TemplateID;
		QString TemplateName;
		QString TemplateDescription;
		QStringList TemplateTags;
		EditorPlugin* Plugin;
	};
	FileTemplateProvider::FileTemplateProvider(EditorPlugin* plugin)
	{
		d = new FileTemplateProviderPrivate;
		d->Plugin = plugin;
	}
	FileTemplateProvider::~FileTemplateProvider()
	{
		delete d;
	}
	QIcon FileTemplateProvider::getTemplateIcon()
	{
		return d->TemplateIcon;
	}
	void FileTemplateProvider::setTemplateIcon(const QIcon& icon)
	{
		d->TemplateIcon = icon;
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