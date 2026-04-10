#include "ASEREnv/ASERDebugIO.h"
#include "ASEREnv/private/ASERDebugIO_p.h"
#include "ASEREnv/ASERProgram.h"
namespace ASERStudio::ASEREnv {
	void  ASERDebugIOPrivate::handleNamedPipeReadable(const QString& context) {
		if (context == QString("started")) {
			emit q->displayStarted();
		}
		else if (context == QString("ended")) {
			emit q->displayEnded();
		}
		else if (context.startsWith("path ")) {
			QString directoryPath = context.mid(QString("path ").length());
			emit q->currentDirectoryChanged(directoryPath);
		}
		else if (context.startsWith("fileName ")) {
			QString filePath = context.mid(QString("fileName ").length());
			emit q->currentFileChanged(filePath);
		}
		else if (context.startsWith("rawIndex ")) {
			qint32 lineIndex = context.mid(QString("rawIndex ").length()).toInt();
			emit q->lineIndexChanged(lineIndex);
		}
		else if (context.startsWith("options ")) {
			QStringList branchNames = context.mid(QString("options ").length()).split('/');
			emit q->branchRequested(branchNames);
		}
		else if (context.startsWith("error ")) {
			QString error = context.mid(QString("error ").length());
			emit q->errorOccurred(error);
		}
	}

	/*!
		\class ASERStudio::ASEREnv::ASERDebugIO
		\brief ASERDebugIO提供与ASE-Remake调试输出的交互接口
		\since ASERStudio 2.0
		\inmodule ASERStudio

		ASERDebugIO类封装了与ASE-Remake调试输出的交互功能。
		它通过连接ASERProgram的具名管道来接收来自ASE-Remake的调试信息，
		并将这些信息转换为相应的信号，以供其他组件使用。

		值得指出的是，ASERDebugIO并不单独维护具名管道，因此你需要
		创建一个ASERProgram实例，并将其设置给ASERDebugIO，以便正确接收调试信息。
		所以，这类也未设置任何与具名管道相关的控制或响应API，它只是依附于ASERProgram来工作的。

		如果没有有效的ASERProgram设置，这类的任何设置函数均无效，也不会有信号发出。
	*/

	/*!
		\since ASERStudio 2.0
		构造函数。
	*/
	ASERDebugIO::ASERDebugIO(){
		d = new ASERDebugIOPrivate;
		d->q = this;
	}

	/*!
		\since ASERStudio 2.0
		析构函数。
	*/
	ASERDebugIO::~ASERDebugIO() {
		delete d;
	}

	/*!
		\since ASERStudio 2.0
		设置ASERProgram实例以接收调试信息。
		\a program ASERProgram实例的指针
	*/
	void ASERDebugIO::setProgram(ASERProgram* program) {
		if (program) {
			d->Program = program;
			connect(this->d, &ASERDebugIOPrivate::handleNamedPipeReadable, this, [this](const QString& context) {
					d->handleNamedPipeReadable(context);
				});
		}
	}

	/*!
		\since ASERStudio 2.0
		获取当前设置的ASERProgram实例。
		\return 当前设置的ASERProgram实例的指针，如果没有设置则返回nullptr
	*/
	ASERProgram* ASERDebugIO::getProgram() const {
		return d->Program;
	}

	/*!
		\since ASERStudio 2.0
		通过具名管道向ASER程序发送跳转到指定行的命令。这里封装了jump命令
		\a lineIndex 要跳转到的行索引
	*/
	void ASERDebugIO::jumpToLine(qint32 lineIndex) {
		if (d->Program) {
			d->Program->writeNamedPipe(QString("jump %1").arg(lineIndex));
		}
	}

	/*!
		\since ASERStudio 2.0
		通过具名管道向ASER程序发送切换当前目录的命令。这里封装了cd命令
		\a directoryPath 要切换到的目录路径
	*/
	void ASERDebugIO::changeDirectory(const QString& directoryPath) {
		if (d->Program) {
			d->Program->writeNamedPipe(QString("cd %1").arg(directoryPath));
		}
	}

	/*!
		\since ASERStudio 2.0
		通过具名管道向ASER程序发送选择分支的命令。这里封装了select命令
		\a branchIndex 要选择的分支索引
	*/
	void ASERDebugIO::selectBranch(qint32 branchIndex) {
		if (d->Program) {
			d->Program->writeNamedPipe(QString("select %1").arg(branchIndex));
		}
	}

	/*!
		\since ASERStudio 2.0
		通过具名管道向ASER程序发送显示指定文件的命令。这里封装了play命令
		\a filePath 要显示的文件路径
	*/
	void ASERDebugIO::displayFile(const QString& filePath) {
		if (d->Program) {
			d->Program->writeNamedPipe(QString("play %1").arg(filePath));
		}
	}

	/*!
		\since ASERStudio 2.0
		通过具名管道向ASER程序发送切换播放速度的命令。这里封装了changespeed命令
	*/
	void ASERDebugIO::changeSpeed() {
		if (d->Program) {
			d->Program->writeNamedPipe("changespeed");
		}
	}

	/*!
		\since ASERStudio 2.0
		通过具名管道向ASER程序发送切换自动播放的命令。这里封装了toggleauto命令
	*/
	void ASERDebugIO::toggleAuto() {
		if (d->Program) {
			d->Program->writeNamedPipe("toggleauto");
		}
	}

}