#include <QtCore/qdir.h>
#include "../VIApplication.h"
#include "../PluginManager.h"
#include "../TranslationHost.h"
#include "../Plugin.h"
#include "../Log.h"
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
			{VIApplication::PluginFolderPath, "./user_data/plugins"}
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

	VIApplication::VIApplication(int& argc, char** argv, AppType appType) :d(new VIApplicationPrivate()) {
		if (VIApplicationPrivate::Instance != nullptr) {
			throw Exception(Exception::InternalError, "VIApplication instance already exists");
		}
		VIApplicationPrivate::Instance = this;
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
			d->LoadingMessageHandler = handler;
		}
	}

	void VIApplication::setExceptionMessageHandler(ApplicationExceptionMessageHandler* handler) {
		if (!d->started) {
			d->ExceptionMessageHandler = handler;
		}
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
		if (d->MainPlugin) {
			d->MainPlugin->onPluginEnable();
			if (d->LoadingMessageHandler){
				d->LoadingMessageHandler->onLoadingMessage(QString("Main plugin %1 enabled").arg(d->MainPlugin->getPluginName()));
			}
		}
		PluginManager::getInstance()->loadAllPlugin();
		PluginManager::getInstance()->enableAllPlugin();
		if (d->LoadingMessageHandler) {
			d->LoadingMessageHandler->disableHandler();
			qApp->processEvents();
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