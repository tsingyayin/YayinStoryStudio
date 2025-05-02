#pragma once
#include <QtWidgets>
#include "TextEdit.h"
#include "StackBar.h"

namespace YSS::Editor {
	class TextWidget :public QWidget {
		Q_OBJECT;
	private:
		StackBar* Bar;
		QWidget* ActiveWidget;
		QVBoxLayout* Layout;
		QWidget* DefaultWidget;
		QLabel* DefaultLabel;
		QVBoxLayout* DefaultLayout;
	public:
		TextWidget(QWidget* parent = nullptr);
		void addTextEdit(const QString& path);
	private:
		void onStackBarLabelChanged(StackBarLabel* label);
	};
}