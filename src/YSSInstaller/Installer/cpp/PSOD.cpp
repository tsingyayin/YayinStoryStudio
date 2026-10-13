#include <QtCore/qeventloop.h>
#include <QtCore/qfile.h>
#include <QtCore/qstringlist.h>
#include <QtGui/qcursor.h>
#include <QtGui/qevent.h>
#include <QtGui/qfont.h>
#include <QtGui/qkeysequence.h>
#include <QtGui/qpalette.h>
#include <QtGui/qscreen.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qscrollarea.h>
#include <General/TranslationHost.h>
#include "Installer/PSOD.h"

namespace YSS::Installer {
	namespace {
		// Visindigo 紫，与 YSS 里的崩溃页面保持同一套取色
		const QColor BackgroundColor = QColor(0xDF, 0x81, 0xE8);
		const QString TextColor = "#FFFFFF";
		// 次要文字：白字压在紫底上淡化后得到的淡紫白
		const QString DimColor = "#F7DEF9";
		// 滚动条把手用底色提亮后的浅紫
		const QString ScrollHandleColor = "#F0C3F4";
		const int FacePixelSize = 110;
		const int HeadlinePixelSize = 30;
		const int BodyPixelSize = 18;
		const int ArtifactPixelSize = 15;
		const int SmallPixelSize = 13;
		const int TraceMinHeight = 140;
		const int PreferredWindowWidth = 1040;
		const int PreferredWindowHeight = 720;
		const int WindowEdgeMargin = 40;

		// 崩溃报告由 CrashGateway 以 UTF-8 写入，不带标记
		const QString ReportTitle = QStringLiteral("Visindigo Crash Report");
		const QString TypePrefix = QStringLiteral("Exception Type: ");
		const QString MessagePrefix = QStringLiteral("Exception Message: ");
		const QString FolderPrefix = QStringLiteral("Report Folder: ");
		const QString ExecutingHeader = QStringLiteral("When executing function:");
		const QString StacktraceHeader = QStringLiteral("Stacktrace:");
		const QString UnknownFrame = QStringLiteral("<unknown>");

		// 从崩溃报告里抽出来的、页面要用到的那几项
		struct CrashReportData {
			bool valid = false;
			QString exceptionType;
			QString message;
			QString function;
			QString file;
			int line = 0;
			QString reportFolder;
			QStringList frames;
		};

		// NullPointer -> NULL_POINTER，与停止代码的观感一致
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

		// 报告里的出错位置形如 "命名空间::函数 (源文件:行号)"
		void parseExecutingLine(const QString& rawLine, CrashReportData& data) {
			const QString line = rawLine.trimmed();
			if (!line.endsWith(QLatin1Char(')'))) {
				data.function = line;
				return;
			}
			const int open = line.lastIndexOf(QLatin1Char('('));
			if (open < 0) {
				data.function = line;
				return;
			}
			data.function = line.left(open).trimmed();
			const QString inside = line.mid(open + 1, line.size() - open - 2);
			const int colon = inside.lastIndexOf(QLatin1Char(':'));
			if (colon < 0) {
				data.file = inside;
				return;
			}
			data.file = inside.left(colon);
			bool ok = false;
			const int lineNumber = inside.mid(colon + 1).toInt(&ok);
			if (ok) {
				data.line = lineNumber;
			}
		}

		// 报告的堆栈行形如 "0<TAB>二进制 ! 函数 (+地址) in 源文件 at 行号"。
		// 解析不出来时原样返回，宁可难看也不要丢掉信息。
		QString formatFrame(const QString& rawLine) {
			const int tab = rawLine.indexOf(QLatin1Char('\t'));
			if (tab < 0) {
				return rawLine.trimmed();
			}
			const QString index = rawLine.left(tab).trimmed();
			const QString rest = rawLine.mid(tab + 1);
			QString function = rest;
			QString source;
			QString lineNumber;
			const int bang = rest.indexOf(QStringLiteral(" ! "));
			if (bang >= 0) {
				function = rest.mid(bang + 3);
				const int address = function.indexOf(QStringLiteral(" (+"));
				if (address >= 0) {
					function = function.left(address);
				}
				const int inPos = rest.lastIndexOf(QStringLiteral(" in "));
				if (inPos > bang) {
					source = rest.mid(inPos + 4);
					const int atPos = source.lastIndexOf(QStringLiteral(" at "));
					if (atPos >= 0) {
						lineNumber = source.mid(atPos + 4).trimmed();
						source = source.left(atPos);
					}
				}
				// 报告里字段为空时会写成 <unknown>，别把 "(<unknown>:0)" 挂到函数名后面
				if (source == UnknownFrame) {
					source.clear();
				}
			}
			QString result = index.rightJustified(2, QLatin1Char('0')) + QStringLiteral("  ")
				+ (function.isEmpty() ? UnknownFrame : function);
			if (!source.isEmpty()) {
				result += QStringLiteral(" (%1:%2)").arg(source, lineNumber);
			}
			return result;
		}

