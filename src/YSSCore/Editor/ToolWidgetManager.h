#ifndef YSSCore_Editor_ToolWidgetManager_h
#define YSSCore_Editor_ToolWidgetManager_h
#include "YSSCoreCompileMacro.h"
#include <QtCore/qstringlist.h>
#include <QtCore/qobject.h>
class QWidget;

namespace YSSCore::Editor {
	class EditorPlugin;
	class ToolWidgetManagerPrivate;
	class YSSCoreAPI ToolWidgetManager :public QObject{
		Q_OBJECT;
	signals:
		void focusRequested(const QString& widgetID);
		void widgetOpened(const QString& widgetID);
		void widgetClosed(const QString& widgetID);
	private:
		ToolWidgetManager();
	public:
		static ToolWidgetManager* getInstance();
		~ToolWidgetManager();
		void registerToolWidget(const QString& widgetID, const QString& widgetName, EditorPlugin* provider);
		void openToolWidget(const QString& widgetID);
		QWidget* getToolWidget(const QString& widgetID);
		QList<QWidget*> getAllOpenToolWidgets() const;
		bool isToolWidgetOpen(const QString& widgetID) const;
		QMap<QString, QString> getRegisteredToolWidgets() const;
	private:
		ToolWidgetManagerPrivate* d;
	};
}

#define YSSTWM YSSCore::Editor::ToolWidgetManager::getInstance()
#endif // YSSCore_Editor_ToolWidgetManager_h