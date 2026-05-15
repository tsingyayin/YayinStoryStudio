#ifndef ASERStudio_ASEREnv_ASEDevIO_h
#define ASERStudio_ASEREnv_ASEDevIO_h
#include "ASERStudioCompileMacro.h"
#include <QtCore/qobject.h>

namespace ASERStudio::ASEREnv {
	class ASERProgram;
	class ASERDebugIOPrivate;
	class ASERAPI ASERDebugIO :public QObject{
		Q_OBJECT;
	public:
		enum OfficialBundleManagerInitializeState {
			Done,
			Skipped
		};
		Q_ENUM(OfficialBundleManagerInitializeState);
		enum Page {
			unknown = 0,
			home,
			player,
			storyset,
			assets,
			connection,
			setting,
		};
		Q_ENUM(Page);
	signals:
		void officialBundleManagerInitializeChanged(OfficialBundleManagerInitializeState state);
		void storySetOpened(const QString& storySetPath);
		void storyLoading(const QString& storyName);
		void storyStarted(const QString& storyName);
		void storyExited(const QString& storyName);
		void optionsActivated(qint32 optionCount);
		void errorMessageShown(const QString& message);
	public:
		ASERDebugIO();
		virtual ~ASERDebugIO();
	public:
		void setProgram(ASERProgram* program);
		ASERProgram* getProgram() const;
		void switchPage(Page page);
		Page getCurrentPage() const;
		void open(const QString& projectPath);
		void play(const QString& fileName);
		void stop();
		void select(qint32 branchIndex);
		void changeDirectory(const QString& directoryPath);
		void selectBranch(qint32 branchIndex);
		void changeSpeed();
		void toggleAuto();
	private:
		ASERDebugIOPrivate* d;
	};
}
#endif // ASERStudio_ASEREnv_ASEDevIO_h