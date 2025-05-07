#pragma once
#include <QtCore/QObject>
#include <QtCore/QString>
#include "../Macro.h"

class QWidget;
namespace YSSCore::Editor {
	class FileServer;
	class FileEditWidget;
	class FileServerManagerPrivate;
	class YSSCoreAPI FileServerManager :public QObject {
		Q_OBJECT;
		friend class FileServerManagerPrivate;
	signals:
		void builtinEditorCreated(FileEditWidget* editor);
		void windowEditorCreated(QWidget* editor);
	public:
		static FileServerManager* getInstance();
		FileServerManager();
		~FileServerManager();
		void registerFileServer(FileServer* server);
		void unregisterFileServer(FileServer* server);
		bool openFile(const QString& filePath);
	private:
		FileServerManagerPrivate* p;
	};
}

#define YSSFSM YSSCore::Editor::FileServerManager::getInstance()