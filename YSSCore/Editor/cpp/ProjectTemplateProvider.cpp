#include "../ProjectTemplateProvider.h"

namespace YSSCore::Editor {
	class ProjectTemplateProviderPrivate
	{
	public:
		QStringList templates;
	};
	ProjectTemplateProvider::ProjectTemplateProvider()
	{
		p = new ProjectTemplateProviderPrivate;
	}
	ProjectTemplateProvider::~ProjectTemplateProvider()
	{
		delete p;
	}
}