#include <QtCore/qfileinfo.h>
#include <QtCore/qset.h>
#include <QtCore/qstring.h>
#include <QtWidgets/qmessagebox.h>
#include <General/Exception.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <General/YSSUniqueFileSystem.h>
#include <Utility/FileUtility.h>
#include "Editor/FileServer.h"
#include "Editor/FileServerManager.h"
#include "Editor/TextEdit.h"
#include "Editor/VirtualFilePath.h"
#include "Editor/YSSCoreBuiltinPlugin.h"
#include "General/YSSLogger.h"
namespace YSSCore::Editor {
	using namespace YSSCore::General;

	class FallBackFileServer :public FileServer {
	public:
		FallBackFileServer(EditorPlugin* plugin) :FileServer("Fallback File Server", "YSSCore.Editor.FallbackFileServer", plugin) {
			setEditorType(FileServer::CodeEditor);
			setSupportedFileExts(QStringList() << "*");
		}
		virtual ~FallBackFileServer() {}
		static FallBackFileServer* Instance;
	};

	FallBackFileServer* FallBackFileServer::Instance = nullptr;

	class FileServerManagerPrivate {
		friend class FileServerManager;
	protected:
		QList<FileServer*> FileServers;
		QMap<QString, FileServer*> VirtualFileServerMap;
		QList<FileServer*> AsToolFileServers;

		QMap<QString, QList<FileServer*>> FileServerMap;
		QMap<QString, QList<FileServer*>> FileServerPriorityMap;
		QMap<QString, bool> EspeciallyFocusEnableMap;

		QMap<QString, FileEditWidget*> OpenedFileEditWidgets;
		QMap<FileEditWidget*, FileServer*> FileEditWidgetSourceServerMap;
		// 已经受理、正在异步读取的文件。用来防止同一个 URI 在读取完成前被重复打开。
		QSet<QString> PendingOpens;
		// 每次 clearPendingOpens() 递增。回调带着提交时的值，值对不上就说明这次打开已经被作废。
		quint64 PendingOpenEpoch = 0;

		static FileServerManager* Instance;

		/*
			创建编辑器控件。返回 nullptr 表示该 FileServer 无法创建控件。
		*/
		FileEditWidget* createWidget(FileServer* server) {
			switch (server->getEditorType()) {
			case FileServer::CodeEditor:
				return new TextEdit();
			case FileServer::BuiltInEditor:
				return server->onCreateFileEditWidget();
			}
			return nullptr;
		}

		/*
			用指定 Server 打开已经读好的内容。失败时控件会被丢弃，返回 false。
			虚拟文件不经过内容，直接走 FileEditWidget 的虚拟打开路径。
		*/
		bool openWithServer(const YSSURI& uri, const QByteArray& data, FileServer* server) {
			FileEditWidget* feWidget = this->createWidget(server);
			if (not feWidget) {
				vgWarningF << "FileServer" << server->getModuleID() << "failed to create FileEditWidget for file" << uri.toString();
				return false;
			}
			const bool ok = uri.isVirtual() ? feWidget->openFile(uri) : feWidget->openFile(uri, data);
			if (not ok) {
				vgWarningF << "FileEditWidget failed to open file" << uri.toString() << "for FileServer" << server->getModuleID();
				feWidget->deleteLater();
				return false;
			}
			FileEditWidgetSourceServerMap.insert(feWidget, server);
			onFileEditWidgetCreated(feWidget);
			return true;
		}

