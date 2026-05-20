#include "ASEREnv/ASERProgram.h"
#include <QtCore/qprocess.h>
#include <QtNetwork/qlocalsocket.h>
#include <QtCore/qtimer.h>
#include <QtGui/qwindow.h>
#include <QtCore/qstandardpaths.h>
#include <QtCore/qfileinfo.h>
#include <Utility/JsonConfig.h>
#include <General/Log.h>
#include <QtWidgets/qmessagebox.h>
#include <General/TranslationHost.h>
#ifdef Q_OS_WIN
#include <Windows.h>
#include <tlhelp32.h>
#endif
namespace ASERStudio::ASEREnv {
	class ASERProgramLaunchParameterPrivate {
		friend class ASERProgramLaunchParameter;
	protected:
		ASERProgramLaunchParameter::SizeMode Mode;
		QString Path;
	};
	/*!
		\class ASERStudio::ASEREnv::ASERProgramLaunchParameter
		\brief 提供ASE-Remake程序启动参数的封装.
		\since ASERStudio 2.0
		\inmodule ASERStudio

		ASERProgramLaunchParameter类封装了启动ASE-Remake程序所需的参数，包括可执行文件名称、
		项目目录和窗口大小模式。它提供了一系列方法来设置和return 这些参数，以便在启动程序时使用。

		需要指出的是，ASE-Remake启动参数中的Path参数指的是需要播放的ASE-Remake项目所在的目录，
		并非工作目录。要改动工作目录，请使用ASERProgram::setWorkingDirectory函数。

		目前，有关窗口大小枚举量的标准，采用的是2026年4月8日ASE-Remake官方文档中提供的分辨率列表。
		未来如果官方文档有更新，可能会增加新的枚举值或改动现有枚举值。因此这个类虽然设计为
		ABI兼容的，但不一定保持一致的行为。
	*/

	/*!
		\enum ASERStudio::ASEREnv::ASERProgramLaunchParameter::SizeMode
		\since ASERStudio 2.0
		\value FullScreen 全屏模式
		\value W3840_H2160 3840x2160分辨率
		\value W2560_H1440 2560x1440分辨率
		\value W1920_H1080 1920x1080分辨率
		\value W1600_H900 1600x900分辨率
		\value W1280_H720 1280x720分辨率
		\value W1024_H576 1024x576分辨率
		\value W640_H360 640x360分辨率

		提供ASE-Remake程序窗口大小的枚举选项，允许用户在启动程序时指定窗口的分辨率或全屏模式。
	*/

	/*!
		\since ASERStudio 2.0
		默认构造函数，初始化ASERProgramLaunchParameter对象
	*/
	ASERProgramLaunchParameter::ASERProgramLaunchParameter() {
		d = new ASERProgramLaunchParameterPrivate;
		d->Path = "";
		d->Mode = FullScreen;
	}

	/*!
		\since ASERStudio 2.0
		\a Path 项目目录
		\a mode 窗口大小模式

		构造函数
	*/
	ASERProgramLaunchParameter::ASERProgramLaunchParameter(const QString& Path, SizeMode mode) {
		d = new ASERProgramLaunchParameterPrivate;
		d->Path = Path;
		d->Mode = mode;
	}

	/*!
		\since ASERStudio 2.0

		析构函数
	*/
	ASERProgramLaunchParameter::~ASERProgramLaunchParameter() {
		delete d;
	}

	/*!
		\fn ASERStudio::ASEREnv::ASERProgramLaunchParameter::ASERProgramLaunchParameter(const ASERProgramLaunchParameter& other)
		\since ASERStudio 2.0
		\a other 另一个ASERProgramLaunchParameter对象。

		复制构造函数。
	*/

	/*!
		\fn ASERStudio::ASEREnv::ASERProgramLaunchParameter::ASERProgramLaunchParameter(ASERProgramLaunchParameter&& other) noexcept
		\since ASERStudio 2.0
		\a other 另一个ASERProgramLaunchParameter对象。

		移动构造函数。
	*/

	/*!
		\fn ASERStudio::ASEREnv::ASERProgramLaunchParameter& ASERStudio::ASEREnv::ASERProgramLaunchParameter::operator=(const ASERProgramLaunchParameter& other) noexcept
		\since ASERStudio 2.0
		\a other 另一个ASERProgramLaunchParameter对象。

		复制赋值运算符。
	*/

	/*!
		\fn ASERStudio::ASEREnv::ASERProgramLaunchParameter& ASERStudio::ASEREnv::ASERProgramLaunchParameter::operator=(ASERProgramLaunchParameter&& other) noexcept
		\since ASERStudio 2.0
		\a other 另一个ASERProgramLaunchParameter对象。

		移动赋值运算符。
	*/
	VICopyable_Impl(ASERProgramLaunchParameter);
	VIMoveable_Impl(ASERProgramLaunchParameter);

