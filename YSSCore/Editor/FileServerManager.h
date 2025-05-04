#pragma once
#include <QtCore>
#include "../Macro.h"
#include "FileServer.h"
namespace YSSCore::Editor {
	class FileServerManagerPrivate;
	class YSSCoreAPI FileServerManager :public QObject{
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