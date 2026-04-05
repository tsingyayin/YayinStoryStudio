#include "YSS/LS_AStoryXTabCompleter.h"
#include "AStorySyntax/AStoryXDocument.h"
#include "AStorySyntax/AStoryXRule.h"
#include "AStorySyntax/AStoryXController.h"
#include "AStorySyntax/AStoryXValueMeta.h"
#include "YSS/LangServer_AStoryX.h"
#include "AStorySyntax/AStoryXControllerParseData.h"
#include <General/Log.h>
#include <Editor/TextEdit.h>

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
		vgDebug << parameter;
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
				items.append(YSSCore::Editor::TabCompleterItem(controller.getControllerTypeString(), controller.getStartSign(), controller.getHeader(), YSSCore::Editor::TabCompleterItem::ItemType::Function));
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
}