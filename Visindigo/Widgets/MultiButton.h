#pragma once
#include "MultiLabel.h"
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
		bool isHovered();
	protected:
		void mousePressEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;
		void mouseDoubleClickEvent(QMouseEvent* event) override;
		void enterEvent(QEnterEvent* event) override;
		void leaveEvent(QEvent* event) override;
	protected:
		Visindigo::__Private__::MultiButtonPrivate* d;
	};
}