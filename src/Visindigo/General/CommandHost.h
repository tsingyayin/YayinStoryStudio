#ifndef Visindigo_General_CommandHost_h
#define Visindigo_General_CommandHost_h
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include "VICompileMacro.h"
// Forward declarations
namespace Visindigo::General {
	class CommandHandler;
	class CommandHostPrivate;
	class CommandErrorData;
}
// Main
namespace Visindigo::General {
	class VisindigoAPI CommandHost :public QObject{
		Q_OBJECT;
	private:
		CommandHost();
	signals:
		void errorOccurred(const CommandErrorData& errorData);
	public:
		~CommandHost();
		static CommandHost* getInstance();
		void setMaxCommandHistorySize(qsizetype size);
		qsizetype getMaxCommandHistorySize() const;
		QStringList getCommandHistory(const QString& startingWith = "") const;
		QString getLastCommand() const;
		void registerCommand(CommandHandler* handler);
		void unregisterCommand(CommandHandler* handler);
		bool isCommandRegistered(CommandHandler* handler);
		CommandHandler* getCommandHandler(const QString& commandName);
		void setStdInListeningEnabled(bool enabled);
		QStringList getRegisteredCommandNames() const;
		QStringList completeCommand(const QString& commandLine);
	public slots:
		void executeCommand(const QString& commandLine);
	private:
		CommandHostPrivate* d;
	};

	class StdInCommandListenerPrivate;
	class VisindigoAPI StdInCommandListener {
	private:
		StdInCommandListener();
	public:
		~StdInCommandListener();
		static StdInCommandListener* getInstance();
		void enable();
		void disable();
		bool isEnabled() const;
	private:
		StdInCommandListenerPrivate* d;
	};
}

#define VISCH Visindigo::General::CommandHost::getInstance()

#endif // Visindigo_General_CommandHost_h