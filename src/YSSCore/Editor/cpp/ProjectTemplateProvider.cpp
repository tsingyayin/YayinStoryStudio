#include <General/Plugin.h>
#include <Utility/JsonConfig.h>
#include <QtCore/qmap.h>
#include "Editor/EditorPlugin.h"
#include "Editor/ProjectTemplateProvider.h"
namespace YSSCore::Editor {
	class ProjectTemplateInitWidgetPrivate {
		friend class ProjectTemplateInitWidget;
	protected:
		YSSCore::General::YSSURI TargetUri;
		Visindigo::Utility::JsonConfig ProjectConfig;
		QMap<QString, QByteArray> FileContents;
		QStringList DefaultOpenFiles;
	};

	/*!
		\class YSSCore::Editor::ProjectTemplateInitWidget
		\brief ProjectTemplateInitWidget是一个QFrame，作为ProjectTemplateProvider的初始化界面.
		\since YSS 0.13.0
		\inmodule YSSCore
		\ingroup CreateService

		ProjectTemplateInitWidget是一个QFrame，作为ProjectTemplateProvider的初始化界面。
		当用户选择某个项目模板来创建项目时，编辑器会调用该模板的\c{projectInitWidget}函数，
		返回一个ProjectTemplateInitWidget实例，并将其显示在界面上。用户可以在这个界面上进行
		一些必要的配置（如目标引擎版本等），然后点击“创建”按钮。

		\warning 与文件模板一致，此类不负责写盘。项目文件夹的创建、所有文件的写入以及项目配置
		的保存都由编辑器通过 \l{YSSUniqueFileSystem} 完成，因此本地项目与远端项目走同一条路径。

		你只需要在这个页面上收集信息，然后：
		\list
		\li 用 \l{setYSSProjectContent()} 给出项目配置。通常先用 \l{YSSProject::initConfig()}
			建一份基础配置，再按模板需要修改插件配置。
		\li 用 \l{setFileContent()} 逐个给出项目内文件的内容，路径相对于项目文件夹。
		\li 用 \l{setDefaultOpenFiles()} 给出项目首次打开时要显示的文件。
		\endlist

		准备好之后发出 \l{projectPrepared()} 信号即可。编辑器会在收到信号后按上述内容
		创建项目文件夹、写入文件、保存 \c{project.yssp}，然后用标准的项目打开流程打开它。

		目标位置由调用方通过构造函数传入，用 \l{getTargetUri()} 读取。它只用于展示与校验，
		不要自己写盘。

		至于此类的生命周期，编辑器会在显示它之前通过QWidget::setAttribute将其设置为关闭时自动销毁。

		如果你认为根据用户的输入无法创建项目，那么只需要不发出任何信号即可。用户直接关闭
		此页面会被编辑器视为取消创建操作，不会有任何后续操作发生。

		\warning 这类在0.18.0改为只提供内容，写盘交给编辑器，ABI与此前不兼容。
	*/

	/*!
		\fn void ProjectTemplateInitWidget::projectPrepared();
		\since YSS 0.18.0

		项目内容准备就绪时（如用户点击了“创建”按钮）发出这个信号。发出之前应当先用
		\l{setYSSProjectContent()}、\l{setFileContent()} 与 \l{setDefaultOpenFiles()}
		把结果填好。
	*/

	/*!
		\since YSS 0.18.0
		\a targetUri 项目文件夹的目标位置。仅用于展示与校验，写盘始终由编辑器完成。
		\a parent 父组件

		构造函数
	*/
	ProjectTemplateInitWidget::ProjectTemplateInitWidget(const YSSCore::General::YSSURI& targetUri,
		QWidget* parent)
		: QFrame(parent) {
		d = new ProjectTemplateInitWidgetPrivate;
		d->TargetUri = targetUri;
	}

	/*!
		\since YSS 0.13.0

		析构函数
	*/
	ProjectTemplateInitWidget::~ProjectTemplateInitWidget() {
		delete d;
	}

	/*!
		\since YSS 0.18.0

		return 项目文件夹的目标位置。
	*/
	YSSCore::General::YSSURI ProjectTemplateInitWidget::getTargetUri() const {
		return d->TargetUri;
	}

	/*!
		\since YSS 0.18.0
		\a inProjRelativePath 相对于项目文件夹的路径，例如 \c{Stories/main.astoryx}
		\a content 该文件的完整内容

		登记一个待写入的文件。路径会被规范化为使用 \c{/} 作分隔符，并去掉前导分隔符。
		同一个路径重复设置时后者覆盖前者。
	*/
	void ProjectTemplateInitWidget::setFileContent(const QString& inProjRelativePath, const QByteArray& content) {
		QString path = inProjRelativePath;
		path.replace(QLatin1Char('\\'), QLatin1Char('/'));
		while (path.startsWith(QLatin1Char('/'))) {
			path.remove(0, 1);
		}
		if (path.isEmpty()) {
			return;
		}
		d->FileContents.insert(path, content);
	}

	/*!
		\since YSS 0.18.0
		\a inProjRelativePath 相对于项目文件夹的路径

		return 该文件的内容；没有登记过时返回空 \c{QByteArray}。
	*/
	QByteArray ProjectTemplateInitWidget::getFileContent(const QString& inProjRelativePath) const {
		return d->FileContents.value(inProjRelativePath);
	}

	/*!
		\since YSS 0.18.0

		return 全部已登记的文件内容，键为相对于项目文件夹的路径。
	*/
	QMap<QString, QByteArray> ProjectTemplateInitWidget::getAllFileContents() const {
		return d->FileContents;
	}

