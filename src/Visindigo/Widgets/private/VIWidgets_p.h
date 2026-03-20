#ifndef Visindigo_Widgets_VIWidgets_p_h
#define Visindigo_Widgets_VIWidgets_p_h
#include "General/CommandHandler.h"
#include "General/Plugin.h"
namespace Visindigo::__Private__ {
	class VisindigoWidgets :public Visindigo::General::Plugin {
		Q_OBJECT;
	public:
		VisindigoWidgets();
		virtual ~VisindigoWidgets();
		virtual void onPluginEnable() override;
		virtual void onPluginDisable() override;
		virtual void onApplicationInit() override;
		virtual void onTest() override;
	};
	class VIWidgetsCommandHandler :public General::CommandHandler {
	public:
		VIWidgetsCommandHandler(Visindigo::General::Plugin* parent);
		virtual ~VIWidgetsCommandHandler();
		virtual General::CommandErrorData onCommand(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs) override;
		// no need to override onComplete for now
	};
}

#endif // Visindigo_Widgets_VIWidgets_p_h
