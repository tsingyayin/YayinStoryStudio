#include "../StackBar.h"
#include "../StackBarLabel.h"
#include <QScrollArea>
#include <QHBoxLayout>
#include <Widgets/ThemeManager.h>
#include "../../GlobalValue.h"
#include <General/YSSProject.h>
namespace YSS::Editor {
	StackBar::StackBar(QWidget* parent) : QWidget(parent) {
		this->setFixedHeight(30);
		ScrollArea = new QScrollArea(this);
		ScrollArea->setWidgetResizable(true);
		ScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

		ScrollAreaWidget = new QWidget(ScrollArea);
		ScrollAreaWidget->setFixedSize(0, 50);
		ScrollArea->setWidget(ScrollAreaWidget);
		Layout = new QHBoxLayout(ScrollAreaWidget);
		ScrollAreaWidget->setLayout(Layout);
		Layout->setContentsMargins(0, 0, 0, 0);
		Layout->setSpacing(0);
		this->setStyleSheet(YSSTMSS("YSS::Default", this));
	}

	bool StackBar::isLabelOpened(const QString& filePath) {
		return LabelMap.contains(filePath);
	}

	bool StackBar::addLabel(StackBarLabel* label) {
		label->setFixedWidth(200);
		QString filePath = label->getFilePath();
		if (LabelMap.contains(filePath)) {
			return false;
		}
		LabelMap[filePath] = label;
		OpenedFiles.append(filePath);
		label->setParent(ScrollAreaWidget);
		ScrollAreaWidget->setFixedSize(OpenedFiles.size() * 200, ScrollArea->height());
		Labels.append(label);
		Layout->insertWidget(-1, label);
		label->show();
		label->setState(StackBarLabel::State::Focused);
		if (ActiveLabel != nullptr) {
			ActiveLabel->setState(StackBarLabel::State::Normal);
		}
		connect(label, &StackBarLabel::labelClicked, this, &StackBar::focus);
		connect(label, &StackBarLabel::closeReady, this, &StackBar::removeLabel);
		ActiveLabel = label;
		YSS::GlobalValue::getCurrentProject()->addEditorOpenedFile(filePath);
		emit stackBarLabelChanged(label);
		return true;
	}

	void StackBar::focusOn(const QString& filePath) {
		if (LabelMap.contains(filePath)) {
			StackBarLabel* label = LabelMap[filePath];
			if (label == ActiveLabel) {
				return;
			}
			label->setState(StackBarLabel::State::Focused);
			if (ActiveLabel != nullptr) {
				ActiveLabel->setState(StackBarLabel::State::Normal);
			}
			ActiveLabel = label;
			YSS::GlobalValue::getCurrentProject()->setFocusedFile(filePath);
			emit stackBarLabelChanged(label);
		}
	}

	void StackBar::focusOnWithLineNumber(const QString& filePath, quint32 lineNumber) {
		if (LabelMap.contains(filePath)) {
			StackBarLabel* label = LabelMap[filePath];
			label->setState(StackBarLabel::State::Focused);
			if (ActiveLabel != nullptr) {
				ActiveLabel->setState(StackBarLabel::State::Normal);
			}
			ActiveLabel = label;
			YSS::GlobalValue::getCurrentProject()->setFocusedFile(filePath);
			emit stackBarLabelChangedWithLineNumber(label, lineNumber);
		}
	}
	void StackBar::focus(StackBarLabel* label) {
		if (label == ActiveLabel) {
			return;
		}
		label->setState(StackBarLabel::State::Focused);
		if (ActiveLabel != nullptr) {
			ActiveLabel->setState(StackBarLabel::State::Normal);
		}
		ActiveLabel = label;
		YSS::GlobalValue::getCurrentProject()->setFocusedFile(label->getFilePath());
		emit stackBarLabelChanged(label);
	}
	void StackBar::resizeEvent(QResizeEvent* event) {
		ScrollArea->resize(this->size());
	}

	void StackBar::removeLabel(StackBarLabel* label) {
		if (LabelMap.contains(label->getFilePath())) {
			if (label == ActiveLabel) {
				int index = Labels.indexOf(ActiveLabel);
				if (index == 0) {
					Labels.removeFirst();
					if (Labels.size() == 0) {
						ActiveLabel = nullptr;
					}
					else {
						ActiveLabel = Labels[0];
						ActiveLabel->setState(StackBarLabel::State::Focused);
					}
				}
				else {
					Labels.removeAt(index);
					index -= 1;
					ActiveLabel = Labels[index];
					ActiveLabel->setState(StackBarLabel::State::Focused);
				}
			}
			LabelMap.remove(label->getFilePath());
			OpenedFiles.removeAll(label->getFilePath());
			Labels.removeAll(label);
			Layout->removeWidget(label);
			YSS::GlobalValue::getCurrentProject()->removeEditorOpenedFile(label->getFilePath());
			ScrollAreaWidget->setFixedSize(OpenedFiles.size() * 200, ScrollArea->height());
			label->deleteLater();
			emit stackBarLabelChanged(ActiveLabel);
		}
	}

	StackBarLabel* StackBar::getActiveLabel() const {
		return ActiveLabel;
	}

	YSSCore::Editor::FileEditWidget* StackBar::getActiveFileEditWidget() const {
		if (ActiveLabel != nullptr) {
			return ActiveLabel->getTargetWidget();
		}
		return nullptr;
	}

	void StackBar::saveAllFiles() {
		for (const QString& filePath : OpenedFiles) {
			if (LabelMap.contains(filePath)) {
				YSSCore::Editor::FileEditWidget* widget = LabelMap[filePath]->getTargetWidget();
				if (widget != nullptr) {
					widget->saveFile();
				}
			}
		}
	}
}