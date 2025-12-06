#include <QtCore/qdir.h>
#include <QtCore/qthread.h>
#include <chrono>
#include "../VIApplication.h"
#include "../private/AUTO_VERSION.h"
#include "../private/VIApplication_p.h"
#include "../PluginManager.h"
#include "../TranslationHost.h"
#include "../Plugin.h"
#include "../Log.h"
#include "../../Utility/Console.h"
#include <QtGui/qfontdatabase.h>

namespace Visindigo::__Private__ {
	void ApplicationLoadingMessageHandlerDefaultConsoleImpl::onLoadingMessage(const QString& message) {
		yNotice << "[Loading Message Handler] " << message;
	}
	void ApplicationLoadingMessageHandlerDefaultConsoleImpl::enableHandler() {
		yNotice << "[Loading Message Handler] Enabled default console handler.";
	}
	void ApplicationLoadingMessageHandlerDefaultConsoleImpl::disableHandler() {
		yNotice << "[Loading Message Handler] Disabled default console handler.";
	}
}

namespace Visindigo::General {
	CoreApplication::CoreApplication(int& argc, char** argv) :QCoreApplication(argc, argv) {
	}
	bool CoreApplication::notify(QObject* receiver, QEvent* event) {
		try {
			return QCoreApplication::notify(receiver, event);
		}
		catch (const Exception& ex) {
			VIApplication::getInstance()->onException(ex);
		}
		catch (const std::exception& e) {
			VIApplication::getInstance()->onException(Exception::fromStdException(e));
		}
		catch (...) {
			VIApplication::getInstance()->onException(Exception(Exception::Unknown, "Unknown exception caught in CoreApplication::notify", true));
		}
		return false;
	}

	GuiApplication::GuiApplication(int& argc, char** argv) :QGuiApplication(argc, argv) {
	}
	bool GuiApplication::notify(QObject* receiver, QEvent* event) {
		try {
			return QGuiApplication::notify(receiver, event);
		}
		catch (const Exception& ex) {
			VIApplication::getInstance()->onException(ex);
		}
		catch (const std::exception& e) {
			VIApplication::getInstance()->onException(Exception::fromStdException(e));
		}
		catch (...) {
			VIApplication::getInstance()->onException(Exception(Exception::Unknown, "Unknown exception caught in GuiApplication::notify", true));
		}
		return false;
	}

	Application::Application(int& argc, char** argv) :QApplication(argc, argv) {
	}
	bool Application::notify(QObject* receiver, QEvent* event) {
		try {
			return QApplication::notify(receiver, event);
		}
		catch (const Exception& ex) {
			VIApplication::getInstance()->onException(ex);
		}
		catch (const std::exception& e) {
			VIApplication::getInstance()->onException(Exception::fromStdException(e));
		}
		catch (...) {
			VIApplication::getInstance()->onException(Exception(Exception::Unknown, "Unknown exception caught in Application::notify", true));
		}
		return false;
	}

	class VIApplicationPrivate {
		friend class VIApplication;
	protected:
		VIApplication::AppType AppType;
		void* AppInstance;
		QMap<VIApplication::EnvKey, QVariant> EnvConfig = {
			{VIApplication::PluginFolderPath, "./user_data/plugins"},
			{VIApplication::MinimumLoadingTimeMS, 3000},
		};
		Plugin* MainPlugin;
		bool started = false;
		ApplicationLoadingMessageHandler* LoadingMessageHandler = nullptr;
		ApplicationExceptionMessageHandler* ExceptionMessageHandler = nullptr;
		static VIApplication* Instance;
	};

	VIApplication* VIApplicationPrivate::Instance = nullptr;

	VIApplication* VIApplication::getInstance() {
		return VIApplicationPrivate::Instance;
	}

