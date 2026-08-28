#ifndef YSS_MainEditor_DebugCommand_h
#define YSS_MainEditor_DebugCommand_h
#include <General/CommandHandler.h>
namespace YSS::Editor {
	class DebugCommand :public Visindigo::General::CommandHandler {
	public:
		DebugCommand(Visindigo::General::Plugin* parent);
		virtual Visindigo::General::CommandErrorData onCommand(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs) override;
		virtual QStringList onComplete(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs) override;
	};
}
#endif // YSS_MainEditor_DebugCommand_h