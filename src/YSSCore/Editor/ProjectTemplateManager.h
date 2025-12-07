#pragma once
#include <QtCore/qlist.h>
#include "../Macro.h"
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
	private:
		ProjectTemplateManagerPrivate* d;
	};
};
// Helper macro
#define YSSPTM YSSCore::Editor::ProjectTemplateManager::getInstance()