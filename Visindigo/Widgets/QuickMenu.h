#pragma once
#include "../Macro.h"
#include <QtWidgets/qframe.h>
#include <QtWidgets/qmenubar.h>
class QString;
namespace Visindigo::Widgets {
	class QuickMenuPrivate;
	class VisindigoAPI QuickMenu :public QMenuBar{
		Q_OBJECT;
	public:
		QuickMenu(QWidget* parent = nullptr);
		~QuickMenu();
		void setActionHandler(QObject* handler);
		void loadFromJson(const QString& jsonStr);
		void setWidgetParent(QWidget* parent);
		QString saveToJson() const;
	private:
		QuickMenuPrivate* d;
	};
}