#include "YSS/LS_AStoryXTabCompleter.h"
#include "AStorySyntax/AStoryXDocument.h"
#include "AStorySyntax/AStoryXRule.h"
#include "AStorySyntax/AStoryXController.h"
#include "AStorySyntax/AStoryXValueMeta.h"
#include "YSS/LangServer_AStoryX.h"
#include "YSS/ResourceMoniter.h"
#include <General/Log.h>
#include <Editor/TextEdit.h>
#include <General/TranslationHost.h>
namespace ASERStudio::YSS {
	class LS_AStoryXTabCompleterPrivate {
		friend class LS_AStoryXTabCompleter;
	protected:
		ASERStudio::AStorySyntax::AStoryXDocument* document;
	};

	LS_AStoryXTabCompleter::LS_AStoryXTabCompleter(YSSCore::Editor::TextEdit* textEdit) :YSSCore::Editor::TabCompleterProvider(textEdit) {
		d = new LS_AStoryXTabCompleterPrivate;
		d->document = ASERStudio::YSS::AStoryXLanguageServer::getAStoryXDocument(textEdit->getFilePath());
	}
	LS_AStoryXTabCompleter::~LS_AStoryXTabCompleter() {
		delete d;
	}
	QList<YSSCore::Editor::TabCompleterItem> LS_AStoryXTabCompleter::onTabComplete(qint32 line, qint32 column, const QString& content) {
		ASERStudio::AStorySyntax::AStoryXRule* rule = d->document->getCurrentRule();
		if (not rule) {
			return QList<YSSCore::Editor::TabCompleterItem>();
		}
		QList<YSSCore::Editor::TabCompleterItem> items;
		ASERStudio::AStorySyntax::AStoryXControllerParseData parseData = d->document->getParseData(line);
		auto parameter = parseData.getCursorParameter(column);
		onSpecificParameters(&items, &parseData, column);
		//vgDebug << parameter;
		if (parameter.isValid()) {
			auto valueMeta = parameter.getValue();
			switch (valueMeta.getType()) {
			case ASERStudio::AStorySyntax::AStoryXValueMeta::Bool: {// just bool now.
				items.append(YSSCore::Editor::TabCompleterItem("true", "true", "Boolean True", YSSCore::Editor::TabCompleterItem::ItemType::Value));
				items.append(YSSCore::Editor::TabCompleterItem("false", "false", "Boolean False", YSSCore::Editor::TabCompleterItem::ItemType::Value));
				break;
			}
			case ASERStudio::AStorySyntax::AStoryXValueMeta::Enum: {
				QStringList enumValues = valueMeta.getEnumCheckList();
				for (auto enumValue : enumValues) {
					items.append(YSSCore::Editor::TabCompleterItem(enumValue, enumValue, "Enum Value", YSSCore::Editor::TabCompleterItem::ItemType::Enum));
				}
				break;
			}
			}
		}
		
		if (content.isEmpty() && not content.startsWith("#")) {
			QList<ASERStudio::AStorySyntax::AStoryXController> controllers = rule->getAvailableControllers();
			for (auto controller : controllers) {
				items.append(YSSCore::Editor::TabCompleterItem(
					VITR(QString("ASERStudio::controller.%1").arg(controller.getControllerTypeString())),
					controller.getStartSign(), controller.getHeader(), YSSCore::Editor::TabCompleterItem::ItemType::Function));
			}
		}
		
		if (content.isEmpty() || content.startsWith("#") && content.size() <= 1) {
			QStringList preprocessors = rule->getSupportedPreprocessors();
			for (auto preprocessor : preprocessors) {
				items.append(YSSCore::Editor::TabCompleterItem(preprocessor, preprocessor, "Preprocessor", YSSCore::Editor::TabCompleterItem::ItemType::Operator));
			}
		}
		return items;
	}

	void LS_AStoryXTabCompleter::onSpecificParameters(QList<YSSCore::Editor::TabCompleterItem>* items, ASERStudio::AStorySyntax::AStoryXControllerParseData* data, qint32 column) {
		if (data->getControllerType() == ASERStudio::AStorySyntax::AStoryXController::ControllerType::Character) {
			vgDebug << data->getCursorInWhichParameter(column) << data->getRequiredParameter().getName();
			if (data->getCursorInWhichParameter(column) == data->getRequiredParameter().getName()){
				QString paramContent = data->getRequiredParameter().getContent();
				vgDebug << paramContent << data->getRequiredParameter().getSeparator();
				if (not paramContent.contains(data->getRequiredParameter().getSeparator())) {
					auto charas = ASERRM->getCharacters();
					vgDebug << charas;
					for (auto chara : charas) {
						items->append(YSSCore::Editor::TabCompleterItem(chara, chara, "Character", YSSCore::Editor::TabCompleterItem::ItemType::Object));
					}
				}
				else {
					auto chara = paramContent.split(data->getRequiredParameter().getSeparator()).first();
					auto diffs = ASERRM->getCharaDiff(chara);
					for (auto diff : diffs) {
						items->append(YSSCore::Editor::TabCompleterItem(chara+data->getRequiredParameter().getSeparator()+diff, diff, "Character Diff", YSSCore::Editor::TabCompleterItem::ItemType::Object));
					}
				}
			}
		}
		else if (data->getControllerType() == ASERStudio::AStorySyntax::AStoryXController::ControllerType::Background) {
			
			if (data->getCursorInWhichParameter(column) == data->getRequiredParameter().getName()) {
				auto bgs = ASERRM->getBackground();
				for (auto bg : bgs) {
					items->append(YSSCore::Editor::TabCompleterItem(bg, bg, "Background", YSSCore::Editor::TabCompleterItem::ItemType::Object));
				}
			}
		}
		else if (data->getControllerType() == ASERStudio::AStorySyntax::AStoryXController::ControllerType::Music) {
			if (data->getCursorInWhichParameter(column) == data->getRequiredParameter().getName()) {
				auto musics = ASERRM->getMusic();
				for (auto music : musics) {
					items->append(YSSCore::Editor::TabCompleterItem(music, music, "Music", YSSCore::Editor::TabCompleterItem::ItemType::Object));
				}
			}
		}
		else if (data->getControllerType() == ASERStudio::AStorySyntax::AStoryXController::ControllerType::SoundEffect) {
			if (data->getCursorInWhichParameter(column) == data->getRequiredParameter().getName()) {
				auto sfxs = ASERRM->getSoundEffect();
				for (auto sfx : sfxs) {
					items->append(YSSCore::Editor::TabCompleterItem(sfx, sfx, "Sound Effect", YSSCore::Editor::TabCompleterItem::ItemType::Object));
				}
			}
		}
	}
}