#ifndef Visindigo_General_CommandHandler_h
#define Visindigo_General_CommandHandler_h
#include "VICompileMacro.h"
#include <QtCore/qstring.h>
#include "General/PluginModule.h"
#include "Utility/JsonConfig.h"

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

	class CommandHandlerPrivate;
	class VisindigoAPI CommandHandler :public PluginModule {
		friend class CommandHandlerPrivate;
		Q_OBJECT;
	public:
		CommandHandler(Plugin* parent, const QString& moduleID, const QString& commandName = QString(), const QStringList& alias = QStringList());
		void setCommandName(const QString& commandName);
		QString getCommandName() const;
		void setAlias(const QStringList& alias);
		QStringList getAlias() const;
		void enable();
		void disable();
		bool isEnabled();
		virtual CommandErrorData onCommand(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs) = 0;
		virtual QStringList onComplete(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs);
	private:
		CommandHandlerPrivate* d;
	};
}
#endif // Visindigo_General_CommandHandler_h