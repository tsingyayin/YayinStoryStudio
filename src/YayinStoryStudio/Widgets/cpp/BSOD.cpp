#include <QtCore/qeventloop.h>
#include <QtCore/qstringlist.h>
#include <QtGui/qcursor.h>
#include <QtGui/qevent.h>
#include <QtGui/qfont.h>
#include <QtGui/qkeysequence.h>
#include <QtGui/qpalette.h>
#include <QtGui/qscreen.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qpushbutton.h>
#include <General/CrashGateway.h>
#include <General/Exception.h>
#include <General/LogCenter.h>
#include <General/TranslationHost.h>
#include "Editor/InstallerClient.h"
#include "Widgets/BSOD.h"

namespace YSS::Widgets {
	namespace {
		const QColor BackgroundColor = QColor(0x00, 0x78, 0xD7);
		const QString TextColor = "#FFFFFF";
		const QString DimColor = "#BFD9F2";
		const int FacePixelSize = 110;
		const int HeadlinePixelSize = 30;
		const int BodyPixelSize = 18;
		const int SmallPixelSize = 13;
		const int TraceMinHeight = 140;
		const int PreferredWindowWidth = 1040;
		const int PreferredWindowHeight = 720;
		const int WindowEdgeMargin = 40;

		QString toStopCodeStyle(const QString& typeName) {
			QString result;
			for (int i = 0; i < typeName.size(); ++i) {
				const QChar ch = typeName.at(i);
				if (ch.isUpper() && i > 0 && !typeName.at(i - 1).isUpper()) {
					result += QLatin1Char('_');
				}
				result += ch.toUpper();
			}
			return result;
		}
	}

	/*!
		\class YSS::Widgets::BSOD
		\since YayinStoryStudio 0.17.0
		\brief 以 Windows 10 蓝屏的样式显示未处理异常的信息。

		BSOD 同时是 Visindigo::General::ApplicationExceptionMessageHandler 的实现，
		由 VIApplication::setExceptionMessageHandler 注册为异常消息处理器。
		崩溃现场在此之前已经由 Visindigo::General::CrashGateway 落盘，本窗口只显示已经存证过的信息，
		不参与存证过程。

		\list
		\li 窗口在鼠标所在的屏幕上居中显示，默认为 1040x720，并按屏幕可用区域收缩。
		\li 没有按钮，任意按键或点击空白处即可关闭。
		\li 调用堆栈在滚动区域里完整显示，出错位置、调用堆栈、报告路径都可以选中复制，Ctrl+C 不会触发关闭。
		\li 关闭只是隐藏窗口，实例会被复用，因此不会被销毁。
		\endlist
	*/

	BSOD::BSOD(QWidget* parent) :QWidget(parent) {
		this->setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowStaysOnTopHint);
		this->setWindowTitle("Yayin Story Studio");
		this->setAutoFillBackground(true);
		QPalette palette = this->palette();
		palette.setColor(QPalette::Window, BackgroundColor);
		this->setPalette(palette);

		Layout = new QVBoxLayout(this);
		Layout->setSpacing(8);
		Layout->setContentsMargins(56, 40, 56, 32);

		Face = makeLabel(": (", TextColor, FacePixelSize, false);
		Headline = makeLabel("", TextColor, HeadlinePixelSize, false);
		Detail = makeLabel("", TextColor, BodyPixelSize, false);
		Progress = makeLabel("", TextColor, BodyPixelSize, false);
		StopCode = makeLabel("", TextColor, BodyPixelSize, false);
		Location = makeLabel("", DimColor, SmallPixelSize, true);
		// 固定文案统一在exec()里取：处理器在插件启用之前就构造好了，那时翻译器还没注册
		TraceCaption = makeLabel("", DimColor, SmallPixelSize, false);
		Trace = makeLabel("", DimColor, SmallPixelSize, true);
		Artifact = makeLabel("", DimColor, SmallPixelSize, true);
		// 右下角的确定键：蓝底上给一个半透明的圆角矩形
		ConfirmButton = new QPushButton("", this);
		ConfirmButton->setCursor(Qt::PointingHandCursor);
		ConfirmButton->setStyleSheet(QStringLiteral(
			"QPushButton{color:#FFFFFF;border:1px solid rgba(255,255,255,0.65);border-radius:4px;"
			"background:rgba(255,255,255,0.12);padding:6px 32px;}"
			"QPushButton:hover,QPushButton:focus{background:rgba(255,255,255,0.22);}"
			"QPushButton:pressed{background:rgba(255,255,255,0.06);}"));
		connect(ConfirmButton, &QPushButton::clicked, this, [this]() {
			this->close();
			});

