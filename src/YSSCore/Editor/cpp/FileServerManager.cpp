#include <QtCore/qstring.h>
#include <QtCore/qfileinfo.h>
#include <General/Log.h>
#include "../FileServerManager.h"
#include "../FileServer.h"
#include "../TextEdit.h"
#include "General/YSSLogger.h"
#include <QtWidgets/qmessagebox.h>
#include <General/TranslationHost.h>
namespace YSSCore::Editor {
	class FileServerManagerPrivate {
		friend class FileServerManager;
	protected:
		QList<FileServer*> FileServers;
		QMap<QString, QList<FileServer*>> FileServerMap;
		QMap<QString, QList<FileServer*>> FileServerPriorityMap;
		QMap<QString, bool> EspeciallyFocusEnableMap;
		QMap<QString, FileEditWidget*> OpenedFileEditWidgets;
		static FileServerManager* Instance;

		bool openFile(const QString& filePath, FileServer* server) {
			bool ok = false;
			if (server != nullptr) {
				FileEditWidget* feWidget = nullptr;
				switch (server->getEditorType()) {
				case FileServer::CodeEditor:
					feWidget = new TextEdit();
					ok = feWidget->openFile(filePath);
					if (ok) {
						onFileEditWidgetCreated(feWidget);
						return true;
					}
					else {
						delete feWidget;
						return false;
					}
					break;
				case FileServer::BuiltInEditor:
					feWidget = server->onCreateFileEditWidget();
					if (not feWidget) {
						return false;
					}
					ok = feWidget->openFile(filePath);
					if (ok) {
						onFileEditWidgetCreated(feWidget);
						return true;
					}
					else {
						delete feWidget;
						return false;
					}
					break;
				}
			}
			return ok;
		}

		void onFileEditWidgetCreated(FileEditWidget* we) {
			if (not we) { return; }
			QString path = we->getFilePath();
			OpenedFileEditWidgets.insert(path, we);
			QObject::connect(we, &FileEditWidget::fileClosed, [this, we](const QString& path) {
				OpenedFileEditWidgets.remove(path);
				emit Instance->fileClosed(path);
				we->deleteLater();
				yDebug << "File closed:" << path;
			});
			QObject::connect(we, &FileEditWidget::fileRenamed, [this, we](const QString& rawPath, const QString& changedPath) {
				if (OpenedFileEditWidgets.contains(rawPath)) {
					OpenedFileEditWidgets.remove(rawPath);
					OpenedFileEditWidgets.insert(changedPath, we);
					emit Instance->fileRenamed(rawPath, changedPath);
				}
				});
			QObject::connect(we, &FileEditWidget::fileChanged, [this](const QString& path) {
				emit Instance->fileChanged(path);
				});
			QObject::connect(we, &FileEditWidget::fileChangeCanceled, [this](const QString& path) {
				emit Instance->fileChangeCanceled(path);
				});
			QObject::connect(we, &FileEditWidget::fileSaved, [this](const QString& path) {
				emit Instance->fileSaved(path);
				});
			emit Instance->fileOpened(path);
		}

		bool isFileAlreadyOpen(const QString& filePath) {
			return OpenedFileEditWidgets.contains(filePath);
		}
	};
	FileServerManager* FileServerManagerPrivate::Instance = nullptr;

	/*!
		\class YSSCore::Editor::FileServerManager
		\inmodule YSSCore
		\since YSS 0.13.0
		\brief 管理FileServer的对象。

		在用户将自己的FileServer注册到FileServerManager后，FileServerManager可以根据已经注册的支持类型选择合适的FileServer来打开文件。

		\section1 打开文件时的基本工作流程
		FileServerManager的工作过程遵循以下步骤：
		\list
		\li 1. 根据打开时的文件扩展名选定全部有效的FileServer对象。
		\li 2. 首先尝试使用preferredServerId参数指定的FileServer对象（如果存在且有效）来打开文件。
		\li 3. 若2失败，则尝试使用特别关注强度决定的优先级列表（如果该功能没有被禁用）来打开文件。
		\li 4. 如果无人特别关注，就回退到手动设置的优先级列表（如果存在）来打开文件。
		\li 5. 如果优先级列表未设置，则按照注册顺序尝试打开文件。
		\li 6. 如果以上步骤全部失败，或者根本不存在支持该文件类型的FileServer，则根据useFallback参数决定是否使用内置文本编辑器打开文件。

		有关特别关注、首选ID、优先级列表等概念的详细信息，请参见相关函数的文档说明。
	*/

