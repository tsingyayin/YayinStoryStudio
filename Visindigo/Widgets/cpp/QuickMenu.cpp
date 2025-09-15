#include "../QuickMenu.h"
#include <QtCore/qstring.h>
#include <QtCore/qobject.h>
#include <QtWidgets/qaction.h>
#include <QtWidgets/qmenu.h>
#include "../../Utility/JsonConfig.h"
#include "../../General/TranslationHost.h"
#include "../../Utility/QtSSHelper.h"
#include "../../General/Log.h"
namespace Visindigo::Widgets {
	class QuickMenuPrivate {
		friend class QuickMenu;
	protected:
		QList<QMenu*> Menus;
		QObject* ActionHandler = nullptr;
		QString LastJson;
	};

	QuickMenu::QuickMenu(QWidget* parent) :QMenuBar(parent) {
		d = new QuickMenuPrivate;
	}
	QuickMenu::~QuickMenu() {
		delete d;
	}
	void QuickMenu::setActionHandler(QObject* handler) {
		d->ActionHandler = handler;
	}
	void QuickMenu::loadFromJson(const QString& jsonStr) {
		if (jsonStr == d->LastJson) {
			return;
		}
		d->LastJson = jsonStr;
		Utility::JsonConfig config;
		config.parse(jsonStr);
		this->clear();
		d->Menus.clear();
		QStringList keys = config.keys("menu");
		for (auto key : keys) {
			QMenu* menu = new QMenu(this);
			yDebug << "Creating menu:" << key;
			QString id = config.getString("menu." + key + ".ID");
			menu->setObjectName(id);
			menu->setTitle(YSSI18N(config.getString("menu." + key + ".Name")));
			QStringList actionGroupKeys = config.keys("menu." + key + ".Actions");
			for (auto actionGroupKey : actionGroupKeys) {
				QString agid = config.getString("menu." + key + ".Actions." + actionGroupKey + ".ID");
				QStringList actionKeys = config.keys("menu." + key + ".Actions." + actionGroupKey + ".Actions");
				for (auto actionKey : actionKeys) {
					QString aid = config.getString("menu." + key + ".Actions." + actionGroupKey + ".Actions." + actionKey + ".ID");
					QString fullID = id + "::" + agid + "::" + aid;
					QAction* action = new QAction(menu);
					action->setObjectName(fullID);
					action->setText(YSSI18N(config.getString("menu." + key + ".Actions." + actionGroupKey + ".Actions." + actionKey + ".Name")));
					// shortcut not support yet.
					menu->addAction(action);
					Utility::QtSSHelper::connect(action, QString("triggered()"), d->ActionHandler, QString(fullID + "()"));
				}
				if (actionGroupKey != actionGroupKeys.last()) {
					menu->addSeparator();
				}
			}
			this->addMenu(menu);
		}
	}
}