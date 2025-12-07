#include <QtCore/qstring.h>
#include <QtCore/qfileinfo.h>
#include <General/Log.h>
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

	/*!
		\class YSSCore::Editor::FileServerManager
		\inmodule Visindigo
		\since Visindigo 0.13.0
		\brief 管理FileServer的对象。

		在用户将自己的FileServer注册到FileServerManager后，FileServerManager可以根据已经注册的支持类型选择合适的FileServer来打开文件。
	*/
	
	/*!
		\fn YSSCore::Editor::FileServerManager::builtinEditorCreated(YSSCore::Editor::FileEditWidget* editor)
		当内置编辑器被创建时发出该信号。

		这包括YSSCore::Editor::FileServer::EditorType为CodeEditor和BuiltInEditor的情况。
	*/

	/*!
		\fn YSSCore::Editor::FileServerManager::windowEditorCreated(QWidget* editor)
		当窗口编辑器被创建时发出该信号。
		这只对应YSSCore::Editor::FileServer::EditorType为WindowEditor的情况。
	*/

	/*!
		\since Visindigo 0.13.0
		构造FileServerManager对象。
	*/
	FileServerManager::FileServerManager() {
		d = new FileServerManagerPrivate();
		ySuccessF << "Success!";
	}

	/*!
		\since Visindigo 0.13.0
		析构FileServerManager对象。一般来说，没有任何情况需要手动析构此对象。FileServerManager应该与使用它的应用程序有一致的生命周期。
	*/
	FileServerManager::~FileServerManager() {
		for (int i = 0; i < d->FileServers.size(); i++) {
			delete d->FileServers[i];
		}
		d->FileServers.clear();
		delete d;
	}

	/*!
		获取FileServerManager的单例对象。
		\return 返回FileServerManager的单例对象。
	*/
	FileServerManager* FileServerManager::getInstance() {
		if (FileServerManagerPrivate::Instance == nullptr) {
			FileServerManagerPrivate::Instance = new FileServerManager();
		}
		return FileServerManagerPrivate::Instance;
	}

	/*!
		注册一个FileServer对象。
		\param server 要注册的FileServer对象指针。
		如果同一个FileServer对象被重复注册，则不会有任何效果。
	*/
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

	/*!
		注销一个已经注册的FileServer对象。
		\param server 要注销的FileServer对象指针。
		如果传入的FileServer对象没有被注册，则不会有任何效果。
	*/
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

	/*!
		打开一个文件。如果有合适的FileServer注册，则使用该FileServer打开文件，否则使用内置的文本编辑器打开文件。
		\a filePath 要打开的文件路径。
		\return 如果文件成功打开则返回true，否则返回false。

		当前的实现中，如果有多个FileServer支持同一种文件类型，则总是使用第一个注册的FileServer来打开该类型的文件。

		\warning 这是设计缺陷，未来的版本中会改进这一点。
	*/
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
					feWidget = server->onCreateFileEditWidget();
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
					widget = server->onCreateWindowEditor(filePath);
					if (feWidget != nullptr) {
						emit windowEditorCreated(widget);
					}
					else {
						return false;
					}
					break;
				case FileServer::ExternalProgram:
					return server->onCreateExternalEditor(filePath);
				case FileServer::OtherEditor:
					return server->onOtherOpenFile(filePath);
				}
			}
		}
		else {
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