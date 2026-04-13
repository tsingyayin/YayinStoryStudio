#include "../FileTemplateProvider.h"
#include <General/Plugin.h>
#include "Editor/EditorPlugin.h"
namespace YSSCore::Editor {
	class FileTemplateInitWidgetPrivate
	{
		friend class FileTemplateInitWidget;
	protected:
		QString InitFolder;
	};

	/*!
		\class YSSCore::Editor::FileTemplateInitWidget
		\brief FileTemplateInitWidget是一个QFrame，作为FileTemplateProvider的初始化界面。
		\since YSS 0.13.0
		\inmodule YSSCore
		\ingroup CreateService

		FileTemplateInitWidget是一个QFrame，作为FileTemplateProvider的初始化界面。
		当用户选择某个文件模板来创建文件时，编辑器会调用该模板的fileInitWidget函数，
		获取一个FileTemplateInitWidget实例，并将其显示在界面上。用户可以在这个界面上进行一些必要的配置（如文件名称、保存路径等），
		然后点击“创建”按钮来完成文件的创建过程。

		请注意，为了与YSS解耦，这个类设计上并未采用要求用户必须创建持续存活YSSCore::Editor::FileEditWidget实例
		并在初始化完毕后将其传递给编辑器的方式来完成文件创建。

		相反，你可以在这个页面上收集到足够的信息之后自由的根据需要创建文件，填充文件内容等，
		最后通过filePrepared信号，将文件路径推送出去，编辑器会监测这个信号，并使用标准的从磁盘打开文件的流程来打开这个新文件。
		这种设计也使得YSS可以拥有统一的文件加载流程，而不需要担心不同模板的创建流程可能会导致的兼容性问题。

		至于此类的生命周期，编辑器会在显示它之前通过QWidget::setAttribute将其设置为关闭时自动销毁。

		如果你认为根据用户的输入，无法创建文件， 那么只需要不发出任何信号即可。用户直接关闭
		此页面会被编辑器视为取消创建操作，不会有任何后续操作发生。

		\warning 这类在0.14.0调整了虚函数，因此ABI与此前不兼容。
	*/

	/*!
		\fn void FileTemplateInitWidget::filePrepared(QString filePath);
		\since YSS 0.13.0
		当文件准备就绪时（如用户点击了“创建”按钮，并且文件已经生成），你应该发出这个信号，并将文件路径作为参数传递出去。
	*/

	/*!
		\since YSS 0.13.0
		构造函数
		\a initFolder 用户希望文件被创建的初始路径。你也可以选择不尊重这个参数。
		\a parent 父对象
	*/
	FileTemplateInitWidget::FileTemplateInitWidget(const QString& initFolder, QWidget* parent)
		: QFrame(parent) {
		d = new FileTemplateInitWidgetPrivate;
		d->InitFolder = initFolder;
	}

	/*!
		\since YSS 0.13.0
		析构函数
	*/
	FileTemplateInitWidget::~FileTemplateInitWidget() {
		delete d;
	}

	/*!
		\since YSS 0.13.0
		获取用户希望文件被创建的初始路径。你也可以选择不尊重这个参数。
	*/
	QString FileTemplateInitWidget::getInitFolder() const {
		return d->InitFolder;
	}

	class FileTemplateProviderPrivate {
		friend class FileTemplateProvider;
	protected:
		QString TemplateIconPath;
		QString TemplateID;
		QString TemplateName;
		QString TemplateDescription;
		QStringList TemplateTags;
	};

	/*!
		\class YSSCore::Editor::FileTemplateProvider
		\brief FileTemplateProvider向编辑器提供一个文件模板。
		\since YSS 0.13.0
		\inmodule YSSCore
		\ingroup CreateService

		FileTemplateProvider是一个插件模块，向编辑器提供一个文件模板。编辑器会在新建文件界面展示所有注册的FileTemplateProvider，
		用户可以选择一个模板来创建文件。

		值得指出的是，YSS假定了所有创建操作都需要通过一个UI来解决，所以这个Provider类本身主要用于记录
		文件的相关信息（如图标、ID、名称、描述、标签等），以及提供一个UI界面来引导用户完成创建过程。真正的创建逻辑应该在FileTemplateInitWidget中实现。

		\warning 这类在0.14.0完善了const语义，因此ABI与此前不兼容。
	*/

	/*!
		\since YSS 0.13.0
		\value name 模板名称
		\value id 模板ID，必须唯一
		\value plugin 提供该模板的插件实例
		构造函数
	*/
	FileTemplateProvider::FileTemplateProvider(const QString& name, const QString& id, EditorPlugin* plugin) :
		Visindigo::General::PluginModule((Visindigo::General::Plugin*)plugin, id, YSSPluginModule_FileTemplateProvider, name) {
		d = new FileTemplateProviderPrivate;
	}

	/*!
		\since YSS 0.13.0
		析构函数
	*/
	FileTemplateProvider::~FileTemplateProvider() {
		delete d;
	}

	/*!
		\since YSS 0.13.0
		 获取模板图标路径
	*/
	QString FileTemplateProvider::getTemplateIconPath() const {
		return d->TemplateIconPath;
	}

	/*!
		\since YSS 0.13.0
		 设置模板图标路径
	*/
	void FileTemplateProvider::setTemplateIconPath(const QString& iconPath) {
		d->TemplateIconPath = iconPath;
	}

	/*!
		\since YSS 0.13.0
		 获取模板ID
	*/
	QString FileTemplateProvider::getTemplateID() const {
		return d->TemplateID;
	}

	/*!
		\since YSS 0.13.0
		 设置模板ID
	*/
	void FileTemplateProvider::setTemplateID(const QString& id) {
		d->TemplateID = id;
	}
	
	/*!
		\since YSS 0.13.0
		 获取模板名称
	*/
	QString FileTemplateProvider::getTemplateName() const	{
		return d->TemplateName;
	}

	/*!
		\since YSS 0.13.0
		 设置模板名称
	*/
	void FileTemplateProvider::setTemplateName(const QString& name) {
		d->TemplateName = name;
	}

	/*!
		\since YSS 0.13.0
		 获取模板描述
	*/
	QString FileTemplateProvider::getTemplateDescription() const {
		return d->TemplateDescription;
	}

	/*!
		\since YSS 0.13.0
		 设置模板描述
	*/
	void FileTemplateProvider::setTemplateDescription(const QString& description) {
		d->TemplateDescription = description;
	}

	/*!
		\since YSS 0.13.0
		 获取模板标签
	*/
	QStringList FileTemplateProvider::getTemplateTags() const {
		return d->TemplateTags;
	}

	/*!
		\since YSS 0.13.0
		 设置模板标签
	*/
	void FileTemplateProvider::setTemplateTags(const QStringList& tags) {
		d->TemplateTags = tags;
	}
}