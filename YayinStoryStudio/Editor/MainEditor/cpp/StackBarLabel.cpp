#include "../StackBarLabel.h"

namespace YSS::Editor {
	StackBarLabel::StackBarLabel(const QString& name, const QString& filePath, StackWidget* target):QWidget() {
		FilePath = filePath;
		NameLabel = new QLabel(name, this);
		PinButton = new QPushButton("Pin", this);
		CloseButton = new QPushButton("X", this);
		PinButton->setFixedWidth(30);
		CloseButton->setFixedWidth(30);
		Layout = new QHBoxLayout(this);
		Layout->addWidget(NameLabel);
		Layout->addWidget(PinButton);
		Layout->addWidget(CloseButton);
		this->setLayout(Layout);
		TargetWidget = target;
		connect(CloseButton, &QPushButton::clicked, this, [this]() {
			bool closeable = TargetWidget->onWidgetClose();
			if (closeable) {
				emit closeReady(this);
			}
			});
		
	}

	StackBarLabel::~StackBarLabel() {
		if (TargetWidget != nullptr) {
			TargetWidget->getWidget()->deleteLater();
		}
	}
	void StackBarLabel::setName(const QString& name) {
		NameLabel->setText(name);
	}

	QString StackBarLabel::getName() const {
		return NameLabel->text();
	}

	void StackBarLabel::setState(State state) {
		CurrentState = state;
		switch (state) {
		case State::Normal:
			this->setStyleSheet("background-color: #FFFFFF;");
			break;
		case State::Focused:
			this->setStyleSheet("background-color: #00A2E8;");
			break;
		case State::Pinned:
			this->setStyleSheet("background-color: #FF0000;");
			break;
		default:
			break;
		}
	}

	StackBarLabel::State StackBarLabel::getState() const {
		return CurrentState;
	}

	void StackBarLabel::setTargetWidget(StackWidget* widget) {
		TargetWidget = widget;
	}

	StackWidget* StackBarLabel::getTargetWidget() const {
		return TargetWidget;
	}

	void StackBarLabel::mousePressEvent(QMouseEvent* event) {
		emit labelClicked(this);
	}

	QString StackBarLabel::getFilePath() const {
		return FilePath;
	}
}