#include "Editor/YSSCommandHandler.h"
#include <General/Log.h>
namespace YSS::Editor {
	YSSCommandHandler::YSSCommandHandler(Visindigo::General::Plugin* parent)
		: Visindigo::General::CommandHandler(parent, "yss") {
		// Constructor implementation (if needed)
	}
	Visindigo::General::CommandErrorData YSSCommandHandler::onCommand(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs) {
		// Handle the command here
		if (entryName == "yss") {
			vgInfo << "YSS test command executed.";
			return true;
		}
		return false; // Command not recognized
	}
	QStringList YSSCommandHandler::onComplete(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs) {
		// Provide command completion suggestions here
		QStringList suggestions;
		if (entryName.isEmpty()) {
			suggestions << "test"; // Suggest the "test" command
		}
		return suggestions;
	}
} // namespace YSS