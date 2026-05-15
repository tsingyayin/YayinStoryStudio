#ifndef ASERStudio_ASEREnv_ASERWarpper_h
#define ASERStudio_ASEREnv_ASERWarpper_h
#include "ASEREnv/ASERProgram.h"
#include <QtWidgets/qframe.h>

namespace ASERStudio::ASEREnv {
	class ASERWarpperPrivate;
	class ASERAPI ASERWarpper :public QFrame {
		Q_OBJECT;
	public:
		ASERWarpper(QWidget* parent = nullptr);
		virtual ~ASERWarpper();
		virtual void resizeEvent(QResizeEvent* event) override;
		void onProgramStarted();
	private:
		ASERWarpperPrivate* d;
	};
}
#endif // ASERStudio_ASEREnv_ASERWarpper_h