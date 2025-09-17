#pragma once
#include "../Macro.h"
#include <QtWidgets/qframe.h>
#include <QtWidgets/qmenubar.h>
// Forward declarations
class QString;
namespace Visindigo::Widgets {
	class QuickMenuPrivate;
}
// Main
namespace Visindigo::Widgets {
	class VisindigoAPI QuickMenu :public QMenuBar {
		Q_OBJECT;
	public:
		QuickMenu(QWidget* parent = nullptr);
		~QuickMenu();
		void setActionHandler(QObject* handler);
		void loadFromJson(const QString& jsonStr);
		QString saveToJson() const;
	private:
		QuickMenuPrivate* d;
	};
}
// Helper macros
#define HandlerNode(ClassName, NodeName) \
public: \
	ClassName(QObject* parent = nullptr) : QObject(parent) { \
		this->setObjectName(NodeName); \
		__createSub(); \
	}
#define SubNodes\
	void __createSub()