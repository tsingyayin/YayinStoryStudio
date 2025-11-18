#pragma once
#include <QtCore/qlist.h>
#include "../Macro.h"
// Forward declarations
namespace Visindigo::Editor {
	class FileTemplateProvider;
	class FileTemplateManagerPrivate;
}
// Main
namespace Visindigo::Editor {
	class VisindigoAPI FileTemplateManager {
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
#define YSSFTM Visindigo::Editor::FileTemplateManager::getInstance()