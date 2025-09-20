#pragma once
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
namespace Visindigo::Widgets
{
	class MultiButton;
	class MultiButtonGroup;
}
namespace Visindigo::__Private__ {
	class MultiButtonGroupPrivate :public QObject {
		friend class Visindigo::Widgets::MultiButtonGroup;
		Q_OBJECT;
	protected:
		Visindigo::Widgets::MultiButton* CurrentPressedButton = nullptr;
		qint32 CurrentPressedIndex = -1;
		QList<Visindigo::Widgets::MultiButton*> Buttons;
		Visindigo::Widgets::MultiButtonGroup* q;
		MultiButtonGroupPrivate(Visindigo::Widgets::MultiButtonGroup* q);
		void onButtonClicked(Visindigo::Widgets::MultiButton* button);
		void onButtonReleased(Visindigo::Widgets::MultiButton* button);
		void onButtonHovered(Visindigo::Widgets::MultiButton* button);
		void onButtonLeft(Visindigo::Widgets::MultiButton* button);
	};
}