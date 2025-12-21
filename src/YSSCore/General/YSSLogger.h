#ifndef YSS_General_YSSLoger_H
#define YSS_General_YSSLoger_H
#include <General/Log.h>
#include "../Macro.h"
namespace YSSCore::General {
	class YSSCoreAPI YSSLogger : public Visindigo::General::Logger {
	private:
		YSSLogger();
		static YSSLogger* Instance;
	public:
		static YSSLogger* getInstance();
	};
}

#define YSSLoggerInstance YSSCore::General::YSSLogger::getInstance()
#define yDebug vDebug(YSSLoggerInstance)
#define yMessage vMessage(YSSLoggerInstance)
#define yInfo vInfo(YSSLoggerInstance)
#define yLog vLog(YSSLoggerInstance)
#define yNotice vNotice(YSSLoggerInstance)
#define ySuccess vSuccess(YSSLoggerInstance)
#define yWarning vWarning(YSSLoggerInstance)
#define yError vError(YSSLoggerInstance)

#define yDebugF vDebugF(YSSLoggerInstance)
#define yMessageF vMessageF(YSSLoggerInstance)
#define yInfoF vInfoF(YSSLoggerInstance)
#define yLogF vLogF(YSSLoggerInstance)
#define yNoticeF vNoticeF(YSSLoggerInstance)
#define ySuccessF vSuccessF(YSSLoggerInstance)
#define yWarningF vWarningF(YSSLoggerInstance)
#define yErrorF vErrorF(YSSLoggerInstance)
#endif // !YSS_General_YSSLoger_H
