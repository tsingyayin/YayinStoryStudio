#include "ASEREnv/ASERDebugIO.h"
#include "ASEREnv/private/ASERDebugIO_p.h"
#include "ASEREnv/ASERProgram.h"
#include <QtCore/qmetaobject.h>
namespace ASERStudio::ASEREnv {
	void  ASERDebugIOPrivate::handleNamedPipeReadable(const QString& context) {
		if (context.startsWith("OfficialBundleManagerInitialization:")) {
			QString state = context.mid(QString("OfficialBundleManagerInitialization:").length());
			emit q->officialBundleManagerInitializeChanged(state == "Done" ? ASERDebugIO::Done : ASERDebugIO::Skipped);
		}
		else if (context.startsWith("StorySetOpened:")) {
			QString storySetPath = context.mid(QString("StorySetOpened:").length());
			CurrentPage = ASERDebugIO::Page::storyset;
			lastStorySetPath = storySetPath;
			if (requestingPlay) {
				requestingPlay = false;
				q->play(requestingStoryName);
			}
			emit q->storySetOpened(storySetPath);
		}
		else if (context.startsWith("StoryLoading:")) {
			QString storyName = context.mid(QString("StoryLoading:").length());
			lastStoryName = storyName;
			emit q->storyLoading(storyName);
		}
		else if (context.startsWith("StoryStarted:")) {
			QString storyName = context.mid(QString("StoryStarted:").length());
			CurrentPage = ASERDebugIO::Page::player;
			emit q->storyStarted(storyName);
		}
		else if (context.startsWith("StoryExited:")) {
			QString storyName = context.mid(QString("StoryExited:").length());
			CurrentPage = ASERDebugIO::Page::storyset;
			emit q->storyExited(storyName);
		}
		else if (context.startsWith("OptionsActivated:")) {
			QString optionCountStr = context.mid(QString("OptionsActivated:").length());
			bool ok = false;
			qint32 optionCount = optionCountStr.toInt(&ok);
			if (ok) {
				emit q->optionsActivated(optionCount);
			}
		}
		else if (context.startsWith("ErrorMessageShown:")) {
			QString message = context.mid(QString("ErrorMessageShown:").length());
			emit q->errorMessageShown(message);
		}
	}

	/*!
		\class ASERStudio::ASEREnv::ASERDebugIO
		\brief ASERDebugIO提供与ASERDebugPipeline交互的API
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
			connect(program, &ASERProgram::namedPipeReadable, this, [this](const QString& context) {
					d->handleNamedPipeReadable(context);
				});
			connect(program, &ASERProgram::programStopped, this, [this](qint64) {
				d->CurrentPage = Page::home;
				d->requestingPlay = false;
				d->requestingStoryName = "";
				d->lastStorySetPath = "";
				d->lastStoryName = "";
				});
		}
	}

	/*!
		\since ASERStudio 2.0
		return 当前设置的ASERProgram实例。
		return 当前设置的ASERProgram实例的指针，如果没有设置则返回nullptr
	*/
	ASERProgram* ASERDebugIO::getProgram() const {
		return d->Program;
	}

	/*!
		\since ASERStudio 2.2
		通过具名管道向ASER程序发送切换当前页面的命令。这里封装了switch命令
	*/
	void ASERDebugIO::switchPage(Page page) {
		QString pageStr = QMetaEnum::fromType<Page>().valueToKey(static_cast<int>(page));
		if (d->Program) {
			d->Program->writeNamedPipe(QString("switch %1").arg(pageStr));
			d->CurrentPage = page;
		}
	}

	/*!
		\since ASERStudio 2.2
		return 当前页面的状态。这个状态是通过switchPage函数设置的。
		目前ASE-Remake <-> ASER Studio属于开环控制，因此状态可能不准确
	*/
	ASERDebugIO::Page ASERDebugIO::getCurrentPage() const {
		return d->CurrentPage;
	}

	/*!
		\since ASERStudio 2.2
		通过具名管道向ASER程序发送打开项目的命令。这里封装了open命令
		这个命令不会自动切换页面，只是向ASER推送一个置顶项目的请求，
		随后在切换到storyset页面时该项目会被打开
	*/
	void ASERDebugIO::open(const QString& projectPath) {
		if (d->Program) {
			d->Program->writeNamedPipe(QString("open %1").arg(projectPath));
		}
	}

	/*!
		\since ASERStudio 2.2
		通过具名管道向ASER程序发送播放故事的命令。这里封装了play命令
		play命令只能在storyset页面执行，如果当前不是，这个函数
		会自动发送相关命令切换到storyset页面。
		\a fileName 要播放的故事文件名
	*/
	void ASERDebugIO::play(const QString& fileName) {
		if (d->Program) {
			if (d->CurrentPage != Page::storyset) {
				d->requestingPlay = true;
				d->requestingStoryName = fileName;
				switchPage(Page::storyset);
			}
			else {
				d->Program->writeNamedPipe(QString("play %1").arg(fileName));
			}
		}
	}

	/*!
		\since ASERStudio 2.2
		通过具名管道向ASER程序发送停止播放的命令。这里封装了stop命令
	*/
	void ASERDebugIO::stop() {
		if (d->Program) {
			d->Program->writeNamedPipe("stop");
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