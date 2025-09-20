#include "../DebugServer.h"

namespace Visindigo::Editor {
	class DebugServerPrivate {
		friend class DebugServer;
	protected:
		DebugServer::SupportedDebugFeature Feature = DebugServer::SupportedDebugFeature::None;
	};
	DebugServer::DebugServer(const QString& name, const QString& id, EditorPlugin* plugin)
		: EditorPluginModule(name, id, plugin) {
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