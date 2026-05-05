#ifndef Visindigo_Widgets_ShortcutSet_h
#define Visindigo_Widgets_ShortcutSet_h
#include <QtGui/qshortcut.h>
#include "VICompileMacro.h"
#include <QtCore/qmap.h>

namespace Visindigo::Widgets {
	class ShortcutSetPrivate;
	class VisindigoAPI ShortcutSet :public QObject {
		Q_OBJECT;
	signals:
		void actionTriggered(const QString& actionName);
	public:
		ShortcutSet(QWidget* parent);
		virtual ~ShortcutSet();
		QWidget* getWatchedWidget() const;
		void addShortcut(const QString& actionName, const QKeySequence& shortcut);
		void addShortcut(const QString& actionName, QKeySequence::StandardKey shortcut);
		QMap<QKeySequence, QString> getShortcuts() const;
		QString getActionNameOfShortcut(const QKeySequence& shortcut) const;
		QList<QKeySequence> getShortcutOfActionName(const QString& actionName) const;
		void clearShortcuts();
	private:
		ShortcutSetPrivate* d;
	};
}

#endif // Visindigo_Widgets_ShortcutSet_h