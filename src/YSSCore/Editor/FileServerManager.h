#pragma once
#include <QtCore/qobject.h>
#include <QtWidgets/qwidget.h>
#include "Editor/FileEditWidget.h"
#include "../Macro.h"
// Forward declarations
class QString;
namespace YSSCore::Editor {
	class FileServer;
	class FileServerManagerPrivate;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI FileWidgetHandler {
	public:
		virtual bool handleBuiltinEditor(FileEditWidget* editor) = 0;
		virtual bool handleWindowEditor(QWidget* editor) = 0;
	};

	class YSSCoreAPI FileServerManager :public QObject {
		Q_OBJECT;
		friend class FileServerManagerPrivate;
	signals:
		void switchLineEdit(const QString& filePath, quint32 lineNumber, qint32 column);
	private:
		FileServerManager();
	public:
		static FileServerManager* getInstance();
		~FileServerManager();
		void setFileWidgetHandler(FileWidgetHandler* handler);
		void registerFileServer(FileServer* server);
		void unregisterFileServer(FileServer* server);
		bool openFile(const QString& filePath, const QString& preferredServerId = QString(), bool useFallback = true);
		QStringList getSupportedFileExts();
		QStringList getAvailableFileServerForFileExt(const QString& fileExt);
		void setPriorityForFileExt(const QString& fileExt, const QStringList& serverIds);
		void setEspeciallyFocusEnable(const QString& fileExt, bool enable);
		bool isEspeciallyFocusEnable(const QString& fileExt);
	private:
		FileServerManagerPrivate* d;
	};
}

#define YSSFSM YSSCore::Editor::FileServerManager::getInstance()