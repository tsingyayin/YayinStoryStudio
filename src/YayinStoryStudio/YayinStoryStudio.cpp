#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>
#include <QtCore/qsize.h>
#include <QtCore/qstandardpaths.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qtimer.h>
#include <QtGui/qbrush.h>
#include <QtGui/qcolor.h>
#include <QtGui/qcursor.h>
#include <QtGui/qevent.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qimage.h>
#include <QtGui/qpainter.h>
#include <QtGui/qpen.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qscreen.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qwidget.h>
#include <AgentPage/AgentWin.h>
#include <Editor/ColorThemeProvider.h>
#include <Editor/FileServerManager.h>
#include <Editor/FileTemplateManager.h>
#include <Editor/LangServerManager.h>
#include <Editor/ProjectTemplateManager.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <General/VIApplication.h>
#include <Utility/BenchmarkTimer.h>
#include <Utility/Console.h>
#include <Utility/ExtTool.h>
#include <Utility/FileOperation.h>
#include <Utility/FileUtility.h>
#include <Utility/SevenZipBinder.h>
#include <Widgets/ConfigWidget.h>
#include <Widgets/LiquidGlassEffect.h>
#include <Widgets/Terminal.h>
#include "Editor/InstallerClient.h"
#include "Editor/MainEditor/MainWin.h"
#include "Editor/ProjectPage/ProjectWin.h"
#include "Editor/YSSCommandHandler.h"
#include "Editor/YSSTranslator.h"
#include "YayinStoryStudio.h"
namespace YSS {
	class MainPrivate {
		friend class Main;
	protected:
		Visindigo::Widgets::ConfigWidget* ConfigWidget = nullptr;
		static Main* Instance;
	};
	Main* MainPrivate::Instance = nullptr;

	Main::Main(): Visindigo::General::Plugin("cn.yxgeneral.yayinstorystudio") {
		d = new MainPrivate;
		MainPrivate::Instance = this;
		setTestEnable();
		setPluginVersion(getPluginAPIVersion()); // YSS uses the same version as Visindigo API version
		setPluginName("Yayin Story Studio");
		setPluginAuthor({ "Tsing Yayin" });
		registerColorScheme(":/resource/cn.yxgeneral.yayinstorystudio/vst/editorTheme.json");
#ifdef Q_OS_ANDROID
		try {
#endif
			VIApp->setGlobalFont(":/resource/cn.yxgeneral.yayinstorystudio/HarmonyOS_Sans_SC_Regular.ttf");
			VIApp->setIconFont(":/resource/cn.yxgeneral.visindigo/Segoe Fluent Icons.ttf");
#ifdef Q_OS_ANDROID
		} catch (...) {
			vgWarningF << "Bundled fonts unavailable on Android; falling back to system font.";
		}
#endif
	}

