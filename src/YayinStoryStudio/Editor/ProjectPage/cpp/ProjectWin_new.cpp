#include <algorithm>
#include <functional>
#include <QtGui/qfontmetrics.h>
#include <QtGui/qpainter.h>
#include <QtGui/qevent.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qgraphicseffect.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qscrollbar.h>
#include <QtWidgets/qtextbrowser.h>
#include <General/Log.h>
#include <General/Plugin.h>
#include <General/TranslationHost.h>
#include <General/Version.h>
#include <General/VIApplication.h>
#include <General/YSSLogger.h>
#include <General/YSSProject.h>
#include <Utility/FileOperation.h>
#include <Utility/FileUtility.h>
#include <Utility/JsonConfig.h>
#include <Widgets/ThemeManager.h>
#include "Editor/MainEditor/MainWin.h"
#include "Editor/NewProjectPage/NewProjectWin.h"
#include "Editor/ProjectPage/ProjectInfoWidget_new.h"
#include "Editor/ProjectPage/ProjectWin_new.h"
#include "General/YSSURI.h"
#include "YayinStoryStudio.h"
namespace YSS::ProjectPage {
	namespace {
		// 用于界面展示的项目位置文本：本地路径用原生分隔符，远端路径保留 ssh:// 形式。
		QString displayablePath(const YSSCore::General::YSSURI& uri) {
			const QString localPath = uri.getLocalPath();
			return localPath.isEmpty() ? uri.toString() : localPath;
		}
	}

	/*
		莱茵生命档案终端配色：炭黑/暖纸表面、研究橙主操作、数据青实时状态、告警红异常。
	*/
	struct ArchivePalette {
		bool dark = true;
		QColor base;
		QColor layer1;
		QColor layer2;
		QColor layer3;
		QColor ink;
		QColor border;
		QColor borderStrong;
		QColor text;
		QColor textMuted;
		QColor textFaint;
		QColor accent;
		QColor accentHover;
		QColor data;
		QColor danger;
	};

	ArchivePalette archivePalette() {
		ArchivePalette palette;
		palette.dark = Visindigo::Widgets::ThemeManager::getInstance()->getCurrentColorTheme() != "Light";
		if (palette.dark) {
			palette.base = QColor(17, 21, 25);
			palette.layer1 = QColor(24, 29, 34);
			palette.layer2 = QColor(31, 37, 42);
			palette.layer3 = QColor(40, 47, 52);
			palette.ink = QColor(9, 11, 13);
			palette.border = QColor(166, 181, 188, 30);
			palette.borderStrong = QColor(166, 181, 188, 78);
			palette.text = QColor(231, 230, 223);
			palette.textMuted = QColor(181, 185, 181);
			palette.textFaint = QColor(145, 153, 154);
			palette.accent = QColor(255, 135, 50);
			palette.accentHover = QColor(255, 158, 91);
			palette.data = QColor(91, 194, 216);
			palette.danger = QColor(244, 100, 88);
		}
		else {
			palette.base = QColor(231, 229, 223);
			palette.layer1 = QColor(242, 240, 234);
			palette.layer2 = QColor(220, 218, 211);
			palette.layer3 = QColor(207, 204, 196);
			palette.ink = QColor(9, 11, 13);
			palette.border = QColor(9, 11, 13, 28);
			palette.borderStrong = QColor(9, 11, 13, 74);
			palette.text = QColor(9, 11, 13);
			palette.textMuted = QColor(69, 69, 66);
			palette.textFaint = QColor(105, 103, 97);
			palette.accent = QColor(242, 122, 34);
			palette.accentHover = QColor(207, 91, 18);
			palette.data = QColor(43, 159, 189);
			palette.danger = QColor(190, 54, 48);
		}
		return palette;
	}

	QString archiveRgba(const QColor& color, int alpha) {
		return QString("rgba(%1, %2, %3, %4)").arg(color.red()).arg(color.green()).arg(color.blue()).arg(alpha);
	}

	QFont archiveMonoFont(qreal pointSize, QFont::Weight weight = QFont::Normal) {
		QFont font("Cascadia Mono");
		font.setStyleHint(QFont::Monospace);
		font.setPointSizeF(pointSize);
		font.setWeight(weight);
		font.setLetterSpacing(QFont::AbsoluteSpacing, 0.8);
		return font;
	}

	/*
		档案卡片：封面铺满卡面，项目名固定在左上角，底部是修改时间与路径铭文，选中时左侧亮起研究橙标记。
	*/
	class ProjectCard :public QFrame {
	public:
		static constexpr int CardHeight = 112;
		std::function<void(ProjectCard*)> selectedHandler;
		std::function<void(ProjectCard*)> activatedHandler;

		explicit ProjectCard(QWidget* parent = nullptr) :QFrame(parent) {
			setCursor(Qt::PointingHandCursor);
			setFocusPolicy(Qt::NoFocus);
			setFrameShape(QFrame::NoFrame);
			setFixedHeight(CardHeight);
			setMinimumWidth(180);
			Colors = archivePalette();
		}

		void setProject(YSSCore::General::YSSProject* project, int index) {
			Project = project;
			Index = index;
			if (project != nullptr) {
				Title = project->getProjectName();
				Meta = project->getProjectLastModifyTime().toString("yyyy-MM-dd hh:mm");
				Location = displayablePath(project->getProjectFolder());
				Cover = QPixmap(project->getProjectIconPath().getLocalPath());
				if (!Cover.isNull() && (Cover.width() > 960 || Cover.height() > 960)) {
					Cover = Cover.scaled(960, 960, Qt::KeepAspectRatio, Qt::SmoothTransformation);
				}
				setToolTip(Title + "\n" + Location);
				setAccessibleName(Title);
			}
			rebuildCoverCache();
			update();
		}

		void setArchivePalette(const ArchivePalette& palette) {
			Colors = palette;
			update();
		}

		void setSelected(bool selected) {
			if (Selected == selected) {
				return;
			}
			Selected = selected;
			update();
		}

		bool isSelected() const {
			return Selected;
		}