		// 堆栈行用等宽字体且不换行，行太长就横向滚动
		Trace->setWordWrap(false);
		Trace->setAlignment(Qt::AlignTop | Qt::AlignLeft);
		Trace->setAutoFillBackground(false);
		QFont traceFont(QStringLiteral("Consolas"));
		traceFont.setStyleHint(QFont::Monospace);
		traceFont.setPixelSize(SmallPixelSize);
		Trace->setFont(traceFont);

		TraceArea = new QScrollArea(this);
		TraceArea->setWidget(Trace);
		TraceArea->setWidgetResizable(true);
		TraceArea->setFrameShape(QFrame::NoFrame);
		TraceArea->setMinimumHeight(TraceMinHeight);
		TraceArea->viewport()->setAutoFillBackground(false);
		// 横竖两条滚动条要一个样式，角落那块拼板（两条同时出现时才看得到）也要透明，否则就是个白方块
		QPalette areaPalette = TraceArea->palette();
		areaPalette.setColor(QPalette::Window, Qt::transparent);
		areaPalette.setColor(QPalette::Base, Qt::transparent);
		TraceArea->setPalette(areaPalette);
		TraceArea->setStyleSheet(QStringLiteral(
			"QScrollArea{background:transparent;border:none;}"
			"QScrollArea>QWidget>QWidget{background:transparent;}"
			"QScrollBar:vertical{background:transparent;width:8px;margin:0;}"
			"QScrollBar:horizontal{background:transparent;height:8px;margin:0;}"
			"QScrollBar::handle:vertical{background:#7FB3E8;border-radius:4px;min-height:24px;}"
			"QScrollBar::handle:horizontal{background:#7FB3E8;border-radius:4px;min-width:24px;}"
			"QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}"
			"QScrollBar::add-line:horizontal,QScrollBar::sub-line:horizontal{width:0;}"
			"QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical{background:transparent;}"
			"QScrollBar::add-page:horizontal,QScrollBar::sub-page:horizontal{background:transparent;}"));

