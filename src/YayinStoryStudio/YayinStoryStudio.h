#pragma once
#include <General/Package.h>
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qlabel.h>
#include <Widgets/DragWidget.h>
namespace YSS {
	class MainPrivate;
	class Main :public Visindigo::General::Package {
		Q_OBJECT;
	public:
		Main();
		virtual ~Main();
		virtual void onPluginEnable() override;
		virtual void onApplicationInit() override;
		virtual void onPluginDisable() override;
		virtual void onTest() override;
		virtual QWidget* getConfigWidget() override;
		static Main* getInstance();
	private:
		MainPrivate* d;
	};

	class TestDragWidget :public QWidget{
		Q_OBJECT;
	public:
		Visindigo::Widgets::DragWidget* DragArea;
		QLabel* Label1;
		QLabel* Label2;
		QLabel* Label3;
		QLabel* Label4;
	public:
		TestDragWidget(QWidget* parent = nullptr);
		virtual void resizeEvent(QResizeEvent* event) override;
	};
}

#define YSSApp YSS::Main::getInstance()