	protected:
		void paintEvent(QPaintEvent* event) override {
			Q_UNUSED(event);
			QPainter painter(this);
			painter.setRenderHint(QPainter::Antialiasing, true);
			const QRectF frameRect = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
			painter.fillRect(rect(), Colors.layer2);

			const bool hasCover = !CoverCache.isNull();
			if (hasCover) {
				const qreal dpr = devicePixelRatioF();
				const QSizeF sourceSize(width() * dpr, height() * dpr);
				QRectF source((CoverCache.width() - sourceSize.width()) / 2.0,
					(CoverCache.height() - sourceSize.height()) / 2.0, sourceSize.width(), sourceSize.height());
				source = source.intersected(QRectF(QPointF(0, 0),
					QSizeF(CoverCache.width(), CoverCache.height())));
				painter.drawPixmap(QRectF(rect()), CoverCache, source);

				const QColor& ink = Colors.ink;
				QLinearGradient scrim(0, 0, width(), 0);
				scrim.setColorAt(0.0, QColor(ink.red(), ink.green(), ink.blue(), 242));
				scrim.setColorAt(0.44, QColor(ink.red(), ink.green(), ink.blue(), 198));
				scrim.setColorAt(0.8, QColor(ink.red(), ink.green(), ink.blue(), 96));
				scrim.setColorAt(1.0, QColor(ink.red(), ink.green(), ink.blue(), 44));
				painter.fillRect(rect(), scrim);

				QLinearGradient bottomFade(0, height() * 0.42, 0, height());
				bottomFade.setColorAt(0.0, QColor(ink.red(), ink.green(), ink.blue(), 0));
				bottomFade.setColorAt(1.0, QColor(ink.red(), ink.green(), ink.blue(), 196));
				painter.fillRect(rect(), bottomFade);
			}
			else {
				painter.setPen(QPen(Colors.border, 1));
				for (int x = -height(); x < width(); x += 11) {
					painter.drawLine(x, height(), x + height(), 0);
				}
			}

			QColor tint = Colors.accent;
			tint.setAlpha(Selected ? 30 : (Hovered ? 18 : 0));
			if (tint.alpha() > 0) {
				painter.fillRect(rect(), tint);
			}

			const QColor titleColor = hasCover ? QColor(231, 230, 223) : Colors.text;
			const QColor metaColor = hasCover ? QColor(181, 185, 181) : Colors.textMuted;
			const int padding = 18;

			QFont titleFont = font();
			titleFont.setPointSizeF(11.5);
			titleFont.setWeight(QFont::DemiBold);
			titleFont.setLetterSpacing(QFont::AbsoluteSpacing, 0.4);
			painter.setFont(titleFont);
			painter.setPen(titleColor);
			const QRectF titleRect(padding, padding - 3, width() - padding * 2 - 48, 24);
			painter.drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter,
				QFontMetricsF(titleFont).elidedText(Title, Qt::ElideRight, titleRect.width()));

			QFont indexFont = archiveMonoFont(8.0, QFont::DemiBold);
			painter.setFont(indexFont);
			painter.setPen(Selected ? Colors.accent : (hasCover ? QColor(181, 185, 181) : Colors.textFaint));
			painter.drawText(QRectF(width() - padding - 44, padding - 3, 44, 24),
				Qt::AlignRight | Qt::AlignVCenter, "#" + QString::number(Index).rightJustified(2, '0'));

			QFont metaFont = archiveMonoFont(7.8);
			const QFontMetricsF metaMetrics(metaFont);
			const qreal lineHeight = metaMetrics.height();
			painter.setFont(metaFont);
			painter.setPen(metaColor);
			const QRectF metaRect(padding, height() - padding + 2 - lineHeight * 2, width() - padding * 2, lineHeight);
			painter.drawText(metaRect, Qt::AlignLeft | Qt::AlignVCenter,
				metaMetrics.elidedText(Location, Qt::ElideMiddle, metaRect.width()));
			painter.drawText(metaRect.translated(0, lineHeight), Qt::AlignLeft | Qt::AlignVCenter,
				metaMetrics.elidedText(Meta, Qt::ElideRight, metaRect.width()));

			painter.setBrush(Qt::NoBrush);
			QPen borderPen(Selected ? Colors.accent : (Hovered ? Colors.borderStrong : Colors.border));
			borderPen.setWidthF(Selected ? 1.6 : 1.0);
			painter.setPen(borderPen);
			painter.drawRect(frameRect);

			if (Selected) {
				painter.fillRect(QRectF(frameRect.left() + 1, frameRect.top() + 1, 3.0, frameRect.height() - 2),
					Colors.accent);
			}

			painter.setPen(QPen(Selected ? Colors.accent : Colors.borderStrong, 1.0));
			const qreal tick = 9.0;
			const QPointF topRight(frameRect.right(), frameRect.top());
			const QPointF bottomLeft(frameRect.left(), frameRect.bottom());
			painter.drawLine(topRight, QPointF(topRight.x() - tick, topRight.y()));
			painter.drawLine(topRight, QPointF(topRight.x(), topRight.y() + tick));
			painter.drawLine(bottomLeft, QPointF(bottomLeft.x() + tick, bottomLeft.y()));
			painter.drawLine(bottomLeft, QPointF(bottomLeft.x(), bottomLeft.y() - tick));
		}

		void resizeEvent(QResizeEvent* event) override {
			QFrame::resizeEvent(event);
			rebuildCoverCache();
		}

		void mousePressEvent(QMouseEvent* event) override {
			if (event->button() == Qt::LeftButton && selectedHandler) {
				selectedHandler(this);
			}
			QFrame::mousePressEvent(event);
		}

		void mouseDoubleClickEvent(QMouseEvent* event) override {
			if (event->button() == Qt::LeftButton && activatedHandler) {
				activatedHandler(this);
				return;
			}
			QFrame::mouseDoubleClickEvent(event);
		}

		void enterEvent(QEnterEvent* event) override {
			Hovered = true;
			update();
			QFrame::enterEvent(event);
		}

		void leaveEvent(QEvent* event) override {
			Hovered = false;
			update();
			QFrame::leaveEvent(event);
		}

	private:
		void rebuildCoverCache() {
			if (Cover.isNull() || width() <= 0 || height() <= 0) {
				CoverCache = QPixmap();
				return;
			}
			const qreal dpr = devicePixelRatioF();
			// 宽度按桶取整，弧形滚动时卡片宽度逐像素变化也不会反复重采样封面。
			const int bucket = 16;
			const int bucketWidth = ((width() + bucket - 1) / bucket) * bucket;
			const QSize target(qMax(1, qRound(bucketWidth * dpr)), qMax(1, qRound(height() * dpr)));
			CoverCache = Cover.scaled(target, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
		}

		YSSCore::General::YSSProject* Project = nullptr;
		QPixmap Cover;
		QPixmap CoverCache;
		QString Title;
		QString Meta;
		QString Location;
		ArchivePalette Colors;
		int Index = 0;
		bool Selected = false;
		bool Hovered = false;
	};

