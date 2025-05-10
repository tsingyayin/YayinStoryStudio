#pragma once
#include <QtCore/qlist.h>
#include "../Macro.h"

class QString;
class QWidget;
namespace YSSCore::Editor {
	class FileEditWidget;
	class FileServerPrivate;
	class YSSCoreAPI FileServer {
	public:
		enum EditorType {
			CodeEditor,  //builtin code editor
			BuiltInEditor, //builtin, but not code editor
			WindowEditor, //new window, but still in local program
			ExternalProgram, //3rd party program
			OtherEditor, //other editor
		};
		FileServer();
		~FileServer();
		EditorType getEditorType();
		QStringList getSupportedFileExts();
		virtual FileEditWidget* createFileEditWidget();
		virtual QWidget* createExternalEditor(const QString& filePath);
		virtual bool openFile(const QString& filePath);
	protected:
		void setEditorType(EditorType type);
		void setSupportedFileExts(const QStringList& exts);
	private:
		FileServerPrivate* d;
	};
}