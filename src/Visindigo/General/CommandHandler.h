#ifndef Visindigo_General_CommandHandler_h
#define Visindigo_General_CommandHandler_h
#include "../Macro.h"
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include "Utility/JsonConfig.h"
namespace Visindigo::General {
	class CommandErrorData;
	class CommandHandlerPrivate;
	class VisindigoAPI CommandHandler {
	public:
		CommandHandler();
		CommandHandler(const QString& commandName, const QStringList& alias = QStringList());
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