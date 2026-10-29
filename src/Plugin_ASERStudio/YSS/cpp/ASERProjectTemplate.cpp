#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qmap.h>
#include <QtCore/qresource.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qpushbutton.h>
#include <Editor/EditorPlugin.h>
#include <General/Log.h>
#include <General/PluginManager.h>
#include <General/TranslationHost.h>
#include <General/YSSProject.h>
#include <Utility/FileOperation.h>
#include <Utility/JsonConfig.h>
#include <Widgets/ConfigWidget.h>
#include "YSS/ASERProjectTemplate.h"
#include "YSS/DS_AStoryXDebugger.h"
namespace ASERStudio::YSS {
	static const QString ASERStudioPluginID = QStringLiteral("cn.yxgeneral.aserstudio");
	static const QString ASERStudioAssets1Rcc = QStringLiteral("ASERStudio_assets1.rcc");

	static QByteArray readTemplateFile(const QString& srcPath) {
		QFile file(srcPath);
		if (!file.open(QIODevice::ReadOnly)) {
			vgErrorF << "Failed to read project template file " << srcPath;
			return QByteArray();
		}
		return file.readAll();
	}

	// ASERStudio_assets1.rcc 的"现场加载、现场使用、现场卸载"生命周期管理。
	// 项目模板文件(template/3.0、3.6.7)不再嵌入插件二进制，而是编译为独立的 .rcc 文件，
	// 与插件 .vpl 一起放在插件二进制目录中。创建项目窗口时加载，窗口销毁后卸载，
	// 避免大体积模板资源常驻内存。使用引用计数，多个模板窗口同时存在时只注册一次。
	class ProjectTemplateAssets {
	public:
		static void registerAssets() {
#ifdef Q_OS_ANDROID
			return;
#else
			++InstanceCount;
			if (InstanceCount == 1) {
				Visindigo::General::Plugin* plugin = Visindigo::General::PluginManager::getInstance()->getPluginByID(ASERStudioPluginID);
				if (plugin) {
					RccFilePath = plugin->getPluginBinaryFolder().filePath(ASERStudioAssets1Rcc);
					Registered = QResource::registerResource(RccFilePath);
					if (!Registered) {
						vgWarning << "Failed to register resource:" << RccFilePath;
					}
				}
				else {
					vgWarning << "ASERStudio plugin instance not found, cannot locate" << ASERStudioAssets1Rcc;
				}
			}
#endif
		}
		static void unregisterAssets() {
#ifdef Q_OS_ANDROID
			return;
#else
			if (InstanceCount > 0) {
				--InstanceCount;
			}
			if (InstanceCount == 0 && Registered) {
				QResource::unregisterResource(RccFilePath);
				Registered = false;
				RccFilePath.clear();
			}
#endif
		}
	private:
		static int InstanceCount;
		static QString RccFilePath;
		static bool Registered;
	};
	int ProjectTemplateAssets::InstanceCount = 0;
	QString ProjectTemplateAssets::RccFilePath;
	bool ProjectTemplateAssets::Registered = false;

	class ProjectTemplateInitWidget_AStoryXPrivate {
		friend class ProjectTemplateInitWidget_AStoryX;
	protected:
		Visindigo::Widgets::ConfigWidget* ConfigWidget;
		QLabel* WhereLabel;
		QPushButton* CreateButton;
		QVBoxLayout* Layout;
		QHBoxLayout* ButtonLayout;
	};
	ProjectTemplateInitWidget_AStoryX::ProjectTemplateInitWidget_AStoryX(const YSSCore::General::YSSURI& targetUri,
		QWidget* parent)
		: YSSCore::Editor::ProjectTemplateInitWidget(targetUri, parent)
	{
		d = new ProjectTemplateInitWidget_AStoryXPrivate();
		ProjectTemplateAssets::registerAssets();
		this->setMinimumWidth(560);
		this->setWindowTitle(VITRL("ASERStudio::provider.window.title"));
		d->ConfigWidget = new Visindigo::Widgets::ConfigWidget(this);
		Visindigo::Utility::FileOperation::Errorable<QString> cwJsonResult = Visindigo::Utility::FileOperation::readAll(":/resource/cn.yxgeneral.aserstudio/configWidget/PTP.json");
		if (cwJsonResult) {
			d->ConfigWidget->loadCWJson(cwJsonResult.value());
		}
		else {
			vgErrorF << "Failed to read project template config widget json, error: " << Visindigo::Utility::FileOperation::errorCodeName(cwJsonResult.error());
		}
		d->Layout = new QVBoxLayout(this);
		d->Layout->addWidget(d->ConfigWidget);
		d->ButtonLayout = new QHBoxLayout();
		d->ButtonLayout->setContentsMargins(10, 0, 10, 0);
		d->Layout->addLayout(d->ButtonLayout);
		d->WhereLabel = new QLabel(this);
		d->WhereLabel->setWordWrap(true);
		d->ButtonLayout->addWidget(d->WhereLabel);
		d->CreateButton = new QPushButton(VITRL("ASERStudio::provider.window.create"), this);
		d->CreateButton->setObjectName("CreateButton");
		d->CreateButton->setMinimumWidth(120);
		d->CreateButton->setMaximumWidth(160);
		d->ButtonLayout->addWidget(d->CreateButton);
		connect(d->CreateButton, &QPushButton::clicked, this, &ProjectTemplateInitWidget_AStoryX::onCreateButtonClicked);
		refreshWhereLabel();
	}

