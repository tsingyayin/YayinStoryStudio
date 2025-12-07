#include "../DebugServer.h"

namespace YSSCore::Editor {
	class DebugServerPrivate {
		friend class DebugServer;
	protected:
		DebugServer::SupportedDebugFeature Feature = DebugServer::SupportedDebugFeature::None;
	};
	DebugServer::DebugServer(const QString& name, const QString& id, EditorPlugin* plugin)
		: Visindigo::General::PluginModule(name, id, (Visindigo::General::Plugin*)plugin) {
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