		/*
			先选定 Server，再异步读取内容，读好后按候选顺序逐个尝试。

			return false 表示连读取请求都没能提交；读取失败或没有 Server 接受内容时
			不会创建控件，也不会再回到调用方——调用方已经把"是否受理"和"是否真的打开"
			分开了。
		*/
		bool openFile(const YSSURI& uri, const QList<FileServer*>& candidates, bool useFallback) {
			if (candidates.isEmpty()) {
				return false;
			}
			if (uri.isVirtual()) {
				return this->openWithServer(uri, QByteArray(), candidates.first());
			}
			const QString key = keyOf(uri);
			const quint64 epoch = PendingOpenEpoch;
			PendingOpens.insert(key);
			YSSUniqueFileSystem* fs = YSSUniqueFileSystem::getInstance();
			YSSFileSystemReply* reply = fs->read(uri);
			if (reply == nullptr) {
				PendingOpens.remove(key);
				vgErrorF << "Failed to submit read request for" << uri.toString();
				return false;
			}
			QObject::connect(reply, &YSSFileSystemReply::succeeded, Instance,
				[this, uri, key, candidates, reply, useFallback, epoch]() {
					if (epoch != PendingOpenEpoch) {
						// 提交这次打开的项目已经关闭：不能再建编辑器，否则没人会接管这个控件。
						PendingOpens.remove(key);
						vgDebugF << "Discard stale open request for" << uri.toString();
						return;
					}
					const QByteArray data = reply->getData();
					for (FileServer* server : candidates) {
						if (this->openWithServer(uri, data, server)) {
							PendingOpens.remove(key);
							return;
						}
					}
					if (useFallback) {
						const int ret = QMessageBox::warning(nullptr,
							VITRL("YSS::editor.textEdit.notSuitable.title"),
							VITRL("YSS::editor.textEdit.notSuitable.message").arg(uri.getFileExt()),
							QMessageBox::Yes | QMessageBox::No);
						if (ret == QMessageBox::Yes) {
							this->openWithServer(uri, data, FallBackFileServer::Instance);
						}
					}
					PendingOpens.remove(key);
				});
			QObject::connect(reply, &YSSFileSystemReply::failed, Instance,
				[this, uri, key](const YSSFileError& error) {
					PendingOpens.remove(key);
					vgErrorF << "Failed to read file" << uri.toString() << error.toString();
				});
			return true;
		}

		/*
			按"首选 ID → 特别关注强度 → 注册/优先级顺序"给出候选 Server，去重。
			这个阶段完全不读文件，因此不支持的类型不会被白白读一遍。
		*/
		QList<FileServer*> candidateServers(const YSSURI& uri, const QString& ext, const QString& preferredServerId) {
			const QList<FileServer*> servers = FileServerPriorityMap.contains(ext)
				? FileServerPriorityMap[ext]
				: FileServerMap.value(ext);
			QList<FileServer*> candidates;
			if (servers.isEmpty()) {
				return candidates;
			}
			if (not preferredServerId.isEmpty()) {
				for (FileServer* server : servers) {
					if (server->getModuleID() == preferredServerId) {
						yDebug << "Preferred Server founded:" << preferredServerId;
						candidates.append(server);
						break;
					}
				}
			}
			if (EspeciallyFocusEnableMap.value(ext, true)) {
				qint64 bestFocus = -1;
				FileServer* bestServer = nullptr;
				for (FileServer* server : servers) {
					const qint64 focus = server->especiallyFocusFile(uri);
					if (focus > 0 && focus > bestFocus) {
						bestFocus = focus;
						bestServer = server;
					}
				}
				if (bestServer && !candidates.contains(bestServer)) {
					yDebug << "Especially focus server:" << bestServer->getModuleID();
					candidates.append(bestServer);
				}
			}
			for (FileServer* server : servers) {
				if (!candidates.contains(server)) {
					candidates.append(server);
				}
			}
			return candidates;
		}