	ProjectTemplateInitWidget_AStoryX::~ProjectTemplateInitWidget_AStoryX() {
		ProjectTemplateAssets::unregisterAssets();
		delete d;
	}

	void ProjectTemplateInitWidget_AStoryX::onCreateButtonClicked() {
		Visindigo::Utility::JsonConfig* config = d->ConfigWidget->getConfig();
		const QString version = config->getString("Project.Version");
		static const QStringList supportedVersions = { "uptodate" };
		if (!supportedVersions.contains(version)) {
			QMessageBox::warning(this, VITRL("ASERStudio::provider.version.unsupported.title"),
				VITRL("ASERStudio::provider.version.unsupported.text").arg(version));
			return;
		}
		if (!prepareProjectV3()) {
			return;
		}
		emit projectPrepared();
		close();
	}

	bool ProjectTemplateInitWidget_AStoryX::prepareProjectV3() {
		const YSSCore::General::YSSURI targetUri = getTargetUri();
		YSSCore::General::YSSProject project;
		if (!project.initConfig(targetUri, targetUri.getFileName())) {
			QMessageBox::warning(this, VITRL("ASERStudio::provider.failed.title"),
				VITRL("ASERStudio::provider.failed.text"));
			return false;
		}
		project.setProjectIconPath(targetUri.resolve("cover.png"));
		project.setProjectDebugServerID(ASERStudio_AStoryXDebuggerID);
		Visindigo::Utility::JsonConfig projectConfig = project.getProjectConfigForPlugin(ASERStudioPluginID);
		projectConfig.setString("ASERVersion", "3.0");
		project.saveProjectConfigForPlugin(ASERStudioPluginID, projectConfig);
		setYSSProjectContent(project.getYSSProjectContent());

		static const QString templatePath = QStringLiteral(":/resource/cn.yxgeneral.aserstudio/template/3.0");
		static const QMap<QString, QString> fileMap = {
			{ "AdvanceRule.json", "Rules/AdvanceRule.json" },
			{ "BaseRule.json", "Rules/BaseRule.json" },
			{ "main.astoryx", "Stories/main.astoryx" },
			{ "tianyu_0.png", "Resources/Char_Picture/tianyu/0.png" },
			{ "tianyu_1.png", "Resources/Char_Picture/tianyu/1.png" },
			{ "classic.png", "Resources/Background/bg.png" }
		};
		for (const QString& key : fileMap.keys()) {
			setFileContent(fileMap.value(key), readTemplateFile(templatePath + "/" + key));
		}
		setFileContent(QStringLiteral("cover.png"), readTemplateFile(templatePath + "/classic.png"));
		setDefaultOpenFiles(QStringList({ QStringLiteral("Stories/main.astoryx") }));
		return true;
	}

	void ProjectTemplateInitWidget_AStoryX::refreshWhereLabel() {
		const YSSCore::General::YSSURI targetUri = getTargetUri();
		if (!targetUri.isValid()) {
			d->WhereLabel->clear();
			d->CreateButton->setEnabled(false);
			return;
		}
		const QString localPath = targetUri.getLocalPath();
		d->WhereLabel->setText(VITRL("ASERStudio::provider.window.where")
			.arg(localPath.isEmpty() ? targetUri.toString() : localPath));
		d->CreateButton->setEnabled(true);
	}

	ProjectTemplate_AStoryX::ProjectTemplate_AStoryX(YSSCore::Editor::EditorPlugin* plugin) :
		ProjectTemplateProvider("ASE AStory Project Template Provider", "ProjectTemplate_AStoryX", plugin) {
		setTemplateIconPath(":/resource/cn.yxgeneral.aserstudio/icon/ASEA_Dark.png");
		setTemplateID("ASEAStoryXProject");
		setTemplateName(VITRL("ASERStudio::provider.astory.name"));
		setTemplateDescription(VITRL("ASERStudio::provider.astory.description"));
		setTemplateTags(QStringList({
				VITRL("ASERStudio::provider.astory.tags.ase"),
				VITRL("ASERStudio::provider.astory.tags.astory"),
				VITRL("ASERStudio::provider.astory.tags.engine-native"),
				VITRL("ASERStudio::provider.astory.tags.script"),
				VITRL("ASERStudio::provider.astory.tags.arknights")
			}));
	}

