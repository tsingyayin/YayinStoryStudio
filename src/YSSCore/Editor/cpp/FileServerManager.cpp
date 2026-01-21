#include <QtCore/qstring.h>
#include <QtCore/qfileinfo.h>
#include <General/Log.h>
#include "../FileServerManager.h"
#include "../FileServer.h"
#include "../TextEdit.h"
#include "General/YSSLogger.h"
namespace YSSCore::Editor {
	class FileServerManagerPrivate {
		friend class FileServerManager;
	protected:
		QList<FileServer*> FileServers;
		QMap<QString, QList<FileServer*>> FileServerMap;
		QMap<QString, QList<FileServer*>> FileServerPriorityMap;
		static FileServerManager* Instance;

		bool openFile(const QString& filePath, FileServer* server) {
			bool ok = false;
			if (server != nullptr) {
				FileEditWidget* feWidget = nullptr;
				QWidget* widget = nullptr;
				switch (server->getEditorType()) {
				case FileServer::CodeEditor:
					feWidget = new TextEdit();
					ok = feWidget->openFile(filePath);
					if (ok) {
						emit Instance->builtinEditorCreated(feWidget);
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
							emit Instance->builtinEditorCreated(feWidget);
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
						emit Instance->windowEditorCreated(widget);
						return true;
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
			return ok;
		}
	};
	FileServerManager* FileServerManagerPrivate::Instance = nullptr;

	/*!
		\class YSSCore::Editor::FileServerManager
		\inmodule YSSCore
		\since Visindigo 0.13.0
		\brief 管理FileServer的对象。

		在用户将自己的FileServer注册到FileServerManager后，FileServerManager可以根据已经注册的支持类型选择合适的FileServer来打开文件。

		打开文件默认按照以下步骤尝试：
		\list
		\li 1. 查找支持该文件类型的FileServer列表。
		\li 2. 依次使用这些FileServer尝试打开文件，直到成功为止。
		\li 3. 如果没有任何FileServer成功打开文件，则使用内置的文本编辑器打开文件（如果启用该选项）。
		\endlist
		请注意，如果有多个FileServer支持同一种文件类型，则FileServerManager会按照注册顺序依次尝试这些FileServer。
		用户可以通过setPriorityForFileExt函数来设置某种文件类型的FileServer优先级，从而改变尝试顺序。

		除此之外，当用户在openFile中指定了preferredServerId参数后，FileServerManager会优先尝试该FileServer。
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
		\since Visindigo 0.13.0
		获取FileServerManager的单例对象。
	*/
	FileServerManager* FileServerManager::getInstance() {
		if (FileServerManagerPrivate::Instance == nullptr) {
			FileServerManagerPrivate::Instance = new FileServerManager();
		}
		return FileServerManagerPrivate::Instance;
	}

	/*!
		\since Visindgo 0.13.0
		注册一个FileServer对象。
		\a server 要注册的FileServer对象指针。
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
		\since Visindigo 0.13.0
		注销一个已经注册的FileServer对象。
		\a server 要注销的FileServer对象指针。
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
		\since Visindgo 0.13.0
		打开一个文件。如果有合适的FileServer注册，则使用该FileServer打开文件，否则使用内置的文本编辑器打开文件。
		\a filePath 要打开的文件路径。
		\a preferredServerId 优先使用的FileServer ID。如果该参数不为空，则FileServerManager会优先尝试该FileServer打开文件。
		\a useFallback 是否在没有任何FileServer成功打开文件时使用内置文本编辑器打开文件。默认为true。
		如果文件成功打开则返回true，否则返回false.
	*/
	bool FileServerManager::openFile(const QString& filePath, const QString& preferredServerId, bool useFallback) {
		QString ext = QFileInfo(filePath).suffix();
		if (d->FileServerMap.contains(ext)) { // means supported
			QList<FileServer*> servers = d->FileServerMap[ext];
			if (d->FileServerPriorityMap.contains(ext)) {
				servers = d->FileServerMap[ext];
			}
			if (!preferredServerId.isEmpty()) {
				FileServer* server = nullptr;
				for (auto s : servers) {
					if (s->getModuleID() == preferredServerId) {
						server = s;
						break;
					}
				}
				servers.prepend(server);
			}
			for (auto server : servers) {
				if (d->openFile(filePath, server)) {
					return true;
				}
			}
		}
		if (useFallback) { // use builtin editor
			FileEditWidget* feWidget = new TextEdit();
			bool ok = feWidget->openFile(filePath);
			if (ok) {
				emit builtinEditorCreated(feWidget);
				return true;
			}
		}
		return false;
	}

	/*!
		\since Visindigo 0.13.0
		获取所有已注册的FileServer支持的文件类型列表。
	*/
	QStringList FileServerManager::getSupportedFileExts() {
		return d->FileServerMap.keys();
	}

	/*!
		\since Visindigo 0.13.0
		获取某种文件类型对应的可用FileServer列表。
		\a fileExt 文件类型后缀名（不含点号）。
		\a 返回某种文件类型对应的可用FileServer ID列表。
	*/
	QStringList FileServerManager::getAvailableFileServerForFileExt(const QString& fileExt) {
		QStringList serverIds;
		if (d->FileServerMap.contains(fileExt)) {
			QList<FileServer*> servers = d->FileServerMap[fileExt];
			for (int i = 0; i < servers.size(); i++) {
				serverIds.append(servers[i]->getModuleID());
			}
		}
		return serverIds;
	}

	/*!
		\since Visindigo 0.13.0
		为某种文件类型设置FileServer优先级。
		\a fileExt 文件类型后缀名（不含点号）。
		\a serverIds 按优先级顺序排列的FileServer ID列表。
	*/
	void FileServerManager::setPriorityForFileExt(const QString& fileExt, const QStringList& serverIds) {
		if (!d->FileServerMap.contains(fileExt)) {
			return;
		}
		QList<FileServer*> servers;
		QList<FileServer*> registeredServers = d->FileServerMap[fileExt];
		for (int i = 0; i < serverIds.size(); i++) {
			QString serverId = serverIds[i];
			for (int j = 0; j < registeredServers.size(); j++) {
				if (registeredServers[j]->getModuleID() == serverId) {
					servers.append(registeredServers[j]);
					break;
				}
			}
		}
	}
}