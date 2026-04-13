#include "../ProjectTemplateProvider.h"
#include <General/Plugin.h>
#include "Editor/EditorPlugin.h"
namespace YSSCore::Editor {
	/*!
		\class YSSCore::Editor::ProjectTemplateInitWidget
		\brief ProjectTemplateInitWidget是一个QFrame，作为ProjectTemplateProvider的初始化界面。
		\since YSS 0.13.0
		\inmodule YSSCore
		\ingroup CreateService

		ProjectTemplateInitWidget是一个QFrame，作为ProjectTemplateProvider的初始化界面。
		当用户选择某个项目模板来创建项目时，编辑器会调用该模板的projectInitWidget函数，
		获取一个ProjectTemplateInitWidget实例，并将其显示在界面上。用户可以在这个界面上进行一些必要的配置（如项目名称、保存路径等），
		然后点击“创建”按钮来完成项目的创建过程。

		请注意，为了与YSS解耦，这个类设计上并未采用要求用户必须创建持续存活YSSCore::General::YSSProject实例
		并在初始化完毕后将其传递给编辑器的方式来完成项目创建。
		
		相反，你可以在这个页面上收集到足够的信息之后自由的根据需要创建项目文件夹、生成必要的配置文件等，
		最后通过projectPrepared信号，将项目路径即yssp文件的路径）推送出去，编辑器会监测这个信号，
		并使用标准的从磁盘打开项目的流程来打开这个新项目。这种设计也使得YSS可以拥有统一的项目加载流程，
		而不需要担心不同模板的创建流程可能会导致的兼容性问题。

		因此，如果你在这个类内利用了YSSCore::General::YSSProject来创建项目实例（如果没有特别特别的
		需求，通常也推荐使用这个类进行初始化操作），请确保你的YSSProject实例能与此类一同销毁。

		至于此类的生命周期，编辑器会在显示它之前通过QWidget::setAttribute将其设置为关闭时自动销毁。

		如果你认为根据用户的输入，无法创建项目， 那么只需要不发出任何信号即可。用户直接关闭
		此页面会被编辑器视为取消创建操作，不会有任何后续操作发生。
	*/

	/*!
		\fn void ProjectTemplateInitWidget::projectPrepared(QString projectPath);
		\since YSS 0.13.0
		当项目准备就绪时（如用户点击了“创建”按钮，并且项目文件已经生成），你应该发出这个信号，并将项目路径（即yssp文件的路径）作为参数传递出去。
	*/

	/*!
		\since YSS 0.13.0
		构造函数
	*/
	ProjectTemplateInitWidget::ProjectTemplateInitWidget(QWidget* parent)
		: QFrame(parent) {
	}

	class ProjectTemplateProviderPrivate {
	public:
		QString TemplateIconPath;
		QString TemplateID;
		QString TemplateName;
		QString TemplateDescription;
		QStringList TemplateTags;
	};

	/*
		\class YSSCore::Editor::ProjectTemplateProvider
		\brief ProjectTemplateProvider向编辑器提供一个项目模板。
		\since YSS 0.13.0
		\inmodule YSSCore
		\ingroup CreateService

		ProjectTemplateProvider是一个插件模块，向编辑器提供一个项目模板。编辑器会在新建项目界面展示所有注册的ProjectTemplateProvider，
		用户可以选择一个模板来创建项目。

		值得指出的是，YSS假定了所有创建操作都需要通过一个UI来解决，所以这个Provider类本身主要用于记录
		项目的相关信息（如图标、ID、名称、描述、标签等），以及提供一个UI界面来引导用户完成创建过程。真正的创建逻辑应该在ProjectTemplateInitWidget中实现。

		\warning 这类在0.14.0完善了const语义，因此ABI与此前不兼容。
	*/

	/*!
		\since YSS 0.13.0
		\value name 模板名称
		\value id 模板ID，必须唯一
		\value plugin 提供该模板的插件实例

		构造函数
	*/
	ProjectTemplateProvider::ProjectTemplateProvider(const QString& name, const QString& id, EditorPlugin* plugin)
		: Visindigo::General::PluginModule((Visindigo::General::Plugin*)plugin, id, YSSPluginModule_ProjectTemplateProvider ,name){
		d = new ProjectTemplateProviderPrivate;
	}

	/*!
		\since YSS 0.13.0
		析构函数
	*/
	ProjectTemplateProvider::~ProjectTemplateProvider(){
		delete d;
	}

	/*!
		\since YSS 0.13.0
		获取模板图标路径
	*/
	QString ProjectTemplateProvider::getTemplateIconPath() const{
		return d->TemplateIconPath;
	}

	/*!
		\since YSS 0.13.0
		设置模板图标路径
	*/
	void ProjectTemplateProvider::setTemplateIconPath(const QString& iconPath){
		d->TemplateIconPath = iconPath;
	}

	/*!
		\since YSS 0.13.0
		获取模板ID
	*/
	QString ProjectTemplateProvider::getTemplateID() const{
		return d->TemplateID;
	}

	/*!
		\since YSS 0.13.0
		设置模板ID，必须唯一
	*/
	void ProjectTemplateProvider::setTemplateID(const QString& id){
		d->TemplateID = id;
	}

	/*!
		\since YSS 0.13.0
		获取模板名称
	*/
	QString ProjectTemplateProvider::getTemplateName() const{
		return d->TemplateName;
	}

	/*!
		\since YSS 0.13.0
		设置模板名称
	*/
	void ProjectTemplateProvider::setTemplateName(const QString& name){
		d->TemplateName = name;
	}

	/*!
		\since YSS 0.13.0
		获取模板描述
	*/
	QString ProjectTemplateProvider::getTemplateDescription() const{
		return d->TemplateDescription;
	}

	/*!
		\since YSS 0.13.0
		设置模板描述
	*/
	void ProjectTemplateProvider::setTemplateDescription(const QString& description){
		d->TemplateDescription = description;
	}

	/*!
		\since YSS 0.13.0
		获取模板标签列表
	*/
	QStringList ProjectTemplateProvider::getTemplateTags() const{
		return d->TemplateTags;
	}

	/*!
		\since YSS 0.13.0
		设置模板标签列表
	*/
	void ProjectTemplateProvider::setTemplateTags(const QStringList& tags){
		d->TemplateTags = tags;
	}

	/*!
		\fn virtual ProjectTemplateInitWidget* ProjectTemplateProvider::projectInitWidget() = 0;
		\since YSS 0.13.0
		YSS内部调用此函数，获取该项目模板的初始化界面。插件开发者需要在子类中实现此函数，返回一个ProjectTemplateInitWidget实例。

		被返回的实例的所有权由YSS接管，并在必要时销毁。
	*/
}