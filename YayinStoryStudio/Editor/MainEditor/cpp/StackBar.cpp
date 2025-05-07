#include "../StackBar.h"
#include "../StackBarLabel.h"
#include <QScrollArea>
#include <QHBoxLayout>

namespace YSS::Editor {
	StackBar::StackBar(QWidget* parent) : QWidget(parent) {
		ScrollArea = new QScrollArea(this);
		ScrollArea->setWidgetResizable(true);
		ScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

		ScrollAreaWidget = new QWidget(this);
		ScrollArea->setWidget(ScrollAreaWidget);
		Layout = new QHBoxLayout(ScrollAreaWidget);
		Layout->setContentsMargins(0, 0, 0, 0);
		Layout->setSpacing(0);
	}

	bool StackBar::isLabelOpened(const QString& filePath) {
		return LabelMap.contains(filePath);
	}

	bool StackBar::addLabel(StackBarLabel* label) {
		QString filePath = label->getFilePath();
		if (LabelMap.contains(filePath)) {
			return false;
		}
		LabelMap[filePath] = label;
		OpenedFiles.append(filePath);
		label->setParent(ScrollAreaWidget);
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
			emit stackBarLabelChanged(label);
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
			label->deleteLater();
			emit stackBarLabelChanged(ActiveLabel);
		}
	}
}