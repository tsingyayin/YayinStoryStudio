#include "Widgets/private/VIWidgets_p.h"
#include "Widgets/ThemeManager.h"
#include "General/Log.h"
namespace Visindigo::__Private__ {
	static VIWidgetsCommandHandler* s_instance = nullptr;
	VIWidgetsCommandHandler::VIWidgetsCommandHandler()
		:General::CommandHandler("viwidgets", {"viw"}) {
	}
	VIWidgetsCommandHandler* VIWidgetsCommandHandler::getInstance() {
		if (s_instance == nullptr) {
			s_instance = new VIWidgetsCommandHandler();
		}
		return s_instance;
	}
	VIWidgetsCommandHandler::~VIWidgetsCommandHandler() {
		s_instance = nullptr;
	}
	General::CommandErrorData VIWidgetsCommandHandler::onCommand(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs) {
		switch (unnamedArgs.size()) {
		case 0:
			vgInfo << "Visindigo Widgets Command Handler:";
			vgInfo << "  Command Name: " << getCommandName();
			vgInfo << "  Aliases: " << getAlias();
			vgInfo << "  Available Commands:";
			vgInfo << "    list-color-schemes";
		break;
		case 1:
			if (unnamedArgs[0] == "list-color-schemes") {
				vgInfo << Visindigo::Widgets::ThemeManager::getInstance()->getAllColorSchemes();
			}else if (unnamedArgs[0] == "list-style-templates") {
				vgInfo << Visindigo::Widgets::ThemeManager::getInstance()->getAllStyleTemplates();
			}else if (unnamedArgs[0] == "reload-themes") {
				Visindigo::Widgets::ThemeManager::getInstance()->loadAndRefresh();
			}
			else {
				vgError << "Unknown command for viwidgets: " << unnamedArgs[0];
				return General::CommandErrorData(General::CommandErrorData::UnknownCommand, "Unknown command: " + unnamedArgs[0], entryName, 0);
			}
		}
		return true;
	}
}