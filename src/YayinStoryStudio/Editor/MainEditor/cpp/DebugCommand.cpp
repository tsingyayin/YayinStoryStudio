#include <General/Log.h>
#include "Editor/MainEditor/DebugCommand.h"
#include "Editor/MainEditor/TreeLayoutWidget.h"
namespace YSS::Editor {
	DebugCommand::DebugCommand(Visindigo::General::Plugin* parent) :
		Visindigo::General::CommandHandler(parent, "DebugCommand", "yss_me") {}
	Visindigo::General::CommandErrorData DebugCommand::onCommand(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs) {
		if (unnamedArgs.isEmpty()) {
			return Visindigo::General::CommandErrorData(Visindigo::General::CommandErrorData::InvalidArguments, "No command specified.");
		}
		QString command = unnamedArgs.first();
		if (command == "print_treelayout") {
			auto list = TreeLayoutWidget::getAllTopLevelLayouts();
			for (auto w : list) {
				vgDebug << w->saveToJson();
			}
		}
		return true;
	}

	QStringList DebugCommand::onComplete(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs) {
		if (unnamedArgs.isEmpty()) {
			return { "print_treelayout" };
		}
		return {};
	}
}