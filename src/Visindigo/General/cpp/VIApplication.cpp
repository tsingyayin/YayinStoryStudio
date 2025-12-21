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
		vgNoticeF << "[Loading Message Handler] " << message;
	}
	void ApplicationLoadingMessageHandlerDefaultConsoleImpl::enableHandler() {
		vgNoticeF << "[Loading Message Handler] Enabled default console handler.";
	}
	void ApplicationLoadingMessageHandlerDefaultConsoleImpl::disableHandler() {
		vgNoticeF << "[Loading Message Handler] Disabled default console handler.";
	}
}

namespace Visindigo::General {
	/*!
		\class Visindigo::General::CoreApplication
		\inheaderfile General/VIApplication.h
		\brief Visindigo控制台应用程序类，继承自QCoreApplication，用于处理全局异常捕获。
		\since Visindigo 0.13.0
		\inmodule Visindigo
		
		CoreApplication类继承自Qt的QCoreApplication类，重写了notify()函数以实现全局异常捕获功能。
		虽然此类是公开的，但用户绝不要直接使用它，它由Visindigo::General::VIApplication类根据应用程序类型自动创建和管理。
	*/
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

	/*!
		\class Visindigo::General::GuiApplication
		\inheaderfile General/VIApplication.h
		\brief Visindigo GUI应用程序类，继承自QGuiApplication，用于处理全局异常捕获。
		\since Visindigo 0.13.0
		\inmodule Visindigo

		GuiApplication类继承自Qt的QGuiApplication类，重写了notify()函数以实现全局异常捕获功能。
		虽然此类是公开的，但用户绝不要直接使用它，它由Visindigo::General::VIApplication类根据应用程序类型自动创建和管理。
	*/
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

	/*!
		\class Visindigo::General::Application
		\inheaderfile General/VIApplication.h
		\brief Visindigo Widget应用程序类，继承自QApplication，用于处理全局异常捕获。
		\since Visindigo 0.13.0
		\inmodule Visindigo

		Application类继承自Qt的QApplication类，重写了notify()函数以实现全局异常捕获功能。
		虽然此类是公开的，但用户绝不要直接使用它，它由Visindigo::General::VIApplication类根据应用程序类型自动创建和管理。
	*/
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

	/*!
		\class Visindigo::General::ApplicationLoadingMessageHandler
		\inheaderfile General/VIApplication.h
		\brief Visindigo应用程序加载消息处理器接口类。
		\since Visindigo 0.13.0
		\inmodule Visindigo

		\note 此类为接口类，用户可以继承此类以实现自定义的加载消息处理器。

		这是一个抽象接口，用户可以任意实现，以处理应用程序加载过程中的消息，例如在GUI界面中或在控制台中显示加载进度等。
	*/

	/*！
		\fn void Visindigo::General::ApplicationLoadingMessageHandler::onLoadingMessage(const QString& msg)
		\since Visindigo 0.13.0
		\a msg 加载消息内容

		当应用程序在加载过程中产生消息时调用此函数。用户可以实现此函数以处理加载消息。
	*/

	/*!
		\fn void Visindigo::General::ApplicationLoadingMessageHandler::enableHandler()
		\since Visindigo 0.13.0

		启用加载消息处理器时调用此函数。用户可以实现此函数以执行启用处理器时的操作。
	*/

	/*!
		\fn void Visindigo::General::ApplicationLoadingMessageHandler::disableHandler()
		\since Visindigo 0.13.0

		禁用加载消息处理器时调用此函数。用户可以实现此函数以执行禁用处理器时的操作。
	*/

	/*!
		\class Visindigo::General::ApplicationExceptionMessageHandler
		\inheaderfile General/VIApplication.h
		\brief Visindigo应用程序异常消息处理器接口类。
		\since Visindigo 0.13.0
		\inmodule Visindigo

		\note 此类为接口类，用户可以继承此类以实现自定义的异常消息处理器。

		这是一个抽象接口，用户可以任意实现，以处理应用程序运行过程中捕获的异常消息，例如在GUI界面中显示异常信息对话框等。
	*/

	/*!
		\fn void Visindigo::General::ApplicationExceptionMessageHandler::onExceptionMessage(const Exception& ex)
		\since Visindigo 0.13.0
		\a ex 捕获的异常对象

		当应用程序在运行过程中捕获到异常时调用此函数。用户可以实现此函数以处理异常消息。
	*/

	/*!
		\fn void Visindigo::General::ApplicationExceptionMessageHandler::enableHandler()
		\since Visindigo 0.13.0

		启用异常消息处理器时调用此函数。用户可以实现此函数以执行启用处理器时的操作。
	*/

	/*!
		\fn void Visindigo::General::ApplicationExceptionMessageHandler::exec()
		\since Visindigo 0.13.0

		执行异常消息处理器时调用此函数。用户可以实现此函数以执行处理异常消息的操作。
	*/

