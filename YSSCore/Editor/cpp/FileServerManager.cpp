#include "../FileServerManager.h"
#include "../FileServer.h"
#include "../TextEdit.h"
#include <QtCore/qstring.h>
#include <QtCore/qfileinfo.h>
#include "../../General/Log.h"

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
		d = new FileServerManagerPrivate();
		FileServerManagerPrivate::Instance = this;
		ySuccessF << "Success!";
	}
	FileServerManager::~FileServerManager() {
		for (int i = 0; i < d->FileServers.size(); i++) {
			delete d->FileServers[i];
		}
		d->FileServers.clear();
		delete d;
	}
	FileServerManager* FileServerManager::getInstance() {
		return FileServerManagerPrivate::Instance;
	}
	void FileServerManager::registerFileServer(FileServer* server) {
		if (d->FileServers.contains(server)) {
			return;
		}
		d->FileServers.append(server);
		QStringList fileTypes = server->getSupportedFileExts();
		for (int i = 0; i < fileTypes.size(); i++) {
			QString type = fileTypes[i];
			if (!d->FileServerMap.contains(type)) {
				d->FileServerMap.insert(type, QList<FileServer*>());
			}
			d->FileServerMap[type].append(server);
		}
	}
	void FileServerManager::unregisterFileServer(FileServer* server) {
		if (!d->FileServers.contains(server)) {
			return;
		}
		d->FileServers.removeOne(server);
		QStringList fileTypes = server->getSupportedFileExts();
		for (int i = 0; i < fileTypes.size(); i++) {
			QString type = fileTypes[i];
			if (d->FileServerMap.contains(type)) {
				d->FileServerMap[type].removeOne(server);
			}
		}
	}
	bool FileServerManager::openFile(const QString& filePath) {
		QString ext = QFileInfo(filePath).suffix();
		if (d->FileServerMap.contains(ext)) {
			FileServer* server = d->FileServerMap[ext][0]; // implement use 0, will be changed later
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
			
		} else {
			FileEditWidget* feWidget = new TextEdit();
			bool ok = feWidget->openFile(filePath);
			if (ok) {
				emit builtinEditorCreated(feWidget);
				return true;
			}
		}
		return false;
	}
}