	/*!
		\since ASERStudio 2.0
		\a Path 项目目录

		设置ASER程序的项目目录
	*/
	void ASERProgramLaunchParameter::setProjectPath(const QString& Path) {
		d->Path = Path;
	}

	/*!
		\since ASERStudio 2.0
		\a mode 窗口大小模式

		设置ASER程序的窗口大小模式
	*/
	void ASERProgramLaunchParameter::setSizeMode(SizeMode mode) {
		d->Mode = mode;
	}

	/*!
		\since ASERStudio 2.0
		return ASER程序的项目目录
	*/
	QString ASERProgramLaunchParameter::getProjectPath() const {
		return d->Path;
	}

	/*!
		\since ASERStudio 2.0
		return ASER程序的窗口大小模式
	*/
	ASERProgramLaunchParameter::SizeMode ASERProgramLaunchParameter::getSizeMode() const {
		return d->Mode;
	}

	class ASERProgramPrivate {
		friend class ASERProgram;
	protected:
		QProcess* ASERProcess;
		QLocalSocket* ASERPipe;
		QString ExecutablePath;
		QString WorkingDirectory;
		QStringList LastArguments;
		qint64 ProcessID;
		qint64 ExitCode;
		bool Running;
		QWindow* ProcessWindow;
	};

	/*!
		\class ASERStudio::ASEREnv::ASERProgram
		\brief ASERProgram提供对ASE应用程序交互和基本属性的封装.
		\since ASERStudio 2.0
		\inmodule ASERStudio

		ASERProgram类封装了与ASE应用程序的交互功能，包括启动和停止程序、
		return 程序属性以及通过具名管道进行通信。它使用Qt的QProcess类来管理外部进程，
		并提供了一系列方法来设置和return 程序的执行路径、工作目录、参数等信息。

		ASERProgram的具名管道只提供基本的读写功能，可供任意接入其他内容的解析。
		如果要使用标准的ASEDevIO协议，请将此类的指针提供给ASERStudio::ASEREnv::ASEDevIO。

		\note 为了正确捕获管道状态，你应该在第一次调用start函数启动ASE-Remake之前就将
		其设置给ASEDevIO。

		这类是可反复使用的，你不必每次启动ASE-Remake都创建一个新的实例。
	*/

