#include "../MultiButton.h"
#include "../MultiButtonGroup.h"
#include "../private/MultiButtonGroup_p.h"
#include "../private/MultiButton_p.h"
#include "../../General/Log.h"
namespace Visindigo::__Private__ {
	MultiButtonGroupPrivate::MultiButtonGroupPrivate(Visindigo::Widgets::MultiButtonGroup* q)
		: QObject(q), q(q) {
	}
	void MultiButtonGroupPrivate::onButtonClicked(Visindigo::Widgets::MultiButton* button) {
		yDebugF;
		if (CurrentPressedButton) {
			if (CurrentPressedButton != button) {
				CurrentPressedButton->setStyleSheet(CurrentPressedButton->d->NormalStyleSheet);
			}
		}
		CurrentPressedButton = button;
		if (CurrentPressedButton) {
			CurrentPressedButton->setStyleSheet(CurrentPressedButton->d->PressedStyleSheet);
		}
		CurrentPressedIndex = Buttons.indexOf(button);
	}
	void MultiButtonGroupPrivate::onButtonReleased(Visindigo::Widgets::MultiButton* button) {
		// PASS
	}
	void MultiButtonGroupPrivate::onButtonHovered(Visindigo::Widgets::MultiButton* button) {
		if (button == CurrentPressedButton) {
			button->setStyleSheet(button->d->PressedStyleSheet);
		}
	}
	void MultiButtonGroupPrivate::onButtonLeft(Visindigo::Widgets::MultiButton* button) {
		if (button == CurrentPressedButton) {
			button->setStyleSheet(button->d->PressedStyleSheet);
		}
	}
}

namespace Visindigo::Widgets {
	MultiButtonGroup::MultiButtonGroup(QObject* parent)
		: QObject(parent), d(new __Private__::MultiButtonGroupPrivate(this)) {
	}
	void MultiButtonGroup::addButton(MultiButton* button) {
		if (!button) return;
		if (d->Buttons.contains(button)) { return; }
		d->Buttons.append(button);
		connect(button, &MultiButton::clicked, this,
			[this, button]() {
				this->d->onButtonClicked(button);
				emit clicked(button);
				emit selectIndexChanged(d->CurrentPressedIndex);
			}
		);
		connect(button, &MultiButton::hover, this, [this, button]() { this->d->onButtonHovered(button); emit hover(button); });
		connect(button, &MultiButton::leave, this, [this, button]() { this->d->onButtonLeft(button); emit leave(button); });
		connect(button, &MultiButton::pressed, this, [this, button]() { emit pressed(button); });
		connect(button, &MultiButton::released, this, [this, button]() { this->d->onButtonReleased(button); emit released(button); });
		connect(button, &MultiButton::doubleClicked, this, [this, button]() { this->d->onButtonClicked(button); emit doubleClicked(button); });
	}

	void MultiButtonGroup::removeAll() {
		d->Buttons.clear();
		d->CurrentPressedButton = nullptr;
		d->CurrentPressedIndex = -1;
	}

	qint32 MultiButtonGroup::selectPrevious() {
		if (d->Buttons.isEmpty()) return -1;
		if (d->CurrentPressedIndex <= 0) {
			d->CurrentPressedIndex = d->Buttons.size() - 1;
		}
		else {
			d->CurrentPressedIndex--;
		}
		d->onButtonClicked(d->Buttons[d->CurrentPressedIndex]);
		return d->CurrentPressedIndex;
	}

	qint32 MultiButtonGroup::selectNext() {
		if (d->Buttons.isEmpty()) return -1;
		if (d->CurrentPressedIndex >= d->Buttons.size() - 1) {
			d->CurrentPressedIndex = 0;
		}
		else {
			d->CurrentPressedIndex++;
		}
		d->onButtonClicked(d->Buttons[d->CurrentPressedIndex]);
		return d->CurrentPressedIndex;
	}

	qint32 MultiButtonGroup::selectButton(MultiButton* button) {
		if (!button) return -1;
		if (!d->Buttons.contains(button)) return -1;
		d->onButtonClicked(button);
		d->CurrentPressedIndex = d->Buttons.indexOf(button);
		return d->CurrentPressedIndex;
	}

	void MultiButtonGroup::selectButton(qint32 index) {
		if (d->Buttons.isEmpty()) return;
		if (index >= d->Buttons.size()) return;
		d->CurrentPressedIndex = index;
		d->onButtonClicked(d->Buttons[index]);
	}

	MultiButton* MultiButtonGroup::getSelectedButton() const {
		return d->CurrentPressedButton;
	}
}