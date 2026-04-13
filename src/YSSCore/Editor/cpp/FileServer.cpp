#include <QtWidgets/qwidget.h>
#include "../FileServer.h"
#include <General/Plugin.h>
#include "Editor/EditorPlugin.h"
namespace YSSCore::Editor {
	class FileServerPrivate {
		friend class FileServer;
	protected:
		FileServer::EditorType Type = FileServer::EditorType::BuiltInEditor;
		QStringList SupportedFileExts;
	};

	/*!
		\class YSSCore::Editor::FileServer
		\inmodule YSSCore
		\brief 此类提供文件打开服务。
		\since YSSCore 0.13.0
		\ingroup FileService

		由于YSSCore::Editor被设计为一个通用的编辑器框架，因此对于“打开文件”这样的需求提供了通用框架，
		即用户通过派生此类，并将其注册到YSSCore::Editor::FileServerManager中来实现对某类文件的打开支持。

		这里的打开，指的就是通过YSSCore::Editor::FileServer::EditorType枚举中定义的几种方式的任意一种方式打开文件。
		用户应该将自己实现的FileServer通过YSSCore::Editor::EditorPlugin::registerFileServer函数注册到YSSCore::Editor::FileServerManager中。
		稍后，在调用YSSCore::Editor::FileServerManager::openFile函数时，Yayin Story Studio会根据文件扩展名选择合适的FileServer来打开文件。

		主要指出的是，FileServer本质上只是提供打开文件的路由，它不负责处理具体文件的保存、编辑等操作。虽然
		CodeEditor和BuiltInEditor类型确实通过标准文件编辑框架（FileEditWidget）来打开文件，其中提供标准化的
		存储行为语义，但其余类型的编辑器类型需要完全由用户自己实现打开、保存等相关功能。
		
		FileServer的职责仅限于根据文件扩展名和编辑器类型来路由文件打开请求。

		\section1 编辑器类型
		\list
		\li CodeEditor 使用内置代码编辑器打开文件。
		\li BuiltInEditor 使用内置编辑器打开文件，但不是代码编辑器。这个编辑器必须是派生自YSSCore::Editor::FileEditWidget的类。
				需要同时实现onCreateFileEditWidget函数。
		\li WindowEditor 使用新窗口打开文件，但仍然在本程序内。这个编辑器必须是派生自QWidget的类。
		需要同时实现onCreateWindowEditor函数。
		\li ExternalProgram 使用第三方程序打开文件。需要同时实现onCreateExternalEditor函数。
		\li OtherEditor 其他方式打开文件。需要同时实现onOtherOpenFile函数。
	*/

	/*!
		\enum YSSCore::Editor::FileServer::EditorType
		\since YSS 0.13.0
		\value CodeEditor 使用内置代码编辑器打开文件。
		\value BuiltInEditor 使用内置编辑器打开文件，但不是代码编辑器。
		\value WindowEditor 使用新窗口打开文件，但仍然在本程序内。
		\value ExternalProgram 使用第三方程序打开文件。
		\value OtherEditor 其他方式打开文件。
	*/

	/*!
		\since YSS 0.13.0
		\a name 为文件服务的名称。
		\a id 为文件服务的唯一标识符。
		\a plugin 为此文件服务所属的插件。
		构造FileServer对象。
	*/
	FileServer::FileServer(const QString& name, const QString& id, EditorPlugin* plugin) :
		Visindigo::General::PluginModule((Visindigo::General::Plugin*)plugin, id, YSSPluginModule_FileServer, name) {
		d = new FileServerPrivate();
	}

	/*!
		\since YSS 0.13.0
		析构FileServer对象。一般来说，没有任何情况需要手动析构此对象。FileServer应该与使用它的插件有一致的生命周期。
	*/
	FileServer::~FileServer() {
		delete d;
	}

	/*!
		\since YSS 0.13.0
		返回此文件服务的编辑器类型。
	*/
	FileServer::EditorType FileServer::getEditorType() {
		return d->Type;
	}

