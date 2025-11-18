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
#define YSSPTM Visindigo::Editor::ProjectTemplateManager::getInstance()