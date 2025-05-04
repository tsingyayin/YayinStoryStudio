#include "../FileServer.h"
#include <QtWidgets>

namespace YSSCore::Editor {
	class FileServerPrivate {
		friend class FileServer;
	protected:
		FileServer::EditorType Type = FileServer::EditorType::BuiltInEditor;
		QStringList SupportedFileExts;
	};
	FileServer::FileServer() {
		p = new FileServerPrivate();
	}
	FileServer::~FileServer() {
		delete p;
	}
	FileServer::EditorType FileServer::getEditorType() {
		return p->Type;
	}
	QStringList FileServer::getSupportedFileExts() {
		return p->SupportedFileExts;
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
	void FileServer::setEditorType(FileServer::EditorType type) {
		p->Type = type;
	}
	void FileServer::setSupportedFileExts(const QStringList& exts) {
		p->SupportedFileExts = exts;
	}
}