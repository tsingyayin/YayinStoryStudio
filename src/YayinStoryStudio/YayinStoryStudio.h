#pragma once
#include <General/Package.h>

namespace YSS {
	class Main :public Visindigo::General::Package {
	public:
		Main();
		virtual ~Main();
		virtual void onPluginEnable() override;
		virtual void onPluginDisbale() override;
	};
}