	ProjectTemplate_AStoryX::~ProjectTemplate_AStoryX() {
	}

	YSSCore::Editor::ProjectTemplateInitWidget* ProjectTemplate_AStoryX::projectInitWidget(
		const YSSCore::General::YSSURI& targetUri) {
		return new ProjectTemplateInitWidget_AStoryX(targetUri);
	}

	class ProjectTemplateInitWidget_AStoryX_3_6_7Private {
		friend class ProjectTemplateInitWidget_AStoryX_3_6_7;
	protected:
		QLabel* WhereLabel;
		QPushButton* CreateButton;
		QVBoxLayout* Layout;
		QHBoxLayout* ButtonLayout;
	};
	ProjectTemplateInitWidget_AStoryX_3_6_7::ProjectTemplateInitWidget_AStoryX_3_6_7(
		const YSSCore::General::YSSURI& targetUri, QWidget* parent)
		: YSSCore::Editor::ProjectTemplateInitWidget(targetUri, parent)
	{
		d = new ProjectTemplateInitWidget_AStoryX_3_6_7Private();
		ProjectTemplateAssets::registerAssets();
		this->setMinimumWidth(560);
		this->setWindowTitle(VITRL("ASERStudio::provider.window.title"));
		d->Layout = new QVBoxLayout(this);
		d->ButtonLayout = new QHBoxLayout();
		d->ButtonLayout->setContentsMargins(10, 10, 10, 10);
		d->Layout->addLayout(d->ButtonLayout);
		d->WhereLabel = new QLabel(this);
		d->WhereLabel->setWordWrap(true);
		d->ButtonLayout->addWidget(d->WhereLabel);
		d->CreateButton = new QPushButton(VITRL("ASERStudio::provider.window.create"), this);
		d->CreateButton->setObjectName("CreateButton");
		d->CreateButton->setMinimumWidth(120);
		d->CreateButton->setMaximumWidth(160);
		d->ButtonLayout->addWidget(d->CreateButton);
		connect(d->CreateButton, &QPushButton::clicked, this, &ProjectTemplateInitWidget_AStoryX_3_6_7::onCreateButtonClicked);
		refreshWhereLabel();
	}

	ProjectTemplateInitWidget_AStoryX_3_6_7::~ProjectTemplateInitWidget_AStoryX_3_6_7() {
		ProjectTemplateAssets::unregisterAssets();
		delete d;
	}

	void ProjectTemplateInitWidget_AStoryX_3_6_7::onCreateButtonClicked() {
		if (!prepareProjectV3()) {
			return;
		}
		emit projectPrepared();
		close();
	}

