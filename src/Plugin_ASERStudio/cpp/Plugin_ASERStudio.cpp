#include <functional>
#include <QtCore/qdir.h>
#include <QtCore/qelapsedtimer.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qthread.h>
#include <QtCore/qtimer.h>
#include <QtWidgets/qmessagebox.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <Utility/FileOperation.h>
#include <Utility/FileUtility.h>
#include <Utility/StringUtility.h>
#include <Widgets/ConfigWidget.h>
#include <Widgets/ThemeManager.h>
#include "ASEREnv/ASERDebugIO.h"
#include "ASEREnv/ASERProgram.h"
#include "ASEREnv/ASERResourceMoniter.h"
#include "ASEREnv/ASERWarpper.h"
#include "Plugin_ASERStudio.h"
#include "AStorySyntax/AStoryXRule.h"
#include "YSS/ASERFileServer.h"
#include "YSS/ASERFileTemplate.h"
#include "YSS/ASERProjectResourceBrowser.h"
#include "YSS/ASERProjectTemplate.h"
#include "YSS/DS_AStoryXDebugger.h"
#include "YSS/ImageCacheHelper.h"
#include "YSS/InlayColorProvider.h"
#include "YSS/LangServer_AStoryX.h"

VI_DECLARE_LOGGER(StartupProbe, ASERStartupProbe, ASERStudio);

namespace ASERStudio {
	ASERStudioTranslator::ASERStudioTranslator(Visindigo::General::Plugin* parent) :
		Visindigo::General::Translator(parent, "ASERStudio")
	{
		setDefaultLang(zh_CN);
		addLangFilePath(zh_CN, ":/resource/cn.yxgeneral.aserstudio/i18n/zh_CN.json");
		addLangFilePath(zh_TW, ":/resource/cn.yxgeneral.aserstudio/i18n/zh_TW.json");
		addLangFilePath(en, ":/resource/cn.yxgeneral.aserstudio/i18n/en.json");
		addLangFilePath(ja, ":/resource/cn.yxgeneral.aserstudio/i18n/ja.json");
		addLangFilePath(jp_less_loanword, ":/resource/cn.yxgeneral.aserstudio/i18n/jp_less_loanword.json");
		addLangFilePath(ko, ":/resource/cn.yxgeneral.aserstudio/i18n/ko.json");
		addLangFilePath(ru, ":/resource/cn.yxgeneral.aserstudio/i18n/ru.json");
		addLangFilePath(de, ":/resource/cn.yxgeneral.aserstudio/i18n/de.json");
		addLangFilePath(fr, ":/resource/cn.yxgeneral.aserstudio/i18n/fr.json");
	}

	/*
		规则文件的扫描与解析都在工作线程上完成。AStoryXRule对象本身不依赖UI，
		只有注册到全局静态表时才必须回到主线程，因为编辑器会从主线程读取它。
	*/
	class RuleLoaderWorker :public QObject {
	public:
		// 结果交付回调，由主线程侧提供，总是被调度到主线程执行。
		using DeliverCallback = std::function<void(quint64, QList<AStorySyntax::AStoryXRule>, QStringList, qint64, int)>;
	public:
		RuleLoaderWorker(QObject* owner, DeliverCallback callback) :
			Owner(owner), Callback(std::move(callback)) {
		}
	public:
		void loadRules(const QString& projectFolder, quint64 generation) {
			QElapsedTimer timer;
			timer.start();
			const QStringList paths = Visindigo::Utility::FileUtility::fileFilter(projectFolder + "/Rules", { "*.json" });
			QList<AStorySyntax::AStoryXRule> rules;
			rules.reserve(paths.size());
			QStringList errors;
			for (const QString& path : paths) {
				Visindigo::Utility::FileOperation::Errorable<QString> contentResult = Visindigo::Utility::FileOperation::readAll(path);
				if (not contentResult) {
					errors.append(QStringLiteral("Failed to read rule file: %1, error: %2").arg(path,
						Visindigo::Utility::FileOperation::errorCodeName(contentResult.error())));
					continue;
				}
				AStorySyntax::AStoryXRule rule("default");
				if (!rule.parseJson(contentResult.value())) {
					errors.append(QStringLiteral("Failed to parse rule file: %1").arg(path));
					continue;
				}
				rule.setName(QFileInfo(path).baseName());
				rules.append(std::move(rule));
			}
			const qint64 elapsed = timer.elapsed();
			const int fileCount = paths.size();
			DeliverCallback callback = Callback;
			QMetaObject::invokeMethod(Owner,
				[callback, generation, rules = std::move(rules), errors = std::move(errors), elapsed, fileCount]() mutable {
					callback(generation, std::move(rules), std::move(errors), elapsed, fileCount);
				}, Qt::QueuedConnection);
		}
	private:
		QObject* Owner = nullptr;
		DeliverCallback Callback;
	};

