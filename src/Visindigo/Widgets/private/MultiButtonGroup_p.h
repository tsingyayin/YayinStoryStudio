#ifndef Visindigo_Widgets_private_MultiButtonGroup_p_h
#define Visindigo_Widgets_private_MultiButtonGroup_p_h
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
		Visindigo::Widgets::MultiButton* CurrentHoveredButton = nullptr;
		MultiButtonGroupPrivate(Visindigo::Widgets::MultiButtonGroup* q);
		void onButtonClicked(Visindigo::Widgets::MultiButton* button);
		void onButtonReleased(Visindigo::Widgets::MultiButton* button);
		void onButtonHovered(Visindigo::Widgets::MultiButton* button);
		void onButtonLeft(Visindigo::Widgets::MultiButton* button);
	};
}
#endif // Visindigo_Widgets_private_MultiButtonGroup_p_h
