#pragma once
#include "../UpdateChain.h"

namespace YSSCore::__Private__ {
	class MyUpdateChain :public YSSCore::General::UpdateChain {
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
		virtual QString getCurrentVersion() override{
			return QString("");
		}
	};
}