		/*
			此函数将已经实质上打开文件的FileEditWidget的相关信号绑定到本类相关功能。
		*/
		void onFileEditWidgetCreated(FileEditWidget* few) {
			if (not few) {
				VI_Throw_ST(Visindigo::General::Exception::NullPointer, "This function requires a valid FileEditWidget pointer.");
			}
			const YSSURI uri = few->getUri();
			OpenedFileEditWidgets.insert(keyOf(uri), few);
			QObject::connect(few, &FileEditWidget::fileClosed, Instance, [this, few](const YSSURI& closedUri) {
				OpenedFileEditWidgets.remove(keyOf(closedUri));
				FileEditWidgetSourceServerMap.remove(few);
				emit Instance->fileClosed(closedUri);
				few->deleteLater();
				yDebug << "File closed:" << closedUri.toString();
				});
			QObject::connect(few, &FileEditWidget::fileRenamed, Instance, [this, few](const YSSURI& rawUri, const YSSURI& changedUri) {
				if (OpenedFileEditWidgets.contains(keyOf(rawUri))) {
					OpenedFileEditWidgets.remove(keyOf(rawUri));
					OpenedFileEditWidgets.insert(keyOf(changedUri), few);
					emit Instance->fileRenamed(rawUri, changedUri);
				}
				else {
					VI_Throw_ST(Visindigo::General::Exception::NotFound, "FileEditWidget emitted fileRenamed signal but the raw path is not found in OpenedFileEditWidgets.");
				}
				});
			QObject::connect(few, &FileEditWidget::fileChanged, Instance, [this](const YSSURI& uri) {
				emit Instance->fileChanged(uri);
				});
			QObject::connect(few, &FileEditWidget::fileChangeCanceled, Instance, [this](const YSSURI& uri) {
				emit Instance->fileChangeCanceled(uri);
				});
			QObject::connect(few, &FileEditWidget::fileSaved, Instance, [this](const YSSURI& uri) {
				emit Instance->fileSaved(uri);
				});
			emit Instance->fileOpened(uri);
		}

		bool isFileAlreadyOpen(const YSSURI& uri) {
			const QString key = keyOf(uri);
			return OpenedFileEditWidgets.contains(key) || PendingOpens.contains(key);
		}

		// 索引键。统一交给 YSSURI::toString()：本机路径是绝对路径（Windows 上比较时
		// 不区分大小写，见 YSSURI::operator==），虚拟文件是 @ext!name?param，
		// 远端是规范化的 ssh:// 字符串。页面层的标签索引也用同一个键。
		static QString keyOf(const YSSURI& uri) {
			return uri.toString();
		}
	};
	FileServerManager* FileServerManagerPrivate::Instance = nullptr;

	/*!
		\class YSSCore::Editor::FileServerManager
		\inmodule YSSCore
		\since YSS 0.13.0
		\brief 管理FileServer的对象.

		在用户将自己的FileServer注册到FileServerManager后，FileServerManager可以根据已经注册的支持类型选择合适的FileServer来打开文件。

		\section1 打开文件时的基本工作流程
		FileServerManager的工作过程遵循以下步骤：
		\list
		\li 1. 根据扩展名给出候选FileServer，顺序为：preferredServerId → 特别关注强度 → 优先级列表 / 注册顺序。
		\li 2. 没有任何候选时不读文件，直接按 useFallback 决定是否询问用内置编辑器。
		\li 3. 有候选时通过 \l{YSSCore::General::YSSUniqueFileSystem} 异步读取内容；
		读取失败则本次打开结束，不再创建编辑器。
		\li 4. 内容到达后按候选顺序逐个创建编辑器并调用 \l{FileEditWidget::openFile}，
		第一个成功者即为结果，其余候选不再尝试。
		\li 5. 全部候选都拒绝内容时，按 useFallback 询问是否用内置文本编辑器打开。
		\endlist

		因此 \l{openFile} 的返回值只表示"是否受理"：真实文件的实际打开结果在
		异步读取完成后由 \l{fileOpened()} 信号体现。

		有关特别关注、首选ID、优先级列表等概念的详细信息，请参见相关函数的文档说明。
	*/

	/*!
		\fn FileServerManager::fileOpened(const QString& filePath)
		\since YSS 0.15.0
		\a filePath 是被打开的文件的绝对路径。

		当一个文件被成功打开时发出的信号。
	*/

	/*!
		\fn FileServerManager::fileClosed(const QString& filePath)
		\since YSS 0.15.0
		\a filePath 是被关闭的文件的绝对路径。

		当一个文件被关闭时发出的信号。
	*/

	/*!
		\fn FileServerManager::fileRenamed(const QString& raw, const QString& changed)
		\since YSS 0.15.0
		\a raw 是文件重命名前的绝对路径
		\a changed 是文件重命名后的绝对路径。

		当一个文件被重命名时发出的信号。参数
	*/

	/*!
		\fn FileServerManager::fileChanged(const QString& filePath)
		\since YSS 0.15.0
		\a filePath 是被修改的文件的绝对路径。

		当一个文件被修改时发出的信号。
	*/