	/*!
		\fn void Visindigo::General::ApplicationExceptionMessageHandler::disableHandler()
		\since Visindigo 0.13.0

		禁用异常消息处理器时调用此函数。用户可以实现此函数以执行禁用处理器时的操作。
	*/

	class VIApplicationPrivate {
		friend class VIApplication;
	protected:
		VIApplication::AppType AppType;
		void* AppInstance;
		static QMap<VIApplication::EnvKey, QVariant> EnvConfig;
		Plugin* MainPlugin;
		bool started = false;
		ApplicationLoadingMessageHandler* LoadingMessageHandler = nullptr;
		ApplicationExceptionMessageHandler* ExceptionMessageHandler = nullptr;
		static VIApplication* Instance;
	};

	VIApplication* VIApplicationPrivate::Instance = nullptr;
	QMap<VIApplication::EnvKey, QVariant> VIApplicationPrivate::EnvConfig = {
			{VIApplication::LogFolderPath, "./user_data/logs"},
			{VIApplication::LogFileNameTimeFormat, "yyyy-MM-dd_hh_mm_ss"},
			{VIApplication::LogTimeFormat, "yyyy-MM-dd hh:mm:ss.zzz"},
			{VIApplication::PluginFolderPath, "./user_data/plugins"},
			{VIApplication::MinimumLoadingTimeMS, 3000},
	};

	/*!
		\since Visindigo 0.13.0
		获取VIApplication单例对象的指针。
		虽然VIApplication是一个单例类，但此函数不会在实例不存在时创建它。请确保在调用此函数前已正确初始化VIApplication。

		这是Visindigo单例类getInstance()函数的一个特例。其他单例类的getInstance()函数均会在实例不存在时创建它们。请注意区分。
	*/
	VIApplication* VIApplication::getInstance() {
		if (VIApplicationPrivate::Instance == nullptr) {
			vgWarningF << "VIApplication instance not exists. Visindigo application needs to be initialized first.";
			throw Exception(Exception::UnsupportedOperation, "VIApplication instance not exists. Visindigo application needs to be initialized first.");
		}
		return VIApplicationPrivate::Instance;
	}

	/*!
		\class Visindigo::General::VIApplication
		\inheaderfile General/VIApplication.h
		\brief Visindigo应用程序的根基，使用部分Visindigo功能必须首先初始化此类。
		\since Visindigo 0.13.0
		\inmodule Visindigo

		VIApplication类是Visindigo应用程序的基类，负责初始化和管理Visindigo的核心功能模块，如插件管理器和翻译管理器。

		在使用Visindigo的任何功能之前，推荐首先创建VIApplication的实例。（确实部分功能不依赖VIApplication工作）
		VIApplication类提供了多种应用程序类型的支持，包括控制台应用程序、GUI应用程序和Widget应用程序，分别
		等同于Qt中的QCoreApplication、QGuiApplication和QApplication。

		此外，VIApplication还提供了环境配置管理功能，允许用户自定义应用程序的日志文件路径、插件文件路径等参数。
		但要注意的是，环境配置必须在创建VIApplication实例之前设置，否则可能无法生效。

		通常来说，一个Visindigo应用程序的main.cpp文件应该是这样：
		\code
		#include <General/VIApplication.h>
		#include "MyApplicationMainClass.h"
		int main(int argc, char* argv[]) {
			VIAppStatic::setEnvConfig(...); // 可选，如果需要变更相关配置

			Visindigo::General::VIApplication app(argc, argv, Visindigo::General::VIApplication::WidgetApp); // 以Widget为例
			app.setMainPlugin(new MyApplicationMainClass()); // 设置作为主插件的类
			app.setLoadingMessageHandler(new MyLoadingMessageHandlerClass()); // 可选，设置加载消息处理器
			int c = app.start(); // 启动应用程序
			return c;
		}
		\endcode

		至于MyApplicationMainClass，则是一个继承了Visindigo::General::Plugin类的派生类，在其中实现应用程序的主要逻辑。

		\note VIApplication类是一个单例类，应用程序中只能存在一个VIApplication实例。如果尝试创建多个实例，将会抛出异常。

		\section1 工作顺序
		\list
		\li 1. QApplication/QGuiApplication/QCoreApplication与VIApplication同时实例化，因此在
		VIApplication实例化之前，也不能使用部分Qt的功能。Qt的环境变量设置函数也应该在VIApplication实例化之前调用。
		\li 2. 启用LoadingMessageHandler（如果有设置）。
		\li 3. 调用主插件的Plugin::onPluginEnable()函数。
		\li 4. 按优先级顺序加载全部插件到内存，然后按优先级顺序调用它们的Plugin::onPluginEnable()函数。
		\li 5. 按优先级顺序调用全部插件的Plugin::onApplicationInit()函数。
		\li 6. 按优先级顺序调用全部插件的Plugin::onTest()函数（如果对应插件启用了测试功能）。
		\li 7. 禁用LoadingMessageHandler（如果有设置）。
		\li 8. 调用主插件的Plugin::onApplicationInit()函数和Plugin::onTest()函数（如果启用了测试功能）。
		\li 9. 应用程序正式启动，进入事件循环。
		\li 10. 当应用程序退出时，按优先级顺序逆序调用全部插件的Plugin::onPluginDisable()函数。
		\li 11. 调用主插件的Plugin::onPluginDisable()函数
		\li 12. 保存日志文件并清理资源。
		\endlist

		\note 如您所见，此表没有给发出日志管理器的实例化时间，因为它是按需实例化的，即在第一次使用日志功能时才会实例化。
		它甚至可以在VIApplication实例化之前被使用（但不推荐）。
	*/

