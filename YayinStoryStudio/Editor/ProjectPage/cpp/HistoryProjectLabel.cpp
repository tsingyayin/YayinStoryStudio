#include "../HistoryProjectLabel.h"
#include <QLabel>
#include <QGridLayout>

namespace YSS::ProjectPage {
	HistoryProjectLabel::HistoryProjectLabel(QWidget* parent) :QFrame(parent) {};
	HistoryProjectLabel::~HistoryProjectLabel() {};
	void HistoryProjectLabel::mousePressEvent(QMouseEvent* event) {};
	void HistoryProjectLabel::mouseDoubleClickEvent(QMouseEvent* event) {};
}