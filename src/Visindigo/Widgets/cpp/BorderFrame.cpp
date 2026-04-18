#include "Widgets/BorderFrame.h"
#include <QtGui/qpainter.h>
#include <QtWidgets/qstyle.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qstyleoption.h>
namespace Visindigo::Widgets {
	/*!
		\class BorderFrame
		\brief 这是个神奇的类，在Windows上的某些情况下特别有用
		\since Visindigo 0.14.0
		\inmodule Visindigo

		这是个神奇的类：它本质是QFrame，未提供任何其他功能，它只做一件事，
		用QPushButton的绘制方式来绘制自己。

		这在Windows（尤其是Windows11）上特别有用，因为在Qt中缺少一种能够模仿
		WinUI3中现代边框的组件，而Qt在Windows11上代理绘制的QPushButton实际上
		正好与这种边框类似，因此这个类就被设计出来了，以满足在Windows11上实现现代边框的需求。

		你可以如同使用QFrame一样使用它，但如果你也需要重载paintEvent，记得先
		调用BorderFrame::paintEvent来绘制边框，否则你就只能得到一个普通的QFrame了。
	*/

	/*!
		\since Visindigo 0.14.0
		BorderFrame的构造函数。
	*/
	BorderFrame::BorderFrame(QWidget* parent) : QFrame(parent) {
		setFrameStyle(QFrame::Box);
	}

	/*!
		\since Visindigo 0.14.0
		BorderFrame的析构函数。
	*/
	BorderFrame::~BorderFrame() {
	}

	/*!
		\since Visindigo 0.14.0
		BorderFrame的paintEvent函数。
	*/
	void BorderFrame::paintEvent(QPaintEvent* event) {
		QPainter painter(this);
		QStyleOptionButton option;
		option.initFrom(this);
		this->style()->drawControl(QStyle::CE_PushButton, &option, &painter, this);
	}
}