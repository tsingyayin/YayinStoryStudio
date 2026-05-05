#ifndef YSSCore_Editor_FileServerManager_h
#define YSSCore_Editor_FileServerManager_h
#include <QtCore/qobject.h>
#include <QtWidgets/qwidget.h>
#include "Editor/FileEditWidget.h"
#include "YSSCoreCompileMacro.h"
// Forward declarations
class QString;
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
		void fileOpened(const QString& filePath);
		void fileClosed(const QString& filePath);
		void fileRenamed(const QString& raw, const QString& changed);
		void fileChanged(const QString& filePath);
		void fileChangeCanceled(const QString& filePath);
		void fileSaved(const QString& filePath);
		void focusFile(const QString& filePath);
	private:
		FileServerManager();
	public:
		static FileServerManager* getInstance();
		~FileServerManager();
		void registerFileServer(FileServer* server);
		void unregisterFileServer(FileServer* server);
		bool openFile(const QString& filePath, const QString& preferredServerId = QString(), bool useFallback = true);
		QStringList getSupportedFileExts();
		QStringList getAvailableFileServerForFileExt(const QString& fileExt);
		QString getNameOfFileServer(const QString& serverId);
		void setPriorityForFileExt(const QString& fileExt, const QStringList& serverIds);
		void setEspeciallyFocusEnable(const QString& fileExt, bool enable);
		bool isEspeciallyFocusEnable(const QString& fileExt);
		QList<FileEditWidget*> getAllFileEditWidgets();
		QStringList getAllOpenedFilePaths();
		FileEditWidget* getFileEditWidget(const QString& filePath);
	private:
		FileServerManagerPrivate* d;
	};
}

#define YSSFSM YSSCore::Editor::FileServerManager::getInstance()

#endif // YSSCore_Editor_FileServerManager_h