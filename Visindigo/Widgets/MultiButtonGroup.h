#pragma once
#include <QtCore/qobject.h>
// Forward declarations
namespace Visindigo::__Private__ {
	class MultiButtonGroupPrivate;
}
namespace Visindigo::Widgets {
	class MultiButton;
	class MultiButtonGroup : public QObject {
		Q_OBJECT;
		friend class Visindigo::__Private__::MultiButtonGroupPrivate;
	signals:
		void clicked(MultiButton* button);
		void doubleClicked(MultiButton* button);
		void pressed(MultiButton* button);
		void released(MultiButton* button);
		void hover(MultiButton* button);
		void leave(MultiButton* button);
		void selectIndexChanged(quint32 index);
	public:
		MultiButtonGroup(QObject* parent = nullptr);
		void addButton(MultiButton* button);
		void removeAll();
		qint32 selectPrevious();
		qint32 selectNext();
		qint32 selectButton(MultiButton* button);
		void selectButton(quint32 index);
	private:
		Visindigo::__Private__::MultiButtonGroupPrivate* d;
	};
}