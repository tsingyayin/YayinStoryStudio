#include <QtCore/qmap.h>
#include <General/Log.h>
#include "../ProjectTemplateProvider.h"
#include "../ProjectTemplateManager.h"
#include "General/YSSLogger.h"
namespace YSSCore::Editor {
	class ProjectTemplateManagerPrivate {
		friend class ProjectTemplateManager;
		QMap<QString, ProjectTemplateProvider*> ProviderMap;
		static ProjectTemplateManager* Instance;
	};
	ProjectTemplateManager* ProjectTemplateManagerPrivate::Instance = nullptr;

	/*!
		\class YSSCore::Editor::ProjectTemplateManager
		\brief ProjectTemplateManager保存Provider的实例
		\since YSS 0.13.0
		\inmodule YSSCore
		\ingroup CreateService
	
		ProjectTemplateManager是一个单例类，负责保存ProjectTemplateProvider的实例。
		它真的只负责保存：如果调用removeProvider，它不会自动销毁该实例。
	*/

	/*!
		\macro YSSPTM
		\since YSS 0.13.0
		\relates YSSCore::Editor::ProjectTemplateManager

		YSSCore::Editor::ProjectTemplateManager::getInstance()的简化写法
	*/

	/*!
		\since YSS 0.13.0
		构造函数
	*/
	ProjectTemplateManager::ProjectTemplateManager() {
		d = new ProjectTemplateManagerPrivate;
		ySuccessF << "Success!";
	}

	/*!
		\since YSS 0.13.0
		析构函数
	*/
	ProjectTemplateManager::~ProjectTemplateManager() {
		for (ProjectTemplateProvider* provider : d->ProviderMap.values()) {
			delete provider;
		}
		delete d;
	}

	/*!
		\since YSS 0.13.0
		获得实例
	*/
	ProjectTemplateManager* ProjectTemplateManager::getInstance() {
		if (ProjectTemplateManagerPrivate::Instance == nullptr) {
			ProjectTemplateManagerPrivate::Instance = new ProjectTemplateManager();
		}
		return ProjectTemplateManagerPrivate::Instance;
	}

	/*!
		\since YSS 0.13.0
		获得所有Provider实例
	*/
	QList<ProjectTemplateProvider*> ProjectTemplateManager::getProviders() {
		return d->ProviderMap.values();
	}

	/*!
		\since YSS 0.13.0
		根据ID获得Provider实例
	*/
	ProjectTemplateProvider* ProjectTemplateManager::getProvider(const QString templateID) {
		return d->ProviderMap[templateID];
	}

	/*!
		\since YSS 0.13.0
		添加Provider实例。如果已经存在相同ID的实例，这个函数不做任何事情。
	*/
	void ProjectTemplateManager::addProvider(ProjectTemplateProvider* provider) {
		if (d->ProviderMap.contains(provider->getTemplateID())) {
			return;
		}
		d->ProviderMap[provider->getTemplateID()] = provider;
	}

	/*!
		\since YSS 0.14.0
		移除Provider实例。这个函数不会销毁该实例。
	*/
	void ProjectTemplateManager::removeProvider(ProjectTemplateProvider* provider) {
		d->ProviderMap.remove(provider->getTemplateID());
	}
}