	/*!
		\since YSS 0.13.0
		返回此文件服务支持的文件扩展名列表。
	*/
	QStringList FileServer::getSupportedFileExts() {
		return d->SupportedFileExts;
	}

	/*!
		\since YSS 0.13.0
		这是个有意思的函数，允许你在一定程度上影响文件打开时使用的文件服务的优先级。

		如果你有一个文件服务A和一个文件服务B，它们都支持打开.txt文件。
		默认情况下，Visindigo会根据它们被注册的先后顺序来决定使用哪个服务打开.txt文件
		（如果通过调用YSSCore::FileServerManager::setPriorityForFileExt函数设置了优先级，则会根据优先级来决定）。

		但如果从来没有通过YSSCore::FileServerManager::setEspeciallyFocusEnable函数的带false调用以禁用
		特别关注功能，那么当YSS尝试打开文件时，它会调用所有支持该文件扩展名的文件服务的especiallyFocusFile函数，
		看看是否有哪个服务特别关注这个文件。并在所有关注度中选取最高的那个服务来打开这个文件。

		这对那些复用现有文件后缀名但用于特殊用途的文件特别有用。譬如，你有一个json文件是作为
		某种配置，你希望为其实现可视化的操作功能，那么你就可以在注册json后缀的文件服务里
		通过这个函数特别关注打开json文件时是否为你的这一配置文件，通过返回一个较高的关注度
		来让YSS优先使用你的文件服务来打开这个文件。

		它默认返回-1。任何小于或等于0的值都会被直接忽略，而且这个函数不用于降低某个文件服务的优先级。
		通过返回更小的值来降低优先级是没有任何效果的。

		虽然不提倡，但如果你需要对某个文件拥有绝对优先权，请直接返回int64的最大值。
	*/
	qint64 FileServer::especiallyFocusFile(const QString& filePath) {
		return -1;
	}

	/*!
		\since YSS 0.13.0
		当需要创建内置编辑器时调用。默认实现返回nullptr。
		此函数没有参数。因为设计上要求派生直接返回一个新创建的YSSCore::Editor::FileEditWidget派生类对象即可。
		
		FileServerManager::openFile函数内部会自动对这个新的派生类对象调用openFile函数打开指定文件。

		一旦成功，FileServerManager会发出FileServerManager::builtinEditorCreated信号，传递这个新的编辑器对象的指针。

		新创建的FileEditWidget派生类的所有权沿着信号被转移。由于使用了unique_ptr，如果没有任何人
		监听此信号，指针被自动销毁。
	*/
	FileEditWidget* FileServer::onCreateFileEditWidget() {
		return nullptr;
	}

	/*!
		\since YSS 0.13.0
		当需要创建新窗口编辑器时调用。默认实现返回nullptr。
		\a filePath 为需要打开的文件路径。
		此函数要求派生类根据\a filePath参数创建一个新的QWidget派生类对象，并返回这个对象的指针。

		只要返回的指针不为nullptr，FileServerManager会发出FileServerManager::windowEditorCreated信号，传递这个新的窗口对象的指针。

		新创建的QWidget派生类的所有权沿着信号被转移。由于使用了unique_ptr，如果没有任何人监听此信号，指针被自动销毁。
	*/
	QWidget* FileServer::onCreateWindowEditor(const QString& filePath) {
		return nullptr;
	}

	/*!
		\since YSS 0.13.0
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
		\since YSS 0.13.0
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
		\since YSS 0.13.0
		设置此文件服务的编辑器类型。
		\a type 为新的编辑器类型。
	*/
	void FileServer::setEditorType(FileServer::EditorType type) {
		d->Type = type;
	}

	/*!
		\since YSS 0.13.0
		设置此文件服务支持的文件扩展名列表。
		\a exts 为新的文件扩展名列表。
	*/
	void FileServer::setSupportedFileExts(const QStringList& exts) {
		d->SupportedFileExts = exts;
	}
}