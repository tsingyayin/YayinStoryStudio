#include "Widgets/ShortcutSet.h"

namespace Visindigo::Widgets {
	class ShortcutSetPrivate {
		friend class ShortcutSet;
	protected:
		QWidget* WatchedWidget = nullptr;
		QMap<QKeySequence, QString> ShortcutActionMap;
		QList<QShortcut*> Shortcuts;
	};

	/*!
		\class Visindigo::Widgets::ShortcutSet
		\brief ShortcutSet基于QShortcut实现了一个更快捷的集合
		\since Visindigo 0.14.0
		\inmodule Visindigo

		ShortcutSet类基于QShortcut实现了一个更快捷的集合，用户可以通过addShortcut方法添加快捷键，
		并且可以通过actionTriggered信号监听快捷键的触发事件。每个快捷键都与一个字符串类型的动作名称相关联，
		当快捷键被触发时，actionTriggered信号会传递对应的动作名称。

		使用这个类有潜在的内存泄露风险，因为该类本身和其创建的QShortcut对象均为被监听对象的同级子对象，
		因此这个类不能在它自己被销毁时尝试销毁其保存的QShortcut指针（无法断定保存的指针是否仍然有效）。
		这个类在销毁时，是指望Qt的父子对象机制来销毁所有的QShortcut对象的，

		所以，如果在添加了快捷键之后不做任何其他操作，任由此类随着被监听对象销毁，那么就不存在内存泄露风险。
		其内部任何QShortcut指针最终都会由Qt释放。

		但如果你出于某种原因，必须先于被监听对象销毁此类，那么你就必须在销毁此类之前手动调用clearShortcuts
		方法来销毁所有的QShortcut对象，以避免内存泄露。
	*/

	/*!
		\fn void ShortcutSet::actionTriggered(const QString& actionName)
		\since Visindigo 0.14.0
		当快捷键被触发时发出，传递对应的动作名称
	*/

	/*!
		\since Visindigo 0.14.0
		构造函数。

		虽然Qt要求QShortcut的父对象可以是QWidget或QWindow，但目前暂时只提供QWidget的支持。
	*/
	ShortcutSet::ShortcutSet(QWidget* parent) :QObject((QObject*)parent) {
		d = new ShortcutSetPrivate;
		d->WatchedWidget = parent;
	}


	/*!
		\since Visindigo 0.14.0
		析构函数

		有关你在手动析构前可能需要做的操作，请参见类的文档说明。
	*/
	ShortcutSet::~ShortcutSet() {
		delete d;
	}

	/*!
		\since Visindigo 0.14.0
		获取被监听的QWidget对象
	*/
	QWidget* ShortcutSet::getWatchedWidget() const {
		return d->WatchedWidget;
	}

	/*!
		\since Visindigo 0.14.0
		添加一个快捷键与动作名称的关联
		\a actionName 动作名称
		\a shortcut 快捷键
	*/
	void ShortcutSet::addShortcut(const QString& actionName, const QKeySequence& shortcut) {
		d->ShortcutActionMap[shortcut] = actionName;
		QShortcut* newShortcut = new QShortcut(shortcut, (QObject*)d->WatchedWidget);
		connect(newShortcut, &QShortcut::activated, this, [this, actionName]() {
			emit actionTriggered(actionName);
			});
		d->Shortcuts.append(newShortcut);
	}

	/*!
		\since Visindigo 0.14.0
		添加一个快捷键与动作名称的关联，快捷键使用QKeySequence::StandardKey枚举值
		\a actionName 动作名称
		\a shortcut 快捷键枚举值
	*/
	void ShortcutSet::addShortcut(const QString& actionName, QKeySequence::StandardKey shortcut) {
		addShortcut(actionName, QKeySequence(shortcut));
	}

	/*!
		\since Visindigo 0.14.0
		获取所有的快捷键与动作名称的关联
		返回一个QMap，键为快捷键，值为对应的动作名称
	*/
	QMap<QKeySequence, QString> ShortcutSet::getShortcuts() const {
		return d->ShortcutActionMap;
	}

	/*!
		\since Visindigo 0.14.0
		根据快捷键获取对应的动作名称
		\a shortcut 快捷键
		返回对应的动作名称，如果没有找到则返回空字符串
	*/
	QString ShortcutSet::getActionNameOfShortcut(const QKeySequence& shortcut) const {
		return d->ShortcutActionMap.value(shortcut, QString());
	}

	/*!
		\since Visindigo 0.14.0
		根据动作名称获取对应的快捷键列表
		\a actionName 动作名称
		返回一个QList，包含所有与该动作名称相关联的快捷键
	*/
	QList<QKeySequence> ShortcutSet::getShortcutOfActionName(const QString& actionName) const {
		QList<QKeySequence> result;
		for (auto it = d->ShortcutActionMap.begin(); it != d->ShortcutActionMap.end(); ++it) {
			if (it.value() == actionName) {
				result.append(it.key());
			}
		}
		return result;
	}

	/*!
		\since Visindigo 0.14.0
		清除所有的快捷键与动作名称的关联
		这个方法会删除所有的QShortcut对象，并清空内部的数据结构。请注意，如果你在调用此方法后继续使用此类，
		你需要重新添加快捷键与动作名称的关联，否则将无法触发任何动作。
	*/
	void ShortcutSet::clearShortcuts() {
		for (QShortcut* shortcut : d->Shortcuts) {
			delete shortcut;
		}
		d->Shortcuts.clear();
		d->ShortcutActionMap.clear();
	}
}