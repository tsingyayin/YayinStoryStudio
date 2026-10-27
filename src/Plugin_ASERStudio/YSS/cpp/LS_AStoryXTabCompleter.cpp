#include <QtCore/qpointer.h>
#include <Editor/TextEdit.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <Utility/StringUtility.h>
#include "ASEREnv/ASERResourceMoniter.h"
#include "AStorySyntax/AStoryXController.h"
#include "AStorySyntax/AStoryXDocument.h"
#include "AStorySyntax/AStoryXRule.h"
#include "AStorySyntax/AStoryXValueMeta.h"
#include "YSS/LangServer_AStoryX.h"
#include "YSS/LS_AStoryXTabCompleter.h"
namespace ASERStudio::YSS {
	class LS_AStoryXTabCompleterPrivate {
		friend class LS_AStoryXTabCompleter;
	protected:
		
		QPointer<ASERStudio::AStorySyntax::AStoryXDocument> document;
	};

	LS_AStoryXTabCompleter::LS_AStoryXTabCompleter(YSSCore::Editor::TextEdit* textEdit) :YSSCore::Editor::TabCompleterProvider(textEdit) {
		d = new LS_AStoryXTabCompleterPrivate;
		d->document = ASERStudio::YSS::AStoryXLanguageServer::getAStoryXDocument(textEdit->getFilePath());
	}
	LS_AStoryXTabCompleter::~LS_AStoryXTabCompleter() {
		delete d;
	}
	QList<YSSCore::Editor::TabCompleterItem> LS_AStoryXTabCompleter::onTabComplete(qint32 line, qint32 column, const QString& content) {
		if (d->document.isNull()) {
			return QList<YSSCore::Editor::TabCompleterItem>();
		}
		ASERStudio::AStorySyntax::AStoryXRule* rule = d->document->getCurrentRule();
		if (not rule) {
			return QList<YSSCore::Editor::TabCompleterItem>();
		}
		QList<YSSCore::Editor::TabCompleterItem> items;
		ASERStudio::AStorySyntax::AStoryXControllerParseData parseData = d->document->getParseData(line);
		auto parameter = parseData.getCursorParameter(column);
		onSpecificParameters(&items, &parseData, column);
		QString paramContent;
		if (parameter.isValid()) {
			paramContent = parameter.getContent();
		}
		//vgDebug << parameter;
		if (parameter.isValid()) {
			auto valueMeta = parameter.getValue();
			switch (valueMeta.getType()) {
			case ASERStudio::AStorySyntax::AStoryXValueMeta::Bool: {// just bool now.
				if (paramContent != "true" && paramContent != "false") {
					const QString boolText = VITRL("ASERStudio::value.bool");
					items.append(YSSCore::Editor::TabCompleterItem("true", "true", boolText, YSSCore::Editor::TabCompleterItem::ItemType::Value));
					items.append(YSSCore::Editor::TabCompleterItem("false", "false", boolText, YSSCore::Editor::TabCompleterItem::ItemType::Value));
				}
				break;
			}
			case ASERStudio::AStorySyntax::AStoryXValueMeta::Enum: {
				QStringList enumValues = valueMeta.getEnumCheckList();
				const QString enumText = VITRL("ASERStudio::value.enum");
				for (auto enumValue : enumValues) {
					items.append(YSSCore::Editor::TabCompleterItem(enumValue, enumValue, enumText, YSSCore::Editor::TabCompleterItem::ItemType::Enum));
				}
				break;
			}
			}
		}
		QStringList optPrefixes = rule->getOptionalParameterPrefixes(parseData.getControllerType());
		optPrefixes = Visindigo::Utility::StringUtility::deduplicate(optPrefixes);
		if (rule->isMonotonicity(parseData.getControllerType())) {
			const QString optionalPrefixText = VITRL("ASERStudio::value.other.optional_para_prefix");
			for (auto s : optPrefixes) {
				items.append(YSSCore::Editor::TabCompleterItem(s, s, optionalPrefixText, YSSCore::Editor::TabCompleterItem::ItemType::Operator, false));
			}
		}
		else {
			QStringList optNames = rule->getOptionalParameterNames(parseData.getControllerType());
			if (not optNames.isEmpty() && optNames.last() != parameter.getName()) {
				const QString optionalPrefixText = VITRL("ASERStudio::value.other.optional_para_prefix");
				for (auto s : optPrefixes) {
					items.append(YSSCore::Editor::TabCompleterItem(s, s, optionalPrefixText, YSSCore::Editor::TabCompleterItem::ItemType::Operator, false));
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
			const QString preprocessorText = VITRL("ASERStudio::controller.Preprocessor");
			for (auto preprocessor : preprocessors) {
				items.append(YSSCore::Editor::TabCompleterItem(preprocessor, preprocessor, preprocessorText, YSSCore::Editor::TabCompleterItem::ItemType::Operator));
			}
		}
		return items;
	}

	void LS_AStoryXTabCompleter::onSpecificParameters(QList<YSSCore::Editor::TabCompleterItem>* items, ASERStudio::AStorySyntax::AStoryXControllerParseData* data, qint32 column) {
		if (data->getControllerType() == ASERStudio::AStorySyntax::AStoryXController::ControllerType::Dialog) {
			if (data->getCursorInWhichParameter(column) == data->getRequiredParameter().getName()) {
				QString paramContent = data->getRequiredParameter().getContent();
				QString separator = data->getRequiredParameter().getSeparator();
				if (not paramContent.contains(separator)) {
					const QString separatorText = VITRL("ASERStudio::value.other.parameter_separator");
					if (separator == "\t") {
						items->append(YSSCore::Editor::TabCompleterItem("[ TAB ]", "\t", separatorText, YSSCore::Editor::TabCompleterItem::ItemType::Operator,
							true, YSSCore::Editor::TabCompleterItem::CompleterLevel::Few, YSSCore::Editor::TabCompleterItem::CompleterPriority::Top));
					}
					else {
						items->append(YSSCore::Editor::TabCompleterItem(separator, separator, separatorText, YSSCore::Editor::TabCompleterItem::ItemType::Operator));
					}
				}
			}
		}
		if (data->getControllerType() == ASERStudio::AStorySyntax::AStoryXController::ControllerType::Character ||
			data->getControllerType() == ASERStudio::AStorySyntax::AStoryXController::ControllerType::Interlude) {
			//.vgDebug << data->getCursorInWhichParameter(column) << data->getRequiredParameter().getName();
			if (data->getCursorInWhichParameter(column) == data->getRequiredParameter().getName()) {
				QString paramContent = data->getRequiredParameter().getContent();
				//vgDebug << paramContent << data->getRequiredParameter().getSeparator();
				if (not paramContent.contains(data->getRequiredParameter().getSeparator())) {
					auto charas = ASERRM->getCharacters();
					charas = Visindigo::Utility::StringUtility::getStartWith(paramContent, charas);
					//vgDebug << charas;

					const QString characterText = VITRL("ASERStudio::value.other.scanned_character");
					for (auto chara : charas) {
						items->append(YSSCore::Editor::TabCompleterItem(chara, chara, characterText, YSSCore::Editor::TabCompleterItem::ItemType::Object));
					}
				}
				else {
					auto chara = paramContent.split(data->getRequiredParameter().getSeparator()).first();
					auto diffs = ASERRM->getCharaDiff(chara);
					diffs = Visindigo::Utility::StringUtility::getStartWith(paramContent.split(data->getRequiredParameter().getSeparator()).last(), diffs);
					//vgDebug << diffs;
					const QString charaDiffText = VITRL("ASERStudio::value.other.scanned_chara_diff");
					for (auto diff : diffs) {
						items->append(YSSCore::Editor::TabCompleterItem(chara + data->getRequiredParameter().getSeparator() + diff, diff, charaDiffText, YSSCore::Editor::TabCompleterItem::ItemType::Object));
					}
				}
			}
		}
		else if (data->getControllerType() == ASERStudio::AStorySyntax::AStoryXController::ControllerType::Background) {
			if (data->getCursorInWhichParameter(column) == data->getRequiredParameter().getName()) {
				auto bgs = ASERRM->getBackground();
				bgs = Visindigo::Utility::StringUtility::getStartWith(data->getRequiredParameter().getContent(), bgs);
				const QString backgroundText = VITRL("ASERStudio::value.other.scanned_background");
				for (auto bg : bgs) {
					items->append(YSSCore::Editor::TabCompleterItem(bg, bg, backgroundText, YSSCore::Editor::TabCompleterItem::ItemType::Object));
				}
			}
		}
		else if (data->getControllerType() == ASERStudio::AStorySyntax::AStoryXController::ControllerType::Music) {
			if (data->getCursorInWhichParameter(column) == data->getRequiredParameter().getName()) {
				auto musics = ASERRM->getMusic();
				musics = Visindigo::Utility::StringUtility::getStartWith(data->getRequiredParameter().getContent(), musics);
				const QString musicText = VITRL("ASERStudio::value.other.scanned_music");
				for (auto music : musics) {
					items->append(YSSCore::Editor::TabCompleterItem(music, music, musicText, YSSCore::Editor::TabCompleterItem::ItemType::Object));
				}
			}
		}
		else if (data->getControllerType() == ASERStudio::AStorySyntax::AStoryXController::ControllerType::SoundEffect) {
			if (data->getCursorInWhichParameter(column) == data->getRequiredParameter().getName()) {
				auto sfxs = ASERRM->getSoundEffect();
				sfxs = Visindigo::Utility::StringUtility::getStartWith(data->getRequiredParameter().getContent(), sfxs);
				const QString soundEffectText = VITRL("ASERStudio::value.other.scanned_sound_effect");
				for (auto sfx : sfxs) {
					items->append(YSSCore::Editor::TabCompleterItem(sfx, sfx, soundEffectText, YSSCore::Editor::TabCompleterItem::ItemType::Object));
				}
			}
		}
	}
}