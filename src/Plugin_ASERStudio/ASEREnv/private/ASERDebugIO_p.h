#ifndef ASERStudio_ASEREnv_ASEDebugIO_p_h
#define ASERStudio_ASEREnv_ASEDebugIO_p_h
#include <QtCore/qobject.h>

namespace ASERStudio::ASEREnv {
	class ASERProgram;
	class ASERDebugIO;
	class ASERDebugIOPrivate :public QObject {
		friend class ASERDebugIO;
		Q_OBJECT;
	protected:
		ASERDebugIO* q;
		ASERProgram* Program = nullptr;
	public:
		void handleNamedPipeReadable(const QString& context);  
	};
}

#endif // ASERStudio_ASEREnv_ASEDebugIO_p_h