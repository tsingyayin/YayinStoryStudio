#include "../StackBarLabel.h"
#include <Widgets/ThemeManager.h>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMouseEvent>

namespace YSS::Editor {
	StackBarLabel::StackBarLabel(const QString& name, const QString& filePath, YSSCore::Editor::FileEditWidget* target) :QWidget() {
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
		Layout->setContentsMargins(2, 2, 2, 2);
		Layout->setSpacing(0);
		this->setLayout(Layout);
		TargetWidget = target;
		connect(CloseButton, &QPushButton::clicked, this, [this]() {
			bool closeable = TargetWidget->closeFile();
			if (closeable) {
				emit closeReady(this);
			}
			});
	}

	StackBarLabel::~StackBarLabel() {
		if (TargetWidget != nullptr) {
			TargetWidget->deleteLater();
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
			this->setStyleSheet("QWidget{\
				background-color: " % YSSTM->getColorString("ThemeColor.Editor.FileLabel.Normal.Background") % ";\
				color:" % YSSTM->getColorString("ThemeColor.Editor.FileLabel.Normal.Text") % "};");
			break;
		case State::Focused:
			this->setStyleSheet("QWidget{\
				background-color: " % YSSTM->getColorString("ThemeColor.Editor.FileLabel.Focused.Background") % ";\
				color:" % YSSTM->getColorString("ThemeColor.Editor.FileLabel.Focused.Text") % "};");
			break;
		case State::Pinned:
			this->setStyleSheet("QWidget{\
				background-color: " % YSSTM->getColorString("ThemeColor.Editor.FileLabel.Pinned.Background") % ";\
				color:" % YSSTM->getColorString("ThemeColor.Editor.FileLabel.Pinned.Text") % "};");
			break;
		default:
			break;
		}
	}

	StackBarLabel::State StackBarLabel::getState() const {
		return CurrentState;
	}

	void StackBarLabel::setTargetWidget(YSSCore::Editor::FileEditWidget* widget) {
		TargetWidget = widget;
	}

	YSSCore::Editor::FileEditWidget* StackBarLabel::getTargetWidget() const {
		return TargetWidget;
	}

	void StackBarLabel::mousePressEvent(QMouseEvent* event) {
		emit labelClicked(this);
	}

	QString StackBarLabel::getFilePath() const {
		return FilePath;
	}
}