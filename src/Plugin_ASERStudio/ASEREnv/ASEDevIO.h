#ifndef ASERStudio_ASEREnv_ASEDevIO_h
#define ASERStudio_ASEREnv_ASEDevIO_h
#include "ASERStudioCompileMacro.h"
#include <QtCore/qobject.h>

namespace ASERStudio::ASEREnv {
	class ASERProgram;
	class ASEDevIOPrivate;
	class ASERAPI ASEDevIO :public QObject{
		Q_OBJECT
	signals:
		void displayStarted();
		void displayEnded();
		void autoDisplayToggled(bool autoDisplay);
		void currentFileChanged(const QString& filePath);
		void lineIndexChanged(qint32 lineIndex, qint32 IRLineIndex = -1);
	public:
		ASEDevIO();
		virtual ~ASEDevIO();
	public:
		void setProgram(ASERProgram* program);
		void jumpToLine(qint32 lineIndex);
		void displayFile(const QString& filePath);
		void pause();
		void resume();
		void stop();
		void toggleAuto();
		void step();
	private:
		ASEDevIOPrivate* d;
	};
}
#endif // ASERStudio_ASEREnv_ASEDevIO_h