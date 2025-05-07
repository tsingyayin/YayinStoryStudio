#pragma once
#include <QObject>
#include <QPointF>
#include <QString>
#include <QWidget>

class QLabel;
class QTextBrowser;
class QVBoxLayout;
class QMouseEvent;

namespace YSS::Editor {
	class HoverTip :public QWidget {
		Q_OBJECT;
	private:
		QLabel* ImageLabel;
		QTextBrowser* TextBrowser;
		QVBoxLayout* Layout;
		QPointF ShowPos;
	public:
		HoverTip(QMouseEvent* event);
		void setImage(const QString& path);
		void setMarkdown(const QString& text);
		void show();
	};
}