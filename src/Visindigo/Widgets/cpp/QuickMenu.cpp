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

	/*!
		\class Visindigo::Widgets::QuickMenu
		\brief QuickMenu提供了一种从JSON快速创建菜单的方式。
		\since Visindigo 0.13.0
		\inmodule Visindigo

		QuickMenu提供了一种从JSON快速创建菜单的方式。
		通过调用loadFromJson函数，传入符合规范的JSON字符串，即可动态生成菜单结构，
		并将菜单项的触发事件连接到指定的槽函数。QuickMenu适用于需要根据配置文件或运行时数据动态构建菜单的场景，
		极大地简化了菜单创建和管理的复杂性。

		\section1 JSON规范
		可以被解析的Json需遵循如下格式：
		\badcode
		{
			"menu": [
				{
					"ID": "__MenuID__",
					"Name": "__i18n_MenuName__",
					"Actions": [
						{
							"ID": "__ActionGroupID__",
							"Actions":[
								{
									"ID": "__ActionID__",
									"Name": "__i18n_ActionName__",
								}
							]
						}
					]
				}
			]
		}
		\endcode
		也就是说，JSON的根对象必须包含一个名为"menu"的数组，每个元素代表一个菜单。每个菜单对象必须包含"ID"和"Name"属性。
		"ID"属性用于唯一标识菜单，"Name"属性支持国际化，以i18n:开头时自动调用VITR翻译功能。在每个菜单内，通过
		Actions设置动作组，每个动作组下包含若干动作。动作组之间自动生成一条横线分割它们。

		\section1 动作处理器
		QuickMenu通过setActionHandler函数设置一个QObject对象作为动作处理器。
		这个QObject对象应该具有和设置的Json等同的子对象树结构，且子对象的objectName和Json中的ID相同。
		QuickMenu会自动将菜单项的triggered信号连接到动作处理器中对应ID的槽函数。
		例如，菜单“File”下的动作组“New”中的动作“Project”，需要设置的handler中有
		一个名为File的子对象，该子对象又拥有一个名为New的子对象，而New子对象又有一个名为Project的槽函数。
		当用户点击这个菜单项时，QuickMenu会自动调用handler->File->New->Project()函数。

		注意这里说的名为均为QObject::setObjectName()，并非其在代码中的变量名。
		此外，这要求槽函数使用public slots:显式的标记，否则将找不到该函数。
	*/

	/*!
		\since Visindigo 0.13.0
		构造函数
	*/
	QuickMenu::QuickMenu(QWidget* parent) :QMenuBar(parent) {
		d = new QuickMenuPrivate;
	}

	/*!
		\since Visindigo 0.13.0
		析构函数
	*/
	QuickMenu::~QuickMenu() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		设置动作处理器。动作处理器是一个QObject对象，QuickMenu会将菜单项的triggered信号连接到动作处理器中对应ID的槽函数。
	*/
	void QuickMenu::setActionHandler(QObject* handler) {
		d->ActionHandler = handler;
	}

	/*!
		\since Visindigo 0.13.0
		从Json字符串加载菜单结构。Json字符串必须符合QuickMenu的规范，否则可能会导致解析失败或运行时错误。
	*/
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
			//vgDebugF << "Creating menu:" << key;
			QString id = config.getString("menu." + key + ".ID");
			menu->setObjectName(id);
			menu->setTitle(VI18N(config.getString("menu." + key + ".Name")));
			QStringList actionGroupKeys = config.keys("menu." + key + ".Actions");
			for (auto actionGroupKey : actionGroupKeys) {
				QString agid = config.getString("menu." + key + ".Actions." + actionGroupKey + ".ID");
				QStringList actionKeys = config.keys("menu." + key + ".Actions." + actionGroupKey + ".Actions");
				for (auto actionKey : actionKeys) {
					QString aid = config.getString("menu." + key + ".Actions." + actionGroupKey + ".Actions." + actionKey + ".ID");
					QString fullID = id + "::" + agid + "::" + aid;
					QAction* action = new QAction(menu);
					action->setObjectName(fullID);
					QString text = VI18N(config.getString("menu." + key + ".Actions." + actionGroupKey + ".Actions." + actionKey + ".Name"));
					if (config.keys("menu." + key + ".Actions." + actionGroupKey + ".Actions." + actionKey).contains("Shortcut")) {
						text += "(" + config.getStringList("menu." + key + ".Actions." + actionGroupKey + ".Actions." + actionKey + ".Shortcut").join("+") + ")";
					}
					action->setText(text);
					// shortcut not support yet.
					menu->addAction(action);
					Utility::QtSSHelper::deepConnect(action, QString("triggered()"), d->ActionHandler, QString(fullID + "()"));
					//Utility::QtSSHelper::deepConnect(d->ActionHandler, QString(fullID + "_VisibleChanged(bool)"), action, QString("setVisible(bool)"));
				}
				if (actionGroupKey != actionGroupKeys.last()) {
					menu->addSeparator();
				}
				//Utility::QtSSHelper::deepConnect(menu, QString("aboutToShow()"), d->ActionHandler, QString(id + "::" + agid + "::aboutToShow()"));
			}
			this->addMenu(menu);
		}
	}
}