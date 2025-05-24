#include "../ProjectTemplateProvider.h"

namespace YSSCore::Editor {
	ProjectTemplateInitWidget::ProjectTemplateInitWidget(QWidget *parent)
		: QFrame(parent)
	{

	}
	class ProjectTemplateProviderPrivate
	{
	public:
		QIcon TemplateIcon;
		QString TemplateID;
		QString TemplateName;
		QString TemplateDescription;
		QStringList TemplateTags;
	};
	ProjectTemplateProvider::ProjectTemplateProvider()
	{
		d = new ProjectTemplateProviderPrivate;
	}
	ProjectTemplateProvider::~ProjectTemplateProvider()
	{
		delete d;
	}
	QIcon ProjectTemplateProvider::getTemplateIcon()
	{
		return d->TemplateIcon;
	}
	void ProjectTemplateProvider::setTemplateIcon(const QIcon &icon)
	{
		d->TemplateIcon = icon;
	}
	QString ProjectTemplateProvider::getTemplateID()
	{
		return d->TemplateID;
	}
	void ProjectTemplateProvider::setTemplateID(const QString &id)
	{
		d->TemplateID = id;
	}
	QString ProjectTemplateProvider::getTemplateName()
	{
		return d->TemplateName;
	}
	void ProjectTemplateProvider::setTemplateName(const QString &name)
	{
		d->TemplateName = name;
	}
	QString ProjectTemplateProvider::getTemplateDescription()
	{
		return d->TemplateDescription;
	}
	void ProjectTemplateProvider::setTemplateDescription(const QString &description)
	{
		d->TemplateDescription = description;
	}
	QStringList ProjectTemplateProvider::getTemplateTags()
	{
		return d->TemplateTags;
	}
	void ProjectTemplateProvider::setTemplateTags(const QStringList &tags)
	{
		d->TemplateTags = tags;
	}
}