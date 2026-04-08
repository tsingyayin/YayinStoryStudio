#include "Plugin_ASERStudio.h"
#include "YSS/ASERProjectTemplate.h"
#include "YSS/ASERFIleServer.h"
#include "YSS/ASERFileTemplate.h"
#include "YSS/LangServer_AStoryX.h"
#include <Utility/FileUtility.h>
#include <General/Log.h>
#include <QtCore/qfileinfo.h>
#include <Widgets/ThemeManager.h>
#include <Widgets/ConfigWidget.h>
#include <QtCore/qdir.h>
namespace ASERStudio {
	ASERStudioTranslator::ASERStudioTranslator(Visindigo::General::Plugin* parent) :
		Visindigo::General::Translator(parent, "ASERStudio")
	{
		setDefaultLang(zh_CN);
		addLangFilePath(zh_CN, ":/resource/cn.yxgeneral.aserstudio/i18n/zh_CN.json");
		addLangFilePath(en_US, ":/resource/cn.yxgeneral.aserstudio/i18n/en_US.json");
	}
	
	class MainPrivate {
		friend class Main;
	protected:
		Visindigo::Widgets::ConfigWidget* ConfigWidget = nullptr;
	};
	Main::Main() : YSSCore::Editor::EditorPlugin() {
		d = new MainPrivate;
		setPluginVersion(Visindigo::General::Version(2, 0, 0));
		setPluginID("cn.yxgeneral.aserstudio");
		setPluginName("ASER Studio");
		setPluginAuthor({ "Gra_dus", "Tsing Yayin" });
	}
	Main::~Main() {
	}
	void Main::onPluginEnable() {
		Visindigo::Utility::JsonConfig config = VISTM->getMergedColorScheme();
		vgDebug << "Merged Color Scheme:" << config;
		registerPluginModule(new ASERStudioTranslator(this));
		registerProjectTemplateProvider(new YSS::ProjectTemplate_AStoryX(this));
		registerFileTemplateProvider(new YSS::FileTemplate_AStoryX(this));
		registerLangServer(new YSS::AStoryXLanguageServer(this));
		registerFileServer(new YSS::FileServer_AStoryX(this));
		registerFileServer(new YSS::FileServer_ASRuleJson(this));
		d->ConfigWidget = new Visindigo::Widgets::ConfigWidget();
		d->ConfigWidget->loadCWJson(Visindigo::Utility::FileUtility::readAll(":/resource/cn.yxgeneral.aserstudio/configWidget/pluginConfig.json"));
		d->ConfigWidget->setTargetConfig(getPluginFolder().filePath("config.json"));
		vgDebug << getPluginFolder().filePath("config.json");
		d->ConfigWidget->setIndependentMode(true);
	}
	void Main::onApplicationInit() {
	}
	void Main::onPluginDisable() {
	}
	void Main::onTest() {
	}

	void Main::onProjectOpen(YSSCore::General::YSSProject* project) {
		QString projectFolder = project->getProjectFolder();
		vgDebug << projectFolder + "/Rules";
		QStringList paths = Visindigo::Utility::FileUtility::fileFilter(projectFolder + "/Rules", { "*.json" });
		vgDebug << paths;
		for (auto path : paths) {
			vgDebug << path;
			QString content = Visindigo::Utility::FileUtility::readAll(path);
			ASERStudio::AStorySyntax::AStoryXRule rule("default");
			if (!rule.parseJson(content)) {
				vgError << "Failed to parse rule file:" << path;
				continue;
			}
			QFileInfo fileInfo(path);
			QString name = fileInfo.baseName();
			rule.setName(name);
			ASERStudio::AStorySyntax::AStoryXRule::registerRule(rule);
		}
	}

	void Main::onProjectClose(YSSCore::General::YSSProject* project) {
	}

	QWidget* Main::getConfigWidget() {
		return d->ConfigWidget;
	}
}