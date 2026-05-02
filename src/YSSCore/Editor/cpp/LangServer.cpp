#include <QtCore/qstring.h>
#include <QtGui/qsyntaxhighlighter.h>
#include "../LangServer.h"
#include <General/Plugin.h>
#include "Editor/EditorPlugin.h"
namespace YSSCore::Editor {
	class LangServerPrivate
	{
		friend class LangServer;
		friend class LangServerManager;
	protected:
		QString LanguageID;
		QStringList LanguageExt;
	};

	/*!
		\class YSSCore::Editor::LangServer
		\brief LangServer是一个插件模块，提供特定编程语言的辅助语言功能
		\since YSS 0.13.0
		\inmodule YSSCore
		\ingroup LangService

		LangServer是一个插件模块，提供特定编程语言的辅助语言功能，如语法高亮、Tab补全、鼠标悬停提示等。
		这个类本身主要作为记录类，如果需要实现特定的功能，你需要分别实现createHighlighter、createTabCompleter、createHoverInfoProvider等函数，
		并且在这些函数中返回对应功能的实例。

		所有虚函数中，只有createHighlighter强制要求实现。其他均默认返回nullptr，表示不提供该功能。
		但如果确实不需要Highlighter，请在实现中返回nullptr。
	*/

	/*!
		\since YSS 0.13.0
		构造函数
		构造函数需要提供语言服务器的名称、ID、所属插件、语言ID和支持的文件扩展名列表。
		其中，语言ID是一个唯一标识符，用于区分不同的语言服务器；文件扩展名列表则用于指定该语言服务器支持哪些类型的文件。
	*/
	LangServer::LangServer(const QString& name, const QString& id, EditorPlugin* plugin, const QString& lang_id, QStringList ext) :
		Visindigo::General::PluginModule((Visindigo::General::Plugin*)plugin, id, YSSPluginModule_LangServer, name){
		d = new LangServerPrivate();
		d->LanguageID = lang_id;
		d->LanguageExt = ext;
	}

	/*!
		\since YSS 0.13.0
		析构函数
	*/
	LangServer::~LangServer(){
		delete d;
	}

	/*!
		\fn virtual SyntaxHighlighter* LangServer::createHighlighter(TextEdit* doc) = 0;
		\since YSS 0.13.0
		创建一个语法高亮器实例。默认实现返回nullptr，表示不提供语法高亮功能。

		传入该函数的TextEdit将会接管SyntaxHighlighter的所有权，且独占使用。
		因此，你应该为每一次调用都创建新的SyntaxHighlighter实例，而不是复用同一个实例。
		如果你需要在不同的文档之间共享某些数据（如变量名称、函数定义等），请另外创建一个数据管理类，并在SyntaxHighlighter中引用它。
	*/

	/*!
		\since YSS 0.13.0
		创建一个Tab补全提供者实例。默认实现返回nullptr，表示不提供Tab补全功能。

		传入该函数的TextEdit将会接管TabCompleterProvider的所有权，且独占使用。
		因此，你应该为每一次调用都创建新的TabCompleterProvider实例，而不是复用同一个实例。
		如果你需要在不同的文档之间共享某些数据（如变量名称、函数定义等），请另外创建一个数据管理类，并在TabCompleterProvider中引用它。
	*/
	TabCompleterProvider* LangServer::createTabCompleter(TextEdit* doc) {
		return nullptr;
	}

	/*!
		\since YSS 0.13.0
		创建一个鼠标悬停提示提供者实例。默认实现返回nullptr，表示不提供鼠标悬停提示功能。

		传入该函数的TextEdit将会接管HoverInfoProvider的所有权，且独占使用。
		因此，你应该为每一次调用都创建新的HoverInfoProvider实例，而不是复用同一个实例。
		如果你需要在不同的文档之间共享某些数据（如变量名称、函数定义等），请另外创建一个数据管理类，并在HoverInfoProvider中引用它。
	*/
	HoverInfoProvider* LangServer::createHoverInfoProvider(TextEdit* doc) {
		return nullptr;
	}

	/*!
		\since YSS 0.13.0
		创建一个格式化规范提供者实例。默认实现返回nullptr，表示不提供格式化规范功能。
		传入该函数的TextEdit将会接管FormatNormalizer的所有权，且独占使用。
		因此，你应该为每一次调用都创建新的FormatNormalizer实例，而不是复用同一个实例。
		如果你需要在不同的文档之间共享某些数据（如变量名称、函数定义等），请另外创建一个数据管理类，并在FormatNormalizer中引用它。
	*/
	FormatNormalizer* LangServer::createFormatNormalizer(TextEdit* doc) {
		return nullptr;
	}

	/*!
		\since YSS 0.13.0
		获取语言ID
	*/
	QString LangServer::getLangID(){
		return d->LanguageID;
	}

	/*!
		\since YSS 0.13.0
		获取支持的文件扩展名列表
	*/
	QStringList LangServer::getLangExts(){
		return d->LanguageExt;
	}
}