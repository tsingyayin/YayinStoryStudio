#ifndef ASERStudio_ASEREnv_ASERProgram_h
#define ASERStudio_ASEREnv_ASERProgram_h
#include "ASERStudioCompileMacro.h"
#include <QtCore/qstringlist.h>
#include <QtCore/qobject.h>
class QWindow;
namespace ASERStudio::ASEREnv {
	class ASERProgramPrivate;
	class ASERAPI ASERProgram :public QObject{
		Q_OBJECT;
	signals:
		void namedPipeReadable(const QString& context);
		void namedPipeError(const QString& error);
		void namedPipeConnected();
		void namedPipeDisconnected();
		void programStarted();
		void programStopped(qint64 exitCode);
	public:
		ASERProgram();
		~ASERProgram();
	public:
		void setExecutablePath(const QString& path);
		QString getExecutablePath() const;
		void setWorkingDirectory(const QString& path);
		QString getWorkingDirectory() const;
		QStringList getLastArguments() const;
		void start(const QStringList& arguments = QStringList());
		void stop();
		bool waitStop(qint32 waitMS = 3000);
		qint64 getProcessID() const;
		qint64 getExitCode() const;
		bool isRunning() const;
		QWindow* getProcessWindow() const;
		QString getLocalLowPath() const;
		QString getResourcePath() const;
		void writeNamedPipe(const QString& context);
	private:
		ASERProgramPrivate* d;
	};
}
#endif // ASERStudio_ASEREnv_ASERProgram_h
