#pragma once
#include "EditorPluginModule.h"
class QString;
class QWidget;
namespace Visindigo::Editor {
	class FileEditWidget;
	class FileServerPrivate;
	class EditorPlugin;
	class VisindigoAPI FileServer :public EditorPluginModule{
		Q_OBJECT;
	public:
		enum EditorType {
			CodeEditor,  //builtin code editor
			BuiltInEditor, //builtin, but not code editor
			WindowEditor, //new window, but still in local program
			ExternalProgram, //3rd party program
			OtherEditor, //other editor
		};
		FileServer(const QString& name, const QString& id, EditorPlugin* plugin);
		virtual ~FileServer();
		EditorType getEditorType();
		QStringList getSupportedFileExts();
		virtual FileEditWidget* onCreateFileEditWidget();
		virtual QWidget* onCreateWindowEditor(const QString& filePath);
		virtual bool onCreateExternalEditor(const QString& filePath);
		virtual bool onOtherOpenFile(const QString& filePath);
	protected:
		void setEditorType(EditorType type);
		void setSupportedFileExts(const QStringList& exts);
	private:
		FileServerPrivate* d;
	};
}