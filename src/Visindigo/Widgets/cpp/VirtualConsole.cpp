#include "Widgets/VirtualConsole.h"
#include "Widgets/private/VirtualConsole_p.h"
#include <QtWidgets/qtextbrowser.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qlayout.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qscrollbar.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qiodevice.h>

namespace Visindigo::__Private__ {
	VirtualConsolePrivate::VirtualConsolePrivate()
		: QObject(nullptr) {
		stdInStream = new QTextStream(stdin);
		stdOutStream = new QTextStream(stdout);
		stdErrStream = new QTextStream(stderr);
		connect(stdInStream->device(), &QIODevice::readyRead, this, &VirtualConsolePrivate::onStdInputAvailable);
		connect(stdOutStream->device(), &QIODevice::readyRead, this, &VirtualConsolePrivate::onStdOutputAvailable);
		connect(stdErrStream->device(), &QIODevice::readyRead, this, &VirtualConsolePrivate::onStdErrorAvailable);
	}
	VirtualConsolePrivate::~VirtualConsolePrivate() {
		if (stdInStream) {
			delete stdInStream;
			stdInStream = nullptr;
		}
		if (stdOutStream) {
			delete stdOutStream;
			stdOutStream = nullptr;
		}
		if (stdErrStream) {
			delete stdErrStream;
			stdErrStream = nullptr;
		}
	}
	void VirtualConsolePrivate::writeToStdIn(const QString& line) {
		*stdInStream << line << "\n";
		stdInStream->flush();
	}

	void VirtualConsolePrivate::onStdInputAvailable() {
		if (!autoListen) return;
		while (!stdInStream->atEnd()) {
			QString line = stdInStream->readLine();
			if (!line.isEmpty()) {
				consoleView->append(line);
				
				if (consoleView->document()->blockCount() > maxCacheLines) {
					// Remove oldest line
					QTextCursor cursor = consoleView->textCursor();
					cursor.movePosition(QTextCursor::Start);
					cursor.select(QTextCursor::BlockUnderCursor);
					cursor.removeSelectedText();
				}
				if (autoScroll) {
					consoleView->verticalScrollBar()->setValue(consoleView->verticalScrollBar()->maximum());
				}
			
			}
		}
	}

	void VirtualConsolePrivate::onStdOutputAvailable() {
		if (!autoListen) return;
		while (!stdOutStream->atEnd()) {
			QString line = stdOutStream->readLine();
			if (!line.isEmpty()) {
				consoleView->append(line);
				if (consoleView->document()->blockCount() > maxCacheLines) {
					// Remove oldest line
					QTextCursor cursor = consoleView->textCursor();
					cursor.movePosition(QTextCursor::Start);
					cursor.select(QTextCursor::BlockUnderCursor);
					cursor.removeSelectedText();
				}
				if (autoScroll) {
					consoleView->verticalScrollBar()->setValue(consoleView->verticalScrollBar()->maximum());
				}
			}
		}
	}

	void VirtualConsolePrivate::onStdErrorAvailable() {
		if (!autoListen) return;
		while (!stdErrStream->atEnd()) {
			QString line = stdErrStream->readLine();
			if (!line.isEmpty()) {
				consoleView->append(line);
				if (consoleView->document()->blockCount() > maxCacheLines) {
					// Remove oldest line
					QTextCursor cursor = consoleView->textCursor();
					cursor.movePosition(QTextCursor::Start);
					cursor.select(QTextCursor::BlockUnderCursor);
					cursor.removeSelectedText();
				}
				if (autoScroll) {
					consoleView->verticalScrollBar()->setValue(consoleView->verticalScrollBar()->maximum());
				}
			}
		}
	}
}
namespace Visindigo::Widgets {

	VirtualConsole::VirtualConsole(QWidget* parent)
		: QFrame(parent), d(new Visindigo::__Private__::VirtualConsolePrivate) {
		d->consoleView = new QTextBrowser(this);
		d->inputLine = new QLineEdit(this);
		d->sendButton = new QPushButton(tr("Send"), this);
		d->layout = new QGridLayout(this);
		d->layout->addWidget(d->consoleView, 0, 0, 1, 2);
		d->layout->addWidget(d->inputLine, 1, 0);
		d->layout->addWidget(d->sendButton, 1, 1);
		setLayout(d->layout);
		connect(d->sendButton, &QPushButton::clicked, [this]() {
			if (d->useInput) {
				QString line = d->inputLine->text();
				if (!line.isEmpty()) {
					addLine(line);
					d->inputLine->clear();
				}
			}
		});
		connect(d->inputLine, &QLineEdit::returnPressed, [this]() {
			if (d->useInput && d->sendOnEnter) {
				QString line = d->inputLine->text();
				if (!line.isEmpty()) {
					addLine(line);
					d->inputLine->clear();
				}
			}
		});
	}

	VirtualConsole::~VirtualConsole() {
		delete d;
	}

	void VirtualConsole::setUseInput(bool enable) {
		d->useInput = enable;
		d->inputLine->setEnabled(enable);
		d->sendButton->setEnabled(enable);
	}

	bool VirtualConsole::isInputUsed() const {
		return d->useInput;
	}

	void VirtualConsole::setSendOnEnter(bool enable) {
		d->sendOnEnter = enable;
	}

	bool VirtualConsole::isSendOnEnter() const {
		return d->sendOnEnter;
	}

	void VirtualConsole::setMaxCacheLines(qint32 lineCount) {
		d->maxCacheLines = lineCount;
	}

	qint32 VirtualConsole::getMaxCacheLines() const {
		return d->maxCacheLines;
	}

	void VirtualConsole::clearConsole() {
		d->consoleView->clear();
	}

	void VirtualConsole::addLine(const QString& line) {
		d->writeToStdIn(line);
	}
}