	/*!
		\since ASERStudio 2.0

		构造函数，初始化ASERProgram对象
	*/
	ASERProgram::ASERProgram() {
		d = new ASERProgramPrivate;
		d->ASERProcess = new QProcess(this);
		d->ASERPipe = new QLocalSocket(this);
		d->ASERPipe->setServerName("ASERDebugPipe");
		connect(d->ASERProcess, &QProcess::started, this, [this]() {
			d->Running = true;
			d->ProcessID = d->ASERProcess->processId();
			emit programStarted();
			});
		connect(d->ASERProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this](int exitCode, QProcess::ExitStatus) {
			d->Running = false;
			d->ExitCode = exitCode;
			d->ASERPipe->disconnectFromServer();
			emit programStopped(exitCode);
			});
		connect(d->ASERPipe, &QLocalSocket::readyRead, this, [this]() {
			while (d->ASERPipe->bytesAvailable() > 0) {
				QByteArray data = d->ASERPipe->readAll();
				QString context = QString::fromUtf8(data);
				QStringList lines = context.split("\r\n", Qt::SkipEmptyParts);
				for (auto& line : lines) {
					vgDebug << "Received from named pipe:" << line;
					emit namedPipeReadable(line);
				}
			}
			});
		connect(d->ASERPipe, &QLocalSocket::connected, this, [this]() {
			emit namedPipeConnected();
			});
		connect(d->ASERPipe, &QLocalSocket::disconnected, this, [this]() {
			emit namedPipeDisconnected();
			});
		connect(d->ASERPipe, qOverload<QLocalSocket::LocalSocketError>(&QLocalSocket::errorOccurred),
			this, [this](QLocalSocket::LocalSocketError error) {
				emit namedPipeError(d->ASERPipe->errorString());
			});
	}

	/*!
		\since ASERStudio 2.0

		析构函数，清理ASERProgram对象。

		如果ASE-Remake还在运行，这会尝试发送一个停止信号给它，但不会
		等待它完全退出。因此在一些极端情况下可能导致ASE-Remake跑飞。
	*/
	ASERProgram::~ASERProgram() {
		if (d->ASERProcess->state() == QProcess::Running) {
			d->ASERProcess->terminate();
		}
		delete d;
	}

	/*!
		\since ASERStudio 2.0
		\a path 可执行文件路径

		设置ASER程序的可执行文件路径
	*/
	void ASERProgram::setExecutablePath(const QString& path) {
		d->ExecutablePath = path;
		d->ASERProcess->setProgram(path);
	}

	/*!
		\since ASERStudio 2.0

		return ASER程序的可执行文件路径
	*/
	QString ASERProgram::getExecutablePath() const {
		return d->ExecutablePath;
	}

	/*!
		\since ASERStudio 2.0
		\a path 工作目录路径

		设置ASER程序的工作目录。请注意，这只干涉ASE-Remake
		扫描默认资源目录的起始位置，不变更ASE-Remake由Unity
		决定的默认配置目录位置，也不代表实际执行AStoryX剧情脚本时的
		项目目录。要调整后者，请从ASERDevIO::changeDirectory()函数调整
		ASE-Remake的执行项目目录，或者在启动时通过Path参数指定项目目录。
	*/
	void ASERProgram::setWorkingDirectory(const QString& path) {
		d->WorkingDirectory = path;
		d->ASERProcess->setWorkingDirectory(path);
	}

	/*!
		\since ASERStudio 2.0
		return ASER程序的工作目录
	*/
	QString ASERProgram::getWorkingDirectory() const {
		return d->WorkingDirectory;
	}

	/*!
		\since ASERStudio 2.0
		return ASER程序的最后一次启动参数列表
	*/
	QStringList ASERProgram::getLastArguments() const {
		return d->LastArguments;
	}

	/*!
		\since ASERStudio 2.0
		\a arguments 启动参数列表

		启动ASER程序，接受可选的参数列表。如果程序已经启动，这函数
		不产生任何效果，也不会记录参数。
	*/
	void ASERProgram::start(const QStringList& arguments) {
		if (d->ASERProcess->state() == QProcess::Running) {
			return;
		}
		d->LastArguments = arguments;
		d->ASERProcess->setArguments(arguments);
		d->ASERProcess->start();
		QTimer* timer = new QTimer(this);
		timer->setInterval(200);
		connect(timer, &QTimer::timeout, this, [this, timer]() {
			if (d->ASERProcess->state() == QProcess::Running) {
				vgDebug << "Waiting for server to connect...";
				if (d->ASERPipe->state() != QLocalSocket::ConnectedState) {
					d->ASERPipe->connectToServer("ASERDebugPipe");
				}
				else {
					vgDebug << "Server connected successfully.";
					timer->stop();
					timer->deleteLater();
				}
			}
			else { // connected.
				timer->stop();
				timer->deleteLater();
			}
			});
		timer->start();
	}

#ifdef Q_OS_WIN
	static bool IsProcessRunning(const wchar_t* processName)
	{
		bool found = false;
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE)
			return false;
		PROCESSENTRY32W pe32 = {0};
		pe32.dwSize = sizeof(PROCESSENTRY32W);
		if (Process32FirstW(hSnapshot, &pe32))
		{
			do
			{
				if (_wcsicmp(pe32.szExeFile, processName) == 0)
				{
					found = true;
					break;
				}
			} while (Process32NextW(hSnapshot, &pe32));
		}
		CloseHandle(hSnapshot);
		return found;
	}
