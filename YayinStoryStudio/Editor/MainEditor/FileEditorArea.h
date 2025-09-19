#pragma once
#include <QObject>
#include <QWidget>
#include <Editor/FileEditWidget.h>
class QVBoxLayout;
class QLabel;

namespace YSS::Editor {
	class StackBar;
	class StackBarLabel;

	class FileEditorArea :public QWidget {
		Q_OBJECT;
	private:
		StackBar* Bar;
		QWidget* ActiveWidget;
		QVBoxLayout* Layout;
		QWidget* DefaultWidget;
		QLabel* DefaultLabel;
		QVBoxLayout* DefaultLayout;
	public:
		FileEditorArea(QWidget* parent = nullptr);
		void addFileEditWidget(Visindigo::Editor::FileEditWidget* widget);
		void focusOn(const QString& filePath);
		Visindigo::Editor::FileEditWidget* getCurrentFileEditWidget();
		void saveAllFiles();
	public:
		virtual void showEvent(QShowEvent* event) override;
	private:
		void onStackBarLabelChanged(StackBarLabel* label);
	};
}