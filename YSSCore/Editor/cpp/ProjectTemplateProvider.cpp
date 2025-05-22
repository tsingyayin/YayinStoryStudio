#include "../ProjectTemplateProvider.h"

namespace YSSCore::Editor {
	class ProjectTemplateProviderPrivate
	{
	public:
		QStringList templates;
	};
	ProjectTemplateProvider::ProjectTemplateProvider()
	{
		d = new ProjectTemplateProviderPrivate;
	}
	ProjectTemplateProvider::~ProjectTemplateProvider()
	{
		delete d;
	}

}