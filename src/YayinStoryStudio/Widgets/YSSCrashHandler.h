#ifndef YayinStoryStudio_Widgets_YSSCrashHandler_h
#define YayinStoryStudio_Widgets_YSSCrashHandler_h
#include <General/VIApplication.h>
namespace YSS::Widgets {
	class YSSCrashHandler : public Visindigo::General::ApplicationExceptionMessageHandler {
	public:
		YSSCrashHandler();
		virtual ~YSSCrashHandler();
		virtual void onExceptionMessage(const Visindigo::General::Exception& ex) override;
		virtual void enableHandler() override;
		virtual void exec() override;
		virtual void disableHandler() override;
	private:
		// 当前异常是否致命。不足以导致崩溃的异常不惊动安装程序，也就不会显示崩溃页面。
		bool Critical = false;
	};
}
#endif // YayinStoryStudio_Widgets_YSSCrashHandler_h
