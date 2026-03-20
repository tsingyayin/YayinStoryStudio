#include "../DebugServer.h"
#include "Editor/EditorPlugin.h"
namespace YSSCore::Editor {
	class DebugServerPrivate {
		friend class DebugServer;
	protected:
		DebugServer::SupportedDebugFeature Feature = DebugServer::SupportedDebugFeatureFlag::None;
	};
	DebugServer::DebugServer(const QString& name, const QString& id, EditorPlugin* plugin)
		: Visindigo::General::PluginModule((Visindigo::General::Plugin*)plugin, id, YSSPluginModule_DebugServer, name ) {
		d = new DebugServerPrivate();
	}
	DebugServer::~DebugServer() {
		delete d;
	}
	void DebugServer::setSupportedFeatures(SupportedDebugFeature features) {
		d->Feature = features;
	}
	DebugServer::SupportedDebugFeature DebugServer::getSupportedFeatures() {
		return d->Feature;
	}
}