		// 读并解析落盘的崩溃报告。读不到或不是报告时 valid 保持 false，页面退回简版。
		CrashReportData parseCrashReport(const QString& reportPath) {
			CrashReportData data;
			QFile file(reportPath);
			if (!file.open(QIODevice::ReadOnly)) {
				return data;
			}
			QString text = QString::fromUtf8(file.readAll());
			file.close();
			text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
			const QStringList lines = text.split(QLatin1Char('\n'));
			if (lines.isEmpty() || lines.first().trimmed() != ReportTitle) {
				return data;
			}
			data.valid = true;
			for (int i = 0; i < lines.size(); ++i) {
				const QString line = lines.at(i);
				if (line.startsWith(TypePrefix)) {
					data.exceptionType = line.mid(TypePrefix.size()).trimmed();
				}
				else if (line.startsWith(FolderPrefix)) {
					data.reportFolder = line.mid(FolderPrefix.size()).trimmed();
				}
				else if (line.startsWith(MessagePrefix)) {
					// 消息本身可能自带换行，一直取到空行为止
					QStringList parts;
					parts << line.mid(MessagePrefix.size());
					while (i + 1 < lines.size() && !lines.at(i + 1).trimmed().isEmpty()) {
						++i;
						parts << lines.at(i);
					}
					data.message = parts.join(QLatin1Char('\n'));
				}
				else if (line.trimmed() == ExecutingHeader) {
					if (i + 1 < lines.size()) {
						++i;
						parseExecutingLine(lines.at(i), data);
					}
				}
				else if (line.trimmed() == StacktraceHeader) {
					// 紧跟的一行固定是列标题，先跳过
					++i;
					while (i + 1 < lines.size() && !lines.at(i + 1).trimmed().isEmpty()) {
						++i;
						data.frames << formatFrame(lines.at(i));
					}
				}
			}
			return data;
		}
	}

	class PSODPrivate {
		friend class PSOD;
	protected:
		QVBoxLayout* Layout = nullptr;
		QLabel* Face = nullptr;
		QLabel* Headline = nullptr;
		QLabel* Detail = nullptr;
		QLabel* StopCode = nullptr;
		QLabel* Location = nullptr;
		QLabel* TraceCaption = nullptr;
		QLabel* Trace = nullptr;
		QScrollArea* TraceArea = nullptr;
		QLabel* Artifact = nullptr;
		QPushButton* ConfirmButton = nullptr;

		QLabel* makeLabel(QWidget* parent, const QString& color, int pixelSize, bool selectable) {
			QLabel* label = new QLabel(parent);
			label->setStyleSheet(QStringLiteral("color:%1;").arg(color));
			label->setWordWrap(true);
			QFont labelFont = label->font();
			labelFont.setPixelSize(pixelSize);
			// 大号的表情用更细的字重，免得笔画糊成一团
			labelFont.setWeight(pixelSize > HeadlinePixelSize ? QFont::Thin : QFont::Light);
			label->setFont(labelFont);
			if (selectable) {
				label->setTextInteractionFlags(Qt::TextSelectableByMouse);
			}
			return label;
		}

		void buildUi(PSOD* host) {
			host->setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowStaysOnTopHint);
			host->setWindowTitle(QStringLiteral("Yayin Story Studio"));
			host->setWindowModality(Qt::ApplicationModal);
			host->setAutoFillBackground(true);
			QPalette palette = host->palette();
			palette.setColor(QPalette::Window, BackgroundColor);
			host->setPalette(palette);

