#ifndef Visindigo_General_VIGeneral_p_h
#define Visindigo_General_VIGeneral_p_h
#include "General/CommandHandler.h"
#include "General/Placeholder.h"
#include "General/Plugin.h"
#include "General/PluginModule.h"
#include "General/Translator.h"
namespace Visindigo::__Private__ {
	class VisindigoCorePrivate;
	class VisindigoCore :public General::Plugin {
		friend class VisindigoCorePrivate;
		Q_OBJECT;
	public:
		VisindigoCore();
		~VisindigoCore();
		virtual void onPluginEnable() override;
		virtual void onApplicationInit() override;
		virtual void onPluginDisable() override;
		virtual void onTest() override;
	};

	class VIGeneralCommandHandler :public Visindigo::General::CommandHandler {
	public:
		VIGeneralCommandHandler(Visindigo::General::Plugin* parent);
		~VIGeneralCommandHandler();
		virtual General::CommandErrorData onCommand(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs) override;
		// no need to override onComplete for now
	};

	class VisindigoTranslator : public Visindigo::General::Translator {
	public:
		VisindigoTranslator(Visindigo::General::Plugin* parent);
	};

	namespace VisindigoBuiltinPlaceholderProvider {
		class VisindigoCore: public ::Visindigo::General::PlaceholderProvider {
		public:
			VisindigoCore(Visindigo::General::Plugin* parent);
			~VisindigoCore();
			virtual QString onPlaceholderRequest(const QString& name, const QString& param) override;
		};

		class VIFormat :public ::Visindigo::General::PlaceholderProvider {
		public:
			VIFormat(Visindigo::General::Plugin* parent);
			~VIFormat();
			virtual QString onPlaceholderRequest(const QString& name, const QString& param) override;
		};
	}
}
#endif // Visindigo_General_VIGeneral_p_h
