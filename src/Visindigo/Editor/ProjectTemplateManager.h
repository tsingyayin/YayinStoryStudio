#pragma once
#include <QtCore/qlist.h>
#include "../Macro.h"
// Forward declarations
namespace Visindigo::Editor {
	class ProjectTemplateProvider;
	class ProjectTemplateManagerPrivate;
}
// Main
namespace Visindigo::Editor {
	class VisindigoAPI ProjectTemplateManager {
	public:
		ProjectTemplateManager();
		ProjectTemplateManager(const ProjectTemplateManager& other) = delete;
		ProjectTemplateManager(ProjectTemplateManager&& other) = delete;
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
#define YSSPTM Visindigo::Editor::ProjectTemplateManager::getInstance()