	/*!
		\since YSS 0.18.0
		\a inProjRelativePaths 首次打开项目时应当显示的文件，路径相对于项目文件夹

		设置项目首次打开时默认打开的文件。编辑器会把它们写进项目的打开记录与焦点。
	*/
	void ProjectTemplateInitWidget::setDefaultOpenFiles(const QStringList& inProjRelativePaths) {
		d->DefaultOpenFiles = inProjRelativePaths;
	}

	/*!
		\since YSS 0.18.0

		return 首次打开项目时应当显示的文件列表。
	*/
	QStringList ProjectTemplateInitWidget::getDefaultOpenFiles() const {
		return d->DefaultOpenFiles;
	}

	/*!
		\since YSS 0.18.0
		\a config 整份项目配置。

		设置项目配置。通常先用 \l{YSSProject::initConfig()} 建一份基础配置，再按模板需要修改，
		最后用 \l{YSSProject::getYSSProjectContent()} 取出交给本函数。
	*/
	void ProjectTemplateInitWidget::setYSSProjectContent(const Visindigo::Utility::JsonConfig& config) {
		d->ProjectConfig = config;
	}

	/*!
		\since YSS 0.18.0

		return 当前项目配置。编辑器会把它设置到新建的 YSSProject 上并保存。
	*/
	Visindigo::Utility::JsonConfig ProjectTemplateInitWidget::getYSSProjectContent() const {
		return d->ProjectConfig;
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
		\brief ProjectTemplateProvider向编辑器提供一个项目模板.
		\since YSS 0.13.0
		\inmodule YSSCore
		\ingroup CreateService

		ProjectTemplateProvider是一个插件模块，向编辑器提供一个项目模板。编辑器会在新建项目界面展示所有注册的ProjectTemplateProvider，
		用户可以选择一个模板来创建项目。

		值得指出的是，YSS假定了所有创建操作都需要通过一个UI来解决，所以这个Provider类本身主要用于记录
		项目的相关信息（如图标、ID、名称、描述、标签等），以及提供一个UI界面来引导用户完成创建过程。
		模板的输入逻辑在ProjectTemplateInitWidget中实现，写盘由编辑器统一完成。

		\warning 这类在0.14.0完善了const语义，因此ABI与此前不兼容。
		\warning 这类在0.18.0把写盘交给编辑器，\l{projectInitWidget()} 因此多了一个目标 URI 参数，ABI与此前不兼容。
	*/

	/*!
		\since YSS 0.13.0
		\a name 模板名称
		\a id 模板ID，必须唯一
		\a plugin 提供该模板的插件实例

		构造函数
	*/
	ProjectTemplateProvider::ProjectTemplateProvider(const QString& name, const QString& id, EditorPlugin* plugin)
		: Visindigo::General::PluginModule((Visindigo::General::Plugin*)plugin, id, YSSPluginModule_ProjectTemplateProvider, name) {
		d = new ProjectTemplateProviderPrivate;
	}

	/*!
		\since YSS 0.13.0

		析构函数
	*/
	ProjectTemplateProvider::~ProjectTemplateProvider() {
		delete d;
	}

	/*!
		\since YSS 0.13.0

		return 模板图标路径
	*/
	QString ProjectTemplateProvider::getTemplateIconPath() const {
		return d->TemplateIconPath;
	}

	/*!
		\since YSS 0.13.0
		\a iconPath 模板图标路径

		设置模板图标路径
	*/
	void ProjectTemplateProvider::setTemplateIconPath(const QString& iconPath) {
		d->TemplateIconPath = iconPath;
	}

	/*!
		\since YSS 0.13.0

		return 模板ID
	*/
	QString ProjectTemplateProvider::getTemplateID() const {
		return d->TemplateID;
	}

	/*!
		\since YSS 0.13.0
		\a id 模板ID，必须唯一

		设置模板ID，必须唯一
	*/
	void ProjectTemplateProvider::setTemplateID(const QString& id) {
		d->TemplateID = id;
	}

	/*!
		\since YSS 0.13.0

		return 模板名称
	*/
	QString ProjectTemplateProvider::getTemplateName() const {
		return d->TemplateName;
	}

	/*!
		\since YSS 0.13.0
		\a name 模板名称

		设置模板名称
	*/
	void ProjectTemplateProvider::setTemplateName(const QString& name) {
		d->TemplateName = name;
	}

	/*!
		\since YSS 0.13.0

		return 模板描述
	*/
	QString ProjectTemplateProvider::getTemplateDescription() const {
		return d->TemplateDescription;
	}

	/*!
		\since YSS 0.13.0
		\a description 模板描述

		设置模板描述
	*/
	void ProjectTemplateProvider::setTemplateDescription(const QString& description) {
		d->TemplateDescription = description;
	}

	/*!
		\since YSS 0.13.0

		return 模板标签列表
	*/
	QStringList ProjectTemplateProvider::getTemplateTags() const {
		return d->TemplateTags;
	}

	/*!
		\since YSS 0.13.0
		\a tags 模板标签列表

		设置模板标签列表
	*/
	void ProjectTemplateProvider::setTemplateTags(const QStringList& tags) {
		d->TemplateTags = tags;
	}

	/*!
		\fn virtual ProjectTemplateInitWidget* ProjectTemplateProvider::projectInitWidget(const YSSCore::General::YSSURI& targetUri) = 0;
		\since YSS 0.18.0
		\a targetUri 项目文件夹的目标位置，由新建项目流程选定后传入。

		YSS内部调用此函数，获取该项目模板的初始化界面。插件开发者需要在子类中实现此函数，
		且必须提供自己的界面：项目模板的输入项差异很大，不存在默认实现。

		return 一个ProjectTemplateInitWidget实例。

		被返回的实例的所有权由YSS接管，并在必要时销毁。
	*/
}