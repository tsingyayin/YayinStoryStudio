#include "../GlobalValue.h"
#include "YayinStoryStudio.h"
#include <Utility/JsonConfig.h>
#include <Widgets/ThemeManager.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qfile.h>
#include <QtGui/qcolor.h>
#include <General/Log.h>
#include <General/YSSProject.h>
#include <General/YSSLogger.h>
namespace YSS {
	GlobalValue* GlobalValue::Instance = nullptr;
	GlobalValue::GlobalValue() {
		Instance = this;
		yMessageF << "Program global value initializing";
		ySuccessF << "Program global value initialized !";
	}
	GlobalValue::~GlobalValue() {
		// Nothing to do
	}
	QColor GlobalValue::getColor(const QString& key) {
		return Instance->Theme->getColor(key);
	}

	Visindigo::Utility::JsonConfig* GlobalValue::getConfig() {
		return YSSApp->getPluginConfig();
	}
	void GlobalValue::saveConfig() {
		YSSApp->savePluginConfig();
	}
	GlobalValue* GlobalValue::getInstance() {
		if (Instance == nullptr) {
			Instance = new GlobalValue();
		}
		return Instance;
	}

	void GlobalValue::setCurrentProject(YSSCore::General::YSSProject* project) {
		YSSCore::General::YSSProject::setCurrentProject(project);
	}

	YSSCore::General::YSSProject* GlobalValue::getCurrentProject() {
		return YSSCore::General::YSSProject::getCurrentProject();
	}
}