	bool ProjectTemplateInitWidget_AStoryX_3_6_7::prepareProjectV3() {
		const YSSCore::General::YSSURI targetUri = getTargetUri();
		YSSCore::General::YSSProject project;
		if (!project.initConfig(targetUri, targetUri.getFileName())) {
			QMessageBox::warning(this, VITRL("ASERStudio::provider.failed.title"),
				VITRL("ASERStudio::provider.failed.text"));
			return false;
		}
		project.setProjectIconPath(targetUri.resolve("cover.png"));
		project.setProjectDebugServerID(ASERStudio_AStoryXDebuggerID);
		Visindigo::Utility::JsonConfig projectConfig = project.getProjectConfigForPlugin(ASERStudioPluginID);
		projectConfig.setString("ASERVersion", "3.6.7");
		project.saveProjectConfigForPlugin(ASERStudioPluginID, projectConfig);
		setYSSProjectContent(project.getYSSProjectContent());

		static const QString templatePath = QStringLiteral(":/resource/cn.yxgeneral.aserstudio/template/3.6.7");
		static const QMap<QString, QString> fileMap = {
			{ "Configs/override_aliases.json", "Configs/override_aliases.json" },
			{ "Configs/override_userConfigs.json", "Configs/override_userConfigs.json" },
			{ "Resources/Background/bg_amb2026.png", "Resources/Background/bg_amb2026.png" },
			{ "Resources/Char_Picture/mon3tr(amb2026)/mon3tr(amb2026)_config.json", "Resources/Char_Picture/mon3tr(amb2026)/mon3tr(amb2026)_config.json" },
			{ "Resources/Char_Picture/mon3tr(amb2026)/normal.png", "Resources/Char_Picture/mon3tr(amb2026)/normal.png" },
			{ "Resources/Char_Spine/Amiya(furnace_finale)/Amiya(furnace_finale).atlas", "Resources/Char_Spine/阿米娅(furnace_finale)/阿米娅(furnace_finale).atlas" },
			{ "Resources/Char_Spine/Amiya(furnace_finale)/Amiya(furnace_finale).png", "Resources/Char_Spine/阿米娅(furnace_finale)/阿米娅(furnace_finale).png" },
			{ "Resources/Char_Spine/Amiya(furnace_finale)/Amiya(furnace_finale).skel", "Resources/Char_Spine/阿米娅(furnace_finale)/阿米娅(furnace_finale).skel" },
			{ "Resources/Char_Spine/Amiya(furnace_finale)/Amiya(furnace_finale)_config.json", "Resources/Char_Spine/阿米娅(furnace_finale)/阿米娅(furnace_finale)_config.json" },
			{ "Resources/Music/immutable_ins.wav", "Resources/Music/immutable_ins.wav" },
			{ "Resources/SoundEffect/MagneticBurst.wav", "Resources/SoundEffect/磁爆.wav" },
			{ "Resources/VisualEffect/manifest.json", "Resources/VisualEffect/manifest.json" },
			{ "Resources/VisualEffect/prefab/furnace_finale.ab", "Resources/VisualEffect/prefab/furnace_finale.ab" },
			{ "Resources/VisualEffect/shader/shader.ab", "Resources/VisualEffect/shader/shader.ab" },
			{ "Resources/VisualEffect/texture/noise.ab", "Resources/VisualEffect/texture/noise.ab" },
			{ "Resources/VisualEffect/texture/ring.ab", "Resources/VisualEffect/texture/ring.ab" },
			{ "Rules/AdvanceRule.json", "Rules/AdvanceRule.json" },
			{ "Rules/BaseRule.json", "Rules/BaseRule.json" },
			{ "Stories/00_BasicExample_1.astoryx", "Stories/00_基础示例上.astoryx" },
			{ "Stories/01_BasicExample_2.astoryx", "Stories/01_基础示例下.astoryx" },
			{ "Stories/02_BasicExample_3.astoryx", "Stories/02_基础？？？.astoryx" }
		};
		for (const QString& key : fileMap.keys()) {
			setFileContent(fileMap.value(key), readTemplateFile(templatePath + "/" + key));
		}
		setFileContent(QStringLiteral("cover.png"),
			readTemplateFile(templatePath + "/Resources/Background/bg_amb2026.png"));
		setDefaultOpenFiles(QStringList({
			QStringLiteral("Stories/00_基础示例上.astoryx"),
			QStringLiteral("Stories/01_基础示例下.astoryx"),
			QStringLiteral("Stories/02_基础？？？.astoryx")
			}));
		return true;
	}

	void ProjectTemplateInitWidget_AStoryX_3_6_7::refreshWhereLabel() {
		const YSSCore::General::YSSURI targetUri = getTargetUri();
		if (!targetUri.isValid()) {
			d->WhereLabel->clear();
			d->CreateButton->setEnabled(false);
			return;
		}
		const QString localPath = targetUri.getLocalPath();
		d->WhereLabel->setText(VITRL("ASERStudio::provider.window.where")
			.arg(localPath.isEmpty() ? targetUri.toString() : localPath));
		d->CreateButton->setEnabled(true);
	}

	ProjectTemplate_AStoryX_3_6_7::ProjectTemplate_AStoryX_3_6_7(YSSCore::Editor::EditorPlugin* plugin) :
		ProjectTemplateProvider("ASE AStory 3.6.7 Project Template Provider", "ProjectTemplate_AStoryX_3_6_7", plugin) {
		setTemplateIconPath(":/resource/cn.yxgeneral.aserstudio/icon/ASEA_Dark.png");
		setTemplateID("ASEAStoryXProject367");
		setTemplateName(VITRL("ASERStudio::provider.astory367.name"));
		setTemplateDescription(VITRL("ASERStudio::provider.astory367.description"));
		setTemplateTags(QStringList({
				VITRL("ASERStudio::provider.astory.tags.ase"),
				VITRL("ASERStudio::provider.astory.tags.astory"),
				VITRL("ASERStudio::provider.astory.tags.engine-native"),
				VITRL("ASERStudio::provider.astory.tags.script"),
				VITRL("ASERStudio::provider.astory.tags.arknights")
			}));
	}

	ProjectTemplate_AStoryX_3_6_7::~ProjectTemplate_AStoryX_3_6_7() {
	}

	YSSCore::Editor::ProjectTemplateInitWidget* ProjectTemplate_AStoryX_3_6_7::projectInitWidget(
		const YSSCore::General::YSSURI& targetUri) {
		return new ProjectTemplateInitWidget_AStoryX_3_6_7(targetUri);
	}
}