	VIApplication::VIApplication(int& argc, char** argv, AppType appType) {
		if (VIApplicationPrivate::Instance != nullptr) {
			throw Exception(Exception::InternalError, "VIApplication instance already exists");
		}
		VIApplicationPrivate::Instance = this;
		d = new VIApplicationPrivate();
		Utility::Console::print("\033[38;2;237;28;36m===================================================================\033[0m");
		Utility::Console::print("\033[38;2;234;54;128m╮ ╭\t─┬─\t╭──\t─┬─\t╭╮╭\t┌─╮\t─┬─\t╭─╮\t╭─╮\033[0m");
		Utility::Console::print("\033[38;2;234;63;247m╰╮│\t │ \t╰─╮\t │ \t│││\t│ │\t │ \t│ ┐\t│ │\033[0m");
		Utility::Console::print("\033[38;2;115;43;235m ╰╯\t─┴─\t──╯\t─┴─\t╯╰╯\t└─╯\t─┴─\t╰─╯\t╰─╯\033[0m");
		Utility::Console::print("   \t   \t———\t  流\t   \t清  \t———\t   \t   \t");
		Utility::Console::print("\033[38;2;50;130;246m===================================================================\033[0m");
		Utility::Console::print("\033[38;2;234;54;128mVisindigo \033[0m" + General::Version::getABIVersion().toString() + " \"" + QString(Visindigo_VERSION_NICKNAME) + "\"" +
#ifdef QT_DEBUG
			" \033[38;2;255;253;85m[DEBUG compilation mode]\033[0m");
#else
			" \033[38;2;255;253;85m[RELEASE compilation mode]\033[0m");
#endif
		Utility::Console::print("\033[38;2;234;63;247mVersion Compilation Time \033[0m: \033[38;2;255;253;85m" + QString(Visindigo_VERSION_BUILD_DATE) + " " + QString(Visindigo_VERSION_BUILD_TIME) + " [" + QSysInfo::buildCpuArchitecture() + "]\033[0m");
		Utility::Console::print(Utility::Console::inWarningStyle("Working Path: ") + Utility::Console::inNoticeStyle(QDir::currentPath()));
		Utility::Console::print("Hello, " + QDir::home().dirName() + "! Welcome to Visindigo!");
		d->AppType = appType;
		d->MainPlugin = nullptr;
		switch (appType) {
		case CoreApp:
			d->AppInstance = new CoreApplication(argc, argv);
			break;
		case GuiApp:
			d->AppInstance = new GuiApplication(argc, argv);
			break;
		case WidgetApp:
			d->AppInstance = new Application(argc, argv);
			break;
		default:
			throw Exception(Exception::InvalidArgument, "Invalid AppType");
		}
		QStringList args = qApp->arguments();
		QDir::setCurrent(QFileInfo(args[0]).absolutePath());
		PluginManager::getInstance(); // Initialize PluginManager
		TranslationHost::getInstance(); // Initialize TranslationHost

		d->LoadingMessageHandler = new __Private__::ApplicationLoadingMessageHandlerDefaultConsoleImpl();

		connect(PluginManager::getInstance(), &PluginManager::pluginLoaded, [this](Plugin* plugin) {
				if (d->LoadingMessageHandler) {
					d->LoadingMessageHandler->onLoadingMessage(QString("Plugin %1 loaded").arg(plugin->getPluginName()));
					qApp->processEvents();
				}
			});

