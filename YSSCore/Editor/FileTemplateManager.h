#pragma once
#include <QtCore/qlist.h>
#include "../Macro.h"
namespace YSSCore::Editor {
	class FileTemplateProvider;
	class FileTemplateManagerPrivate;

	class YSSCoreAPI FileTemplateManager {
	public:
		FileTemplateManager();
		FileTemplateManager(const FileTemplateManager& other) = delete;
		FileTemplateManager(FileTemplateManager&& other) = delete;
		~FileTemplateManager();
		static FileTemplateManager* getInstance();
		QList<FileTemplateProvider*> getProviders();
		FileTemplateProvider* getProvider(const QString templateID);
		void addProvider(FileTemplateProvider* provider);
	private:
		FileTemplateManagerPrivate* d;
	};
};

#define YSSFTM YSSCore::Editor::FileTemplateManager::getInstance()