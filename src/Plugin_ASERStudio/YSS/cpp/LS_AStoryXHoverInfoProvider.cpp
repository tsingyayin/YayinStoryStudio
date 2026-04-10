#include "YSS/LS_AStoryXHoverInfoProvider.h"
#include "AStorySyntax/AStoryXDocument.h"
#include "YSS/LangServer_AStoryX.h"
#include "AStorySyntax/AStoryXControllerParseData.h"
#include <General/Log.h>
#include <Widgets/ThemeManager.h>
#include <Utility/ColorTool.h>
#include "AStorySyntax/AStoryXValueMeta.h"
#include <Editor/TextEdit.h>
#include <General/TranslationHost.h>
namespace ASERStudio::YSS {
	class LS_AStoryXHoverInfoProviderPrivate {
		friend class LS_AStoryXHoverInfoProvider;
	protected:
		ASERStudio::AStorySyntax::AStoryXDocument* Document = nullptr;
		QString filePath;
	};

	LS_AStoryXHoverInfoProvider::LS_AStoryXHoverInfoProvider(YSSCore::Editor::TextEdit* textEdit) : 
		HoverInfoProvider(textEdit), d(new LS_AStoryXHoverInfoProviderPrivate) {
		d->filePath = textEdit->getFilePath();
		d->Document = AStoryXLanguageServer::getAStoryXDocument(d->filePath);
	}

	LS_AStoryXHoverInfoProvider::~LS_AStoryXHoverInfoProvider() {
		delete d;
	}

	void LS_AStoryXHoverInfoProvider::onMouseHover(qint32 lineNumber, qint32 column, const QString& content) {
		if (triggerFromHover()) {
			if (column < 0 || column >= content.length()) {
				return;
			}
		}
		ASERStudio::AStorySyntax::AStoryXRule* rule = d->Document->getCurrentRule();
		if (not rule) {
			return;
		}
		auto parseData = d->Document->getParseData(lineNumber);
		if (not parseData.isValid()) {
			return;
		}
		ASERStudio::AStorySyntax::AStoryXParameter parameter = parseData.getCursorParameter(column);
		QString controllerName = ASERStudio::AStorySyntax::AStoryXController::controllerTypeToString(parseData.getControllerType());
		QString controllerNameI18N = VITR(QString("ASERStudio::controller.%1").
			arg(ASERStudio::AStorySyntax::AStoryXController::controllerTypeToString(parseData.getControllerType())));
		QString md = "# " + controllerNameI18N % " (" + controllerName + ")\n\n";
		md += VITR(QString("ASERStudio::document.%1.__description").arg(controllerName)) + "\n\n";
		QString currentParameterName = parseData.getCursorInWhichParameter(column);
		QString requiredParaName = rule->getRequiredParameterName(parseData.getControllerType());
		QStringList optionalParaNames = rule->getOptionalParameterNames(parseData.getControllerType());
		optionalParaNames.prepend(requiredParaName);
		for (auto s : optionalParaNames) {
			if (s != currentParameterName) [[likely]] {
				md += " " + s + " ";
			}
			else {
				md += " <font color=#66CCFF>" + s + "</font> ";
			}
		}
		md += "\n\n" + VITR(QString("ASERStudio::document.%1.%2").arg(controllerName).arg(currentParameterName)) + "\n";

		//vgDebug << md;
		setMarkdown(md);
	}
}