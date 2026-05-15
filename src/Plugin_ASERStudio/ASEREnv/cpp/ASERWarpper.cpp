#include "ASEREnv/ASERWarpper.h"
#include "Plugin_ASERStudio.h"
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qboxlayout.h>
#include <QtGui/qevent.h>
#include <General/TranslationHost.h>
#include <QtGui/qwindow.h>
#include <General/Log.h>
#include <QtCore/qtimer.h>
namespace ASERStudio::ASEREnv {
	class ASERWarpperPrivate {
		friend class ASERWarpper;
	protected:
		ASERProgram* Program;
		QWidget* ASERWindow;
		QLabel* StatusLabel;
		QPushButton* StartButton;
		QVBoxLayout* Layout;
	};
	ASERWarpper::ASERWarpper(QWidget* parent) :QFrame(parent) {
		d = new ASERWarpperPrivate;
		d->Program = ASERStudio::Main::getInstance()->getASERProgram();
		d->ASERWindow = nullptr;
		d->StartButton = new QPushButton(VITR("ASERStudio::ASEREnv.startButton"), this);
		d->StatusLabel = new QLabel(VITR("ASERStudio::ASEREnv.noProgram"), this);
		d->Layout = new QVBoxLayout(this);
		d->Layout->addWidget(d->StatusLabel);
		d->Layout->addWidget(d->StartButton);
		connect(d->Program, &ASERProgram::programStarted, this, &ASERWarpper::onProgramStarted);
		connect(d->StartButton, &QPushButton::clicked, this, [this]() {
			if (d->Program->isRunning()) {
				d->Layout->removeWidget(d->ASERWindow);
				d->ASERWindow->deleteLater(); // 解除
				return;
			}
			d->Program->start();
			});
	}
	ASERWarpper::~ASERWarpper() {
		delete d;
	}
	void ASERWarpper::resizeEvent(QResizeEvent* event) {
		QFrame::resizeEvent(event);
		if (d->ASERWindow) {
			//d->ASERWindow->setGeometry(this->rect());
		}
	}
	void ASERWarpper::onProgramStarted() {
		QTimer::singleShot(3000, this, [this]() {
			d->StatusLabel->hide();
			//d->StartButton->hide();
			QWindow* programWindow = d->Program->getProcessWindow();
			if (programWindow) {
				vgDebug << "ASER program window found, creating window container.";
				programWindow->setVisibility(QWindow::Windowed);
				d->ASERWindow = QWidget::createWindowContainer(programWindow, this);
				d->ASERWindow->setFixedSize(640, 360);
				d->ASERWindow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
				d->ASERWindow->setFocusPolicy(Qt::StrongFocus);
				d->Layout->addWidget(d->ASERWindow);
				d->ASERWindow->show();
			}
			else {
				vgDebug << "Failed to create window container for ASER program window.";
			}
			});
	}

}