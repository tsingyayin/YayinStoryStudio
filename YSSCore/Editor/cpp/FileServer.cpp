#include "../FileServer.h"
#include <QtWidgets/qwidget.h>

namespace YSSCore::Editor {
	class FileServerPrivate {
		friend class FileServer;
	protected:
		FileServer::EditorType Type = FileServer::EditorType::BuiltInEditor;
		QStringList SupportedFileExts;
	};
	FileServer::FileServer(const QString& name, const QString& id, EditorPlugin* plugin):
	EditorPluginModule(name,id,plugin){
		d = new FileServerPrivate();
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
	FileEditWidget* FileServer::onCreateFileEditWidget() {
		return nullptr;
	}
	QWidget* FileServer::onCreateWindowEditor(const QString& filePath) {
		return nullptr;
	}
	bool FileServer::onCreateExternalEditor(const QString& filePath) {
		return false;
	}
	bool FileServer::onOtherOpenFile(const QString& filePath) {
		return false;
	}
	void FileServer::setEditorType(FileServer::EditorType type) {
		d->Type = type;
	}
	void FileServer::setSupportedFileExts(const QStringList& exts) {
		d->SupportedFileExts = exts;
	}
}