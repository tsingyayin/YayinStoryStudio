#include "../ProjectTemplateProvider.h"
#include "../ProjectTemplateManager.h"
#include <QtCore/qmap.h>
#include "../../General/Log.h"
namespace YSSCore::Editor {
	class ProjectTemplateManagerPrivate {
		friend class ProjectTemplateManager;
		QList<ProjectTemplateProvider*> Providers;
		QMap<QString, ProjectTemplateProvider*> ProviderMap;
		static ProjectTemplateManager* Instance;
	};
	ProjectTemplateManager* ProjectTemplateManagerPrivate::Instance = nullptr;

	ProjectTemplateManager::ProjectTemplateManager() {
		d = new ProjectTemplateManagerPrivate;
		if (ProjectTemplateManagerPrivate::Instance == nullptr) {
			ProjectTemplateManagerPrivate::Instance = this;
		}
		ySuccessF << "Success!";
	}

	ProjectTemplateManager::~ProjectTemplateManager() {
		for (ProjectTemplateProvider* provider : d->Providers) {
			delete provider;
		}
		delete d;
	}

	ProjectTemplateManager* ProjectTemplateManager::getInstance() {
		return ProjectTemplateManagerPrivate::Instance;
	}

	QList<ProjectTemplateProvider*> ProjectTemplateManager::getProviders() {
		return d->Providers;
	}

	ProjectTemplateProvider* ProjectTemplateManager::getProvider(const QString templateID) {
		return d->ProviderMap[templateID];
	}

	void ProjectTemplateManager::addProvider(ProjectTemplateProvider* provider) {
		if (d->ProviderMap.contains(provider->getTemplateID())) {
			return;
		}
		d->ProviderMap[provider->getTemplateID()] = provider;
		d->Providers.append(provider);
	}
}