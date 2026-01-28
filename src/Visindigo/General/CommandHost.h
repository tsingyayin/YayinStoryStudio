#pragma once
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include "../Macro.h"
// Forward declarations
namespace Visindigo::General {
	class CommandHandler;
	class CommandHostPrivate;
}
// Main
namespace Visindigo::General {
	class CommandErrorDataPrivate;
	class VisindigoAPI CommandErrorData {
	public:
		enum ErrorType {
			UnknownError,
			NoError,
			UnknownCommand,
			InvalidArguments,
			NoSuchKey,
			ExecutionFailed
		};
	public:
		CommandErrorData(bool noError);
		CommandErrorData(ErrorType type = NoError);
		CommandErrorData(ErrorType type, const QString& message = "", const QString& command = "", qsizetype errorPosition = -1);
		VIMoveable(CommandErrorData);
		VICopyable(CommandErrorData);
		~CommandErrorData();
		ErrorType getErrorType() const;
		QString getErrorMessage() const;
		QString getCommand() const;
		qsizetype getErrorPosition() const;
		
	private:
		CommandErrorDataPrivate* d;
	};

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

	class VisindigoAPI StdInCommandListener {
	private:
		StdInCommandListener();
	public:
		~StdInCommandListener();
		static StdInCommandListener* getInstance();
		void enable();
		void disable();
		bool isEnabled() const;
	};
}

#define VISCH Visindigo::General::CommandHost::getInstance()