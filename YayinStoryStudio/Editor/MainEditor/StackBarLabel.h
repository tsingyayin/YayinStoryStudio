#pragma once
#include <QtWidgets>
#include "StackWidget.h"

namespace YSS::Editor {
	class StackBar;
	
	class StackBarLabel :public QWidget {
		Q_OBJECT;
	signals:
		void labelClicked(StackBarLabel*);
		void pinClicked();
		void closeReady(StackBarLabel*);
	public:
		enum State {
			Normal,
			Focused,
			Pinned,
		};
	private:
		QLabel* NameLabel;
		QPushButton* PinButton;
		QPushButton* CloseButton;
		State CurrentState = State::Normal;
		QHBoxLayout* Layout;
		StackWidget* TargetWidget;
		QString FilePath;
	public:
		StackBarLabel(const QString& name, const QString& filePath, StackWidget* target);
		virtual ~StackBarLabel() override;
		void setName(const QString& name);
		QString getName() const;
		void setState(State state);
		State getState() const;
		void setTargetWidget(StackWidget* widget);
		StackWidget* getTargetWidget() const;
		QString getFilePath() const;
	private:
		virtual void mousePressEvent(QMouseEvent* event) override;
	};
}