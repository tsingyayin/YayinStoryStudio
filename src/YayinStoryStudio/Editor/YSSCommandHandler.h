#ifndef YayinStoryStudio_Editor_YSSCommandHandler_h
#define YayinStoryStudio_Editor_YSSCommandHandler_h
#include <General/CommandHandler.h>
#include <General/CommandHost.h>
namespace YSS::Editor {
	class YSSCommandHandler : public Visindigo::General::CommandHandler {
	public:
		YSSCommandHandler(Visindigo::General::Plugin* parent);
		virtual Visindigo::General::CommandErrorData onCommand(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs) override;
		virtual QStringList onComplete(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs) override;
	};
}
#endif // YayinStoryStudio_Editor_YSSCommandHandler_h