	/*!
		\enum Visindigo::General::VIApplication::AppType
		\since Visindigo 0.13.0
		此枚举定义了VIApplication支持的应用程序类型。
		\value CoreApp 表示控制台应用程序，等同于Qt中的QCoreApplication。
		\value GuiApp 表示GUI应用程序，等同于Qt中的QGuiApplication。
		\value WidgetApp 表示Widget应用程序，等同于Qt中的QApplication。

		提示：如果要创建QML应用程序，请使用GuiApp类型。
	*/

	/*!
		\enum Visindigo::General::VIApplication::EnvKey
		\since Visindigo 0.13.0
		此枚举定义了VIApplication支持的环境配置键。
		\value LogFolderPath 日志文件夹路径，类型为QString，默认值为"./user_data/logs"。
		\value LogFileNameTimeFormat 日志文件名时间格式，类型为QString，默认值为"yyyy-MM-dd_hh_mm_ss"。
		\value LogTimeFormat 日志时间格式，类型为QString，默认值为"yyyy-MM-dd hh:mm:ss.zzz"。
		\value PluginFolderPath 插件文件夹路径，类型为QString，默认值为"./user_data/plugins"。
		\value MinimumLoadingTimeMS 最小加载时间（毫秒），类型为qint, 默认值为3000。
	*/

