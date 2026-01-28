#pragma once
#include <General/CommandHandler.h>
#include <General/CommandHost.h>
namespace YSS {
	class YSSCommandHandler : public Visindigo::General::CommandHandler {
	public:
		YSSCommandHandler();
		virtual Visindigo::General::CommandErrorData onCommand(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs) override;
		virtual QStringList onComplete(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs) override;
	};
}