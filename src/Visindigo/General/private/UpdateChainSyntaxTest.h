#ifndef Visindigo_General_private_UpdateChainSyntaxTest_h
#define Visindigo_General_private_UpdateChainSyntaxTest_h
#include "General/UpdateChain.h"

namespace Visindigo::__Private__ {
	class MyUpdateChain :public Visindigo::General::UpdateChain {
	public:
		MyUpdateChain() {
			UNode(0.0.1, 0.0.2) {
				qDebug() << "Update 0.1.0";
			};
			UNode(0.0.2, 0.0.3) {
				qDebug() << "Update 0.2.0";
			};
			doUpdate();
		}
		virtual QString getCurrentVersion() override {
			return QString("");
		}
	};
}
#endif // Visindigo_General_private_UpdateChainSyntaxTest_h
