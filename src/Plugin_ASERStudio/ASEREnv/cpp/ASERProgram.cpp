#include "ASEREnv/ASERProgram.h"
#include <QtCore/qprocess.h>
#include <QtNetwork/qlocalsocket.h>
#include <QtCore/qtimer.h>
#include <QtGui/qwindow.h>
#include <QtCore/qstandardpaths.h>
#include <QtCore/qfileinfo.h>
#ifdef Q_OS_WIN
#include <Windows.h>
#endif
namespace ASERStudio::ASEREnv {
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
		\brief ASERProgram提供对ASE应用程序交互和基本属性的封装
		\since ASERStudio 2.0
		\inmodule ASERStudio

		ASERProgram类封装了与ASE应用程序的交互功能，包括启动和停止程序、
		获取程序属性以及通过具名管道进行通信。它使用Qt的QProcess类来管理外部进程，
		并提供了一系列方法来设置和获取程序的执行路径、工作目录、参数等信息。
		
		ASERProgram的具名管道只提供基本的读写功能，可供任意接入其他内容的解析。
		如果要使用标准的ASEDevIO协议，请将此类的指针提供给ASERStudio::ASEREnv::ASEDevIO。
	*/

	/*!
		\since ASERStudio 2.0
		构造函数，初始化ASERProgram对象
	*/
	ASERProgram::ASERProgram() {
		d = new ASERProgramPrivate;
		d->ASERProcess = new QProcess(this);
		d->ASERPipe = new QLocalSocket(this);
		d->ASERPipe->setServerName("ASEDebugPipe");
		connect(d->ASERProcess, &QProcess::started, this, [this]() {
			d->Running = true;
			d->ProcessID = d->ASERProcess->processId();
			emit programStarted();
			});
		connect(d->ASERProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this](int exitCode, QProcess::ExitStatus) {
			d->Running = false;
			d->ExitCode = exitCode;
			emit programStopped(exitCode);
			});
		connect(d->ASERPipe, &QLocalSocket::readyRead, this, [this]() {
			while (d->ASERPipe->bytesAvailable() > 0) {
				QByteArray data = d->ASERPipe->readAll();
				emit namedPipeReadable(QString::fromUtf8(data));
			}
			});
		connect(d->ASERPipe, &QLocalSocket::connected, this, [this]() {
			emit namedPipeConnected();
			});
		connect(d->ASERPipe, &QLocalSocket::disconnected, this, [this]() {
			emit namedPipeDisconnected();
			});
		connect(d->ASERPipe, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::errorOccurred), 
			this, [this](QLocalSocket::LocalSocketError error) {
			emit namedPipeError(d->ASERPipe->errorString());
			});
	}

	/*!
		\since ASERStudio 2.0
		析构函数，清理ASERProgram对象
	*/
	ASERProgram::~ASERProgram() {
		delete d;
	}

	/*!
		\since ASERStudio 2.0
		设置ASER程序的可执行文件路径
	*/
	void ASERProgram::setExecutablePath(const QString& path) {
		d->ExecutablePath = path;
		d->ASERProcess->setProgram(path);
	}

	/*!
		\since ASERStudio 2.0
		获取ASER程序的可执行文件路径
	*/
	QString ASERProgram::getExecutablePath() const {
		return d->ExecutablePath;
	}

	/*!
		\since ASERStudio 2.0
		设置ASER程序的工作目录
	*/
	void ASERProgram::setWorkingDirectory(const QString& path) {
		d->WorkingDirectory = path;
		d->ASERProcess->setWorkingDirectory(path);
	}

	/*!
		\since ASERStudio 2.0
		获取ASER程序的工作目录
	*/
	QString ASERProgram::getWorkingDirectory() const {
		return d->WorkingDirectory;
	}

	/*!
		\since ASERStudio 2.0
		获取ASER程序的最后一次启动参数列表
	*/
	QStringList ASERProgram::getLastArguments() const {
		return d->LastArguments;
	}

	/*!
		\since ASERStudio 2.0
		启动ASER程序，接受可选的参数列表。如果程序已经启动，这函数
		不产生任何效果，也不会记录参数。在调用这函数前，应首先使用
		isRunning()检查程序是否已经在运行，以避免重复启动同一程序。
	*/
	void ASERProgram::start(const QStringList& arguments) {
		if (d->ASERProcess->state() == QProcess::Running) {
			return;
		}
		d->LastArguments = arguments;
		d->ASERProcess->start(arguments.join(' '));
		QTimer* timer = new QTimer(this);
		timer->setInterval(2000);
		connect(timer, &QTimer::timeout, this, [this, timer]() {
			if (d->ASERProcess->state() == QProcess::Running) {
				if (d->ASERPipe->state() == QLocalSocket::UnconnectedState) {
					d->ASERPipe->connectToServer();
				}
			}
			else {
				timer->stop();
				timer->deleteLater();
			}
			});
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
		等待ASER程序停止，接受一个可选的超时时间（毫秒）。如果程序在
		指定时间内停止，返回true；否则返回false。如果waitMS为-1，则
		无限等待直到程序停止。
		不推荐使用这个函数，如果ASER意外卡死，它会连带卡死YSS。
		如果必须等待程序完全停止，建议使用programStopped信号来处理程序停止事件，而不是阻塞等待。
	*/
	bool ASERProgram::waitStop(qint32 waitMS) {
		if (d->ASERProcess->state() == QProcess::Running) {
			return d->ASERProcess->waitForFinished(waitMS);
		}
		else {
			return true;
		}
	}

	/*!
		\since ASERStudio 2.0
		获取ASER程序的进程ID，如果程序正在运行；否则返回-1。
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
		获取ASER程序的退出代码，如果程序已经停止；否则返回-1。
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
		检查ASER程序是否正在运行，返回true如果程序正在运行；否则返回false。
	*/
	bool ASERProgram::isRunning() const {
		return d->ASERProcess->state() == QProcess::Running;
	}

	/*!
		\fn QWindow* ASERStudio::ASEREnv::ASERProgram::getProcessWindow() const
		\since ASERStudio 2.0
		获取ASER程序的主窗口，如果程序正在运行且窗口可用；否则返回nullptr。
		这函数的实现可能依赖于特定平台的窗口管理机制，因此在不同操作系统上可能表现不同。
	*/
#ifdef Q_OS_WIN
	QWindow* ASERProgram::getProcessWindow() const {
		if (d->ASERProcess->state() == QProcess::Running) {
			HWND hwnd = FindWindowExW(nullptr, nullptr, nullptr, nullptr);
			while (hwnd) {
				DWORD pid;
				GetWindowThreadProcessId(hwnd, &pid);
				if (pid == d->ASERProcess->processId()) {
					return QWindow::fromWinId((WId)hwnd);
				}
				hwnd = FindWindowExW(nullptr, hwnd, nullptr, nullptr);
			}
		}
		return nullptr;
	}
#else
	QWindow* ASERProgram::getProcessWindow() const {
		// 在非Windows平台上，获取进程窗口的方法可能不同，且可能不可靠。
		// 这里暂时返回nullptr，表示未实现。
		return nullptr;
	}
#endif

	/*!
		\since ASERStudio 2.0
		获取ASER程序的配置文件路径。由于ASER是Unity编写的，因此
		它默认在当前用户文件夹\LocalLow\Gradus\ASE-Remake（在Windows上）
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
		获取ASER程序的资源目录路径，ASER的外部资源存放在此处。
	*/
	QString ASERProgram::getResourcePath() const {
		QFileInfo exeInfo(d->ExecutablePath);
		QString resourcePath = exeInfo.path() + "/StreamingAssets";
		return resourcePath;
	}

	/*!
		\since ASERStudio 2.0
		通过具名管道向ASER程序发送数据。接受一个字符串参数，表示要发送的数据内容。
	*/
	void ASERProgram::writeNamedPipe(const QString& context) {
		if (d->ASERPipe->state() == QLocalSocket::ConnectedState) {
			d->ASERPipe->write(context.toUtf8());
			d->ASERPipe->flush();
		}
	}
}