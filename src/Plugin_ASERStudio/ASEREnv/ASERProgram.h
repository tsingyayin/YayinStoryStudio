#ifndef ASERStudio_ASEREnv_ASERProgram_h
#define ASERStudio_ASEREnv_ASERProgram_h
#include "ASERStudioCompileMacro.h"
#include <QtCore/qstringlist.h>
#include <QtCore/qobject.h>
#include <VIMacro.h>

class QWindow;
namespace ASERStudio::ASEREnv {
	class ASERProgramLaunchParameterPrivate;
	class ASERProgramLaunchParameter {
	public:
		enum SizeMode {
			FullScreen = 0,
			W3840_H2160 = 1,
			W2560_H1440 = 2,
			W1920_H1080 = 3,
			W1600_H900 = 4,
			W1280_H720 = 5,
			W1024_H576 = 6,
			W640_H360 = 7
		};
	public:
		ASERProgramLaunchParameter();
		ASERProgramLaunchParameter(const QString& FileName, const QString& Path, SizeMode mode);
		~ASERProgramLaunchParameter();
		VICopyable(ASERProgramLaunchParameter);
		VIMoveable(ASERProgramLaunchParameter);
	public:
		void setFileName(const QString& FileName);
		void setPath(const QString& Path);
		void setSizeMode(SizeMode mode);
		QString getFileName() const;
		QString getPath() const;
		SizeMode getSizeMode() const;
	private:
		ASERProgramLaunchParameterPrivate* d;
	};
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
		void start(const ASERProgramLaunchParameter& parameter, const QStringList& arguments = QStringList());
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
