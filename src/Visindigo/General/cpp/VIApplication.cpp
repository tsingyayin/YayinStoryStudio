#include "../VIApplication.h"
#include "../Plugin.h"
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
		QMap<VIApplication::EnvKey, QVariant> EnvConfig;
		Plugin* MainPlugin;
		bool started = false;
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

	void VIApplication::onException(const Exception& ex) {
		// TODO
	}

	int VIApplication::start() {
		if (d->MainPlugin) {
			d->MainPlugin->onPluginEnable();
		}
		int ret = 0;
		switch (d->AppType) {
		case CoreApp:
			ret = static_cast<CoreApplication*>(d->AppInstance)->exec();
		case GuiApp:
			ret = static_cast<GuiApplication*>(d->AppInstance)->exec();
		case WidgetApp:
			ret = static_cast<Application*>(d->AppInstance)->exec();
		default:
			throw Exception(Exception::InvalidArgument, "Invalid AppType");
		}
		if (d->MainPlugin){
			d->MainPlugin->onPluginDisbale();
		}	
		return ret;
	}
}