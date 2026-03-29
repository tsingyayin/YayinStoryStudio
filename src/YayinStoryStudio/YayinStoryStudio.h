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

	class TestSerialization{
		Q_GADGET;
		Q_PROPERTY(int value1 MEMBER value1);
		Q_PROPERTY(QString value2 MEMBER value2);
		Q_PROPERTY(QStringList testList MEMBER testList);
	public:
		TestSerialization() {};
		~TestSerialization() {};
	public:
		int value1;
		QString value2;
		QStringList testList;
	};
}

#define YSSApp YSS::Main::getInstance()