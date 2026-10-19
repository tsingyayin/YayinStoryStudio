#ifndef YSSCore_Editor_FileServerManager_h
#define YSSCore_Editor_FileServerManager_h
#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>
#include <QtWidgets/qwidget.h>
#include <General/YSSURI.h>
#include "Editor/FileEditWidget.h"
#include "YSSCoreCompileMacro.h"
// Forward declarations
class QString;
class QFileInfo;
namespace YSSCore::Editor {
	class FileServer;
	class FileServerManagerPrivate;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI FileServerManager :public QObject {
		Q_OBJECT;
		friend class FileServerManagerPrivate;
	signals:
		void fileOpened(const YSSCore::General::YSSURI& uri);
		void fileClosed(const YSSCore::General::YSSURI& uri);
		void fileRenamed(const YSSCore::General::YSSURI& rawUri, const YSSCore::General::YSSURI& changedUri);
		void fileChanged(const YSSCore::General::YSSURI& uri);
		void fileChangeCanceled(const YSSCore::General::YSSURI& uri);
		void fileSaved(const YSSCore::General::YSSURI& uri);
		void focusOnFile(const YSSCore::General::YSSURI& uri, qint32 lineNumber = 0, qint32 column = 0);
	private:
		FileServerManager();
	public:
		static FileServerManager* getInstance();
		~FileServerManager();
		void registerFileServer(FileServer* server);
		void unregisterFileServer(FileServer* server);
		bool openFile(const YSSCore::General::YSSURI& uri, const QString& preferredServerId = QString(), bool useFallback = true);
		bool openFile(const QFileInfo& fileInfo, const QString& preferredServerId = QString(), bool useFallback = true);
		bool openFile(const QString& filePath, const QString& preferredServerId = QString(), bool useFallback = true);
		QStringList getSupportedFileExts();
		QString getNameOfFileServer(const QString& serverId);
		void setPriorityForFileExt(const QString& fileExt, const QStringList& serverIds);
		void setEspeciallyFocusEnable(const QString& fileExt, bool enable);
		bool isEspeciallyFocusEnable(const QString& fileExt);
		QList<FileEditWidget*> getAllFileEditWidgets();
		QStringList getAllOpenedFilePaths();
		FileEditWidget* getFileEditWidget(const YSSCore::General::YSSURI& uri);
		FileEditWidget* getFileEditWidget(const QFileInfo& fileInfo);
		FileEditWidget* getFileEditWidget(const QString& filePath);
		QList<FileServer*> getRegisteredFileServers() const;
		QList<FileServer*> getFileServersForFileExt(const QString& fileExt) const;
		QList<FileServer*> getVirtualFileServers() const;
		QList<FileServer*> getAsToolFileServers() const;
		QStringList getSupportedVirtualFileExts() const;
		FileServer* getFileServerById(const QString& serverId) const;
		FileServer* getFileEditWidgetSourceServer(const YSSCore::General::YSSURI& uri) const;
		FileServer* getFileEditWidgetSourceServer(const QString& filePath) const;
		FileServer* getFileEditWidgetSourceServer(const QFileInfo& fileInfo) const;
		FileServer* getFileEditWidgetSourceServer(FileEditWidget* widget) const;
	private:
		FileServerManagerPrivate* d;
	};
}

#define YSSFSM YSSCore::Editor::FileServerManager::getInstance()

#endif // YSSCore_Editor_FileServerManager_h