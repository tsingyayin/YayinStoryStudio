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
		//vgDebugF; // <- WHY THIS F**KING FUNCTION CALLED WITHOUT ANY CONTENT?
		if (CurrentPressedButton) {
			if (CurrentPressedButton != button) {
				CurrentPressedButton->d->buttonGroupChecked = false;
				if (CurrentPressedButton->d->NormalStyleSheet.isEmpty()) {
					CurrentPressedButton->repaint();
				}
				else {
					CurrentPressedButton->setStyleSheet(CurrentPressedButton->d->NormalStyleSheet);
				}
			}
		}
		CurrentPressedButton = button;
		if (CurrentPressedButton) {
			CurrentPressedButton->d->buttonGroupChecked = true;
			if (CurrentPressedButton->d->PressedStyleSheet.isEmpty()) {
				CurrentPressedButton->repaint();
			}
			else {
				CurrentPressedButton->setStyleSheet(CurrentPressedButton->d->PressedStyleSheet);
			}
		}
		CurrentPressedIndex = Buttons.indexOf(button);
	}
	void MultiButtonGroupPrivate::onButtonReleased(Visindigo::Widgets::MultiButton* button) {
		// PASS
	}
	void MultiButtonGroupPrivate::onButtonHovered(Visindigo::Widgets::MultiButton* button) {
		if (button == CurrentPressedButton) {
			//button->setStyleSheet(button->d->PressedStyleSheet);
		}
	}
	void MultiButtonGroupPrivate::onButtonLeft(Visindigo::Widgets::MultiButton* button) {
		if (button == CurrentPressedButton) {
			//button->setStyleSheet(button->d->PressedStyleSheet);
		}
	}
}

namespace Visindigo::Widgets {
	/*!
		\class Visindigo::Widgets::MultiButtonGroup
		\brief 管理多个MultiButton的状态，使它们表现得像一个单选按钮组。
		\since Visindigo 0.13.0
		\inmodule Visindigo

		这个类可以管理多个MultiButton的状态，使它们表现得像一个单选按钮组。
		当一个按钮被点击时，其他按钮会自动恢复到未选中状态。可以通过信号槽机制响应用户的点击、悬停等操作。

		一般来讲，如果将MultiButton加入该组，就不需要再从外部单独连接每个按钮的信号，你可以直接使用
		MultiButtonGroup的完全等价的信号来响应用户的操作。

		此外，这组虽然是QObject子类，但并不管理按钮的生命周期，也不负责布局，它只是一个状态管理器。
	*/

	/*!
		\fn Visindigo::Widgets::MultiButtonGroup::clicked(MultiButton* button)
		\since Visindigo 0.13.0
		当组内的某个按钮被"咔哒"时发出此信号。即在该按钮区域内按下且释放鼠标时发出此信号。
		参数button表示被点击的按钮。
	*/

	/*!
		\fn Visindigo::Widgets::MultiButtonGroup::doubleClicked(MultiButton* button)
		\since Visindigo 0.13.0
		当组内的某个按钮被双击时发出此信号。即在该按钮区域内连续两次按下鼠标时发出此信号。
		参数button表示被双击的按钮。
	*/

	/*!
		\fn Visindigo::Widgets::MultiButtonGroup::pressed(MultiButton* button)
		\since Visindigo 0.13.0
		当组内的某个按钮被按下时发出此信号。即在该按钮区域内按下鼠标时发出此信号。
		参数button表示被按下的按钮。
	*/

	/*!
		\fn Visindigo::Widgets::MultiButtonGroup::released(MultiButton* button)
		\since Visindigo 0.13.0
		当组内的某个按钮被释放时发出此信号。即在该按钮区域内释放鼠标时发出此信号。
		参数button表示被释放的按钮。
	*/

	/*!
		\fn Visindigo::Widgets::MultiButtonGroup::hover(MultiButton* button)
		\since Visindigo 0.13.0
		当鼠标悬停在组内的某个按钮上时发出此信号。即当鼠标进入该按钮区域时发出此信号。
		参数button表示被悬停的按钮。
	*/

	/*!
		\fn Visindigo::Widgets::MultiButtonGroup::leave(MultiButton* button)
		\since Visindigo 0.13.0
		当鼠标离开组内的某个按钮时发出此信号。即当鼠标离开该按钮区域时发出此信号。
		参数button表示被离开的按钮。
	*/

	/*!
		\fn Visindigo::Widgets::MultiButtonGroup::selectIndexChanged(quint32 index)
		\since Visindigo 0.13.0
		当组内的选中按钮发生变化时发出此信号。即当用户点击某个按钮使其成为选中状态时发出此信号。
		参数index表示当前被选中的按钮在组内的索引，如果没有按钮被选中则为-1。
	*/

