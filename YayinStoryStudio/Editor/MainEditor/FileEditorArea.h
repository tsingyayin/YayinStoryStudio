#pragma once
#include <QtWidgets>
#include "StackBar.h"

namespace YSS::Editor {
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
		void addFileEditWidget(YSSCore::Editor::FileEditWidget* widget);
	private:
		void onStackBarLabelChanged(StackBarLabel* label);
	};
}