	/*!
		\since Visindigo 0.13.0
		创建VIApplication类的新实例。
		\a argc 和 \a argv 参数与Qt的QCoreApplication、QGuiApplication和QApplication类的构造函数参数相同。
		\a appType 参数指定应用程序的类型，默认为WidgetApp。
		\a changeWorkingDirToExeDir 参数指定是否将当前工作目录更改为可执行文件所在目录，默认为true。
	*/
	VIApplication::VIApplication(int& argc, char** argv, AppType appType, bool changeWorkingDirToExeDir) :QObject(nullptr) {
		if (VIApplicationPrivate::Instance != nullptr) {
			vgWarningF << "VIApplication instance already exists.";
			throw Exception(Exception::UnsupportedOperation, "VIApplication instance already exists");
		}
		VIApplicationPrivate::Instance = this;
		d = new VIApplicationPrivate();
		if (changeWorkingDirToExeDir) {
			QDir::setCurrent(QFileInfo(argv[0]).absolutePath());
		}
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

	/*!
		\since Visindigo 0.13.0
		销毁VIApplication实例并释放相关资源。
	*/
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

	/*!
		\since Visindigo 0.13.0
		设置应用程序的主插件。
		\a plugin 指向主插件对象的指针。主插件必须继承自Visindigo::General::Plugin类。

		主插件是应用程序的核心逻辑所在，VIApplication会在启动过程中调用主插件的相关函数以初始化应用程序。
		必须在调用start()函数之前设置主插件，如果在应用程序已经启动后调用此函数，将不会生效。

		Visindigo允许用户不使用主插件，在start()函数前使用自由代码初始化自己的程序，
		但这强烈不推荐，因为会破坏Visinidgo对于应用程序加载与释放逻辑的封装。
	*/
	void VIApplication::setMainPlugin(Plugin* plugin) {
		if (!d->started) {
			d->MainPlugin = plugin;
		}
	}

	/*!
		\since Visindigo 0.13.0
		设置应用程序的环境配置参数。
		\a key 环境配置键，参见EnvKey枚举。
		\a value 环境配置值，类型为QVariant。

		此函数允许用户自定义应用程序的环境配置参数，如日志文件路径、插件文件路径等。
		必须在创建VIApplication实例之前调用此函数，否则可能无法生效。

		实际上在启动程序之后仍然会正确保存这些配置，但某些功能可能已经初始化完毕，无法应用新的配置。
	*/
	void VIApplication::setEnvConfig(EnvKey key, const QVariant& value) {
		VIApplicationPrivate::EnvConfig.insert(key, value);
	}

	/*!
		\since Visindigo 0.13.0
		设置应用程序的加载消息处理器。
		\a handler 指向加载消息处理器对象的指针。加载消息处理器必须继承自Visindigo::General::ApplicationLoadingMessageHandler类。

		加载消息处理器用于处理应用程序在启动过程中产生的加载消息，例如在GUI界面中或在控制台中显示加载进度等。
		必须在调用start()函数之前设置加载消息处理器，如果在应用程序已经启动后调用此函数，将不会生效。
	*/
	void VIApplication::setLoadingMessageHandler(ApplicationLoadingMessageHandler* handler) {
		if (!d->started) {
			if (d->LoadingMessageHandler) {
				delete d->LoadingMessageHandler;
			}
			d->LoadingMessageHandler = handler;
		}
	}

	/*!
		\since Visindigo 0.13.0
		设置应用程序的异常消息处理器。
		\a handler 指向异常消息处理器对象的指针。异常消息处理器必须继承自Visindigo::General::ApplicationExceptionMessageHandler类。

		异常消息处理器用于处理应用程序在运行过程中捕获的异常消息，例如在GUI界面中显示异常信息对话框等。
		必须在调用start()函数之前设置异常消息处理器，如果在应用程序已经启动后调用此函数，将不会生效。
	*/
	void VIApplication::setExceptionMessageHandler(ApplicationExceptionMessageHandler* handler) {
		if (!d->started) {
			if (d->ExceptionMessageHandler) {
				delete d->ExceptionMessageHandler;
			}
			d->ExceptionMessageHandler = handler;
		}
	}

	/*!
		\since Visindigo 0.13.0
		设置应用程序的全局字体。
		\a fontPath 字体文件的路径。
		\a fontID 字体文件中字体的索引，默认为0。

		此函数允许用户为整个应用程序设置统一的字体。字体文件可以是TTF、OTF等常见字体格式。
		如果加载字体失败，将会抛出异常。

		这是一个对Qt字体加载的简单封装。
	*/
	void VIApplication::setGlobalFont(const QString& fontPath, int fontID) {
		int id = QFontDatabase::addApplicationFont(fontPath);
		if (id == -1) {
			throw Exception(Exception::InternalError, QString("Failed to load font from %1").arg(fontPath));
		}
		QString family = QFontDatabase::applicationFontFamilies(id).at(fontID);
		QFont font(family);
		qApp->setFont(font);
		vgNoticeF << "Global font set to" << family;
	}

	/*!
		\since Visindigo 0.13.0
		处理应用程序运行过程中捕获的异常。
		\a ex 捕获的异常对象。

		此函数由VIApplication内部调用，用于处理在应用程序运行过程中捕获的异常。
		如果设置了异常消息处理器，将会调用其相关函数来处理异常消息。
		如果异常是严重的且应用程序已经启动，将会退出应用程序。

		如果用户有需要，也可以手动塞入一个异常对象来调用此函数，以便使用VIApplication的异常处理机制。
	*/
	void VIApplication::onException(const Exception& ex) {
		if (d->ExceptionMessageHandler) {
			d->ExceptionMessageHandler->enableHandler();
			d->ExceptionMessageHandler->onExceptionMessage(ex);
			d->ExceptionMessageHandler->exec();
			d->ExceptionMessageHandler->disableHandler();
		}
		else {
			vgError << "No exception message handler set, exception message:" << ex.getMessage();
		}
		if (ex.isCritical() && d->started) {
			vgError << "Critical exception caught, exiting application.";
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

	/*!
		\since Visindigo 0.13.0
		启动应用程序的主事件循环。
		此函数会加载并启用所有插件，初始化应用程序，并进入主事件循环。
		返回值为应用程序退出时的返回码。

		在调用此函数之前，应该先设置主插件和加载消息处理器（如果有的话）。
		如果应用程序已经启动，将会抛出异常。

		此函数是VIApplication类的核心功能，负责管理应用程序的生命周期。
	*/
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
		vgNoticeF << "Application exited with code" << ret;
		return ret;
	}

	/*!
		\since Visindigo 0.13.0
		获取应用程序的环境配置参数。
		\a key 环境配置键，参见EnvKey枚举。
		返回值为对应的环境配置值，类型为QVariant。如果键不存在，则返回一个默认构造的QVariant对象。

		此函数允许用户获取应用程序的环境配置参数，如日志文件路径、插件文件路径等。
	*/
	QVariant VIApplication::getEnvConfig(EnvKey key){
		return VIApplicationPrivate::EnvConfig.value(key, QVariant());
	}

	/*!
		\since Visindigo 0.13.0
		检查应用程序是否已经启动。
		返回值为true表示应用程序已经启动，false表示尚未启动。

		此函数允许用户检查应用程序的启动状态。
	*/
	bool VIApplication::applicationStarted() const {
		return d->started;
	}
}