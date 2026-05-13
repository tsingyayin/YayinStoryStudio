#include "Editor/ToolWidgetManager.h"
#include "Editor/EditorPlugin.h"
#include <QtCore/qmap.h>
#include <QtWidgets/qwidget.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
namespace YSSCore::Editor {
	class ToolWidgetManagerPrivate {
		friend class ToolWidgetManager;
	protected:
		QMap<QString, EditorPlugin*> WidgetProviderMap;
		QMap<QString, QString> WidgetNameMap;
		QMap<QString, QWidget*> WidgetInstanceMap;
		static ToolWidgetManager* Instance;
	};

	ToolWidgetManager* ToolWidgetManagerPrivate::Instance = nullptr;

	/*!
		\class YSSCore::Editor::ToolWidgetManager
		\brief 此类管理由插件提供的工具窗口部件。
		\since YSS 0.15.0
		\inmodule YSSCore

		ToolWidgetManager是一个单例类，负责管理由插件提供的工具窗口部件。
		插件可以通过registerToolWidget函数注册它们提供的工具窗口部件，
		YSS编辑器在需要显示工具窗口内容时会调用requestToolWidget
		函数来获取对应ID的工具窗口部件实例。
	*/

	/*!
		\fn void ToolWidgetManager::focusRequested(const QString& widgetID)
		\since 0.15.0
		触发此信号，请求前台将具有ID \a widgetID 的工具窗口部件聚焦。
	*/

	/*!
		\since 0.15.0
		构造函数
	*/
	ToolWidgetManager::ToolWidgetManager() {
		d = new ToolWidgetManagerPrivate();
	}

	/*!
		\since 0.15.0
		析构函数
	*/
	ToolWidgetManager::~ToolWidgetManager() {
		delete d;
	}

	/*!
		\since 0.15.0
		获取ToolWidgetManager实例的静态函数
	*/
	ToolWidgetManager* ToolWidgetManager::getInstance() {
		if (!ToolWidgetManagerPrivate::Instance) {
			ToolWidgetManagerPrivate::Instance = new ToolWidgetManager();
		}
		return ToolWidgetManagerPrivate::Instance;
	}

	/*!
		\since 0.15.0
		注册工具窗口部件的函数。\a widgetID 是工具窗口的唯一ID，
		\a widgetName 是工具窗口的显示名称，\a provider 是提供此工具窗口的插件。

		当后来者的ID重复时，将被直接忽略。
	*/
	void ToolWidgetManager::registerToolWidget(const QString& widgetID, const QString& widgetName, EditorPlugin* provider) {
		if (d->WidgetProviderMap.contains(widgetID)) {
			vgErrorF << "Tool widget with ID" << widgetID << "is already registered. Ignoring.";
			return;
		}
		d->WidgetProviderMap[widgetID] = provider;
		d->WidgetNameMap[widgetID] = widgetName;
	}

	/*!
		\since 0.15.0
		请求工具窗口部件实例的函数。\a widgetID 是被请求的工具窗口ID。

		当请求一个工具窗口时，ToolWidgetManager会首先检查是否已经存在对应ID的工具窗口实例，
		如果存在则直接返回该实例；否则会检查是否有插件提供了对应ID的工具窗口，
		如果有则调用插件的onToolWidgetRequested函数来获取工具窗口实例，

		如果插件返回了一个有效的QWidget指针，则ToolWidgetManager会将其保存并返回；
		如果没有插件提供或者插件返回了nullptr，则返回nullptr。
	*/
	QWidget* ToolWidgetManager::requestToolWidget(const QString& widgetID) {
		if (d->WidgetInstanceMap.contains(widgetID)) {
			return d->WidgetInstanceMap[widgetID];
		}
		if (d->WidgetProviderMap.contains(widgetID)) {
			auto widget = d->WidgetProviderMap[widgetID]->onToolWidgetRequested(widgetID);
			if (widget) {
				widget->setAttribute(Qt::WA_DeleteOnClose);
				widget->setWindowTitle(VI18N(d->WidgetNameMap[widgetID]));
				d->WidgetInstanceMap[widgetID] = widget;
				QObject::connect((QObject*)widget, &QObject::destroyed, [this, widgetID]() {
					d->WidgetInstanceMap.remove(widgetID);
					});
			}
			return widget;
		}
		else {
			return nullptr;
		}
	}
}