	/*!
		\fn FileServerManager::fileChangeCanceled(const QString& filePath)
		\since YSS 0.15.0
		\a filePath 是被撤销修改的文件的绝对路径。

		当一个文件的修改被撤销时发出的信号。
	*/

	/*!
		\fn FileServerManager::fileSaved(const QString& filePath)
		\since YSS 0.15.0
		当一个文件被保存时发出的信号。参数 \a filePath 是被保存的文件的绝对路径。
	*/

	/*!
		\fn FileServerManager::focusOnFile(const QString& filePath, qint32 lineNumber, qint32 column)
		\since YSS 0.15.0
		\a filePath 被聚焦的文件的绝对路径， \a lineNumber 聚焦的行号， \a column 聚焦的列号。

		请求聚焦一个文件时应该尝试发出此信号。

		具体如何聚焦文件，由连接了该信号的前台决定。不过，YSSCore要求
		连接了此信号的前台在聚焦时，应使用 \a lineNumber 和 \a column 调用响应的
		FileEditWidget::cursorToPosition函数来聚焦文件的特定位置。
	*/

	/*!
		\since YSS 0.13.0
		构造FileServerManager对象。
	*/
	FileServerManager::FileServerManager() {
		d = new FileServerManagerPrivate();
		if (FallBackFileServer::Instance == nullptr) {
			FallBackFileServer::Instance = new FallBackFileServer(YSSCoreBuiltinPlugin::getInstance());
		}
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
		delete FallBackFileServer::Instance;
		FallBackFileServer::Instance = nullptr;
	}