			Layout = new QVBoxLayout(host);
			Layout->setSpacing(8);
			Layout->setContentsMargins(56, 40, 56, 32);

			Face = makeLabel(host, TextColor, FacePixelSize, false);
			Face->setText(QStringLiteral("(；′⌒`)"));
			Headline = makeLabel(host, TextColor, HeadlinePixelSize, false);
			Detail = makeLabel(host, TextColor, BodyPixelSize, false);
			StopCode = makeLabel(host, DimColor, SmallPixelSize, false);
			Location = makeLabel(host, DimColor, SmallPixelSize, true);
			TraceCaption = makeLabel(host, DimColor, SmallPixelSize, false);
			TraceCaption->setText(VITRL("YSSInstaller::CrashScreen.Stacktrace"));
			Trace = makeLabel(host, DimColor, SmallPixelSize, true);
			// 报告目录和求助提示是要用户动手照做的一段，给亮色，别跟下面的技术细节一起沉下去
			Artifact = makeLabel(host, TextColor, ArtifactPixelSize, true);

			// 右下角的确定键：紫底上给一个半透明的圆角矩形
			ConfirmButton = new QPushButton(VITRL("YSSInstaller::CrashScreen.Dismiss"), host);
			ConfirmButton->setCursor(Qt::PointingHandCursor);
			ConfirmButton->setStyleSheet(QStringLiteral(
				"QPushButton{color:#FFFFFF;border:1px solid rgba(255,255,255,0.65);border-radius:4px;"
				"background:rgba(255,255,255,0.12);padding:6px 32px;}"
				"QPushButton:hover,QPushButton:focus{background:rgba(255,255,255,0.22);}"
				"QPushButton:pressed{background:rgba(255,255,255,0.06);}"));
			QObject::connect(ConfirmButton, &QPushButton::clicked, host, [host]() {
				host->close();
				});

			// 堆栈行用等宽字体且不换行，行太长就横向滚动
			Trace->setWordWrap(false);
			Trace->setAlignment(Qt::AlignTop | Qt::AlignLeft);
			Trace->setAutoFillBackground(false);
			QFont traceFont(QStringLiteral("Consolas"));
			traceFont.setStyleHint(QFont::Monospace);
			traceFont.setPixelSize(SmallPixelSize);
			Trace->setFont(traceFont);

