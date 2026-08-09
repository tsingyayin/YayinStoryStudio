#include "YSS/LangServer_AStoryX.h"
#include "YSS/LS_AStoryXSyntaxHighlighter.h"
#include "YSS/LS_AStoryXTabCompleter.h"
#include "YSS/LS_AStoryXHoverInfoProvider.h"
#include <Widgets/ThemeManager.h>
#include <Editor/ColorThemeProvider.h>
#include <Utility/FileUtility.h>
namespace ASERStudio::YSS {
	class AStoryXLanguageServerPrivate {
		friend class AStoryXLanguageServer;
	protected:
		static QMap<QString, ASERStudio::AStorySyntax::AStoryXDocument*> DocumentMap;
	};
	QMap<QString, ASERStudio::AStorySyntax::AStoryXDocument*> AStoryXLanguageServerPrivate::DocumentMap;

	AStoryXLanguageServer::AStoryXLanguageServer(YSSCore::Editor::EditorPlugin* plugin) :
		LangServer("ASE-Remake AStoryX File Language Server", "ASEAStoryXLS", plugin, "AStoryX", { "astoryx" }) {
		connect(VISTM, &Visindigo::Widgets::ThemeManager::programThemeChanged, this, [this]() {
			if (VISTM->getCurrentColorTheme() == "Dark") {
				QString syntaxTheme = getColorThemeProvider()->getCurrentTheme();
				if (syntaxTheme == "Visindigo Light 2024") {
					getColorThemeProvider()->setCurrentTheme("Visindigo Dark 2024");
				}
				else if (syntaxTheme == "Gradus Code Light") {
					getColorThemeProvider()->setCurrentTheme("Gradus Code Dark");
				}
			}
			else if (VISTM->getCurrentColorTheme() == "Light") {
				QString syntaxTheme = getColorThemeProvider()->getCurrentTheme();
				if (syntaxTheme == "Visindigo Dark 2024") {
					getColorThemeProvider()->setCurrentTheme("Visindigo Light 2024");
				}
				else if (syntaxTheme == "Gradus Code Dark") {
					getColorThemeProvider()->setCurrentTheme("Gradus Code Light");
				}
			}
			});
		getColorThemeProvider()->parseStaticThemeFrom(
			Visindigo::Utility::FileUtility::readAll(":/resource/cn.yxgeneral.aserstudio/syntaxColorTheme/visindigo_dark_2024.json"));
		getColorThemeProvider()->parseStaticThemeFrom(
			Visindigo::Utility::FileUtility::readAll(":/resource/cn.yxgeneral.aserstudio/syntaxColorTheme/visindigo_light_2024.json"));
		getColorThemeProvider()->setTemplateTextPath(
			":/resource/cn.yxgeneral.aserstudio/syntaxColorTheme/templateFile.astoryx");
		if (getPlugin()->getPluginConfig()->getString("_yss_auto_.LangServer." + getModuleID() + ".CurrentTheme").isEmpty()) {
			getPlugin()->getPluginConfig()->setString("_yss_auto_.LangServer." + getModuleID() + ".CurrentTheme", "Visindigo Dark 2024");
		}
	}
	YSSCore::Editor::SyntaxHighlighter* AStoryXLanguageServer::createHighlighter(YSSCore::Editor::TextEdit* textEdit) {
		return new LS_AStoryXSyntaxHighlighter(textEdit);
	}
	YSSCore::Editor::TabCompleterProvider* AStoryXLanguageServer::createTabCompleter(YSSCore::Editor::TextEdit* textEdit) {
		return new LS_AStoryXTabCompleter(textEdit);
	}
	YSSCore::Editor::HoverInfoProvider* AStoryXLanguageServer::createHoverInfoProvider(YSSCore::Editor::TextEdit* textEdit) {
		return new LS_AStoryXHoverInfoProvider(textEdit);
	}
	void AStoryXLanguageServer::setAStoryXDocument(const QString& who, ASERStudio::AStorySyntax::AStoryXDocument* doc) {
		AStoryXLanguageServerPrivate::DocumentMap.insert(who, doc);
	}
	void AStoryXLanguageServer::deleteAStoryXDocument(const QString& who) {
		ASERStudio::AStorySyntax::AStoryXDocument* doc = AStoryXLanguageServerPrivate::DocumentMap.value(who, nullptr);
		if (doc) {
			delete doc;
		}
		AStoryXLanguageServerPrivate::DocumentMap.remove(who);
	}
	ASERStudio::AStorySyntax::AStoryXDocument* AStoryXLanguageServer::getAStoryXDocument(const QString& who) {
		return AStoryXLanguageServerPrivate::DocumentMap.value(who, nullptr);
	}
}