	class MainPrivate {
		friend class Main;
	protected:
		Visindigo::Widgets::ConfigWidget* ConfigWidget = nullptr;
		ASERStudio::ASEREnv::ASERProgram* ASERProgram = nullptr;
		ASERStudio::ASEREnv::ASERDebugIO* ASERDebugIO = nullptr;
		QThread* RuleLoaderThread = nullptr;
		RuleLoaderWorker* RuleLoader = nullptr;
		// 每次打开项目都会递增，用来丢弃已经过期的加载批次。
		quint64 RuleLoadGeneration = 0;
		static Main* Instance;
	public:
		void setupRuleLoader(Main* owner) {
			if (RuleLoaderThread != nullptr) {
				return;
			}
			RuleLoaderThread = new QThread();
			RuleLoaderThread->setObjectName("ASERRuleLoader");
			RuleLoader = new RuleLoaderWorker(owner, [this](quint64 generation, QList<AStorySyntax::AStoryXRule> rules,
				QStringList errors, qint64 elapsed, int fileCount) {
					deliverRules(generation, std::move(rules), errors, elapsed, fileCount);
				});
			RuleLoader->moveToThread(RuleLoaderThread);
			QObject::connect(RuleLoaderThread, &QThread::finished, RuleLoader, &QObject::deleteLater);
			RuleLoaderThread->start();
		}
		void startRuleLoading(const QString& projectFolder, quint64 generation) {
			RuleLoaderWorker* worker = RuleLoader;
			if (worker == nullptr) {
				return;
			}
			QMetaObject::invokeMethod(worker, [worker, projectFolder, generation]() {
				worker->loadRules(projectFolder, generation);
				}, Qt::QueuedConnection);
		}
		void deliverRules(quint64 generation, QList<AStorySyntax::AStoryXRule> rules, const QStringList& errors,
			qint64 elapsed, int fileCount) {
			if (generation != RuleLoadGeneration) {
				ASERStartupProbe->debug() << "discard stale ASER rule batch, generation:" << generation
					<< "current generation:" << RuleLoadGeneration;
				return;
			}
			for (const AStorySyntax::AStoryXRule& rule : rules) {
				AStorySyntax::AStoryXRule::registerRule(rule);
			}
			for (const QString& error : errors) {
				vgWarning << error;
			}
			ASERStartupProbe->debug() << "ASER rule loading finished, files:" << fileCount
				<< "registered:" << rules.size() << "worker elapsed(ms):" << elapsed;
			// 规则就绪前打开的文档是按默认规则着色的，这里让它们重新解析。
			YSS::AStoryXLanguageServer::reloadRulesForOpenDocuments();
		}
		void shutdownRuleLoader() {
			++RuleLoadGeneration;
			if (RuleLoaderThread == nullptr) {
				return;
			}
			if (RuleLoaderThread->isRunning()) {
				// 等待正在进行的批次结束；它只读文件并解析JSON，不会访问UI。
				RuleLoaderThread->quit();
				RuleLoaderThread->wait();
			}
			RuleLoader = nullptr;
			delete RuleLoaderThread;
			RuleLoaderThread = nullptr;
		}
	};

