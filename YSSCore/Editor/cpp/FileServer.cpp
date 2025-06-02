#include "../FileServer.h"
#include <QtWidgets/qwidget.h>

namespace YSSCore::Editor {
	class FileServerPrivate {
		friend class FileServer;
	protected:
		EditorPlugin* Plugin;
		FileServer::EditorType Type = FileServer::EditorType::BuiltInEditor;
		QStringList SupportedFileExts;
	};
	FileServer::FileServer(EditorPlugin* plugin) {
		d = new FileServerPrivate();
		d->Plugin = plugin;
	}
	FileServer::~FileServer() {
		delete d;
	}
	FileServer::EditorType FileServer::getEditorType() {
		return d->Type;
	}
	QStringList FileServer::getSupportedFileExts() {
		return d->SupportedFileExts;
	}
	FileEditWidget* FileServer::createFileEditWidget() {
		return nullptr;
	}
	QWidget* FileServer::createExternalEditor(const QString& filePath) {
		return nullptr;
	}
	bool FileServer::openFile(const QString& filePath) {
		return false;
	}
	EditorPlugin* FileServer::getPlugin() {
		return d->Plugin;
	}
	void FileServer::setEditorType(FileServer::EditorType type) {
		d->Type = type;
	}
	void FileServer::setSupportedFileExts(const QStringList& exts) {
		d->SupportedFileExts = exts;
	}
}