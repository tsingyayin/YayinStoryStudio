#pragma once
#include <QtCore/qobject.h>
#include "../Macro.h"
// Forward declarations
class QString;
class QWidget;
namespace YSSCore::Editor {
	class FileServer;
	class FileEditWidget;
	class FileServerManagerPrivate;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI FileServerManager :public QObject {
		Q_OBJECT;
		friend class FileServerManagerPrivate;
	signals:
		void builtinEditorCreated(FileEditWidget* editor);
		void windowEditorCreated(QWidget* editor);
	private:
		FileServerManager();
	public:
		static FileServerManager* getInstance();
		~FileServerManager();
		void registerFileServer(FileServer* server);
		void unregisterFileServer(FileServer* server);
		bool openFile(const QString& filePath);
	private:
		FileServerManagerPrivate* d;
	};
}

#define YSSFSM YSSCore::Editor::FileServerManager::getInstance()