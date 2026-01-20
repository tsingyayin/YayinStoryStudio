#pragma once
#include <General/Package.h>

namespace YSS {
	class Main :public Visindigo::General::Package {
		Q_OBJECT;
	public:
		Main();
		virtual ~Main();
		virtual void onPluginEnable() override;
		virtual void onApplicationInit() override;
		virtual void onPluginDisbale() override;
		virtual void onTest() override;
	};
}