			TraceArea = new QScrollArea(host);
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
				"QScrollBar::handle:vertical{background:%1;border-radius:4px;min-height:24px;}"
				"QScrollBar::handle:horizontal{background:%2;border-radius:4px;min-width:24px;}"
				"QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}"
				"QScrollBar::add-line:horizontal,QScrollBar::sub-line:horizontal{width:0;}"
				"QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical{background:transparent;}"
				"QScrollBar::add-page:horizontal,QScrollBar::sub-page:horizontal{background:transparent;}")
				.arg(ScrollHandleColor, ScrollHandleColor));

			Layout->addWidget(Face);
			Layout->addWidget(Headline);
			Layout->addSpacing(16);
			Layout->addWidget(Detail);
			Layout->addSpacing(16);
			Layout->addWidget(Artifact);
			Layout->addWidget(StopCode);
			Layout->addWidget(Location);
			Layout->addSpacing(16);
			Layout->addWidget(TraceCaption);
			Layout->addWidget(TraceArea, 1);
			Layout->addSpacing(16);
			Layout->addWidget(ConfirmButton, 0, Qt::AlignRight);
		}

		void applyReport(const CrashReportData& data, const QString& fallbackFolder) {
			// 安装程序只会因致命崩溃被叫来，标题不区分致命与否
			Headline->setText(VITRL("YSSInstaller::CrashScreen.Headline"));
			Detail->setText(data.message.isEmpty()
				? QString()
				: VITRL("YSSInstaller::CrashScreen.Message") + data.message);
			StopCode->setText(data.exceptionType.isEmpty()
				? QString()
				: VITRL("YSSInstaller::CrashScreen.StopCode").arg(toStopCodeStyle(data.exceptionType)));
			StopCode->setVisible(!StopCode->text().isEmpty());

			QStringList locationParts;
			if (!data.function.isEmpty()) {
				locationParts << data.function;
			}
			if (!data.file.isEmpty()) {
				locationParts << QStringLiteral("%1:%2").arg(data.file).arg(data.line);
			}
			Location->setText(locationParts.isEmpty()
				? QString()
				: VITRL("YSSInstaller::CrashScreen.Location").arg(locationParts.join(QLatin1Char(' '))));
			Location->setVisible(!Location->text().isEmpty());

			Trace->setText(data.frames.join(QLatin1Char('\n')));
			TraceCaption->setVisible(!data.frames.isEmpty());
			TraceArea->setVisible(!data.frames.isEmpty());

			QString artifactText = VITRL("YSSInstaller::CrashScreen.ReportFolder")
				.arg(data.reportFolder.isEmpty() ? fallbackFolder : data.reportFolder);
			artifactText += QLatin1Char('\n');
			if (data.valid) {
				artifactText += VITRL("YSSInstaller::CrashScreen.ReportHint");
			}
			else {
				// 没读到报告就别再让人去找里面的内容，直接说清楚下一步该做什么
				artifactText += VITRL("YSSInstaller::CrashScreen.NoReport");
			}
			Artifact->setText(artifactText);
		}

		void centerOnCursorScreen(PSOD* host) {
			QScreen* target = QApplication::screenAt(QCursor::pos());
			if (target == nullptr) {
				target = QApplication::primaryScreen();
			}
			if (target == nullptr) {
				return;
			}
			const QRect available = target->availableGeometry();
			const int width = qMax(420, qMin(PreferredWindowWidth, available.width() - WindowEdgeMargin * 2));
			const int height = qMax(320, qMin(PreferredWindowHeight, available.height() - WindowEdgeMargin * 2));
			host->resize(width, height);
			host->move(available.x() + (available.width() - width) / 2,
				available.y() + (available.height() - height) / 2);
		}
	};

	/*!
		\class YSS::Installer::PSOD
		\inmodule YSSInstaller
		\since YSSInstaller 0.17.0
		\brief 紫屏崩溃页面。

		崩溃页面由安装程序而不是崩溃进程自己显示：进程在捕获到异常之后还能做什么并不可靠，
		而安装程序此刻是健康的，手里又已经拿到了打包好的崩溃产物。

		页面内容全部来自落盘的崩溃报告，不依赖崩溃进程再传任何东西，
		因此崩溃进程即使在发出请求后立刻消失，页面依然完整。

		\list
		\li 窗口在鼠标所在的屏幕上居中显示，默认为 1040x720，并按屏幕可用区域收缩。
		\li 窗口是应用级模态，显示期间不能操作安装程序的其它窗口；按 Esc 或右下角的确定键关闭。
		\li 调用堆栈在滚动区域里完整显示，出错位置、调用堆栈、报告路径都可以选中复制，Ctrl+C 不会触发关闭。
		\endlist
	*/

	/*!
		\since YSSInstaller 0.17.0
		构造崩溃页面。
		\a reportPath 为崩溃报告文件的落盘路径，读不到时页面退回只显示目录与提示的简版。
		\a reportFolder 为报告不可用时的兜底目录，报告里自带目录时以报告为准。
	*/
	PSOD::PSOD(const QString& reportPath, const QString& reportFolder, QWidget* parent) :QWidget(parent) {
		d = new PSODPrivate();
		d->buildUi(this);
		d->applyReport(parseCrashReport(reportPath), reportFolder);
	}

	PSOD::~PSOD() {
		delete d;
	}

	/*!
		\since YSSInstaller 0.17.0
		居中显示崩溃页面并等待用户关闭，返回后即可安全退出。
	*/
	void PSOD::execute() {
		QEventLoop loop;
		connect(this, &PSOD::dismissed, &loop, &QEventLoop::quit);
		d->centerOnCursorScreen(this);
		this->show();
		this->raise();
		this->activateWindow();
		d->ConfirmButton->setFocus();
		// 极端情况下窗口在显示时就被关掉了，此时不需要再等
		if (this->isVisible()) {
			loop.exec();
		}
	}

	void PSOD::keyPressEvent(QKeyEvent* event) {
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

	void PSOD::closeEvent(QCloseEvent* event) {
		QWidget::closeEvent(event);
		emit dismissed();
	}
}
