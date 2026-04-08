#ifndef ASERStudio_ASEREnv_ASEDevIO_h
#define ASERStudio_ASEREnv_ASEDevIO_h
#include "ASERStudioCompileMacro.h"
#include <QtCore/qobject.h>

namespace ASERStudio::ASEREnv {
	class ASERProgram;
	class ASERDebugIOPrivate;
	class ASERAPI ASERDebugIO :public QObject{
		Q_OBJECT
	signals:
		void displayStarted();
		void displayEnded();
		void currentDirectoryChanged(const QString& directoryPath);
		void currentFileChanged(const QString& filePath);
		void lineIndexChanged(qint32 lineIndex);
		void errorOccurred(const QString& error);
		void branchRequested(const QStringList& branchNames);
	public:
		ASERDebugIO();
		virtual ~ASERDebugIO();
	public:
		void setProgram(ASERProgram* program);
		ASERProgram* getProgram() const;
		void jumpToLine(qint32 lineIndex);
		void changeDirectory(const QString& directoryPath);
		void selectBranch(qint32 branchIndex);
		void displayFile(const QString& filePath);
		void changeSpeed();
		void toggleAuto();
	private:
		ASERDebugIOPrivate* d;
	};
}
#endif // ASERStudio_ASEREnv_ASEDevIO_h