	/*
		背景叠层：技术制图网格、扫描线与四角定位标记，压在模糊封面之上。
	*/
	class ArchiveBackdrop :public QWidget {
	public:
		explicit ArchiveBackdrop(QWidget* parent = nullptr) :QWidget(parent) {
			setAttribute(Qt::WA_TransparentForMouseEvents);
			setAttribute(Qt::WA_NoSystemBackground);
			Colors = archivePalette();
		}

		void setArchivePalette(const ArchivePalette& palette) {
			Colors = palette;
			update();
		}

	protected:
		void paintEvent(QPaintEvent* event) override {
			Q_UNUSED(event);
			QPainter painter(this);
			QColor top = Colors.base;
			top.setAlpha(Colors.dark ? 176 : 158);
			QColor middle = Colors.base;
			middle.setAlpha(Colors.dark ? 128 : 118);
			QColor bottom = Colors.base;
			bottom.setAlpha(Colors.dark ? 216 : 186);
			QLinearGradient shade(0, 0, 0, height());
			shade.setColorAt(0.0, top);
			shade.setColorAt(0.55, middle);
			shade.setColorAt(1.0, bottom);
			painter.fillRect(rect(), shade);

			painter.setPen(QPen(Colors.dark ? QColor(166, 181, 188, 20) : QColor(9, 11, 13, 16), 1));
			for (int x = 0; x < width(); x += 24) {
				painter.drawLine(x, 0, x, height());
			}
			for (int y = 0; y < height(); y += 24) {
				painter.drawLine(0, y, width(), y);
			}

			painter.setPen(QPen(Colors.dark ? QColor(0, 0, 0, 24) : QColor(9, 11, 13, 12), 1));
			for (int y = 0; y < height(); y += 4) {
				painter.drawLine(0, y, width(), y);
			}

			QRadialGradient vignette(rect().center(), qMax(width(), height()) * 0.72);
			vignette.setColorAt(0.0, QColor(0, 0, 0, 0));
			vignette.setColorAt(1.0, QColor(0, 0, 0, Colors.dark ? 120 : 36));
			painter.fillRect(rect(), vignette);

			QColor mark = Colors.accent;
			mark.setAlpha(Colors.dark ? 170 : 140);
			painter.setPen(QPen(mark, 1.4));
			const int margin = 16;
			const int length = 26;
			const QRect area = rect();
			painter.drawLine(area.left() + margin, area.top() + margin, area.left() + margin + length, area.top() + margin);
			painter.drawLine(area.left() + margin, area.top() + margin, area.left() + margin, area.top() + margin + length);
			painter.drawLine(area.right() - margin, area.top() + margin, area.right() - margin - length, area.top() + margin);
			painter.drawLine(area.right() - margin, area.top() + margin, area.right() - margin, area.top() + margin + length);
			painter.drawLine(area.left() + margin, area.bottom() - margin, area.left() + margin + length, area.bottom() - margin);
			painter.drawLine(area.right() - margin, area.bottom() - margin, area.right() - margin - length, area.bottom() - margin);
		}

	private:
		ArchivePalette Colors;
	};