#endif

	static bool anotherInstanceRunning() {
#ifdef Q_OS_WIN
		// 注意，由于某种缺陷，ASE-Remake可能会丢失窗口句柄，成为后台进程，因此只通过进程名判断
		if (IsProcessRunning(L"ASE-Remake.exe")) {
			QMessageBox::warning(nullptr, VITR("ASERStudio::debugger.isRunning.title"), VITR("ASERStudio::debugger.isRunning.message"), QMessageBox::Ok);
			return true;
		}
		return false;
#endif
	}
	/*!
		\since ASERStudio 2.0

		通过ASERProgramLaunchParameter结构体启动ASER程序。这个函数会根据
		参数中的文件路径和大小模式来构建适当的命令行参数。
		值得指出的是，当前ASE-Remake程序的有效启动参数均通过一个JSON结构体传递，
		因此 \a parameter 中的有效参数会被格式化为一个Json字符串并作为首个参数
		传递给ASE-Remake程序。除此之外其他Unity通用参数可以通过 \a arguments 参数传递。
	*/
	void ASERProgram::start(const ASERProgramLaunchParameter& parameter, const QStringList& arguments) {
		if (anotherInstanceRunning()) {
			return;
		}
		if (d->ASERProcess->state() == QProcess::Running) {
			return;
		}
		Visindigo::Utility::JsonConfig jsonParam;
		jsonParam.setString("storySetPath", parameter.getProjectPath());
		jsonParam.setInt("SizeMode", static_cast<int>(parameter.getSizeMode()));
		vgDebug << "ASERProgram::start - Launching ASER with parameters:" << jsonParam;
		QString jsonString = jsonParam.toString();
		QStringList paramArgs;
		paramArgs += jsonString;
		paramArgs += arguments;
		start(paramArgs);
	}
	/*!
		\since ASERStudio 2.0

		停止ASER程序，如果程序正在运行。这函数只发送终止信号给程序，
		且不等待程序完全退出。如果需要等待程序退出，请使用waitStop()函数。
	*/
	void ASERProgram::stop() {
		if (d->ASERProcess->state() == QProcess::Running) {
			d->ASERProcess->terminate();
		}
	}

	/*!
		\since ASERStudio 2.0

		等待ASER程序停止，接受一个可选的超时时间（毫秒） \a waitMS 。

		return 程序是否在指定时间内停止

		如果waitMS为-1，则无限等待直到程序停止。
		不推荐使用这个函数，如果ASER意外卡死，它会连带卡死YSS。
		如果必须等待程序完全停止，建议使用programStopped信号来处理程序停止事件，而不是阻塞等待。
	*/
	bool ASERProgram::waitStop(qint32 waitMS) {
		if (d->ASERProcess->state() == QProcess::Running) {
			d->ASERProcess->terminate();
			return d->ASERProcess->waitForFinished(waitMS);
		}
		else {
			return true;
		}
	}

	/*!
		\since ASERStudio 2.0

		return ASER程序的进程ID，如果程序正在运行；否则返回-1。
	*/
	qint64 ASERProgram::getProcessID() const {
		if (d->ASERProcess->state() == QProcess::Running) {
			return d->ASERProcess->processId();
		}
		else {
			return -1;
		}
	}

	/*!
		\since ASERStudio 2.0

		return ASER程序的退出代码。

		如果程序已经停止；否则返回-1。
	*/
	qint64 ASERProgram::getExitCode() const {
		if (d->ASERProcess->state() == QProcess::Running) {
			return -1;
		}
		else {
			return d->ASERProcess->exitCode();
		}
	}

	/*!
		\since ASERStudio 2.0

		return ASER程序是否正在运行。

		返回true如果程序正在运行；否则返回false。
	*/
	bool ASERProgram::isRunning() const {
		return d->ASERProcess->state() == QProcess::Running;
	}

	/*!
		\fn QWindow* ASERStudio::ASEREnv::ASERProgram::getProcessWindow() const
		\since ASERStudio 2.0
		return ASER程序的主窗口，如果程序正在运行且窗口可用；否则返回nullptr。
		这函数的实现可能依赖于特定平台的窗口管理机制，因此在不同操作系统上可能表现不同。
	*/
#ifdef Q_OS_WIN
	QWindow* ASERProgram::getProcessWindow() const {
		if (d->ASERProcess->state() == QProcess::Running) {
			HWND hwnd = FindWindow(nullptr, L"ASE-Remake");
			if (not hwnd) {
				return nullptr;
			}
			return QWindow::fromWinId((WId)hwnd);
		}
		return nullptr;
	}
#else
	QWindow* ASERProgram::getProcessWindow() const {
		// 在非Windows平台上，return 进程窗口的方法可能不同，且可能不可靠。
		// 这里暂时返回nullptr，表示未实现。
		return nullptr;
	}
#endif

	/*!
		\since ASERStudio 2.0
		return ASER程序的配置文件路径。由于ASER是Unity编写的，因此
		它默认在
		\badcode
		当前用户文件夹\LocalLow\Gradus\ASE-Remake（在Windows上）
		\endcode
	*/
	QString ASERProgram::getLocalLowPath() const {
#ifdef Q_OS_WIN
		return QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/AppData/LocalLow/Gradus/ASE-Remake";
#else
		return QString();
#endif
	}
	/*!
		\since ASERStudio 2.0

		return ASER程序的资源目录路径，ASER的外部资源存放在此处。
	*/
	QString ASERProgram::getResourcePath() const {
		QFileInfo exeInfo(d->ExecutablePath);
		QString resourcePath = exeInfo.path() + "/StreamingAssets";
		return resourcePath;
	}

	/*!
		\since ASERStudio 2.0
		\a context 要发送的数据内容

		通过具名管道向ASER程序发送数据。接受一个字符串参数，表示要发送的数据内容。
	*/
	void ASERProgram::writeNamedPipe(const QString& context) {
		if (d->ASERPipe->state() == QLocalSocket::ConnectedState) {
			vgDebug << "Writing to named pipe:" << context;
			QString dataToSend = context + "\r\n";
			d->ASERPipe->write(dataToSend.toUtf8());
			d->ASERPipe->flush();
		}
	}
}