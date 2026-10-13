#include <General/Exception.h>
#include <General/LogCenter.h>
#include "Editor/InstallerClient.h"
#include "Editor/MainEditor/MainWin.h"
#include "Widgets/YSSCrashHandler.h"

namespace YSS::Widgets {
	/*!
		\class YSS::Widgets::YSSCrashHandler
		\since YayinStoryStudio 0.17.0
		\brief 崩溃发生后的最后一道处理：存证、托付安装程序、抢救用户成果。

		YSSCrashHandler 是 Visindigo::General::ApplicationExceptionMessageHandler 的实现，
		由 VIApplication::setExceptionMessageHandler 注册为异常消息处理器。

		它在本进程里只做几件都不依赖界面的事：落盘日志、把致命崩溃交给安装程序打包、
		保存项目和已打开的文件。崩溃页面由 YSSInstaller 在另一个进程里依据落盘文件渲染，
		本进程不再运行任何界面代码。

		\note 只有致命异常才会通知安装程序。不足以导致崩溃的异常既不打包也不弹页面，
		程序会带着它继续运行。
		\note 本处理器不向安装程序传递异常内容，安装程序只根据 \c{last_crash.txt}
		与其指向的崩溃报告构造页面，因此即使本进程随即消亡也不影响崩溃页面的显示。
	*/

	YSSCrashHandler::YSSCrashHandler() {}

	YSSCrashHandler::~YSSCrashHandler() {}

	/*!
		\since YayinStoryStudio 0.17.0
		接收捕获到的异常，记下它是否致命。本处理器不显示任何内容，因此此处不做别的事。
	*/
	void YSSCrashHandler::onExceptionMessage(const Visindigo::General::Exception& ex) {
		Critical = ex.isCritical();
	}

	/*!
		\since YayinStoryStudio 0.17.0
		处理器被启用。本处理器没有需要预先准备的状态，因此此处不做任何事。
	*/
	void YSSCrashHandler::enableHandler() {}

	/*!
		\since YayinStoryStudio 0.17.0
		收尾处理：落盘日志并保存项目和已打开的文件；异常致命时再请安装程序打包崩溃产物。

		打包要读此刻仍被本进程占用的文件，所以放在保存用户成果之前，
		免得保存过程中弹出的对话框拖住进程、让文件连内容都没来得及进包。
	*/
	void YSSCrashHandler::exec() {
		Visindigo::General::LogCenter::getInstance()->finalSave();
		if (Critical && YSS::Editor::InstallerClient::getInstance() != nullptr) {
			YSS::Editor::InstallerClient::getInstance()->requestCrashArchive();
		}
		// 抢救用户的成果：项目和已打开的文件都保存一次。
		if (YSS::Editor::MainWin* mainWin = YSS::Editor::MainWin::getInstance()) {
			mainWin->saveProject();
		}
	}

	/*!
		\since YayinStoryStudio 0.17.0
		处理器被停用。本处理器没有需要清理的状态，因此此处不做任何事。
	*/
	void YSSCrashHandler::disableHandler() {}
}
