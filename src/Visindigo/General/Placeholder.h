#ifndef Visindigo_General_Placeholder_h
#define Visindigo_General_Placeholder_h
#include "../VICompileMacro.h"
#include "General/PluginModule.h"
class QString;
namespace Visindigo::General {
	class PlaceholderProviderPrivate;
	class PlaceholderProvider :public PluginModule {
		Q_OBJECT;
	public:
		PlaceholderProvider(Plugin* plugin, const QString& moduleID, const QString& providerName = "");
		~PlaceholderProvider();
	public:
		QString getPlaceholderName() const;
		virtual QString onPlaceholderRequest(const QString& name, const QString& param) = 0;
	private:
		PlaceholderProviderPrivate* d;
	};

	class PlaceholderManagerPrivate;
	class PlaceholderManager {
	private:
		PlaceholderManager();
	public:
		static PlaceholderManager* getInstance();
		~PlaceholderManager();
	public:
		void registerProvider(PlaceholderProvider* provider);
		void registerQObject(QObject* provider);
		void unregisterProvider(const QString& providerID);
		void unregisterQObject(const QString& providerObjectName);
		QString requestPlaceholder(QString rawText);
		QString parse(QString rawText);
	private:
		PlaceholderManagerPrivate* d;
	};
}
#endif // Visindigo_General_Placeholder_h
