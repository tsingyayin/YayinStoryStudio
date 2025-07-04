#pragma once
#include <QtCore/qlist.h>
#include "../Macro.h"
namespace YSSCore::Editor {
	class ProjectTemplateProvider;
	class ProjectTemplateManagerPrivate;

	class YSSCoreAPI ProjectTemplateManager {
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

#define YSSPTM YSSCore::Editor::ProjectTemplateManager::getInstance()