	/*!
		\fn FileServerManager::fileOpened(const QString& filePath)
		\since YSS 0.15.0
		当一个文件被成功打开时发出的信号。参数\a filePath是被打开的文件的绝对路径。
	*/

	/*!
		\fn FileServerManager::fileClosed(const QString& filePath)
		\since YSS 0.15.0
		当一个文件被关闭时发出的信号。参数\a filePath是被关闭的文件的绝对路径。
	*/


	/*!
		\since YSS 0.13.0
		构造FileServerManager对象。
	*/
	FileServerManager::FileServerManager() {
		d = new FileServerManagerPrivate();
		ySuccessF << "Success!";
	}

	/*!
		\since YSS 0.13.0
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
		\since YSS 0.13.0
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
		\since YSS 0.13.0
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
		\a preferredServerId 优先使用的FileServer ID。
		\a useFallback 是否在没有任何FileServer成功打开文件时使用内置文本编辑器打开文件。默认为true。
		如果文件成功打开则返回true，否则返回false.

		如果存在优先使用的FileServerID且该FileServer成功打开文件，则会优先使用该FileServer。
		否则首先回退到特别关注强度决定的优先级列表（除非为改扩展名禁用了该功能），然后回退到注册顺序决定的优先级列表。
		如果以上操作全部失败，或者根本不存在支持该文件类型的FileServer，则会根据useFallback参数决定是否使用内置文本编辑器打开文件。

		\warning 值得指出的是，为了统一下游的使用体验，FileServerManager在尝试使用FileServer打开文件时会将文件路径转换为绝对路径，
		并且不区分路径的大小写（如果操作系统不区分大小写）。如果FileServer有必要对文件路径进行处理，请严格使用QFile、QFileInfo、
		QDir等文件系统相关类对输入的路径字符串进行处理，而不是直接对字符串进行操作，以避免字面值发生变化导致逻辑错误。
	*/
	bool FileServerManager::openFile(const QString& filePath, const QString& preferredServerId, bool useFallback) {
		QString ext = QFileInfo(filePath).suffix();
		QString absPath = QFileInfo(filePath).absoluteFilePath();
		if (d->isFileAlreadyOpen(absPath)) {
			yDebug << "File already open:" << absPath;
			emit fileOpened(absPath);
			return true;
		}
		if (d->FileServerMap.contains(ext)) { // means supported
			QList<FileServer*> servers = d->FileServerMap[ext];
			if (d->FileServerPriorityMap.contains(ext)) {
				servers = d->FileServerPriorityMap[ext];
			}
			if (not preferredServerId.isEmpty()) {
				FileServer* server = nullptr;
				for (auto s : servers) {
					if (s->getModuleID() == preferredServerId) {
						yDebug << "Preferred Server founded:" << preferredServerId;
						server = s;
						break;
					}
				}
				if (server) {
					if (d->openFile(absPath, server)) {
						return true;
					}
				}
			}
			yDebug << "Especially focus:" << isEspeciallyFocusEnable(ext);
			if (isEspeciallyFocusEnable(ext)) {
				qint64 especiallyFocus = -1;
				FileServer* especiallyFocusServer = nullptr;
				for (auto server : servers) {
					qint64 currentFocus = server->especiallyFocusFile(absPath);
					if (currentFocus > 0 && currentFocus > especiallyFocus) {
						especiallyFocus = currentFocus;
						especiallyFocusServer = server;
					}
				}
				if (especiallyFocusServer) {
					yDebug << "Especially focus server:" << especiallyFocusServer->getModuleID();
					if (d->openFile(absPath, especiallyFocusServer)) {
						return true;
					}
				}
			}
			for (auto server : servers) {
				yDebug << "FileServer " << server->getModuleID() << "is trying to open the file.";
				if (d->openFile(absPath, server)) {
					return true;
				}
			}
		}
		if (useFallback) { // use builtin editor
			// create a warning dialog
			int ret = QMessageBox::warning(
				nullptr, VITR("YSS::editor.textEdit.notSuitable.title"),
				 VITR("YSS::editor.textEdit.notSuitable.message").arg(ext),
				QMessageBox::Yes | QMessageBox::No);
			if (ret == QMessageBox::Yes) {
				FileEditWidget* feWidget = new TextEdit();
				bool ok = feWidget->openFile(absPath);
				if (ok) {
					d->onFileEditWidgetCreated(feWidget);
					return true;
				}
				else {
					delete feWidget;
					return false;
				}
			}
		}
		return false;
	}

