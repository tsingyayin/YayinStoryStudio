#pragma once
#include <General/Package.h>

namespace YSS {
	class Main :public Visindigo::General::Package {
		Q_OBJECT;
	private:
		static Main* Instance;
	public:
		Main();
		virtual ~Main();
		virtual void onPluginEnable() override;
		virtual void onApplicationInit() override;
		virtual void onPluginDisable() override;
		virtual void onTest() override;
		static Main* getInstance();
	};
}

#define YSSApp YSS::Main::getInstance()