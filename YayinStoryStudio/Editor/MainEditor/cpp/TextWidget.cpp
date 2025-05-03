#include "../TextWidget.h"

namespace YSS::Editor {
	TextWidget::TextWidget(QWidget* parent) : QWidget(parent) {
		Bar = new StackBar(this);
		Bar->setFixedHeight(50);
		Layout = new QVBoxLayout(this);
		Layout->addWidget(Bar);
		DefaultWidget = new QWidget(this);
		DefaultLabel = new QLabel("No file opened", DefaultWidget);
		DefaultLayout = new QVBoxLayout(DefaultWidget);
		DefaultLayout->addWidget(DefaultLabel);
		DefaultWidget->setLayout(DefaultLayout);
		ActiveWidget = DefaultWidget;
		Layout->addWidget(ActiveWidget);
		Layout->setContentsMargins(0, 0, 0, 0);
		Layout->setSpacing(0);
		connect(Bar, &StackBar::stackBarLabelChanged, this, &TextWidget::onStackBarLabelChanged);
	}

	void TextWidget::addTextEdit(const QString& filePath) {
		if (Bar->isLabelOpened(filePath)) {
			Bar->focusOn(filePath);
			return;
		}
		TextEdit* textEdit = new TextEdit();
		textEdit->openFile(filePath);
		QString fileName = QFileInfo(filePath).fileName();
		StackBarLabel* label = new StackBarLabel(fileName, filePath, textEdit);
		Bar->addLabel(label);
	}

	void TextWidget::onStackBarLabelChanged(StackBarLabel* label) {
		qDebug() << ActiveWidget;
		ActiveWidget->hide();
		Layout->removeWidget(ActiveWidget);
		if (label != nullptr) {
			ActiveWidget = label->getTargetWidget()->getWidget();
		}
		else {
			ActiveWidget = DefaultWidget;
		}
		Layout->addWidget(ActiveWidget);
		ActiveWidget->show();
	}
}