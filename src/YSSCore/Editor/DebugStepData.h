#ifndef YSSCore_Editor_DebugStepData_h
#define YSSCore_Editor_DebugStepData_h
#include "../Macro.h"
class QString;
namespace YSSCore::Editor {
	class DebugStepDataPrivate;
	class YSSCoreAPI DebugStepData final {
	public:
		DebugStepData(const QString& filePath, qint32 lineCount, qint32 column = -1, qint32 selected = 0);
		~DebugStepData();
		VIMoveable(DebugStepData);
		VICopyable(DebugStepData);
		void setFilePath(const QString& filePath);
		void setLineCount(qint32 lineCount);
		void setCursorPosition(qint32 column, qint32 selected = 0);
		QString getFilePath() const;
		qint32 getLineCount() const;
		qint32 getCursorColumn() const;
	private:
		DebugStepDataPrivate* d;
	};
}
#endif // YSSCore_Editor_DebugStepData_h