	bool ProjectWin_new::firstOpen = true;
	ProjectWin_new::ProjectWin_new() :QFrame() {
		NetworkManager = new QNetworkAccessManager(this);
		// access http://yxgeneral.cn/DirectFiles/YSS/meta.json to get news info
		auto request = QNetworkRequest(QUrl("http://yxgeneral.cn/DirectFiles/YSS/meta.json"));
		QNetworkReply* reply = NetworkManager->get(request);
		connect(reply, &QNetworkReply::finished, this, [this, reply]() {
			if (reply->error() == QNetworkReply::NoError) {
				QByteArray responseData = reply->readAll();
				QString jsonStr = QString::fromUtf8(responseData);
				jsonStr = jsonStr.replace("\r", "");
				Visindigo::Utility::FileOperation::Errorable<QString> cacheResult = Visindigo::Utility::FileOperation::readAll(VIApp->getMainPlugin()->getPluginFolder().filePath("meta_cache"));
				if (not cacheResult) {
					// 缓存不存在时视为与远端不一致，走下面的更新流程
					if (cacheResult.error() != Visindigo::Utility::FileOperation::FileNotFound) {
						yErrorF << "Failed to read news meta cache, error: " << Visindigo::Utility::FileOperation::errorCodeName(cacheResult.error());
					}
				}
				else if (jsonStr == cacheResult.value()) {
					Visindigo::Utility::FileOperation::Errorable<QString> contentResult = Visindigo::Utility::FileOperation::readAll(VIApp->getMainPlugin()->getPluginFolder().filePath("meta_content"));
					if (not contentResult) {
						yErrorF << "Failed to read news content cache, error: " << Visindigo::Utility::FileOperation::errorCodeName(contentResult.error());
					}
					else if (not contentResult.value().isEmpty()) {
						NewsWidget->setMarkdown(contentResult.value());
					}
					bool needUpdate = VIApp->getMainPlugin()->getPluginConfig()->getBool("Update.Needed");
					QString latestVersion = VIApp->getMainPlugin()->getPluginConfig()->getString("Update.LatestVersion");
					if (needUpdate) {
						QString currentVersion = VIApp->getMainPlugin()->getPluginVersion().toString();
						QMessageBox::information(this, VITRL("YSS::update.newVersionAvailable"), 
							VITRL("YSS::update.newVersionAvailableDesc").arg(latestVersion).arg(currentVersion));
					}
					reply->deleteLater();
					return;
				}
				Visindigo::Utility::FileOperation::ErrorCode cacheSaveResult = Visindigo::Utility::FileOperation::saveAll(VIApp->getMainPlugin()->getPluginFolder().filePath("meta_cache"), jsonStr);
				if (cacheSaveResult != Visindigo::Utility::FileOperation::Success) {
					yErrorF << "Failed to save news meta cache, error: " << Visindigo::Utility::FileOperation::errorCodeName(cacheSaveResult);
				}
				Visindigo::Utility::JsonConfig jsonConfig;
				if (jsonConfig.parse(jsonStr).error == QJsonParseError::NoError) {
					onNewsMetaGot(jsonConfig);
				}
			}
			else {
				NewsWidget->setMarkdown(VITRL("YSS::update.newsLoadFailed"));
				NewsWidget->setAlignment(Qt::AlignCenter);
			}
			reply->deleteLater();
			});

		this->setWindowIcon(QIcon(":/resource/cn.yxgeneral.yayinstorystudio/icon.png"));
		this->setMinimumSize(1180, 720);
		this->setWindowTitle(VITRL("YSS::project.projectManager"));
		this->setObjectName("ProjectWin_new");
		this->setAttribute(Qt::WA_StyledBackground, true);
		BackgroundLabel = new QLabel(this);
		BackgroundLabel->setObjectName("ProjectBackground");
		BackgroundLabel->setAlignment(Qt::AlignCenter);
		BackgroundLabel->setScaledContents(true);
		BackgroundLabel->setGeometry(rect());
		auto* blur = new QGraphicsBlurEffect(BackgroundLabel);
		blur->setBlurRadius(42);
		BackgroundLabel->setGraphicsEffect(blur);
		BackgroundLabel->lower();
		BackdropOverlay = new ArchiveBackdrop(this);
		BackdropOverlay->setGeometry(rect());
		TitleLabel = new QLabel(this);
		TitleLabel->setObjectName("ProjectTitle");
		TitleLabel->setText("YAYIN STORY STUDIO");
		SubtitleLabel = new QLabel(this);
		SubtitleLabel->setObjectName("ProjectSubtitle");
		SubtitleLabel->setText("RL / ARCHIVE TERMINAL " + QString::fromUtf8("\xC2\xB7") + " PROJECT RECORDS "
			+ QString::fromUtf8("\xC2\xB7") + " " + Visindigo::General::Version::getAPIVersion().toString());
		StatusLabel = new QLabel(this);
		StatusLabel->setObjectName("ProjectStatus");
		StatusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		StatusLabel->setText("ARCHIVE / 00 RECORDS");
		QFont titleFont("Bahnschrift");
		titleFont.setStyleHint(QFont::SansSerif);
		titleFont.setPointSizeF(19.0);
		titleFont.setWeight(QFont::DemiBold);
		titleFont.setLetterSpacing(QFont::AbsoluteSpacing, 2.6);
		TitleLabel->setFont(titleFont);
		SubtitleLabel->setFont(archiveMonoFont(8.0));
		StatusLabel->setFont(archiveMonoFont(8.0, QFont::DemiBold));

		HistoryProjectArea = new QScrollArea(this);
		HistoryProjectArea->setObjectName("ProjectRail");
		HistoryProjectArea->setWidgetResizable(true);
		HistoryProjectArea->setFrameShape(QFrame::NoFrame);
		HistoryProjectArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		HistoryProjectArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		HistoryProjectArea->viewport()->setAutoFillBackground(false);
		HistoryProjectWidget = new QWidget();
		HistoryProjectWidget->setObjectName("HistoryProjectWidget");
		HistoryProjectLayout = new QVBoxLayout(HistoryProjectWidget);
		HistoryProjectLayout->setContentsMargins(14, 16, 14, 16);
		HistoryProjectLayout->setSpacing(10);
		EmptyRailLabel = new QLabel(HistoryProjectWidget);
		EmptyRailLabel->setObjectName("EmptyRailHint");
		EmptyRailLabel->setAlignment(Qt::AlignCenter);
		EmptyRailLabel->setWordWrap(true);
		EmptyRailLabel->setFont(archiveMonoFont(8.0));
		EmptyRailLabel->setText("NO ARCHIVE RECORDS\n\nCREATE A NEW PROJECT WITH +");
		EmptyRailLabel->hide();
		HistoryProjectLayout->addWidget(EmptyRailLabel, 1);
		HistoryProjectWidget->setLayout(HistoryProjectLayout);
		HistoryProjectArea->setWidget(HistoryProjectWidget);
		connect(HistoryProjectArea->verticalScrollBar(), &QScrollBar::valueChanged, this,
			[this](int) { updateProjectCardLayout(); });
		RecordHeaderLabel = new QLabel(this);
		RecordHeaderLabel->setObjectName("RecordHeader");
		RecordHeaderLabel->setText("SPECIMEN / EXECUTION RECORD");
		RecordHeaderLabel->setFont(archiveMonoFont(8.0, QFont::DemiBold));

		NewsWidget = new QTextBrowser(this);
		NewsWidget->hide();

		OptionWidget = new QWidget(this);
		OptionWidget->setObjectName("ProjectActions");
		CreateProjectButton = new QPushButton(OptionWidget);
		CreateProjectButton->setObjectName("CreateProjectButton");
		CreateProjectButton->setText("+");
		CreateProjectButton->setToolTip(VITRL("YSS::project.createNewProject"));
		CreateProjectButton->setAccessibleName(VITRL("YSS::project.createNewProject"));
		CreateProjectButton->setCursor(Qt::PointingHandCursor);
		CreateProjectButton->setFixedSize(58, 58);
		OpenFolderButton = new QPushButton(OptionWidget);
		OpenFolderButton->setObjectName("OpenFolderButton");
		OpenFolderButton->setText(QString::fromUtf8("\xE2\x86\x97"));
		OpenFolderButton->setToolTip(VITRL("YSS::project.openFolder"));
		OpenFolderButton->setAccessibleName(VITRL("YSS::project.openFolder"));
		OpenFolderButton->setCursor(Qt::PointingHandCursor);
		OpenFolderButton->setFixedSize(58, 58);
		ButtonLayout = new QHBoxLayout(OptionWidget);
		OptionWidget->setLayout(ButtonLayout);
		ButtonLayout->setContentsMargins(0, 0, 0, 0);
		ButtonLayout->setSpacing(16);
		ButtonLayout->addStretch(1);
		ButtonLayout->addWidget(CreateProjectButton);
		ButtonLayout->addWidget(OpenFolderButton);
		ButtonLayout->addStretch(1);
		InfoWidget = new ProjectInfoWidget_new(this);
		InfoWidget->setMinimumWidth(430);
		InfoWidget->setMinimumHeight(240);
		Layout = new QGridLayout(this);

		this->setLayout(Layout);
		Layout->addWidget(TitleLabel, 0, 0, 1, 1);
		Layout->addWidget(SubtitleLabel, 1, 0, 1, 1);
		Layout->addWidget(StatusLabel, 0, 1, 2, 1);
		Layout->addWidget(HistoryProjectArea, 2, 0, 2, 1);
		Layout->addWidget(RecordHeaderLabel, 2, 1, 1, 1);
		Layout->addWidget(InfoWidget, 3, 1, 1, 1);
		Layout->addWidget(OptionWidget, 4, 0, 1, 2);
		Layout->setColumnStretch(0, 5);
		Layout->setColumnStretch(1, 6);
		Layout->setRowStretch(3, 1);
		Layout->setHorizontalSpacing(24);
		Layout->setVerticalSpacing(10);
		Layout->setContentsMargins(34, 24, 34, 26);
		loadProject();

		int width = VIApp->getMainPlugin()->getPluginConfig()->getInt("Window.Project.Width");
		int height = VIApp->getMainPlugin()->getPluginConfig()->getInt("Window.Project.Height");

		this->resize(width, height);
		if (VIApp->getMainPlugin()->getPluginConfig()->getBool("Window.Project.Maximized")) {
			this->showMaximized();
		}
		connect(CreateProjectButton, &QPushButton::clicked, this, &ProjectWin_new::onCreateProject);
		connect(InfoWidget, &ProjectInfoWidget_new::removeConfirmed, this, &ProjectWin_new::onProjectRemoved);
		connect(OpenFolderButton, &QPushButton::clicked, this, &ProjectWin_new::onOpenProjectClicked);
		setColorfulEnable(true);
		onThemeChanged();
		BackgroundLabel->setGeometry(rect());
		BackdropOverlay->setGeometry(rect());
		updateBackdropPixmap();
		if (firstOpen) {
			firstOpen = false;
			QStringList launchArgs = qApp->arguments();
			if (launchArgs.size() > 1) {
				QString ysspFilePath = launchArgs[1];
				if (ysspFilePath.endsWith(".yssp") || ysspFilePath.endsWith("yssproj.json")) {
					onOpenProject(ysspFilePath);
				}
				else {
					yMessageF << "Invalid project file path:" << ysspFilePath;
				}
			}
		}
	}

