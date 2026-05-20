#ifndef ASERStudio_ASEREnv_ASEDebugIO_p_h
#define ASERStudio_ASEREnv_ASEDebugIO_p_h
#include <QtCore/qobject.h>
#include "ASEREnv/ASERDebugIO.h"
namespace ASERStudio::ASEREnv {
	class ASERProgram;
	class ASERDebugIO;
	class ASERDebugIOPrivate :public QObject {
		friend class ASERDebugIO;
		Q_OBJECT;
	protected:
		ASERDebugIO* q;
		ASERProgram* Program = nullptr;
		ASERDebugIO::Page CurrentPage = ASERDebugIO::Page::home;
		bool requestingPlay = false;
		QString requestingStoryName;
		QString lastStorySetPath;
		QString lastStoryName;
	public:
		void handleNamedPipeReadable(const QString& context);
	};
}

#endif // ASERStudio_ASEREnv_ASEDebugIO_p_h