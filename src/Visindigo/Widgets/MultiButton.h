#ifndef Visindigo_Widgets_MultiButton_h
#define Visindigo_Widgets_MultiButton_h
#include "Widgets/MultiLabel.h"
// Forward declarations
namespace Visindigo::__Private__ {
	class MultiButtonPrivate;
	class MultiButtonGroupPrivate;
}
namespace Visindigo::Widgets {
	class MultiButtonGroup;
}
// Main
namespace Visindigo::Widgets {
	class VisindigoAPI MultiButton :public MultiLabel {
		friend class MultiButtonGroup;
		friend class Visindigo::__Private__::MultiButtonGroupPrivate;
		Q_OBJECT;
	signals:
		void clicked();
		void doubleClicked();
		void pressed();
		void released();
		void hover();
		void leave();
	public:
		MultiButton(QWidget* parent = nullptr);
		virtual ~MultiButton();
		void setHoverStyleSheet(const QString& styleSheet);
		void setPressedStyleSheet(const QString& styleSheet);
		void setNormalStyleSheet(const QString& styleSheet);
		void setInactiveStyleSheet(const QString& styleSheet);
		void setActive(bool active);
		bool inButtonGroup();
		bool isActive();
		bool isHovered();
	protected:
		virtual void paintEvent(QPaintEvent* event) override;
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseReleaseEvent(QMouseEvent* event) override;
		virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
		virtual void enterEvent(QEnterEvent* event) override;
		virtual void leaveEvent(QEvent* event) override;
	protected:
		Visindigo::__Private__::MultiButtonPrivate* d;
	};
}

#endif // Visindigo_Widgets_MultiButton_h