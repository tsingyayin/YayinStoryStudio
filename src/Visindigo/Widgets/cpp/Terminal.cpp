#include "Widgets/Terminal.h"
#include "Widgets/private/Terminal_p.h"
#include <QtWidgets/qtextbrowser.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qlayout.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qscrollbar.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qiodevice.h>
#include <QtGui/qevent.h>
#include "General/Log.h"
#include "Utility/Console.h"
#include "General/CommandHost.h"
namespace Visindigo::__Private__ {
	TerminalPrivate::TerminalPrivate()
		: QObject(nullptr) {
		stdInStream = new QTextStream(stdin);
		connect(stdInStream->device(), &QIODevice::readyRead, this, &TerminalPrivate::onStdInputAvailable);
	}
	TerminalPrivate::~TerminalPrivate() {
		if (stdInStream) {
			delete stdInStream;
			stdInStream = nullptr;
		}
	}
	void TerminalPrivate::writeToStdIn(const QString& line) {
		*stdInStream << line << "\n";
		stdInStream->flush();
	}

	void TerminalPrivate::onStdInputAvailable() {
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

	bool TerminalPrivate::eventFilter(QObject* obj, QEvent* event) {
		if (obj == inputLine) {
			if (event->type() == QEvent::KeyPress) {
				QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
				if (keyEvent->key() == Qt::Key_Up){
					QString startWith = inputLine->text().mid(0, inputLine->cursorPosition());
					if (startWith != this->commandStartWith) {
						commandHistory = VISCH->getCommandHistory(startWith);
						commandStartWith = startWith;
						historyIndex = -1;
					}
					if (!commandHistory.isEmpty()) {
						if (historyIndex + 1 < commandHistory.size()) {
							historyIndex++;
							inputLine->setText(commandHistory[historyIndex]);
							inputLine->setCursorPosition(inputLine->text().length());
						}
					}
				}
				else if (keyEvent->key() == Qt::Key_Down) {
					if (!commandHistory.isEmpty()) {
						if (historyIndex - 1 >= 0) {
							historyIndex--;
							inputLine->setText(commandHistory[historyIndex]);
							inputLine->setCursorPosition(inputLine->text().length());
						}
						else {
							historyIndex = -1;
							inputLine->clear();
						}
					}
				}
				return false;
			}
		}
		return QObject::eventFilter(obj, event);
	}
}
namespace Visindigo::Widgets {

	Terminal::Terminal(QWidget* parent)
		: QFrame(parent), d(new Visindigo::__Private__::TerminalPrivate) {
		this->setMinimumSize(800, 600);
		d->consoleView = new QTextBrowser(this); 
		d->consoleView->setLineWrapMode(QTextEdit::NoWrap);
		connect(Visindigo::General::LoggerManager::getInstance(), &Visindigo::General::LoggerManager::logReceived, 
			[this](
				const QString& handlerName, Visindigo::General::Logger::Level level, 
				const QString& message, const QString& consoleStr, const Visindigo::General::LogMetaData& metaData
			) {
					d->consoleView->append(Visindigo::Utility::Console::cmdColorToHtmlString(consoleStr));
			});
		d->inputLine = new QLineEdit(this);
		d->inputLine->installEventFilter(d);
		d->sendButton = new QPushButton(tr("Send"), this);
		d->layout = new QGridLayout(this);
		d->layout->addWidget(d->consoleView, 0, 0, 1, 2);
		d->layout->addWidget(d->inputLine, 1, 0);
		d->layout->addWidget(d->sendButton, 1, 1);
		setLayout(d->layout);
		connect(d->sendButton, &QPushButton::clicked, [this]() {
			if (d->useInput) {
				QString line = d->inputLine->text();
				VISCH->executeCommand(line);
				d->inputLine->clear();
			}
		});
		connect(d->inputLine, &QLineEdit::returnPressed, [this]() {
			if (d->useInput && d->sendOnEnter) {
				QString line = d->inputLine->text();
				VISCH->executeCommand(line);
				d->inputLine->clear();
			}
		});

		this->setStyleSheet("\
				QWidget{color: #FFFFFF;background-color: #000000}\
			");
		QFont font("Cascadia Mono");
		d->consoleView->setFont(font);
		d->inputLine->setFont(font);
		vgDebug << "Terminal initialized.";
	}

	Terminal::~Terminal() {
		delete d;
	}

	void Terminal::setUseInput(bool enable) {
		d->useInput = enable;
		d->inputLine->setEnabled(enable);
		d->sendButton->setEnabled(enable);
	}

	bool Terminal::isInputUsed() const {
		return d->useInput;
	}

	void Terminal::setSendOnEnter(bool enable) {
		d->sendOnEnter = enable;
	}

	bool Terminal::isSendOnEnter() const {
		return d->sendOnEnter;
	}

	void Terminal::setMaxCacheLines(qint32 lineCount) {
		d->maxCacheLines = lineCount;
	}

	qint32 Terminal::getMaxCacheLines() const {
		return d->maxCacheLines;
	}

	void Terminal::clearConsole() {
		d->consoleView->clear();
	}

	void Terminal::addLine(const QString& line) {
		d->consoleView->append(line);
		d->writeToStdIn(line);
	}
}