	/*!
		\since YSS 0.13.0
		return FileServerManager的单例对象。
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

		对于普通的FileServer对象，由于同一个文件扩展名可能有多个FileServer支持，
		因此文件扩展名可以在多个FileServer之间重复。

		而对于作为虚拟文件服务器的FileServer，由于虚拟文件里的扩展名语义是作为ID
		使用的，因此对于一个虚拟文件扩展名，只能有一个FileServer支持，
		如果有多个FileServer支持同一个虚拟文件扩展名，则只有第一个被注册的FileServer会生效，
		后续的注册会被忽略，并且会在日志中输出警告信息。
	*/
	void FileServerManager::registerFileServer(FileServer* server) {
		if (d->FileServers.contains(server)) {
			return;
		}
		d->FileServers.append(server);
		QStringList fileTypes = server->getSupportedFileExts();
		for (auto type : fileTypes) {
			if (not server->isVirtualFileServer()) {
				if (not d->FileServerMap.contains(type)) {
					d->FileServerMap.insert(type, QList<FileServer*>());
				}
				d->FileServerMap[type].append(server);
			}
			else {
				if (d->VirtualFileServerMap.contains(type)) {
					vgErrorF << "Virtual FileServer for type" << type << "already exists! Ignoring the new one.";
					continue;
				}
				d->VirtualFileServerMap.insert(type, server);
				if (server->isListAsTool()) {
					d->AsToolFileServers.append(server);
				}
			}
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
		for (auto type : fileTypes) {
			if (not server->isVirtualFileServer()) {
				if (d->FileServerMap.contains(type)) {
					d->FileServerMap[type].removeOne(server);
				}
			}
			else {
				d->VirtualFileServerMap.remove(type);
			}
		}
	}

	/*!
		\since YSS 0.18.0
		打开一个文件。这是 openFile 的主实现版本。

		\a uri 文件身份。真实文件的内容由本类通过 \l{YSSCore::General::YSSUniqueFileSystem}
		异步读取，再连同 URI 一起交给 FileEditWidget；虚拟文件不读内容。
		\a preferredServerId 优先使用的FileServer ID。
		\a useFallback 是否在没有任何FileServer接受文件时使用内置文本编辑器打开文件。默认为true。

		return 是否已经受理这次打开请求：URI 合法、已经打开过、或找到了愿意尝试的
		FileServer 时返回 true。读取失败或所有候选 FileServer 都拒绝内容发生在
		\l{openFile} 返回之后，不会反映在返回值里，此时由 \l{fileOpened()} 是否发出
		和日志体现。

		候选顺序：优先使用的FileServerID → 特别关注强度（除非为该扩展名禁用）→
		优先级列表 / 注册顺序。选择阶段不读文件；没有任何候选时才直接询问是否用
		内置编辑器兜底。所有候选都拒绝内容时同样会询问兜底。

		虚拟文件（\l{YSSCore::General::YSSURI::Scheme::VirtualFile}）只匹配支持该虚拟文件的FileServer，
		不考虑 \a preferredServerId 、特别关注强度、优先级列表、 \a useFallback 等因素。
	*/
	bool FileServerManager::openFile(const YSSURI& uri, const QString& preferredServerId, bool useFallback) {
		if (!uri.isValid()) {
			vgErrorF << "File URI is invalid!";
			return false;
		}
		if (d->isFileAlreadyOpen(uri)) {
			yDebug << "File already open:" << uri.toString();
			emit fileOpened(uri);
			return true;
		}
		if (uri.isVirtual()) {
			const QString ext = uri.getHost();
			FileServer* server = d->VirtualFileServerMap.value(ext, nullptr);
			if (server) {
				yDebug << "Virtual FileServer is trying to open the file.";
				return d->openFile(uri, QList<FileServer*>() << server, false);
			}
			vgErrorF << "No Virtual FileServer found for type" << ext;
			return false;
		}
		if (uri.isDir()) {
			// 目录没有文件内容，也没有任何编辑器会接受它；保持旧行为：直接失败，不弹兜底询问。
			vgWarningF << "Cannot open a directory as a file:" << uri.toString();
			return false;
		}

		// 先选 Server，再决定要不要读文件。没有候选时宁可直接走兜底，也不白读一遍。
		const QString ext = uri.getFileExt();
		const QList<FileServer*> candidates = d->candidateServers(uri, ext, preferredServerId);
		if (!candidates.isEmpty()) {
			return d->openFile(uri, candidates, useFallback);
		}
		if (!useFallback) {
			vgErrorF << "No FileServer supports file type" << ext;
			return false;
		}
		const int ret = QMessageBox::warning(
			nullptr, VITRL("YSS::editor.textEdit.notSuitable.title"),
			VITRL("YSS::editor.textEdit.notSuitable.message").arg(ext),
			QMessageBox::Yes | QMessageBox::No);
		if (ret == QMessageBox::Yes) {
			return d->openFile(uri, QList<FileServer*>() << FallBackFileServer::Instance, false);
		}
		return false;
	}

	/*!
		\since YSS 0.13.0
		打开一个文件。路径字符串形式的便利版本，内部转成 \l{YSSCore::General::YSSURI}。
		\a filePath 文件路径。虚拟文件用 \c{@ext!name?param}，其余按本机绝对路径处理。
		\a preferredServerId 优先使用的FileServer ID。
		\a useFallback 是否使用内置文本编辑器兜底。

		return 是否已经受理这次打开请求；实际读取结果见 \l{fileOpened()}。
	*/
	bool FileServerManager::openFile(const QString& filePath, const QString& preferredServerId, bool useFallback) {
		if (filePath.isEmpty()) {
			vgErrorF << "File path is empty!";
			return false;
		}
		if (VirtualFilePath::isVirtualFilePath(filePath)) {
			return openFile(YSSURI::fromVirtualPath(filePath), preferredServerId, useFallback);
		}
		return openFile(QFileInfo(filePath), preferredServerId, useFallback);
	}

	/*!
		\since YSS 0.16.0
		打开一个文件。QFileInfo 形式的便利版本，内部转成 \l{YSSCore::General::YSSURI}。
		\a fileInfo 文件信息对象。
		\a preferredServerId 优先使用的FileServer ID。
		\a useFallback 是否使用内置文本编辑器兜底。

		return 是否已经受理这次打开请求；实际读取结果见 \l{fileOpened()}。

		\warning QFileInfo不处理虚拟文件路径，虚拟文件请用 openFile(const QString&) 或 URI 版本。
	*/
	bool FileServerManager::openFile(const QFileInfo& fileInfo, const QString& preferredServerId, bool useFallback) {
		if (!fileInfo.exists()) {
			vgErrorF << "File does not exist:" << fileInfo.absoluteFilePath();
			return false;
		}
		return openFile(YSSURI::fromLocalPath(fileInfo.absoluteFilePath()), preferredServerId, useFallback);
	}

	/*!
		\since YSS 0.18.0

		作废所有已受理但尚未结束的异步打开请求。

		真实文件的打开是"先异步读取、读完再创建编辑器"的两段流程。调用方（例如项目窗口）
		可能要在这段读取结束之前就关闭，此时如果读完之后照旧创建 FileEditWidget，这个控件
		既不会出现在任何区域里，也不会被谁负责关闭，等于一直留在 FileServerManager 里。
		本函数让这些已经受理的请求在回调到达时直接作废，同时清空 \l{openFile()} 用来去重的
		受理记录。

		\note 本函数只影响尚未读完的请求，已经打开的编辑器不受影响。
	*/
	void FileServerManager::clearPendingOpens() {
		++d->PendingOpenEpoch;
		d->PendingOpens.clear();
	}

	/*!
		\since YSS 0.13.0
		return 所有已注册的FileServer支持的文件类型列表。不含虚拟文件类型。
	*/
	QStringList FileServerManager::getSupportedFileExts() {
		return d->FileServerMap.keys();
	}

	/*!
		\since YSS 0.13.0
		return 某个FileServer的名称。
		\a serverId FileServer的ID。
		return 返回某个FileServer的名称，如果没有找到对应ID的FileServer，则返回一个空字符串。
	*/
	QString FileServerManager::getNameOfFileServer(const QString& serverId) {
		for (int i = 0; i < d->FileServers.size(); i++) {
			if (d->FileServers[i]->getModuleID() == serverId) {
				return d->FileServers[i]->getModuleName();
			}
		}
		return QString();
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
		d->FileServerPriorityMap[fileExt] = servers;
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

		return 某种文件类型的特别关注强度功能是否启用。
	*/
	bool FileServerManager::isEspeciallyFocusEnable(const QString& fileExt) {
		if (d->EspeciallyFocusEnableMap.contains(fileExt)) {
			return d->EspeciallyFocusEnableMap[fileExt];
		}
		return true; // default enable
	}

	/*!
		\since YSS 0.15.0
		return 所有打开的文件编辑窗口。

		return 所有打开的文件编辑窗口列表。

		该返回值应即用即弃，不应被缓存或长期持有，因为FileServerManager会负责在窗口关闭时删除窗口对象。
	*/
	QList<FileEditWidget*> FileServerManager::getAllFileEditWidgets() {
		return d->OpenedFileEditWidgets.values();
	}

	/*!
		\since YSS 0.15.0
		return 所有打开的文件路径列表。

		该返回值应即用即弃，不应被缓存或长期持有，因为实际打开的文件列表可能会发生变化，缓存的列表可能会过时并导致逻辑错误。
	*/
	QStringList FileServerManager::getAllOpenedFilePaths() {
		QStringList paths;
		for (auto* widget : d->OpenedFileEditWidgets) {
			if (widget) {
				paths.append(widget->getFilePath());
			}
		}
		return paths;
	}

	/*!
		\since YSS 0.15.0
		return 指定文件路径对应的文件编辑窗口。
		\a filePath 文件路径。
		return 指定文件路径对应的文件编辑窗口指针，如果没有找到则返回nullptr。

		委托到getFileEditWidget(const QFileInfo&)版本。
	*/
	FileEditWidget* FileServerManager::getFileEditWidget(const QString& filePath) {
		if (VirtualFilePath::isVirtualFilePath(filePath)) {
			return getFileEditWidget(YSSURI::fromVirtualPath(filePath));
		}
		return getFileEditWidget(QFileInfo(filePath));
	}

	/*!
		\since YSS 0.16.0
		return 指定文件信息对应的文件编辑窗口。
		\a fileInfo 文件信息对象。
		return 指定文件对应的文件编辑窗口指针，如果没有找到则返回nullptr。

		这是getFileEditWidget的主实现版本。直接接受QFileInfo，以规范化路径进行索引查找。
	*/
	FileEditWidget* FileServerManager::getFileEditWidget(const QFileInfo& fileInfo) {
		return getFileEditWidget(YSSURI::fromLocalPath(fileInfo.absoluteFilePath()));
	}

	/*!
		\since YSS 0.18.0
		return 指定 URI 对应的文件编辑窗口。
		\a uri 文件身份。
		return 对应的文件编辑窗口指针，如果没有找到则返回nullptr。
	*/
	FileEditWidget* FileServerManager::getFileEditWidget(const YSSURI& uri) {
		return d->OpenedFileEditWidgets.value(FileServerManagerPrivate::keyOf(uri), nullptr);
	}

	/*!
		\since YSS 0.17.0

		return 全部已经注册的文件服务器。
	*/
	QList<FileServer*> FileServerManager::getRegisteredFileServers() const {
		return d->FileServers;
	}

	/*!
		\since YSS 0.17.0
		return 全部已经注册的文件服务器中，支持指定文件类型的服务器。

		\a fileExt 文件类型后缀名（不含点号）。

		请注意，这个返回值不包括对虚拟文件路径中扩展名的搜索。
	*/
	QList<FileServer*> FileServerManager::getFileServersForFileExt(const QString& fileExt) const {
		if (d->FileServerMap.contains(fileExt)) {
			return d->FileServerMap[fileExt];
		}
		return QList<FileServer*>();
	}

	/*!
		\since YSS 0.17.0
		return 全部已经注册的文件服务器中，支持虚拟文件路径的服务器。
	*/
	QList<FileServer*> FileServerManager::getVirtualFileServers() const {
		return d->VirtualFileServerMap.values();
	}

	/*!
		\since YSS 0.17.0
		return 全部已经注册的文件服务器中，标记为“作为工具列出”的服务器。
	*/
	QList<FileServer*> FileServerManager::getAsToolFileServers() const {
		return d->AsToolFileServers;
	}

	/*!
		\since YSS 0.17.0
		return 全部支持的虚拟文件扩展名。
	*/
	QStringList FileServerManager::getSupportedVirtualFileExts() const {
		return d->VirtualFileServerMap.keys();
	}

	/*!
		\since YSS 0.17.0
		\a serverId 文件服务器ID。

		return 指定ID的FileServer对象。
	*/
	FileServer* FileServerManager::getFileServerById(const QString& serverId) const {
		for (auto server : d->FileServers) {
			if (server->getModuleID() == serverId) {
				return server;
			}
		}
		return nullptr;
	}

	/*!
		\since YSS 0.17.0
		return 指定文件路径对应的FileEditWidget的源FileServer对象。
	*/
	FileServer* FileServerManager::getFileEditWidgetSourceServer(const QString& filePath) const {
		if (VirtualFilePath::isVirtualFilePath(filePath)) {
			return getFileEditWidgetSourceServer(YSSURI::fromVirtualPath(filePath));
		}
		return getFileEditWidgetSourceServer(QFileInfo(filePath));
	}

	/*!
		\since YSS 0.17.0
		return 指定文件信息对应的FileEditWidget的源FileServer对象。
	*/
	FileServer* FileServerManager::getFileEditWidgetSourceServer(const QFileInfo& fileInfo) const {
		return getFileEditWidgetSourceServer(YSSURI::fromLocalPath(fileInfo.absoluteFilePath()));
	}

	/*!
		\since YSS 0.18.0
		return 指定 URI 对应的FileEditWidget的源FileServer对象。
		\a uri 文件身份。
	*/
	FileServer* FileServerManager::getFileEditWidgetSourceServer(const YSSURI& uri) const {
		auto we = d->OpenedFileEditWidgets.value(FileServerManagerPrivate::keyOf(uri), nullptr);
		if (we) {
			return getFileEditWidgetSourceServer(we);
		}
		return nullptr;
	}

	/*!
		\since YSS 0.17.0
		\a few 文件编辑控件。

		return 指定FileEditWidget的源FileServer对象。
	*/
	FileServer* FileServerManager::getFileEditWidgetSourceServer(FileEditWidget* few) const {
		if (d->FileEditWidgetSourceServerMap.contains(few)) {
			return d->FileEditWidgetSourceServerMap[few];
		}
		return nullptr;
	}
}
