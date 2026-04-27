#pragma once
#include <QObject>
#include <QtWidgets/qframe.h>
#include <Widgets/ThemeManager.h>
#include <Editor/FileServerManager.h>
class QMenuBar;
class QMenu;
class QAction;
class QHBoxLayout;
class QVBoxLayout;
namespace Visindigo::Widgets {
	class QuickMenu;
}
namespace YSS::Editor {
	class StackWidgetArea;
	class ResourceBrowser;

	class MainWin : public QFrame, Visindigo::Widgets::ColorfulWidget, YSSCore::Editor::FileWidgetHandler {
		Q_OBJECT;
	private:
		QWidget* CentralWidget = nullptr;
		ResourceBrowser* Browser = nullptr;
		StackWidgetArea* Editor = nullptr;
		QVBoxLayout* MainLayout = nullptr;
		QHBoxLayout* Layout = nullptr;
		Visindigo::Widgets::QuickMenu* Menu = nullptr;
		bool asked = false;
		static MainWin* Instance;
	public:
		MainWin();
		static MainWin* getInstance();
		ResourceBrowser* getResourceBrowser();
		StackWidgetArea* getStackWidgetArea();
		void saveAll();
		void backToProjectWin();
		virtual void onThemeChanged() override;
	public:
		virtual void closeEvent(QCloseEvent* event) override;
		virtual void hideEvent(QHideEvent* event) override;
		virtual void showEvent(QShowEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual bool handleBuiltinEditor(YSSCore::Editor::FileEditWidget* editor) override;
		virtual bool handleWindowEditor(QWidget* editor) override;
	private:
		void initMenu();
		bool checkProjectNeedToSave();
		void saveProject();
	};
}