	void ProjectWin_new::closeEvent(QCloseEvent* event) {
		Visindigo::Utility::JsonConfig* config = VIApp->getMainPlugin()->getPluginConfig();
		config->setInt("Window.Project.Width", this->normalGeometry().width());
		config->setInt("Window.Project.Height", this->normalGeometry().height());
		config->setBool("Window.Project.Maximized", this->isMaximized());
		VIApp->getMainPlugin()->savePluginConfig();
		// 还在加载中的批次作废：它们的回调虽然会继续执行，但不会再碰本窗口的列表。
		++HistoryLoadGeneration;
		HistoryLoadResults.clear();
		for (YSSCore::General::YSSProject* project : HistoryProjectList) {
			delete project;
		}
		// 项目对象已经删掉了，容器不能再留着悬空指针，免得后续任何一次访问踩到已释放的对象。
		HistoryProjectList.clear();
		HistoryProjectMap.clear();
		HistoryProjectCardList.clear();
		SelectedProject = nullptr;
		BackgroundSource = QPixmap();
		InfoWidget->showProject(nullptr);
		if (YSSCore::General::YSSProject::getCurrentProject()) {
			if (YSS::Editor::MainWin::getInstance() == nullptr) {
				new YSS::Editor::MainWin();
			}
			YSS::Editor::MainWin::getInstance()->show();
		}
		this->deleteLater();
	}

	void ProjectWin_new::resizeEvent(QResizeEvent* event) {
		QFrame::resizeEvent(event);
		if (BackgroundLabel != nullptr) {
			BackgroundLabel->setGeometry(rect());
			updateBackdropPixmap();
		}
		if (BackdropOverlay != nullptr) {
			BackdropOverlay->setGeometry(rect());
		}
		updateProjectCardLayout();
	}

	void ProjectWin_new::onProjectRemoved(YSSCore::General::YSSProject* project) {
		ProjectCard* card = nullptr;
		for (auto it = HistoryProjectMap.constBegin(); it != HistoryProjectMap.constEnd(); ++it) {
			if (it.value() == project) {
				card = it.key();
				break;
			}
		}
		if (card == nullptr || !HistoryProjectList.contains(project)) {
			// 面板持有的项目已经不在历史列表里（列表已重建或已被移除）。
			InfoWidget->showProject(nullptr);
			return;
		}
		HistoryProjectMap.remove(card);
		HistoryProjectCardList.removeAll(card);
		HistoryProjectList.removeAll(project);
		HistoryProjectLayout->removeWidget(card);
		card->hide();
		card->deleteLater();

		if (SelectedProject == project) {
			SelectedProject = nullptr;
			BackgroundSource = QPixmap();
			BackgroundLabel->clear();
			InfoWidget->showProject(nullptr);
		}
		Visindigo::Utility::JsonConfig* Config = VIApp->getMainPlugin()->getPluginConfig();
		QStringList keys = Config->keys("RecentProjects");
		for (int i = 0; i < keys.size(); i++) {
			if (Config->getString("RecentProjects." + keys[i]) == project->getProjectPath().toString()) {
				yMessageF << i;
				Config->remove("RecentProjects." + keys[i]);
				break;
			}
		}
		VIApp->getMainPlugin()->savePluginConfig();

		updateRailSummary();
		if (SelectedProject == nullptr && !HistoryProjectCardList.isEmpty()) {
			selectProjectCard(HistoryProjectCardList.first());
		}
		delete project;
		project = nullptr;
	}

	void ProjectWin_new::selectProjectCard(ProjectCard* card) {
		if (card == nullptr) {
			return;
		}
		YSSCore::General::YSSProject* project = HistoryProjectMap.value(card, nullptr);
		if (project == nullptr) {
			return;
		}
		SelectedProject = project;
		for (ProjectCard* item : HistoryProjectCardList) {
			item->setSelected(item == card);
		}
		InfoWidget->showProject(project);
		BackgroundSource = QPixmap(project->getProjectIconPath().getLocalPath());
		updateBackdropPixmap();
		updateProjectCardLayout();
		HistoryProjectArea->ensureWidgetVisible(card, 0, 24);
	}

	void ProjectWin_new::openProjectCard(ProjectCard* card) {
		YSSCore::General::YSSProject* project = HistoryProjectMap.value(card, nullptr);
		if (project == nullptr) {
			return;
		}
		if (not preCheckProject(project)) {
			return;
		}
		// 所有权移交给“当前项目”，必须先摘出历史列表，否则closeEvent会把它一起删掉。
		HistoryProjectList.removeAll(project);
		HistoryProjectMap.remove(card);
		HistoryProjectCardList.removeAll(card);
		HistoryProjectLayout->removeWidget(card);
		card->hide();
		card->deleteLater();
		YSSCore::General::YSSProject::setCurrentProject(project);
		this->close();
	}