	void Main::onPluginEnable() {
#ifndef Q_OS_ANDROID
		releaseInstaller();
		YSS::Editor::InstallerClient* installerClient = new YSS::Editor::InstallerClient();
		connect(installerClient, &YSS::Editor::InstallerClient::installerRequestProgramClose, this, []() {
			qApp->quit();
			});
		connect(installerClient, &YSS::Editor::InstallerClient::connected, this, []() {
			vgDebug << "Connected to installer.";
			});
		YSS::Editor::InstallerClient::getInstance()->connectToInstaller();
#endif
		auto LangID = Visindigo::General::Translator::stringToLangID(getPluginConfig()->getString("Settings.General.Language"));
		VITRH->setLangID(LangID);
		VISTM->setAnimationDuration(500);
		YSSCore::Editor::FileServerManager::getInstance();
		YSSCore::Editor::ProjectTemplateManager::getInstance();
		YSSCore::Editor::FileTemplateManager::getInstance();
		YSSCore::Editor::LangServerManager::getInstance();
#ifndef Q_OS_ANDROID
		Visindigo::Utility::ExtTool::registerFileExtMetaInfo("vst", "Visindigo StyleSheet Template",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,0");
		Visindigo::Utility::ExtTool::registerFileExtMetaInfo("vpl", "Visindigo Plugin Library",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,1");
		Visindigo::Utility::ExtTool::registerFileExtMetaInfo("yssp", "YayinStoryStudio Project",
			Visindigo::Utility::FileUtility::getProgramPath() + "/YayinStoryStudio.exe,2");
#endif
		
		registerPluginModule(new YSS::Editor::YSSCommandHandler(this));
		registerPluginModule(new YSS::Editor::YSSTranslator(this));
		d->ConfigWidget = new Visindigo::Widgets::ConfigWidget();
		Visindigo::Utility::FileOperation::Errorable<QString> cwJsonResult = Visindigo::Utility::FileOperation::readAll(":/resource/cn.yxgeneral.yayinstorystudio/configWidget/programConfig.json");
		if (cwJsonResult) {
			d->ConfigWidget->loadCWJson(cwJsonResult.value());
		}
		else {
			vgErrorF << "Failed to read program config widget json, error: " << Visindigo::Utility::FileOperation::errorCodeName(cwJsonResult.error());
		}
		d->ConfigWidget->setTargetConfig(getPluginFolder().filePath("config.json"));
		connect(d->ConfigWidget, &Visindigo::Widgets::ConfigWidget::comboBoxIndexChanged, this, [](const QString& node, int index, QString data) {
			vgDebug << node;
			if (node == "General.Theme") {
				VISTM->changeColorTheme(data);
			}
			else if (node == "General.UpdateChannel") {
				const QString metaCachePath = VIApp->getMainPlugin()->getPluginFolder().filePath("meta_cache");
				Visindigo::Utility::FileOperation::ErrorCode deleteResult = Visindigo::Utility::FileOperation::deleteFile(metaCachePath);
				if (deleteResult != Visindigo::Utility::FileOperation::Success && deleteResult != Visindigo::Utility::FileOperation::FileNotFound) {
					vgErrorF << "Failed to delete project meta cache: " << metaCachePath
						<< ", error: " << Visindigo::Utility::FileOperation::errorCodeName(deleteResult);
				}
			}
			});
		connect(d->ConfigWidget, &Visindigo::Widgets::ConfigWidget::saved, this, &Visindigo::General::Plugin::reloadPluginConfig);
		vgDebug << getPluginFolder().filePath("config.json");
		VISTM->setStyleTemplatePriority({ "YSS" });
		VISTM->setColorSchemePriority({ "YSSEditor", "#Default" }); // NOTE: YSS does not have color scheme yet, this is for future us
	}

	// TODO: 临时测试窗口：人工查看 LiquidGlassEffect 的边缘光效果，验证完把这个函数和 onApplicationInit() 里的调用一起删掉
	static QWidget* createRimLightTestWin() {
		using LGE = Visindigo::Widgets::LiquidGlassEffect;
		const QSize winSize(1020, 710);
		const QSize panelSize(125, 150);
		const QSize sampleSize(820, 170);
		QWidget* win = new QWidget();
		win->setWindowTitle("光效测试（边缘光 / 全局打光）");
		win->resize(winSize);
		// 居中到鼠标所在的屏幕：换显示器、改分辨率之后这个窗口也不会跑到屏幕外面去
		QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
		if (!screen) {
			screen = QGuiApplication::primaryScreen();
		}
		if (screen) {
			const QRect available = screen->availableGeometry();
			win->move(available.center() - QPoint(winSize.width() / 2, winSize.height() / 2));
		}
		// 这张底图既是窗口背景，也是下面各个玻璃面板的模糊源
		const QImage background = LGE::drawDebugBackground(winSize.width(), winSize.height());
		QLabel* backgroundLabel = new QLabel(win);
		backgroundLabel->setGeometry(0, 0, winSize.width(), winSize.height());
		backgroundLabel->setPixmap(QPixmap::fromImage(background));
		QLabel* titleLabel = new QLabel(win);
		titleLabel->setGeometry(0, 24, winSize.width(), 40);
		titleLabel->setAlignment(Qt::AlignCenter);
		titleLabel->setStyleSheet("color:#FFFFFF; font-size:19px; background:transparent;");
		titleLabel->setText("把鼠标移到下面的玻璃面板上，边缘光会贴着鼠标最近的那段边界");
		struct PanelSpec {
			const char* title;
			bool rimLight;
			bool globalRim;
			QPointF lightDirection;
			QColor rimColor;
			int rimThickness;
			int rimRange;
			int rimActivation;
			QColor lightColor;
			int lightThickness;
			QColor shadowColor;
			int shadowThickness;
			qreal dispersion;
			int blur;
		};
		// 默认光方向：从左上角来、与竖直方向成30度
		const QPointF defaultLight(-0.5, -0.8660254);
		const PanelSpec specs[] = {
			{ "默认（细·柔）", true, true, defaultLight, QColor(255, 255, 255, 130), 1, 80, 40, QColor(255, 255, 255, 90), 1, QColor(0, 0, 0, 60), 2, 0.0, 16 },
			{ "对照：全关", false, false, defaultLight, QColor(255, 255, 255, 130), 1, 80, 40, QColor(255, 255, 255, 90), 1, QColor(0, 0, 0, 60), 2, 0.0, 16 },
			{ "柔光：厚 3", true, true, defaultLight, QColor(255, 255, 255, 130), 3, 80, 40, QColor(255, 255, 255, 90), 3, QColor(0, 0, 0, 60), 2, 0.0, 16 },
			{ "扭曲：普通", true, true, defaultLight, QColor(255, 255, 255, 130), 6, 80, 40, QColor(255, 255, 255, 90), 6, QColor(0, 0, 0, 60), 3, 0.0, 16 },
			{ "扭曲：色散 0.2", true, true, defaultLight, QColor(255, 255, 255, 130), 6, 80, 40, QColor(255, 255, 255, 90), 6, QColor(0, 0, 0, 60), 3, 0.2, 16 },
			{ "光从左（厚 2）", false, true, QPointF(-1, 0), QColor(255, 255, 255, 130), 2, 80, 40, QColor(255, 255, 255, 90), 2, QColor(0, 0, 0, 60), 2, 0.0, 16 },
		};
		// 用户要看的样例：打光细（亮面厚 1、阴影厚 2）+ 色散 0.5 + 模糊 15，扭曲半径放大到 24 让彩边更明显
		const PanelSpec sampleSpec = {
			"样例：打光细（亮 1 / 暗 2） + 色散 0.5 + 模糊 15，扭曲半径 24",
			true, true, defaultLight, QColor(255, 255, 255, 130), 1, 96, 40,
			QColor(255, 255, 255, 90), 1, QColor(0, 0, 0, 60), 2, 0.5, 15
		};
		auto buildPanel = [&](const PanelSpec& spec, const QRect& geometry, int distortRadius) {
			QWidget* panel = new QWidget(win);
			panel->setGeometry(geometry);
			LGE* effect = new LGE(panel);
			effect->setBackgroundPolicy(LGE::BackgroundPolicy::CustomImage);
			effect->setPositionPolicy(LGE::PositionPolicy::ParentLocalGeometry);
			effect->setBackgroundImage(background);
			effect->setBorderRadius(16);
			effect->setBlurRadius(spec.blur);
			effect->setLiquidDistortRadius(distortRadius);
			effect->setColorMask(QColor(0, 0, 0), 0.1);
			effect->setLiquidDistortDispersion(spec.dispersion);
			LGE::EffectTypes effectTypes(LGE::EffectType::NormalDistort);
			if (spec.dispersion > 0.0) {
				effectTypes |= LGE::EffectType::DispersionDistort;
			}
			effectTypes |= LGE::EffectType::Blur;
			if (spec.rimLight) {
				effectTypes |= LGE::EffectType::RimLight;
			}
			if (spec.globalRim) {
				effectTypes |= LGE::EffectType::GlobalRim;
			}
			effect->setEffectTypes(effectTypes);
			if (spec.rimLight) {
				effect->setRimLightColor(spec.rimColor);
				effect->setRimLightThickness(spec.rimThickness);
				effect->setRimLightRange(spec.rimRange);
				effect->setRimLightActivationDistance(spec.rimActivation);
			}
			if (spec.globalRim) {
				effect->setGlobalRimLightDirection(spec.lightDirection);
				effect->setGlobalRimLightColor(spec.lightColor);
				effect->setGlobalRimLightThickness(spec.lightThickness);
				effect->setGlobalRimShadowColor(spec.shadowColor);
				effect->setGlobalRimShadowThickness(spec.shadowThickness);
			}
			panel->setGraphicsEffect(effect);
			QLabel* nameLabel = new QLabel(panel);
			nameLabel->setGeometry(0, 14, geometry.width(), 60);
			nameLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
			nameLabel->setWordWrap(true);
			nameLabel->setStyleSheet("color:#FFFFFF; font-size:13px; background:transparent;");
			nameLabel->setText(spec.title);
		};
		// 全默认值面板：连被模糊源都走默认的Render策略（从父窗口取），一个setter都不调，
		// 所以它用的是默认的扭曲半径20、色散0.2、模糊13、圆角20，以及默认的光效强度
		QWidget* defaultPanel = new QWidget(win);
		defaultPanel->setGeometry(30, 115, panelSize.width(), panelSize.height());
		defaultPanel->setGraphicsEffect(new LGE(defaultPanel));
		QLabel* defaultLabel = new QLabel(defaultPanel);
		defaultLabel->setGeometry(0, 14, panelSize.width(), 60);
		defaultLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
		defaultLabel->setWordWrap(true);
		defaultLabel->setStyleSheet("color:#FFFFFF; font-size:13px; background:transparent;");
		defaultLabel->setText("全默认值（不调 setter）");
		for (int i = 0; i < int(sizeof(specs) / sizeof(specs[0])); i++) {
			buildPanel(specs[i], QRect(30 + (i + 1) * (panelSize.width() + 15), 115, panelSize.width(), panelSize.height()), 16);
		}
		buildPanel(sampleSpec, QRect((winSize.width() - sampleSize.width()) / 2, 300, sampleSize.width(), sampleSize.height()), 24);
		// 可拖动的玻璃面板：按住左键拖，把玻璃拖到哪就看哪儿的背景内容
		class DragPanel : public QWidget {
		public:
			using QWidget::QWidget;
		protected:
			void mousePressEvent(QMouseEvent* event) override {
				if (event->button() != Qt::LeftButton) {
					QWidget::mousePressEvent(event);
					return;
				}
				// 这里只记鼠标的全局坐标：子控件的 frameGeometry()/geometry() 是父坐标系下的值，
				// 拿它和全局坐标相减会得到一个混了窗口位置的错误偏移，一按下去控件就会跳。
				lastGlobalPosition = event->globalPosition().toPoint();
				raise();
				event->accept();
			}
			void mouseMoveEvent(QMouseEvent* event) override {
				if (!(event->buttons() & Qt::LeftButton) || !parentWidget()) {
					return;
				}
				// 用两帧之间鼠标的位移累加，这样既不受控件自身移动的影响，也不需要做坐标系换算
				const QPoint globalPosition = event->globalPosition().toPoint();
				const QPoint moved = pos() + (globalPosition - lastGlobalPosition);
				lastGlobalPosition = globalPosition;
				move(qBound(0, moved.x(), qMax(0, parentWidget()->width() - width())),
					qBound(0, moved.y(), qMax(0, parentWidget()->height() - height())));
				event->accept();
			}
		private:
			QPoint lastGlobalPosition;
		};
		// 拖动的两块：一块圆角正方形、一块纯圆形（圆角半径取边长的一半就是正圆）
		const int dragSize = 150;
		auto buildDragPanel = [&](const QRect& geometry, int radius, const QString& title) {
			DragPanel* panel = new DragPanel(win);
			panel->setGeometry(geometry);
			if (radius * 2 >= dragSize) {
				// 正圆：连事件区域也裁成圆形，点圆外不会把它拖走
				panel->setMask(QRegion(QRect(0, 0, dragSize, dragSize), QRegion::Ellipse));
			}
			LGE* effect = new LGE(panel);
			// 用默认的Render策略：拖到别的控件上时能把它们（未经过效果处理的原始内容）也叠进玻璃里。
			// 压在下面的控件如果自己也带QGraphicsEffect，它的效果结果拿不到（Qt 渲染子控件不经过效果），
			// 但它的内容（文字、不透明的东西）能看到。实测重建一次约 5.5ms(Release) / 8.4ms(Debug)。
			effect->setBorderRadius(radius);
			panel->setGraphicsEffect(effect);
			QLabel* nameLabel = new QLabel(title, panel);
			nameLabel->setGeometry(0, 12, geometry.width(), 30);
			nameLabel->setAlignment(Qt::AlignCenter);
			nameLabel->setStyleSheet("color:#FFFFFF; font-size:13px; background:transparent;");
		};
		buildDragPanel(QRect(30, 490, dragSize, dragSize), 28, "拖我：圆角正方形");
		buildDragPanel(QRect(210, 490, dragSize, dragSize), dragSize / 2, "拖我：正圆");
		QLabel* hintLabel = new QLabel(win);
		hintLabel->setGeometry(0, 645, winSize.width(), 60);
		hintLabel->setAlignment(Qt::AlignCenter);
		hintLabel->setStyleSheet("color:#FFFFFF; font-size:13px; background:transparent;");
		hintLabel->setWordWrap(true);
		hintLabel->setText("第一块面板是全默认值（不调任何setter，被模糊源取自父窗口），后面是对照组和样例；左下两块可以按住鼠标拖来拖去。\n边缘光：从面板外靠近→淡入，贴边界滑动→跟着滑动，离开窗口→熄灭。全局光：光从哪边来就亮哪边，垂直两侧无光，对面是阴影。");
		return win;
	}

	void Main::onApplicationInit() {
		VISTM->changeColorTheme(getPluginConfig()->getString("Settings.General.Theme"));
		YSS::ProjectPage::ProjectWin* projectWin = new YSS::ProjectPage::ProjectWin();
		projectWin->show();
		createRimLightTestWin()->show(); // TODO: 临时测试窗口，验证完删除
		Visindigo::AgentPage::AgentWin* agentWin = new Visindigo::AgentPage::AgentWin();
		agentWin->show();
	}

	void Main::onPluginDisable() {
		savePluginConfig();
	}

	void Main::onTest() {

	}

	void Main::releaseInstaller(){
		QString installerPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + 
		"/AppData/Local/TsingYayin/YayinStoryStudio/Installer/YSSInstaller.exe";
		bool needToRelease = false;
		if (not Visindigo::Utility::FileUtility::isFileExist(installerPath)) {
			vgDebug << "Installer not found at:" << installerPath;
			vgDebug << "Releasing installer...";
			needToRelease = true;
		}
		else {
			vgDebug << "Installer found at:" << installerPath;
			QDateTime installerLastModified = Visindigo::Utility::FileUtility::getFileModifyTime(installerPath);
			QDateTime currentInstallerLastModified = 
				Visindigo::Utility::FileUtility::getFileModifyTime(Visindigo::Utility::FileUtility::getProgramPath() + "/YSSInstaller.exe");
			if (installerLastModified < currentInstallerLastModified) {
				vgDebug << "Installer is outdated, releasing new version...";
				needToRelease = true;
			}
			else {
				vgDebug << "Installer is up to date.";
			}
		}
		if (needToRelease) {
			YSS::Editor::InstallerClient::releaseInstaller();
		}
	}
	QWidget* Main::getConfigWidget() {
		return d->ConfigWidget;
	}

	Main::~Main() {
		delete d;
	}

	Main* Main::getInstance() {
		return MainPrivate::Instance;
	}

	TestDragWidget::TestDragWidget(QWidget* parent) :QWidget(parent) {
		DragArea = new Visindigo::Widgets::DragWidget(this);
		Label1 = new QLabel("Test1", this);
		Label2 = new QLabel("Test2", this);
		Label3 = new QLabel("Test3", this);
		Label4 = new QLabel("Test4", this);
		DragArea->addWidget(Label1);
		DragArea->addWidget(Label2);
		DragArea->addWidget(Label3);
		DragArea->addWidget(Label4);
		this->setStyleSheet(R"(
	QWidget{
		border: 1px solid white;
	}
)");
	}
	void TestDragWidget::resizeEvent(QResizeEvent* event) {
		DragArea->setGeometry(0, 0, width(), height());
	}
}