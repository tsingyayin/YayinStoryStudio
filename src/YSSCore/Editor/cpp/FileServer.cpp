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
		bool asVirtualFileServer = false;
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

		\section1 虚拟文件
		从0.15.0开始，引入虚拟文件概念，以便在文件编辑区域打开一些并非真实存在的文件进行编辑，譬如程序设置、
		插件设置等页面。

		在虚拟文件模式下，文件路径被固定为以下写法：
		\badcode
			@file_ext!file_name?param
		\endcode
		譬如，对于setSupportedFileExts为YSS.SettingsWidget的虚拟文件服务器，
		当打开的文件路径为
		\badcode
			@YSS.SettingsWidget!ProgramSettings?from=menubar
		\endcode
		则会对应调用该服务器，且file_name会在FileEditWidget中解析为SettingsWidget。
		
		有关虚拟文件模式下，FileEditWidget的其他特殊行为，请参考相关文档。

		至于参数部分，目前推荐按url参数格式书写，但不强制规定。

		既然提到了参数格式，我们也需要对file_ext和file_name做一些推荐标准：
		
		由于file_name被约定为显示出来的文件名，而虚拟文件的编辑器可能通常
		是一些配置界面，需要本地化，因此不推荐使用file_name进行功能区分，
		而推荐只将file_ext作为功能区分的ID进行使用，且使用域命名法来书写。
		
		事实上，在YSS内部，file_ext确实也是作为ID使用的，即不能有多个FileServer
		同时注册一个file_ext，否则后来者无效。

		譬如，对于上面那个YSS程序设置的例子，其实我们更推荐写为：
		\badcode
			@YSS.SettingsWidget.ProgramSettings!程序设置?from=menubar
		\endcode
		这样file_ext就可以直接用来区分不同的功能，而file_name则
		可以直接用来显示给用户看。
	*/

	/*!
		\enum YSSCore::Editor::FileServer::EditorType
		\since YSS 0.13.0
		\value CodeEditor 使用内置代码编辑器打开文件。
		\value BuiltInEditor 使用内置编辑器打开文件，但不是代码编辑器。
		\value WindowEditor 使用新窗口打开文件，但仍然在本程序内。这个枚举计划从0.17开始废弃。
		\value ExternalProgram 使用第三方程序打开文件。这个枚举计划从0.17开始废弃。
		\value OtherEditor 其他方式打开文件。这个枚举计划从0.17开始废弃。

		请注意，为了实现一些更深入的编辑器功能（如重命名），YSS必须有办法将资源管理器的重命名
		操作通知到正在打开的文件，因此我们决定废弃除了CodeEditor和BuiltInEditor以外的编辑器类型，
		以确保所有的编辑器都必须使用内置的编辑器框架来打开文件。
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

		\note 从0.15.0开始引入虚拟文件概念。在虚拟文件服务器模式时，该功能无效
	*/
	qint64 FileServer::especiallyFocusFile(const QString& filePath) {
		return -1;
	}

	/*!
		\since YSS 0.13.0
		当需要创建内置编辑器时调用。默认实现返回nullptr。
		此函数没有参数。因为设计上要求派生直接返回一个新创建的YSSCore::Editor::FileEditWidget派生类对象即可。
		
		FileServerManager::openFile函数内部会自动对这个新的派生类对象调用openFile函数打开指定文件。

		一旦成功，FileServerManager会调用FileWidgetHandler::handleBuiltinEditor函数将这个新的编辑器对象的指针传递给外部，
		以便外部可以将它添加到UI中。
		
		/warning 该函数返回指针的所有权问题在0.15之前和0.15及之后有不同的处理方式。请务必注意。

		在0.15之前，如果Handler返回true，该指针所有权会被转移到该FileWidgetHandler。
		在0.15及之后，如果Handler返回true，该指针所有权会被转移到FileServerManager，并且FileServerManager会负责在
		closed()信号发出时删除这个对象。
	*/
	FileEditWidget* FileServer::onCreateFileEditWidget() {
		return nullptr;
	}

	/*!
		\since YSS 0.15.0
		返回此文件服务是否为虚拟文件服务器。
	*/
	bool FileServer::isVirtualFileServer() {
		return d->asVirtualFileServer;
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

	/*!
		\since YSS 0.15.0
		设置此文件服务器作为虚拟文件服务器。
		在虚拟文件模式时，特别关注功能不可用。
	*/
	void FileServer::setAsVitrualFileServer(bool isVirtual) {
		d->asVirtualFileServer = isVirtual;
	}
}