	void ProjectWin_new::onOpenProjectClicked() {
		onOpenProject();
	}
	void ProjectWin_new::onOpenProject(QString filePath) {
		if (filePath.isEmpty()) {
			filePath = QFileDialog::getOpenFileName(this, VITRL("YSS::project.openYSSProject"), "./user_data/repos", "YSS Project (*.yssp);;YSS Project (yssproj.json)");
			if (filePath.isEmpty()) {
				return;
			}
		}
		YSSCore::General::YSSProject* project = new YSSCore::General::YSSProject();
		// 加载是异步的，结论只能从信号里拿；这里只连接一次，之后本窗口不再关心这个对象。
		connect(project, &YSSCore::General::YSSProject::projectLoaded, this, [this, project]() {
			Visindigo::Utility::JsonConfig* Config = VIApp->getMainPlugin()->getPluginConfig();
			const QString projectPath = project->getProjectPath().toString();
			bool inList = false;
			for (QString key : Config->keys("RecentProjects")) {
				QString recentPath = Config->getString("RecentProjects." + key);
				if (recentPath == projectPath) { inList = true; break; }
			}
			if (not inList) {
				yMessageF << "Add" << projectPath << "to project list.";
				Config->setString("RecentProjects." + QString::number(Config->keys("RecentProjects").size()), projectPath);
			}
			if (not preCheckProject(project)) {
				project->deleteLater();
				return;
			}
			YSSCore::General::YSSProject::setCurrentProject(project);
			this->close();
			});
		connect(project, &YSSCore::General::YSSProject::projectLoadFailed, this,
			[this, project](YSSCore::General::YSSProject::LoadError error) {
				QMessageBox msgBox;
				msgBox.setIcon(QMessageBox::Critical);
				msgBox.setText(VITRL("YSS::project.openProjectFailed.message"));
				switch (error) {
				case YSSCore::General::YSSProject::LoadError::NotFound:
					msgBox.setInformativeText(VITRL("YSS::project.openProjectFailed.NotFound"));
					break;
				case YSSCore::General::YSSProject::LoadError::ReadFailed:
					msgBox.setInformativeText(VITRL("YSS::project.openProjectFailed.ReadFailed"));
					break;
				case YSSCore::General::YSSProject::LoadError::ParseError:
					msgBox.setInformativeText(VITRL("YSS::project.openProjectFailed.ParseError"));
					break;
				case YSSCore::General::YSSProject::LoadError::InvalidConfig:
					msgBox.setInformativeText(VITRL("YSS::project.openProjectFailed.InvalidConfig"));
					break;
				default:
					msgBox.setInformativeText(VITRL("YSS::project.openProjectFailed.UnknownError"));
					break;
				}
				msgBox.setStandardButtons(QMessageBox::Ok);
				msgBox.exec();
				project->deleteLater();
			});
		project->loadProject(YSSCore::General::YSSURI::fromString(filePath));
	}