		connect(PluginManager::getInstance(), &PluginManager::pluginEnabled, [this](Plugin* plugin) {
				if (d->LoadingMessageHandler) {
					d->LoadingMessageHandler->onLoadingMessage(QString("Plugin %1 enabled").arg(plugin->getPluginName()));
					qApp->processEvents();
				}
			});
	}

	VIApplication::~VIApplication() {
		if (d->AppInstance) {
			switch (d->AppType) {
			case CoreApp:
				delete static_cast<CoreApplication*>(d->AppInstance);
				break;
			case GuiApp:
				delete static_cast<GuiApplication*>(d->AppInstance);
				break;
			case WidgetApp:
				delete static_cast<Application*>(d->AppInstance);
				break;
			}
			d->AppInstance = nullptr;
		}
		VIApplicationPrivate::Instance = nullptr;
		delete d;
	}

	void VIApplication::setMainPlugin(Plugin* plugin) {
		if (!d->started) {
			d->MainPlugin = plugin;
		}
	}

	void VIApplication::setEnvConfig(EnvKey key, const QVariant& value) {
		if (!d->started) {
			d->EnvConfig.insert(key, value);
		}
	}

	void VIApplication::setLoadingMessageHandler(ApplicationLoadingMessageHandler* handler) {
		if (!d->started) {
			if (d->LoadingMessageHandler) {
				delete d->LoadingMessageHandler;
			}
			d->LoadingMessageHandler = handler;
		}
	}

	void VIApplication::setExceptionMessageHandler(ApplicationExceptionMessageHandler* handler) {
		if (!d->started) {
			if (d->ExceptionMessageHandler) {
				delete d->ExceptionMessageHandler;
			}
			d->ExceptionMessageHandler = handler;
		}
	}

	void VIApplication::setGlobalFont(const QString& fontPath, int fontID) {
		int id = QFontDatabase::addApplicationFont(fontPath);
		if (id == -1) {
			throw Exception(Exception::InternalError, QString("Failed to load font from %1").arg(fontPath));
		}
		QString family = QFontDatabase::applicationFontFamilies(id).at(fontID);
		QFont font(family);
		qApp->setFont(font);
		yNotice << "Global font set to" << family;
	}

	void VIApplication::onException(const Exception& ex) {
		if (d->ExceptionMessageHandler) {
			d->ExceptionMessageHandler->enableHandler();
			d->ExceptionMessageHandler->onExceptionMessage(ex);
			d->ExceptionMessageHandler->exec();
			d->ExceptionMessageHandler->disableHandler();
		}
		else {
			yError << "No exception message handler set, exception message:" << ex.getMessage();
		}
		if (ex.isCritical() && d->started) {
			yError << "Critical exception caught, exiting application.";
			switch (d->AppType) {
			case CoreApp:
				static_cast<CoreApplication*>(d->AppInstance)->exit(-1);
				break;
			case GuiApp:
				static_cast<GuiApplication*>(d->AppInstance)->exit(-1);
				break;
			case WidgetApp:
				static_cast<Application*>(d->AppInstance)->exit(-1);
				break;
			}
		}
	}

	int VIApplication::start() {
		if (d->started) {
			throw Exception(Exception::InternalError, "Application already started");
		}
		d->started = true;
		if (d->LoadingMessageHandler) {
			d->LoadingMessageHandler->enableHandler();
			qApp->processEvents();
		}
		auto start = std::chrono::high_resolution_clock::now();
		if (d->MainPlugin) {
			d->MainPlugin->onPluginEnable();
			if (d->LoadingMessageHandler){
				d->LoadingMessageHandler->onLoadingMessage(QString("Main plugin %1 enabled").arg(d->MainPlugin->getPluginName()));
			}
		}
		
		PluginManager::getInstance()->loadAllPlugin();
		PluginManager::getInstance()->enableAllPlugin();
		PluginManager::getInstance()->applicationInitAllPlugin();
		PluginManager::getInstance()->testAllPlugin();
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		qint32 minLoadingTime = d->EnvConfig.value(MinimumLoadingTimeMS, 3000).toInt();
		if (duration < minLoadingTime) {
			QThread::msleep(minLoadingTime - duration);
		}
		if (d->LoadingMessageHandler) {
			d->LoadingMessageHandler->disableHandler();
			qApp->processEvents();
		}
		if (d->MainPlugin) {
			d->MainPlugin->onApplicationInit();
			if (d->MainPlugin->isTestEnable()) {
				d->MainPlugin->onTest();
			}
		}
		int ret = 0;
		switch (d->AppType) {
		case CoreApp:
			ret = static_cast<CoreApplication*>(d->AppInstance)->exec();
			break;
		case GuiApp:
			ret = static_cast<GuiApplication*>(d->AppInstance)->exec();
			break;
		case WidgetApp:
			ret = static_cast<Application*>(d->AppInstance)->exec();
			break;
		default:
			throw Exception(Exception::InvalidArgument, "Invalid AppType");
		}
		PluginManager::getInstance()->disableAllPlugin();
		if (d->MainPlugin){
			d->MainPlugin->onPluginDisbale();
		}	
		Visindigo::General::LoggerManager::getInstance()->finalSave();
		d->started = false;
		yNotice << "Application exited with code" << ret;
		return ret;
	}

	QVariant VIApplication::getEnvConfig(EnvKey key) const {
		return d->EnvConfig.value(key, QVariant());
	}

	bool VIApplication::applicationStarted() const {
		return d->started;
	}
}