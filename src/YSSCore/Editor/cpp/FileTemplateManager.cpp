#include <QtCore/qmap.h>
#include <General/Log.h>
#include "../FileTemplateProvider.h"
#include "../FileTemplateManager.h"
#include "General/YSSLogger.h"
namespace YSSCore::Editor {
	class FileTemplateManagerPrivate {
		friend class FileTemplateManager;
		QList<FileTemplateProvider*> Providers;
		QMap<QString, FileTemplateProvider*> ProviderMap;
		static FileTemplateManager* Instance;
	};
	FileTemplateManager* FileTemplateManagerPrivate::Instance = nullptr;

	FileTemplateManager::FileTemplateManager() {
		d = new FileTemplateManagerPrivate;
		ySuccessF << "Success!";
	}

	FileTemplateManager::~FileTemplateManager() {
		for (FileTemplateProvider* provider : d->Providers) {
			delete provider;
		}
		delete d;
	}

	FileTemplateManager* FileTemplateManager::getInstance() {
		if (FileTemplateManagerPrivate::Instance == nullptr) {
			FileTemplateManagerPrivate::Instance = new FileTemplateManager();
		}
		return FileTemplateManagerPrivate::Instance;
	}

	QList<FileTemplateProvider*> FileTemplateManager::getProviders() {
		return d->Providers;
	}

	FileTemplateProvider* FileTemplateManager::getProvider(const QString templateID) {
		return d->ProviderMap[templateID];
	}

	void FileTemplateManager::addProvider(FileTemplateProvider* provider) {
		if (d->ProviderMap.contains(provider->getTemplateID())) {
			return;
		}
		d->ProviderMap[provider->getTemplateID()] = provider;
		d->Providers.append(provider);
	}
}