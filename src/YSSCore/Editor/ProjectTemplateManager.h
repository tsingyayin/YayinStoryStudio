#ifndef YSSCore_Editor_ProjectTemplateManager_h
#define YSSCore_Editor_ProjectTemplateManager_h
#include <QtCore/qlist.h>
#include "YSSCoreCompileMacro.h"
// Forward declarations
namespace YSSCore::Editor {
	class ProjectTemplateProvider;
	class ProjectTemplateManagerPrivate;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI ProjectTemplateManager {
	private:
		ProjectTemplateManager();
	public:
		~ProjectTemplateManager();
		static ProjectTemplateManager* getInstance();
		QList<ProjectTemplateProvider*> getProviders();
		ProjectTemplateProvider* getProvider(const QString templateID);
		void addProvider(ProjectTemplateProvider* provider);
		void removeProvider(ProjectTemplateProvider* provider);
	private:
		ProjectTemplateManagerPrivate* d;
	};
};
// Helper macro
#define YSSPTM YSSCore::Editor::ProjectTemplateManager::getInstance()
#endif // YSSCore_Editor_ProjectTemplateManager_h