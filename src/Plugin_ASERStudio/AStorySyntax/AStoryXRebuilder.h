#ifndef ASERStudio_AStorySyntax_AStoryXRebuilder_h
#define ASERStudio_AStorySyntax_AStoryXRebuilder_h
#include "ASERStudioCompileMacro.h"
#include <QtCore/qstring.h>
#include <QtCore/qmap.h>
#include "AStorySyntax/AStoryXController.h"
namespace ASERStudio::AStorySyntax {
	class AStoryXControllerParseData;
	class ASERAPI AStoryXRebuilder {
	public:
		enum RebuildErrorCode {
			Success = 0,
			InvalidRuleName,
			InvalidControllerType,
			InvalidRequiredParameter,
			InvalidOptionalParameter,
			InvalidRawLine,
			FileNotFound,
			ReadFileFailed,
			WriteFileFailed,
			UnknownError
		};
	public:
		static QString rebuild(const QString& ruleName, const AStoryXControllerParseData& data, RebuildErrorCode* errorCode = nullptr);
		static QString rebuild(const QString& ruleName, AStoryXController::ControllerType type, 
			const QString& reqPara, const QMap<QString, QString>& optParas, RebuildErrorCode* errorCode = nullptr);
		static QString rebuild(const QString& ruleName, const QString& rawLine, RebuildErrorCode* errorCode = nullptr);
		static bool rebuildDocument(const QString& fromFile, const QString& toFile, const QString& fromRule, const QString& toRule,
			RebuildErrorCode* errorCode = nullptr);
	};
}
#endif // ASERStudio_AStorySyntax_AStoryXRebuilder_h