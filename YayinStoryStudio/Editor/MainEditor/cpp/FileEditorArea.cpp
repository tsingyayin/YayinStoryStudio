#include "../FileEditorArea.h"
#include "../StackBar.h"
#include "../StackBarLabel.h"
#include <QVBoxLayout>
#include <QLabel>

namespace YSS::Editor {
	FileEditorArea::FileEditorArea(QWidget* parent) : QWidget(parent) {
		Bar = new StackBar(this);
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
		connect(Bar, &StackBar::stackBarLabelChanged, this, &FileEditorArea::onStackBarLabelChanged);
	}

	void FileEditorArea::addFileEditWidget(YSSCore::Editor::FileEditWidget* widget) {
		qDebug() << widget->getFilePath();
		if (Bar->isLabelOpened(widget->getFilePath())) {
			qDebug() << "FileEditorArea: file already opened!";
			Bar->focusOn(widget->getFilePath());
			delete widget;
			return;
		}
		StackBarLabel* label = new StackBarLabel(widget->getFileName(), widget->getFilePath(), widget);
		Bar->addLabel(label);
	}
	void FileEditorArea::focusOn(const QString& filePath) {
		if (Bar->isLabelOpened(filePath)) {
			Bar->focusOn(filePath);
		}
	}
	void FileEditorArea::onStackBarLabelChanged(StackBarLabel* label) {
		ActiveWidget->hide();
		Layout->removeWidget(ActiveWidget);
		if (label != nullptr) {
			ActiveWidget = label->getTargetWidget();
		}
		else {
			ActiveWidget = DefaultWidget;
		}
		Layout->addWidget(ActiveWidget);
		ActiveWidget->show();
	}
}