	/*!
		\since Visindigo 0.13.0
		构造函数
	*/
	MultiButtonGroup::MultiButtonGroup(QObject* parent)
		: QObject(parent), d(new __Private__::MultiButtonGroupPrivate(this)) {
	}

	/*!
		\since Visindigo 0.14.0
		析构函数
	*/
	MultiButtonGroup::~MultiButtonGroup() {
		// NOTICE: Private class will automatically delete by Qt's parent-child mechanism, so we don't need to manually delete it here.
	}

	/*!
		\since Visindigo 0.13.0
		添加一个按钮到组中。这个函数会连接按钮的相关信号到本类内置的槽函数，以便管理按钮的状态。
		参数button表示要添加的按钮，如果button为nullptr或者已经在组内，则此函数不会执行任何操作。
	*/
	void MultiButtonGroup::addButton(MultiButton* button) {
		if (!button) return;
		if (d->Buttons.contains(button)) { return; }
		button->d->inButtonGroup = true;
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

	/*!
		\since Visindigo 0.13.0
		移除组内的所有按钮，并重置选中状态。这个函数不会删除按钮对象本身，只是将它们从组内移除。
		如果你已经在外部丢失了这些按钮的指针，请用getMultiButtons()函数先获得它们，否则会造成内存泄漏。
	*/
	void MultiButtonGroup::removeAll() {
		for (MultiButton* button : d->Buttons) {
			if (button) {
				button->d->inButtonGroup = false;
			}
		}
		d->Buttons.clear();
		d->CurrentPressedButton = nullptr;
		d->CurrentPressedIndex = -1;
	}

	/*!
		\since Visindigo 0.13.0
		从组内移除一个按钮，并重置选中状态。这个函数不会删除按钮对象本身，只是将它从组内移除。
		如果参数button为nullptr或者不在组内，则此函数不会执行任何操作。
	*/
	void MultiButtonGroup::removeButton(MultiButton* button) {
		if (!button) return;
		if (!d->Buttons.contains(button)) return;
		button->d->inButtonGroup = false;
		d->Buttons.removeAll(button);
		if (d->CurrentPressedButton == button) {
			d->CurrentPressedButton = nullptr;
			d->CurrentPressedIndex = -1;
		}
	}

	/*!
		\since Visindigo 0.13.0
		选择上一个按钮。这个函数会将当前选中的按钮切换到组内的上一个按钮，并发出相关信号。
		如果当前没有按钮被选中，则会选择最后一个按钮。返回值是当前被选中的按钮在组内的索引，如果没有按钮被选中则为-1。
	*/
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

	/*!
		\since Visindigo 0.13.0
		选择下一个按钮。这个函数会将当前选中的按钮切换到组内的下一个按钮，并发出相关信号。
		如果当前没有按钮被选中，则会选择第一个按钮。返回值是当前被选中的按钮在组内的索引，如果没有按钮被选中则为-1。
	*/
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

	/*!
		\since Visindigo 0.13.0
		选择指定的按钮。这个函数会将当前选中的按钮切换到参数button所指定的按钮，并发出相关信号。
		如果参数button为nullptr或者不在组内，则此函数不会执行任何操作。返回值是当前被选中的按钮在组内的索引，如果没有按钮被选中则为-1。
	*/
	qint32 MultiButtonGroup::selectButton(MultiButton* button) {
		if (!button) return -1;
		if (!d->Buttons.contains(button)) return -1;
		d->onButtonClicked(button);
		d->CurrentPressedIndex = d->Buttons.indexOf(button);
		return d->CurrentPressedIndex;
	}

	/*!
		\since Visindigo 0.13.0
		选择指定索引的按钮。这个函数会将当前选中的按钮切换到组内索引为index的按钮，并发出相关信号。
		如果参数index超出组内按钮的范围，则此函数不会执行任何操作。
	*/
	void MultiButtonGroup::selectButton(qint32 index) {
		if (d->Buttons.isEmpty()) return;
		if (index >= d->Buttons.size()) return;
		d->CurrentPressedIndex = index;
		d->onButtonClicked(d->Buttons[index]);
	}

	/*!
		\since Visindigo 0.13.0
		获取当前被选中的按钮。这个函数会返回当前被选中的按钮的指针，如果没有按钮被选中则返回nullptr。
	*/
	MultiButton* MultiButtonGroup::getSelectedButton() const {
		return d->CurrentPressedButton;
	}

	/*!
		\since Visindigo 0.14.0
		获取组内的所有按钮。这个函数会返回一个包含组内所有按钮指针的列表。
	*/
	QList<MultiButton*> MultiButtonGroup::getMultiButtons() const {
		return d->Buttons;
	}
}