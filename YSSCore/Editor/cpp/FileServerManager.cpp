#include "../FileServerManager.h"
#include "../FileServer.h"
#include "../TextEdit.h"
namespace YSSCore::Editor {
	class FileServerManagerPrivate {
		friend class FileServerManager;
	protected:
		QList<FileServer*> FileServers;
		QMap<QString, QList<FileServer*>> FileServerMap;
		static FileServerManager* Instance;
	};
	FileServerManager* FileServerManagerPrivate::Instance = nullptr;
	FileServerManager::FileServerManager() {
		p = new FileServerManagerPrivate();
		FileServerManagerPrivate::Instance = this;
	}
	FileServerManager::~FileServerManager() {
		for (int i = 0; i < p->FileServers.size(); i++) {
			delete p->FileServers[i];
		}
		p->FileServers.clear();
		delete p;
	}
	FileServerManager* FileServerManager::getInstance() {
		return FileServerManagerPrivate::Instance;
	}
	void FileServerManager::registerFileServer(FileServer* server) {
		if (p->FileServers.contains(server)) {
			return;
		}
		p->FileServers.append(server);
		QStringList fileTypes = server->getSupportedFileExts();
		for (int i = 0; i < fileTypes.size(); i++) {
			QString type = fileTypes[i];
			if (!p->FileServerMap.contains(type)) {
				p->FileServerMap.insert(type, QList<FileServer*>());
			}
			p->FileServerMap[type].append(server);
		}
	}
	void FileServerManager::unregisterFileServer(FileServer* server) {
		if (!p->FileServers.contains(server)) {
			return;
		}
		p->FileServers.removeOne(server);
		QStringList fileTypes = server->getSupportedFileExts();
		for (int i = 0; i < fileTypes.size(); i++) {
			QString type = fileTypes[i];
			if (p->FileServerMap.contains(type)) {
				p->FileServerMap[type].removeOne(server);
			}
		}
	}
	bool FileServerManager::openFile(const QString& filePath) {
		QString ext = QFileInfo(filePath).suffix();
		if (p->FileServerMap.contains(ext)) {
			FileServer* server = p->FileServerMap[ext][0]; // implement use 0, will be changed later
			if (server != nullptr) {
				bool ok;
				FileEditWidget* feWidget = nullptr;
				QWidget* widget = nullptr;
				switch (server->getEditorType()) {
				case FileServer::CodeEditor:
					feWidget = new TextEdit();
					ok = feWidget->openFile(filePath);
					if (ok) {
						emit builtinEditorCreated(feWidget);
						return true;
					}
					else {
						delete feWidget;
						return false;
					}
					break;
				case FileServer::BuiltInEditor:
					feWidget = server->createFileEditWidget();
					if (feWidget != nullptr) {
						ok = feWidget->openFile(filePath);
						if (ok) {
							emit builtinEditorCreated(feWidget);
							return true;
						}
						else {
							delete widget;
							return false;
						}
					}
					else {
						delete widget;
						return false;
					}
					break;
				case FileServer::WindowEditor:
					widget = server->createFileEditWidget();
					if (widget != nullptr) {
						ok = server->openFile(filePath);
						if (ok) {
							emit windowEditorCreated(widget);
						}
						else {
							delete widget;
							return false;
						}
					}
					else {
						delete widget;
						return false;
					}
					break;
				case FileServer::ExternalProgram:
				case FileServer::OtherEditor:
					return server->openFile(filePath);
				}
			}
		}
		return false;
	}
}