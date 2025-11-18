#include "YayinStoryStudio.h"
#include <Editor/FileServerManager.h>
#include <Editor/ProjectTemplateManager.h>
#include <Editor/FileTemplateManager.h>
#include <Editor/LangServerManager.h>
namespace YSS {
	Main::Main() {
	
	}

	void Main::onPluginEnable() {
		Visindigo::Editor::FileServerManager::getInstance();
		Visindigo::Editor::ProjectTemplateManager::getInstance();
		Visindigo::Editor::FileTemplateManager::getInstance();
		Visindigo::Editor::LangServerManager::getInstance();
	}

	void Main::onPluginDisbale() {
		// Plugin disabled actions
	}


	Main::~Main() {

	}
}