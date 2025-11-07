#pragma once
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include "../Macro.h"
// Forward declarations
namespace Visindigo::General {
	class CommandHandlerPrivate;
	class CommandHostPrivate;
}
// Main
namespace Visindigo::General {
	class CommandHandler {
	public:
		CommandHandler(const QString& commandName);
		void setAlias(const QStringList alias);
		QStringList getAlias();
		virtual bool onCommand() = 0;
		QStringList getUnnamedArgs();
		QMap<QString, QString> getNamedArgs();
	};

	class CommandHost {
	private:
		CommandHost(); // Singleton not allow public constructor
	public:
		~CommandHost();
		static CommandHost* getInstance();
		void registerCommand(CommandHandler* handler);
		void unregisterCommand(CommandHandler* handler);
		CommandHandler* getCommandHandler(const QString& commandName);
		bool executeCommand(const QString& commandLine);
		void setStdIOListening(bool enable);
	};
}