	/*
		检查 \a project 是否满足打开条件：所需插件是否齐全、项目版本是否兼容。

		本函数只检查和询问，不接管 \a project 的所有权。
		return 用户选择继续打开返回true，选择取消返回false。
	*/
	bool ProjectWin_new::preCheckProject(YSSCore::General::YSSProject* project) {
		auto requiredPlugins = project->getRequiredPlugins();
		bool allPluginsAvailable = true;
		QStringList missingPlugins;
		for (auto& pluginID : requiredPlugins) {
			if (not VIPLM->isPluginEnable(pluginID)) {
				allPluginsAvailable = false;
				missingPlugins << pluginID;
			}
		}
		if (not allPluginsAvailable) {
			QMessageBox msgBox;
			msgBox.setIcon(QMessageBox::Warning);
			msgBox.setWindowTitle(VITRL("YSS::project.requiredPluginsNotAvailable.title"));
			msgBox.setText(VITRL("YSS::project.requiredPluginsNotAvailable.message").arg(project->getProjectName()) +
				"\n" + missingPlugins.join("\n"));
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::No);
			auto rtn = msgBox.exec();
			if (rtn == QMessageBox::No) {
				return false;
			}
		}
		auto projectVersion = project->getProjectVersion();
		yDebug << "Project version:" << projectVersion.toString() << ", Current version:" << YSSCore::General::YSSProject::getCurrentProjectVersion().toString();
		if (projectVersion != YSSCore::General::YSSProject::getCurrentProjectVersion()) {
			if (projectVersion < YSSCore::General::YSSProject::getCurrentProjectVersion()) {
				QMessageBox msgBox;
				msgBox.setIcon(QMessageBox::Warning);
				msgBox.setWindowTitle(VITRL("YSS::project.projectVersionTooOld.title"));
				auto versionRange = YSSCore::General::YSSProject::getProjectVersionRange();
				auto leftVersion = std::get<0>(versionRange.value(projectVersion));
				auto rightVersion = std::get<1>(versionRange.value(projectVersion));
				msgBox.setText(VITRL("YSS::project.projectVersionTooOld.message").arg(project->getProjectName()).arg(projectVersion.toString()).arg(leftVersion.toString()).arg(rightVersion.toString()));
				msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
				msgBox.setDefaultButton(QMessageBox::No);
				auto rtn = msgBox.exec();
				if (rtn == QMessageBox::No) {
					return false;
				}
				project->updateProject();
			}
			else {
				QMessageBox msgBox;
				msgBox.setIcon(QMessageBox::Warning);
				msgBox.setWindowTitle(VITRL("YSS::project.projectVersionTooNew.title"));
				msgBox.setText(VITRL("YSS::project.projectVersionTooNew.message").arg(project->getProjectName()));
				msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
				auto rtn = msgBox.exec();
				if (rtn == QMessageBox::No) {
					return false;
				}
			}
		}
		return true;
	}
	void ProjectWin_new::onCreateProject() {
		NewProjectPage::NewProjectWin* win = new NewProjectPage::NewProjectWin();
		win->setAttribute(Qt::WA_DeleteOnClose);
		win->setWindowModality(Qt::ApplicationModal);
		win->setWindowFlags(win->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
		win->show();
		connect(win, &NewProjectPage::NewProjectWin::projectPrepared, this, &ProjectWin_new::onOpenProject);
	}

	void ProjectWin_new::loadProject() {
		for (YSSCore::General::YSSProject* project : HistoryProjectList) {
			delete project;
			project = nullptr;
		}
		HistoryProjectList.clear();
		for (ProjectCard* card : HistoryProjectCardList) {
			HistoryProjectLayout->removeWidget(card);
			card->hide();
			card->deleteLater();
		}
		HistoryProjectCardList.clear();
		HistoryProjectMap.clear();
		SelectedProject = nullptr;

		Visindigo::Utility::JsonConfig* Config = VIApp->getMainPlugin()->getPluginConfig();
		QStringList recentPaths;
		for (QString key : Config->keys("RecentProjects")) {
			recentPaths.append(Config->getString("RecentProjects." + key));
		}
		// 加载是异步的，一批加载全部结束之后再重建列表。用代数丢弃过期批次的结果，
		// 避免两次 loadProject 的完成顺序交错时把旧结果写进新列表。
		++HistoryLoadGeneration;
		const int generation = HistoryLoadGeneration;
		HistoryLoadResults.clear();
		HistoryLoadPending = recentPaths.size();
		if (HistoryLoadPending == 0) {
			finishHistoryLoad(generation);
			return;
		}
		for (const QString& projectPath : recentPaths) {
			YSSCore::General::YSSProject* project = new YSSCore::General::YSSProject();
			connect(project, &YSSCore::General::YSSProject::projectLoaded, this, [this, project, generation]() {
				if (generation != HistoryLoadGeneration) {
					project->deleteLater();
					return;
				}
				HistoryLoadResults.append(project);
				if (--HistoryLoadPending == 0) {
					finishHistoryLoad(generation);
				}
				});
			connect(project, &YSSCore::General::YSSProject::projectLoadFailed, this, [this, project, generation]() {
				project->deleteLater();
				if (generation != HistoryLoadGeneration) {
					return;
				}
				if (--HistoryLoadPending == 0) {
					finishHistoryLoad(generation);
				}
				});
			project->loadProject(YSSCore::General::YSSURI::fromString(projectPath));
		}
	}

	/*
		一批历史项目加载完成后重建列表，并把仍然可用的项目路径写回配置。
	*/
	void ProjectWin_new::finishHistoryLoad(int generation) {
		if (generation != HistoryLoadGeneration) {
			return;
		}
		HistoryProjectList = HistoryLoadResults;
		HistoryLoadResults.clear();
		QStringList okProjects;
		for (YSSCore::General::YSSProject* project : HistoryProjectList) {
			okProjects.append(project->getProjectPath().toString());
		}
		Visindigo::Utility::JsonConfig* Config = VIApp->getMainPlugin()->getPluginConfig();
		Config->setArray("RecentProjects", okProjects);
		std::sort(HistoryProjectList.begin(), HistoryProjectList.end(), [](YSSCore::General::YSSProject* a, YSSCore::General::YSSProject* b) {
			return a->getProjectLastModifyTime() > b->getProjectLastModifyTime();
			});
		for (int i = 0; i < HistoryProjectList.size(); i++) {
			YSSCore::General::YSSProject* project = HistoryProjectList[i];
			ProjectCard* card = new ProjectCard(HistoryProjectWidget);
			card->setProject(project, i + 1);
			card->selectedHandler = [this](ProjectCard* who) { selectProjectCard(who); };
			card->activatedHandler = [this](ProjectCard* who) { openProjectCard(who); };
			HistoryProjectCardList.append(card);
			HistoryProjectMap.insert(card, project);
			HistoryProjectLayout->addWidget(card, 0, Qt::AlignHCenter);
			card->show();
		}
		const int count = HistoryProjectCardList.size();
		updateRailSummary();
		if (count > 0 && !HistoryProjectCardList.isEmpty()) {
			selectProjectCard(HistoryProjectCardList.first());
		}
	}

	void ProjectWin_new::updateRailSummary() {
		const int count = HistoryProjectCardList.size();
		if (EmptyRailLabel != nullptr) {
			EmptyRailLabel->setVisible(count == 0);
		}
		if (HistoryProjectWidget != nullptr && HistoryProjectLayout != nullptr) {
			if (count == 0) {
				// 空档案时让容器跟随视口高度，占位铭文才能落在轨道中央。
				HistoryProjectWidget->setMinimumHeight(0);
				HistoryProjectWidget->setMaximumHeight(QWIDGETSIZE_MAX);
			}
			else {
				const int spacing = HistoryProjectLayout->spacing();
				const int verticalMargins = HistoryProjectLayout->contentsMargins().top()
					+ HistoryProjectLayout->contentsMargins().bottom();
				HistoryProjectWidget->setFixedHeight(count * ProjectCard::CardHeight
					+ (count - 1) * spacing + verticalMargins + 8);
			}
		}
		if (StatusLabel != nullptr) {
			StatusLabel->setText("ARCHIVE / " + QString::number(count).rightJustified(2, '0') + " RECORDS");
		}
		updateProjectCardLayout();
	}

	void ProjectWin_new::updateProjectCardLayout() {
		if (LayoutArcGuard || HistoryProjectCardList.isEmpty() || HistoryProjectWidget == nullptr
			|| HistoryProjectArea == nullptr) {
			return;
		}
		LayoutArcGuard = true;
		HistoryProjectLayout->activate();
		const QMargins margins = HistoryProjectLayout->contentsMargins();
		const int railWidth = HistoryProjectWidget->width() - margins.left() - margins.right();
		const int viewportHeight = HistoryProjectArea->viewport()->height();
		const int scrollOffset = HistoryProjectArea->verticalScrollBar()->value();
		const qreal center = viewportHeight / 2.0;
		for (ProjectCard* card : HistoryProjectCardList) {
			qreal distance = 0.0;
			if (center > 1.0) {
				distance = (card->y() + card->height() / 2.0 - scrollOffset - center) / center;
			}
			distance = qBound(-1.0, distance, 1.0);
			// 越靠近滚动区中心越宽，上下两端内收，形成抽屉式的弧线。
			int inset = qRound(44.0 * distance * distance);
			if (card->isSelected()) {
				inset = qMax(0, inset - 8);
			}
			const int cardWidth = qMax(200, railWidth - inset * 2);
			if (card->width() != cardWidth) {
				card->setFixedWidth(cardWidth);
			}
		}
		LayoutArcGuard = false;
	}

	void ProjectWin_new::updateBackdropPixmap() {
		if (BackgroundLabel == nullptr) {
			return;
		}
		BackgroundLabel->setGeometry(rect());
		if (BackgroundSource.isNull()) {
			BackgroundLabel->clear();
			return;
		}
		// 先降采样再交给模糊效果，避免每帧对整张封面做大半径模糊。
		const QSize target(qMax(48, width() / 10), qMax(48, height() / 10));
		QPixmap scaled = BackgroundSource.scaled(target, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
		const QRect crop((scaled.width() - target.width()) / 2, (scaled.height() - target.height()) / 2,
			target.width(), target.height());
		BackgroundLabel->setPixmap(scaled.copy(crop));
	}

	void ProjectWin_new::applyArchiveTheme() {
		const ArchivePalette palette = archivePalette();
		for (ProjectCard* card : HistoryProjectCardList) {
			card->setArchivePalette(palette);
		}
		if (BackdropOverlay != nullptr) {
			static_cast<ArchiveBackdrop*>(BackdropOverlay)->setArchivePalette(palette);
		}
		if (BackgroundLabel != nullptr) {
			BackgroundLabel->setStyleSheet("background: transparent;");
		}

		QString sheet = QStringLiteral(
			"#ProjectWin_new { background: @base; }"
			"QLabel#ProjectTitle { color: @text; background: transparent; }"
			"QLabel#ProjectSubtitle { color: @faint; background: transparent; }"
			"QLabel#ProjectStatus { color: @data; background: transparent; }"
			"QLabel#RecordHeader { color: @accent; background: transparent; border-bottom: 1px solid @borderStrong; padding: 2px 0 8px 2px; }"
			"QScrollArea#ProjectRail { background: @panel; border: 1px solid @border; }"
			"QScrollArea#ProjectRail > QWidget > QWidget { background: transparent; }"
			"QWidget#HistoryProjectWidget { background: transparent; }"
			"QLabel#EmptyRailHint { color: @faint; background: transparent; }"
			"QWidget#ProjectActions { background: transparent; }"
			"QScrollBar:vertical { background: transparent; width: 6px; margin: 8px 3px; }"
			"QScrollBar::handle:vertical { background: @borderStrong; min-height: 36px; }"
			"QScrollBar::handle:vertical:hover { background: @accent; }"
			"QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
			"QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
			"QPushButton#CreateProjectButton { background: @accent; color: @base; border: 1px solid @accent; border-radius: 29px; font-size: 24px; font-weight: 600; padding-bottom: 2px; }"
			"QPushButton#CreateProjectButton:hover { background: @accentHover; border-color: @accentHover; }"
			"QPushButton#CreateProjectButton:pressed { background: @accentHover; }"
			"QPushButton#OpenFolderButton { background: transparent; color: @text; border: 1px solid @borderStrong; border-radius: 29px; font-size: 20px; }"
			"QPushButton#OpenFolderButton:hover { background: @accentSoft; border-color: @accent; color: @accent; }"
			"QPushButton#OpenFolderButton:pressed { background: @accentSoft; }"
			"#ProjectInfoWidget_new { background: @panel; border: 1px solid @border; }"
			"#ProjectInfoWidget_new QLabel { color: @muted; }"
			"#ProjectInfoWidget_new QLabel#ProjectTitleLabel { color: @text; }"
			"#ProjectInfoWidget_new QPushButton { background: transparent; color: @text; border: 1px solid @borderStrong; padding: 8px 16px; }"
			"#ProjectInfoWidget_new QPushButton:hover { border-color: @accent; color: @accent; }"
			"#ProjectInfoWidget_new QPushButton#DeleteButton:hover { border-color: @danger; color: @danger; }"
		);
		sheet.replace("@accentHover", palette.accentHover.name());
		sheet.replace("@accentSoft", archiveRgba(palette.accent, palette.dark ? 34 : 26));
		sheet.replace("@borderStrong", archiveRgba(palette.dark ? QColor(166, 181, 188) : QColor(9, 11, 13), palette.dark ? 82 : 66));
		sheet.replace("@base", palette.base.name());
		sheet.replace("@layer1", palette.layer1.name());
		sheet.replace("@panel", archiveRgba(palette.layer1, palette.dark ? 236 : 238));
		sheet.replace("@layer2", palette.layer2.name());
		sheet.replace("@text", palette.text.name());
		sheet.replace("@muted", palette.textMuted.name());
		sheet.replace("@faint", palette.textFaint.name());
		sheet.replace("@data", palette.data.name());
		sheet.replace("@accent", palette.accent.name());
		sheet.replace("@border", archiveRgba(palette.dark ? QColor(166, 181, 188) : QColor(9, 11, 13), palette.dark ? 34 : 26));
		sheet.replace("@danger", palette.danger.name());
		this->setStyleSheet(sheet);
		updateProjectCardLayout();
	}

	void ProjectWin_new::onThemeChanged() {
		applyArchiveTheme();
	}

	void ProjectWin_new::onNewsMetaGot(const Visindigo::Utility::JsonConfig& config) {
		QString versionKey = "version";
		QString updateNewsKey = "update_news";
		QString newsKey = "news";
		QString updateChannel = VIApp->getMainPlugin()->getPluginConfig()->getString("Settings.General.UpdateChannel");
		if (updateChannel == "beta") {
			versionKey = "version_beta";
			updateNewsKey = "update_news_beta";
			newsKey = "news_beta";
		}
		else if (updateChannel == "tp") {
			versionKey = "version_tp";
			updateNewsKey = "update_news_tp";
			newsKey = "news_tp";
		}
		else {
			VIApp->getMainPlugin()->getPluginConfig()->setString("Settings.General.UpdateChannel", "release");
		}
		if (config.contains(versionKey)) {
			QString version = config.getString(versionKey);
			Visindigo::General::Version metaVersion(version);
			auto req = QNetworkRequest();
			bool isUpdate = metaVersion > VIApp->getMainPlugin()->getPluginVersion();
			if (isUpdate) {
				req = QNetworkRequest(QUrl(config.getString(updateNewsKey)));
			}
			else {
				req = QNetworkRequest(QUrl(config.getString(newsKey)));
			}
			QNetworkReply* reply = NetworkManager->get(req);
			connect(reply, &QNetworkReply::finished, this, [this, reply, isUpdate, metaVersion]() {
				if (reply->error() == QNetworkReply::NoError) {
					QByteArray responseData = reply->readAll();
					QString newsStr = QString::fromUtf8(responseData);
					NewsWidget->setMarkdown(newsStr);
					Visindigo::Utility::FileOperation::ErrorCode contentSaveResult = Visindigo::Utility::FileOperation::saveAll(VIApp->getMainPlugin()->getPluginFolder().filePath("meta_content"), newsStr);
					if (contentSaveResult != Visindigo::Utility::FileOperation::Success) {
						yErrorF << "Failed to save news content cache, error: " << Visindigo::Utility::FileOperation::errorCodeName(contentSaveResult);
					}
					VIApp->getMainPlugin()->getPluginConfig()->setBool("Update.Needed", isUpdate);
					VIApp->getMainPlugin()->getPluginConfig()->setString("Update.LatestVersion", metaVersion.toString());
					if (isUpdate) {
						auto version = YSS::Main::getInstance()->getPluginVersion().toString();
						QMessageBox::information(this, VITRL("YSS::update.newVersionAvailable"), 
							VITRL("YSS::update.newVersionAvailableDesc").arg(metaVersion.toString()).arg(version));
					}
				}
				reply->deleteLater();
				});
		}
	}
}