#include <QtWidgets/qwidget.h>
#include "../FileServer.h"

namespace Visindigo::Editor {
	class FileServerPrivate {
		friend class FileServer;
	protected:
		FileServer::EditorType Type = FileServer::EditorType::BuiltInEditor;
		QStringList SupportedFileExts;
	};

	/*!
		\class Visindigo::Editor::FileServer
		\inmodule Visindigo
		\brief 此类提供文件打开服务。
		\since Visindigo 0.13.0

		由于Visindigo::Editor被设计为一个通用的编辑器框架，因此对于“打开文件”这样的需求只能提供通用框架，
		即用户通过派生此类，并将其注册到Visindigo::Editor::FileServerManager中来实现对某类文件的打开支持。

		这里的打开，指的就是通过Visindigo::Editor::FileServer::EditorType枚举中定义的几种方式的任意一种方式打开文件。
		用户应该将自己实现的FileServer通过Visindigo::Editor::EditorPlugin::registerFileServer函数注册到Visindigo::Editor::FileServerManager中。
		稍后，在调用Visindigo::Editor::FileServerManager::openFile函数时，Visindigo会根据文件扩展名选择合适的FileServer来打开文件。
	*/

	/*!
		\enum Visindigo::Editor::FileServer::EditorType
		\since Visindigo 0.13.0
		\value CodeEditor 使用内置代码编辑器打开文件。
		\value BuiltInEditor 使用内置编辑器打开文件，但不是代码编辑器。这个编辑器必须是派生自Visindigo::Editor::FileEditWidget的类。
				需要同时实现Visindigo::Editor::FileServer::onCreateFileEditWidget函数。
		\value WindowEditor 使用新窗口打开文件，但仍然在本程序内。这个编辑器必须是派生自QWidget的类。
			需要同时实现Visindigo::Editor::FileServer::onCreateWindowEditor函数。
		\value ExternalProgram 使用第三方程序打开文件。需要同时实现Visindigo::Editor::FileServer::onCreateExternalEditor函数。
		\value OtherEditor 其他方式打开文件。需要同时实现Visindigo::Editor::FileServer::onOtherOpenFile函数。
	*/

	/*!
		\since Visindigo 0.13.0
		\a name 为文件服务的名称。
		\a id 为文件服务的唯一标识符。
		\a plugin 为此文件服务所属的插件。
		构造FileServer对象。
	*/
	FileServer::FileServer(const QString& name, const QString& id, EditorPlugin* plugin) :
		EditorPluginModule(name, id, plugin) {
		d = new FileServerPrivate();
	}

	/*!
		\since Visindigo 0.13.0
		析构FileServer对象。一般来说，没有任何情况需要手动析构此对象。FileServer应该与使用它的插件有一致的生命周期。
	*/
	FileServer::~FileServer() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		返回此文件服务的编辑器类型。
	*/
	FileServer::EditorType FileServer::getEditorType() {
		return d->Type;
	}

	/*!
		\since Visindigo 0.13.0
		返回此文件服务支持的文件扩展名列表。
	*/
	QStringList FileServer::getSupportedFileExts() {
		return d->SupportedFileExts;
	}

	/*!
		\since Visindigo 0.13.0
		当需要创建内置编辑器时调用。默认实现返回nullptr。
		此函数没有参数。因为设计上要求派生直接返回一个新创建的Visindigo::Editor::FileEditWidget派生类对象即可。
		
		FileServerManager::openFile函数内部会自动对这个新的派生类对象调用openFile函数打开指定文件。

		一旦成功，FileServerManager会发出FileServerManager::builtinEditorCreated信号，传递这个新的编辑器对象的指针。
	*/
	FileEditWidget* FileServer::onCreateFileEditWidget() {
		return nullptr;
	}

	/*!
		\since Visindigo 0.13.0
		当需要创建新窗口编辑器时调用。默认实现返回nullptr。
		\a filePath 为需要打开的文件路径。
		此函数要求派生类根据\a filePath参数创建一个新的QWidget派生类对象，并返回这个对象的指针。

		只要返回的指针不为nullptr，FileServerManager会发出FileServerManager::windowEditorCreated信号，传递这个新的窗口对象的指针。
	*/
	QWidget* FileServer::onCreateWindowEditor(const QString& filePath) {
		return nullptr;
	}

	/*!
		\since Visindigo 0.13.0
		当需要使用第三方程序打开文件时调用。默认实现返回false。
		\a filePath 为需要打开的文件路径。
		此函数要求派生类根据\a filePath参数使用第三方程序打开这个文件，并返回是否成功的布尔值。
		
		FileServerManager不会对这个函数的返回值做任何处理。它会直接返回这个值给调用者。

		Visindigo对第三方程序的调用没有任何要求和限制。派生类可以使用任何方式调用第三方程序。
		因此Visindigo也不打算提供生命周期辅助函数。如果插件需要对第三方程序的生命周期进行管理，请自行实现。
		建议使用EditorPlugin::onPluginDisable函数在插件被禁用时关闭第三方程序。
	*/
	bool FileServer::onCreateExternalEditor(const QString& filePath) {
		return false;
	}

	/*!
		\since Visindigo 0.13.0
		当需要使用其他方式打开文件时调用。默认实现返回false。
		\a filePath 为需要打开的文件路径。
		此函数要求派生类根据\a filePath参数使用其他方式打开这个文件，并返回是否成功的布尔值。
		FileServerManager不会对这个函数的返回值做任何处理。它会直接返回这个值给调用者。

		很难想象“其他方式”具体指什么，因此Visindigo对这个函数没有任何要求和限制。派生类可以使用任何方式打开文件。
		同样的，Visindigo没有为这种方式提供任何生命周期辅助函数。如果插件需要对这种方式的生命周期进行管理，请自行实现。
	*/
	bool FileServer::onOtherOpenFile(const QString& filePath) {
		return false;
	}

	/*!
		\since Visindigo 0.13.0
		设置此文件服务的编辑器类型。
		\a type 为新的编辑器类型。
	*/
	void FileServer::setEditorType(FileServer::EditorType type) {
		d->Type = type;
	}

	/*!
		\since Visindigo 0.13.0
		设置此文件服务支持的文件扩展名列表。
		\a exts 为新的文件扩展名列表。
	*/
	void FileServer::setSupportedFileExts(const QStringList& exts) {
		d->SupportedFileExts = exts;
	}
}