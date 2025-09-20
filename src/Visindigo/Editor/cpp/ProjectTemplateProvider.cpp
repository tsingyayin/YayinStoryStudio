#include "../ProjectTemplateProvider.h"

namespace Visindigo::Editor {
	ProjectTemplateInitWidget::ProjectTemplateInitWidget(QWidget* parent)
		: QFrame(parent)
	{
	}
	void ProjectTemplateInitWidget::closeEvent(QCloseEvent* event)
	{
		emit closed();
	}
	class ProjectTemplateProviderPrivate
	{
	public:
		QString TemplateIconPath;
		QString TemplateID;
		QString TemplateName;
		QString TemplateDescription;
		QStringList TemplateTags;
	};
	ProjectTemplateProvider::ProjectTemplateProvider(const QString& name, const QString& id, EditorPlugin* plugin)
		: EditorPluginModule(name, id, plugin)
	{
		d = new ProjectTemplateProviderPrivate;
	}
	ProjectTemplateProvider::~ProjectTemplateProvider()
	{
		delete d;
	}
	QString ProjectTemplateProvider::getTemplateIconPath()
	{
		return d->TemplateIconPath;
	}
	void ProjectTemplateProvider::setTemplateIconPath(const QString& iconPath)
	{
		d->TemplateIconPath = iconPath;
	}
	QString ProjectTemplateProvider::getTemplateID()
	{
		return d->TemplateID;
	}
	void ProjectTemplateProvider::setTemplateID(const QString& id)
	{
		d->TemplateID = id;
	}
	QString ProjectTemplateProvider::getTemplateName()
	{
		return d->TemplateName;
	}
	void ProjectTemplateProvider::setTemplateName(const QString& name)
	{
		d->TemplateName = name;
	}
	QString ProjectTemplateProvider::getTemplateDescription()
	{
		return d->TemplateDescription;
	}
	void ProjectTemplateProvider::setTemplateDescription(const QString& description)
	{
		d->TemplateDescription = description;
	}
	QStringList ProjectTemplateProvider::getTemplateTags()
	{
		return d->TemplateTags;
	}
	void ProjectTemplateProvider::setTemplateTags(const QStringList& tags)
	{
		d->TemplateTags = tags;
	}
}