	Main* MainPrivate::Instance = nullptr;

	Main::Main() : YSSCore::Editor::EditorPlugin("cn.yxgeneral.aserstudio") {
		d = new MainPrivate;
		MainPrivate::Instance = this;
		setPluginVersion(Visindigo::General::Version(Visindigo_VERSION_MAJOR + 2, Visindigo_VERSION_MINOR - 13, Visindigo_VERSION_PATCH));
		setPluginName("ASER Studio");
		setPluginAuthor({ "Gra_dus", "Tsing Yayin" });
	}

	Main* Main::getInstance() {
		return MainPrivate::Instance;
	}

	Main::~Main() {
		MainPrivate::Instance = nullptr;
		d->shutdownRuleLoader();
		delete d->ConfigWidget;
		delete d;
	}

	void Main::onPluginEnable() {
		Visindigo::Utility::JsonConfig config = VISTM->getMergedColorScheme();
		//vgDebug << "Merged Color Scheme:" << config;
		// not prepared.
		//registerToolWidget("cn.yxgeneral.aserstudio.aser_warpper", "i18n:ASERStudio::toolWidget.aserWarpper.title");
		registerPluginModule(new ASERStudioTranslator(this));
		registerProjectTemplateProvider(new YSS::ProjectTemplate_AStoryX(this));
		registerProjectTemplateProvider(new YSS::ProjectTemplate_AStoryX_3_6_7(this));
		registerFileTemplateProvider(new YSS::FileTemplate_AStoryX(this));
		registerLangServer(new YSS::AStoryXLanguageServer(this));
		registerInlayHintProvider(new YSS::InlayColorProvider(QStringLiteral("Inlay Color"),
			YSS::InlayColorProviderID, this));
		registerFileServer(new YSS::FileServer_AStoryX(this));
		registerFileServer(new YSS::FileServer_ASRuleJson(this));
		registerFileServer(new YSS::FileServer_ASERProjectResourceBrowser(this));
		registerDebugServer(new YSS::DS_AStoryXDebugger(this));
		d->ConfigWidget = new Visindigo::Widgets::ConfigWidget();
		Visindigo::Utility::FileOperation::Errorable<QString> cwJsonResult = Visindigo::Utility::FileOperation::readAll(":/resource/cn.yxgeneral.aserstudio/configWidget/pluginConfig.json");
		if (cwJsonResult) {
			d->ConfigWidget->loadCWJson(cwJsonResult.value());
		}
		else {
			vgErrorF << "Failed to read plugin config widget json, error: " << Visindigo::Utility::FileOperation::errorCodeName(cwJsonResult.error());
		}
		d->ConfigWidget->setTargetConfig(getPluginFolder().filePath("config.json"));
		vgDebug << getPluginFolder().filePath("config.json");
		connect(d->ConfigWidget, &Visindigo::Widgets::ConfigWidget::saved, [this]() {
			this->reloadPluginConfig();
			this->onConfigLoaded();
			});
		connect(d->ConfigWidget, &Visindigo::Widgets::ConfigWidget::reseted, [this]() {
			this->reloadPluginConfig();
			this->onConfigLoaded();
			});
		d->ASERProgram = new ASERStudio::ASEREnv::ASERProgram();
		d->ASERDebugIO = new ASERStudio::ASEREnv::ASERDebugIO();
		d->ASERDebugIO->setProgram(d->ASERProgram);
		onConfigLoaded();
		d->setupRuleLoader(this);
		// 资源监视器的构造会读取官方配置并建立文件监视，放在启动阶段完成，
		// 避免第一次打开项目时才承担这部分初始化。
		ASERStudio::ASEREnv::ASERResourceMoniter::getInstance();
	}
	void Main::onApplicationInit() {}
	void Main::onPluginDisable() {
		// 规则加载线程需要在插件卸载前收尾，避免它继续持有本插件的对象
		d->shutdownRuleLoader();
		// 缩略图缓存的工作线程需要在这里收尾，否则插件被卸载后它仍然会持有数据库连接
		YSS::ImageCacheHelper::getInstance()->shutdown();
		if (d->ASERDebugIO) {
			delete d->ASERDebugIO;
			d->ASERDebugIO = nullptr;
		}
		if (d->ASERProgram) {
			delete d->ASERProgram;
			d->ASERProgram = nullptr;
		}
	}
	void Main::onTest() {}