		Layout->addWidget(Face);
		Layout->addWidget(Headline);
		Layout->addSpacing(16);
		Layout->addWidget(Detail);
		Layout->addSpacing(16);
		Layout->addWidget(Progress);
		Layout->addSpacing(16);
		Layout->addWidget(StopCode);
		Layout->addWidget(Location);
		Layout->addSpacing(16);
		Layout->addWidget(TraceCaption);
		Layout->addWidget(TraceArea, 1);
		Layout->addSpacing(8);
		Layout->addWidget(Artifact);
		Layout->addSpacing(16);
		Layout->addWidget(ConfirmButton, 0, Qt::AlignRight);
	}

	BSOD::~BSOD() {}

	QLabel* BSOD::makeLabel(const QString& text, const QString& color, int pixelSize, bool selectable) {
		QLabel* label = new QLabel(text, this);
		label->setStyleSheet(QStringLiteral("color:%1;").arg(color));
		label->setWordWrap(true);
		QFont labelFont = label->font();
		labelFont.setPixelSize(pixelSize);
		// 大号的 ":(" 用更细的字重，贴近 Win10 蓝屏的观感
		labelFont.setWeight(pixelSize > HeadlinePixelSize ? QFont::Thin : QFont::Light);
		if (pixelSize > HeadlinePixelSize) {
			// 冒号和括号之间不要拉得那么开
			labelFont.setLetterSpacing(QFont::AbsoluteSpacing, -pixelSize / 8);
		}
		label->setFont(labelFont);
		if (selectable) {
			label->setTextInteractionFlags(Qt::TextSelectableByMouse);
		}
		return label;
	}

	QString BSOD::buildStacktrace(const Visindigo::General::Exception& ex) const {
		const QList<Visindigo::General::StacktraceFrame> frames = ex.getStacktrace();
		if (frames.isEmpty()) {
			return QString();
		}
		const int count = static_cast<int>(frames.size());
		QStringList lines;
		for (int i = 0; i < count; ++i) {
			const Visindigo::General::StacktraceFrame& frame = frames.at(i);
			QString line = QStringLiteral("%1  %2").arg(i, 2, 10, QLatin1Char('0')).arg(frame.getFunctionName());
			const QString source = frame.getSourceFileName();
			if (!source.isEmpty()) {
				line += QStringLiteral(" (%1:%2)").arg(source).arg(frame.getLineNumber());
			}
			lines << line;
		}
		return lines.join(QLatin1Char('\n'));
	}

	void BSOD::onExceptionMessage(const Visindigo::General::Exception& ex) {
		Headline->setText(ex.isCritical()
			? VITRL("YSS::crashScreen.headline")
			: VITRL("YSS::crashScreen.headlineNonCritical"));
		Detail->setText(ex.getMessage());
		Progress->setText(VITRL("YSS::crashScreen.progress"));
		StopCode->setText(VITRL("YSS::crashScreen.stopCode")
			.arg(toStopCodeStyle(Visindigo::General::Exception::typeToString(ex.getType()))));

		QStringList locationParts;
		if (!ex.getFunction().isEmpty()) {
			locationParts << ex.getFunction();
		}
		if (!ex.getFile().isEmpty()) {
			locationParts << QStringLiteral("%1:%2").arg(ex.getFile()).arg(ex.getLine());
		}
		Location->setText(locationParts.isEmpty()
			? QString()
			: VITRL("YSS::crashScreen.location").arg(locationParts.join(QLatin1Char(' '))));
		Location->setVisible(!Location->text().isEmpty());

		Trace->setText(buildStacktrace(ex));
		TraceCaption->setVisible(!Trace->text().isEmpty());
		TraceArea->setVisible(!Trace->text().isEmpty());

		Artifact->setText(VITRL("YSS::crashScreen.reportFolder")
			.arg(Visindigo::General::CrashGateway::getReportFolderPath())
			+ QLatin1Char('\n') + VITRL("YSS::crashScreen.reportHint"));
	}

	void BSOD::enableHandler() {
		// 此时界面内容还没有填好，真正的显示在 exec() 里完成
	}

	void BSOD::exec() {
		// 打包要读日志文件，而页面关掉之后进程就要退出了，所以先落盘再请安装程序打包（不等回执），最后才显示页面。
		Visindigo::General::LogCenter::getInstance()->finalSave();
		if (YSS::Editor::InstallerClient::getInstance() != nullptr) {
			YSS::Editor::InstallerClient::getInstance()->requestCrashArchive();
		}
		QEventLoop loop;
		connect(this, &BSOD::dismissed, &loop, &QEventLoop::quit);
		// 固定文案在这里取，理由同构造函数里的注释
		TraceCaption->setText(VITRL("YSS::crashScreen.stacktrace"));
		ConfirmButton->setText(VITRL("YSS::crashScreen.dismiss"));
		ConfirmButton->setFocus();
		QScreen* target = QApplication::screenAt(QCursor::pos());
		if (target == nullptr) {
			target = QApplication::primaryScreen();
		}
		if (target != nullptr) {
			const QRect available = target->availableGeometry();
			const int width = qMax(420, qMin(PreferredWindowWidth, available.width() - WindowEdgeMargin * 2));
			const int height = qMax(320, qMin(PreferredWindowHeight, available.height() - WindowEdgeMargin * 2));
			this->resize(width, height);
			this->move(available.x() + (available.width() - width) / 2,
				available.y() + (available.height() - height) / 2);
		}
		this->show();
		this->raise();
		this->activateWindow();
		// 极端情况下窗口在显示时就被关掉了，此时不需要再等
		if (this->isVisible()) {
			loop.exec();
		}
	}

	void BSOD::disableHandler() {
		this->close();
	}

	void BSOD::keyPressEvent(QKeyEvent* event) {
		// 允许复制选中的文本；Esc 关闭，其余按键放行（Tab 要能走到确定按钮上）
		if (event->matches(QKeySequence::Copy) || event->matches(QKeySequence::SelectAll)) {
			QWidget::keyPressEvent(event);
			return;
		}
		if (event->key() == Qt::Key_Escape) {
			this->close();
			return;
		}
		QWidget::keyPressEvent(event);
	}

	void BSOD::closeEvent(QCloseEvent* event) {
		QWidget::closeEvent(event);
		emit dismissed();
	}
}
