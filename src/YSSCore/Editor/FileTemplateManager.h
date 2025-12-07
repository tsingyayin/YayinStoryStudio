#pragma once
#include <QtCore/qlist.h>
#include "../Macro.h"
// Forward declarations
namespace YSSCore::Editor {
	class FileTemplateProvider;
	class FileTemplateManagerPrivate;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI FileTemplateManager {
	private:
		FileTemplateManager();
	public:
		~FileTemplateManager();
		static FileTemplateManager* getInstance();
		QList<FileTemplateProvider*> getProviders();
		FileTemplateProvider* getProvider(const QString templateID);
		void addProvider(FileTemplateProvider* provider);
	private:
		FileTemplateManagerPrivate* d;
	};
};
// Helper macro
#define YSSFTM YSSCore::Editor::FileTemplateManager::getInstance()