	void Main::onProjectOpen(YSSCore::General::YSSProject* project) {
		QElapsedTimer startupProbeTimer;
		startupProbeTimer.start();
		QString projectFolder = project->getProjectFolder().getLocalPath();
		if (projectFolder.isEmpty()) {
			vgWarning << "ASERStudio rule loading currently only supports local projects:"
				<< project->getProjectPath().toString();
			return;
		}
		// 旧规则先失效，避免新项目在规则加载完成前误用上一个项目的规则。
		ASERStudio::AStorySyntax::AStoryXRule::clearRules();
		const quint64 generation = ++d->RuleLoadGeneration;
		d->setupRuleLoader(this);
		d->startRuleLoading(projectFolder, generation);
		// 资源扫描是同步的，放到事件循环的下一轮执行，避免阻塞 MainWin 的构造和首次显示。
		QTimer::singleShot(0, this, [this, projectFolder, generation]() {
			if (generation != d->RuleLoadGeneration) {
				return;
			}
			QElapsedTimer resourceProbeTimer;
			resourceProbeTimer.start();
			ASERRM->changeProjectPath(projectFolder);
			ASERStartupProbe->debug() << "ASER resource monitor updated, generation:" << generation
				<< "elapsed(ms):" << resourceProbeTimer.elapsed();
		});
		ASERStartupProbe->debug() << "ASER rule loading scheduled, generation:" << generation
			<< "resource monitor deferred, synchronous total(ms):" << startupProbeTimer.elapsed();
	}

	void Main::onConfigLoaded() {
		d->ASERProgram->setExecutablePath(getPluginConfig()->getString("ASERExeSettings.ExePath"));
	}

	void Main::onProjectClose(YSSCore::General::YSSProject* project) {
		// 作废还在加载中的批次，避免项目关闭后它们再注册进来。
		++d->RuleLoadGeneration;
		ASERStudio::AStorySyntax::AStoryXRule::clearRules();
		if (d->ASERProgram->isRunning()) {
			d->ASERProgram->stop();
		}
	}

	bool Main::onProjectAboutToClose(YSSCore::General::YSSProject* project) {
		if (d->ASERProgram->isRunning()) {
			QMessageBox::StandardButton result = 
				QMessageBox::question(nullptr, VITRL("ASERStudio::project.close.stillRunning.title"), VITRL("ASERStudio::project.close.stillRunning.message"),
					QMessageBox::Yes | QMessageBox::No);
			if (result == QMessageBox::Yes) {
				return true; // allow to close, ASER program will be stopped in onProjectClose.
			} else {
				return false;
			}
		}
		return true;
	}

	QWidget* Main::getConfigWidget() {
		auto aser_programs = ASEREnv::ASERResourceMoniter::getInstance()->getASERProgramPaths();
		QList<QPair<QString, QString>> items;
		for (auto aser_info : aser_programs) {
			QString elidedPath = Visindigo::Utility::StringUtility::autoElide(aser_info.first, 30);
			QString text = "(" + aser_info.second + ") " + elidedPath;
			items.append({text, aser_info.first});
		}
		d->ConfigWidget->setLineEditPrefabricatedItems("ASERExeSettings.ExePath", items);
		return d->ConfigWidget;
	}

	ASERStudio::ASEREnv::ASERDebugIO* Main::getASERDebugIO() const {
		return d->ASERDebugIO;
	}

	ASERStudio::ASEREnv::ASERProgram* Main::getASERProgram() const {
		return d->ASERProgram;
	}
}