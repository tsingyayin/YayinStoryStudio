#pragma once
#include "MultiLabel.h"

namespace YSSCore::Widgets {
	class MultiButtonPrivate;
	class YSSCoreAPI MultiButton :public MultiLabel {
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
	private:
		MultiButtonPrivate* d;
	};
}