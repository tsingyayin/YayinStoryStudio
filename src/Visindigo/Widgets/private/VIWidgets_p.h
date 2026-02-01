#ifndef Visindigo_Widgets_VIWidgets_p_h
#define Visindigo_Widgets_VIWidgets_p_h
#include "General/CommandHandler.h"
namespace Visindigo::__Private__ {
	class VIWidgetsCommandHandler :public General::CommandHandler {
	private:
		VIWidgetsCommandHandler();
	public:
		static VIWidgetsCommandHandler* getInstance();
		~VIWidgetsCommandHandler();
		virtual General::CommandErrorData onCommand(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs) override;
		// no need to override onComplete for now
	};
}

#endif // Visindigo_Widgets_VIWidgets_p_h
