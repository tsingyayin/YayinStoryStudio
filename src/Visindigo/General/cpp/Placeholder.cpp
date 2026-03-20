#include "General/Placeholder.h"
#include <QtCore/qstring.h>
#include <QtCore/qmap.h>
#include <QtCore/qregularexpression.h>
#include "General/Log.h"
#include "General/private//VIGeneral_p.h"

namespace Visindigo::General {
	class PlaceholderProviderPrivate {
		friend class PlaceholderProvider;
	protected:
		QString name;
	};
	PlaceholderProvider::PlaceholderProvider(Plugin* plugin, const QString& moduleID, const QString& providerName) : 
		PluginModule(plugin, moduleID, VIModuleType_PlaceholderProvider, providerName), d(new PlaceholderProviderPrivate) {
		if (providerName == QString()) {
			d->name = moduleID;
		}
		else {
			d->name = providerName;
		}
	}
	PlaceholderProvider::~PlaceholderProvider() {
		delete d;
	}
	QString PlaceholderProvider::getPlaceholderName() const {
		return d->name;
	}

	class PlaceholderManagerPrivate {
		friend class PlaceholderManager;
	protected:
		QMap<QString, PlaceholderProvider*> providers;
		QMap<QString, QObject*> qobjectProviders;
		static PlaceholderManager* Instance;
	};
	PlaceholderManager* PlaceholderManagerPrivate::Instance = nullptr;

	PlaceholderManager::PlaceholderManager() : d(new PlaceholderManagerPrivate) {

	}
	PlaceholderManager::~PlaceholderManager() {
		delete d;
	}
	PlaceholderManager* PlaceholderManager::getInstance() {
		if (PlaceholderManagerPrivate::Instance == nullptr) {
			PlaceholderManagerPrivate::Instance = new PlaceholderManager();
		}
		return PlaceholderManagerPrivate::Instance;
	}
	void PlaceholderManager::registerProvider(PlaceholderProvider* provider) {
		if (!provider) {
			return;
		}
		if(d->providers.contains(provider->getPlaceholderName())) {
			vgWarningF << "Provider with ID " << provider->getPlaceholderName() << " already exists, skipping registration.";
			return;
		}
		d->providers.insert(provider->getPlaceholderName(), provider);
		vgSuccessF << "Registered provider with ID " << provider->getPlaceholderName();
	}
	void PlaceholderManager::registerQObject(QObject* provider) {
		if (!provider) {
			return;
		}
		if ( d->qobjectProviders.contains(provider->objectName()) ) {
			vgWarningF << "QObject provider with object name " << provider->objectName() << " already exists, skipping registration.";
			return;
		}
		d->qobjectProviders.insert(provider->objectName(), provider);
		vgSuccessF << "Registered QObject provider with object name " << provider->objectName();
	}
	void PlaceholderManager::unregisterProvider(const QString& providerID) {
		d->providers.remove(providerID);
		vgSuccessF << "Unregistered provider with ID " << providerID;
	}
	void PlaceholderManager::unregisterQObject(const QString& providerObjectName) {
		d->qobjectProviders.remove(providerObjectName);
		vgSuccessF << "Unregistered QObject provider with object name " << providerObjectName;
	}
	QString PlaceholderManager::requestPlaceholder(QString rawText) {
		// Placeholder format: $(ID::Name::Param), Param is optional.
		// maybe more than one match
		QRegularExpression re(R"(\$\((\w+)::(\w+)(::(.*))*\))");
		QRegularExpressionMatchIterator it = re.globalMatch(rawText);
		if (!it.isValid()) {
			return rawText;
		}
		while (it.hasNext()) {
			QRegularExpressionMatch match = it.next();
			QString providerID = match.captured(1);
			QString name = match.captured(2);
			QString param = match.captured(4);
			if (providerID=="#QObject") {
				QObject* provider = d->qobjectProviders.value(name);
				if (!provider) {
					vgErrorF << "QObject provider with object name " << name << " unexpectedly null, will be removed from QObject providers list.";
					d->qobjectProviders.remove(name);
					continue;
				}
				QVariant result = provider->property(param.toStdString().c_str());
				rawText.replace(match.capturedStart(), match.capturedLength(), result.toString());
			}
			else if (d->providers.contains(providerID)) {
				if(param.contains("$(")) {
					//vgDebugF << "recursive detected: " << param;
					param = requestPlaceholder(param); // recursive 
					//vgDebugF << param;
				}
				PlaceholderProvider* provider = d->providers.value(providerID);
				if (!provider) {
					vgErrorF << "Provider with ID " << providerID << " unexpectedly null, will be removed from providers list.";
					d->providers.remove(providerID);
					continue;
				}
				QString result = provider->onPlaceholderRequest(name, param);
				rawText.replace(match.capturedStart(), match.capturedLength(), result);
			}
		}
		return rawText; 
	}

	QString PlaceholderManager::parse(QString rawText) {
		return requestPlaceholder(rawText);
	}
}