	/*!
		\since YSS 0.13.0
		获取所有已注册的FileServer支持的文件类型列表。
	*/
	QStringList FileServerManager::getSupportedFileExts() {
		return d->FileServerMap.keys();
	}

	/*!
		\since YSS 0.13.0
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
		\since YSS 0.13.0
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

	/*!
		\since YSS 0.13.0
		启用或禁用某种文件类型的特别关注强度功能。
		\a fileExt 文件类型后缀名（不含点号）。
		\a enable 是否启用特别关注强度功能。默认为true。
	*/
	void FileServerManager::setEspeciallyFocusEnable(const QString& fileExt, bool enable) {
		d->EspeciallyFocusEnableMap[fileExt] = enable;
	}

	/*!
		\since YSS 0.13.0
		检查某种文件类型的特别关注强度功能是否启用。
		\a fileExt 文件类型后缀名（不含点号）。

		返回某种文件类型的特别关注强度功能是否启用。
	*/
	bool FileServerManager::isEspeciallyFocusEnable(const QString& fileExt) {
		if (d->EspeciallyFocusEnableMap.contains(fileExt)) {
			return d->EspeciallyFocusEnableMap[fileExt];
		}
		return true; // default enable
	}

	/*!
		\since YSS 0.15.0
		获取所有打开的文件编辑窗口。

		返回所有打开的文件编辑窗口列表。

		该返回值应即用即弃，不应被缓存或长期持有，因为FileServerManager会负责在窗口关闭时删除窗口对象。
	*/
	QList<FileEditWidget*> FileServerManager::getAllFileEditWidgets() {
		return d->OpenedFileEditWidgets.values();
	}

	/*!
		\since YSS 0.15.0
		获取所有打开的文件路径列表。

		该返回值应即用即弃，不应被缓存或长期持有，因为实际打开的文件列表可能会发生变化，缓存的列表可能会过时并导致逻辑错误。
	*/
	QStringList FileServerManager::getAllOpenedFilePaths() {
		return d->OpenedFileEditWidgets.keys();
	}

	/*!
		\since YSS 0.15.0
		获取指定文件路径对应的文件编辑窗口。
		\a filePath 文件路径。
		返回指定文件路径对应的文件编辑窗口指针，如果没有找到则返回nullptr。
		该返回值应即用即弃，不应被缓存或长期持有，因为FileServerManager会负责在窗口关闭时删除窗口对象。
	*/
	FileEditWidget* FileServerManager::getFileEditWidget(const QString& filePath) {
		QFileInfo fileInfo(filePath);
		QString absPath = fileInfo.absoluteFilePath();
		if (d->OpenedFileEditWidgets.contains(absPath)) {
			return d->OpenedFileEditWidgets[absPath];
		}
		return nullptr;
	}
}