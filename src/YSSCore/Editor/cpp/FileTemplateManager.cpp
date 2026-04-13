#include <QtCore/qmap.h>
#include <General/Log.h>
#include "../FileTemplateProvider.h"
#include "../FileTemplateManager.h"
#include "General/YSSLogger.h"
namespace YSSCore::Editor {
	class FileTemplateManagerPrivate {
		friend class FileTemplateManager;
		QMap<QString, FileTemplateProvider*> ProviderMap;
		static FileTemplateManager* Instance;
	};
	FileTemplateManager* FileTemplateManagerPrivate::Instance = nullptr;

	/*!
		\class YSSCore::Editor::FileTemplateManager
		\brief FileTemplateManager保存Provider的实例
		\since YSS 0.13.0
		\inmodule YSSCore
		\ingroup CreateService

		FileTemplateManager是一个单例类，负责保存FileTemplateProvider的实例。
		它真的只负责保存：如果调用removeProvider，它不会自动销毁该实例。
	*/

	/*!
		\macro YSSFTM
		\since YSS 0.13.0
		\relates YSSCore::Editor::FileTemplateManager
		YSSCore::Editor::FileTemplateManager::getInstance()的简化写法
	*/

	/*!
		\since YSS 0.13.0
		构造函数
	*/
	FileTemplateManager::FileTemplateManager() {
		d = new FileTemplateManagerPrivate;
		ySuccessF << "Success!";
	}

	/*!
		\since YSS 0.13.0
		析构函数
	*/
	FileTemplateManager::~FileTemplateManager() {
		for (FileTemplateProvider* provider : d->ProviderMap.values()) {
			delete provider;
		}
		delete d;
	}

	/*!
		\since YSS 0.13.0
		获得实例
	*/
	FileTemplateManager* FileTemplateManager::getInstance() {
		if (FileTemplateManagerPrivate::Instance == nullptr) {
			FileTemplateManagerPrivate::Instance = new FileTemplateManager();
		}
		return FileTemplateManagerPrivate::Instance;
	}

	/*!
		\since YSS 0.13.0
		获得所有Provider实例
	*/
	QList<FileTemplateProvider*> FileTemplateManager::getProviders() {
		return d->ProviderMap.values();
	}

	/*!
		\since YSS 0.13.0
		根据模板ID获得Provider实例
	*/
	FileTemplateProvider* FileTemplateManager::getProvider(const QString templateID) {
		return d->ProviderMap[templateID];
	}

	/*!
		\since YSS 0.13.0
		添加Provider实例
	*/
	void FileTemplateManager::addProvider(FileTemplateProvider* provider) {
		if (d->ProviderMap.contains(provider->getTemplateID())) {
			return;
		}
		d->ProviderMap[provider->getTemplateID()] = provider;
	}

	/*!
		\since YSS 0.13.0
		移除Provider实例。这个函数不会销毁该实例。
	*/
	void FileTemplateManager::removeProvider(FileTemplateProvider* provider) {
		if (!d->ProviderMap.contains(provider->getTemplateID())) {
			return;
		}
		d->ProviderMap.remove(provider->getTemplateID());
	}
}