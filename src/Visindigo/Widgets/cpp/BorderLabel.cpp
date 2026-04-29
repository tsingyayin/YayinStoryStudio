#include "Widgets/BorderLabel.h"
#include <QtGui/qpainter.h>
#include <QtWidgets/qstyle.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qstyleoption.h>
namespace Visindigo::Widgets {
	/*!
		\class Visindigo::Widgets::BorderLabel
		\brief 这是个神奇的类，在Windows上的某些情况下特别有用
		\since Visindigo 0.14.0
		\inmodule Visindigo

		这是个神奇的类：它本质是QLabel，未提供任何其他功能，它只做一件事，
		默认画一个现代边框（在Windows11上看起来像WinUI3中的边框）。

		这在Windows（尤其是Windows11）上特别有用，因为在Qt中缺少一种能够模仿
		WinUI3中现代边框的组件，而Qt在Windows11上代理绘制的某些窗口实际上
		正好与这种边框类似，因此这个类就被设计出来了，以满足在Windows11上实现现代边框的需求。

		你可以如同使用QLabel一样使用它，但如果你也需要重载paintEvent，记得先
		调用Visindigo::Widgets::BorderLabel::paintEvent来绘制边框，否则你就只能得到一个普通的QLabel了。
	*/

	/*!
		\since Visindigo 0.14.0
		BorderLabel的构造函数。
	*/
	BorderLabel::BorderLabel(QWidget* parent) : QLabel(parent) {

	}

	/*!
		\since Visindigo 0.14.0
		BorderLabel的析构函数。
	*/
	BorderLabel::~BorderLabel() {
	}

	/*!
		\since Visindigo 0.14.0
		BorderLabel的paintEvent函数。
	*/
	void BorderLabel::paintEvent(QPaintEvent* event) {
		QPainter painter(this);
		QStyleOptionFrame opLabel;
		opLabel.initFrom(this);
		opLabel.features = QStyleOptionFrame::Rounded;
		opLabel.frameShape = QFrame::Box;
		this->style()->drawPrimitive(QStyle::PE_Frame, &opLabel, &painter, this);
		QLabel::paintEvent(event);
	}
}