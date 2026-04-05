#ifndef ASERStudio_AStorySyntax_AStoryXPreprocessor_h
#define ASERStudio_AStorySyntax_AStoryXPreprocessor_h
#include "ASERStudioCompileMacro.h"
#include <VIMacro.h>
#include <QtCore/qtypes.h>
#include <QtCore/qstringlist.h>

namespace ASERStudio::AStorySyntax {
	class AStoryXControllerParseData;
	class AStoryXPreprocessorPrivate;
	class ASERAPI AStoryXPreprocessor {
	public:
		AStoryXPreprocessor();
		~AStoryXPreprocessor();
		VIMoveable(AStoryXPreprocessor);
		VICopyable(AStoryXPreprocessor);
	public:
		QStringList getSupportedPreprocessors() const;
		bool isPreprocessor(const QString& str);
		AStoryXControllerParseData parse(const QString& str, qint32 cursorPosition = -1, bool diagnostic = false, qint32 lineIndex = -1);
	private:
		AStoryXPreprocessorPrivate* d;
	};
}
#endif // ASERStudio_AStorySyntax_AStoryXPreprocessor_h