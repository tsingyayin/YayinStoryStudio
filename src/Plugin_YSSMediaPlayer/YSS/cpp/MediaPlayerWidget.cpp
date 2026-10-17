#include <QtCore/qdatetime.h>
#include <QtCore/qdir.h>
#include <QtCore/qeasingcurve.h>
#include <QtCore/qelapsedtimer.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qpropertyanimation.h>
#include <QtCore/qrect.h>
#include <QtCore/qset.h>
#include <QtCore/qsize.h>
#include <QtCore/qstandardpaths.h>
#include <QtCore/qstring.h>
#include <QtCore/qtimer.h>
#include <QtCore/qurl.h>
#include <QtCore/quuid.h>
#include <mutex>
#include <QtGui/qcursor.h>
#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>
#include <QtMultimedia/qaudiooutput.h>
#include <QtMultimedia/qmediametadata.h>
#include <QtMultimedia/qmediaplayer.h>
#include <QtMultimedia/qvideoframe.h>
#include <QtMultimedia/qvideosink.h>
#include <QtMultimediaWidgets/qgraphicsvideoitem.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qframe.h>
#include <QtWidgets/qgraphicseffect.h>
#include <QtWidgets/qgraphicsscene.h>
#include <QtWidgets/qgraphicsview.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qslider.h>
#include <QtWidgets/qstackedwidget.h>
#include <QtWidgets/qtoolbutton.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <General/VIApplication.h>
#include <Widgets/ThemeManager.h>
#include <Widgets/LiquidGlassEffect.h>
#include "YSS/MediaFormats.h"
#include "YSS/MediaPlayerWidget.h"

namespace YSS::MediaPlayer {
	class MediaPlayerWidgetPrivate {
		friend class MediaPlayerWidget;
	protected:
		QMediaPlayer* Player = nullptr;
		QAudioOutput* AudioOutput = nullptr;
		QGraphicsView* VideoView = nullptr;
		QGraphicsScene* VideoScene = nullptr;
		QGraphicsVideoItem* VideoItem = nullptr;
		QStackedWidget* DisplayArea = nullptr;
		QLabel* AudioPlaceholder = nullptr;
		QFrame* ControlBar = nullptr;
		QWidget* ButtonsHost = nullptr;
		QWidget* VolumeBlock = nullptr;
		QToolButton* Rewind5Button = nullptr;
		QToolButton* Rewind1Button = nullptr;
		QToolButton* PlayPauseButton = nullptr;
		QToolButton* Forward1Button = nullptr;
		QToolButton* Forward5Button = nullptr;
		QSlider* ProgressSlider = nullptr;
		QLabel* TimeLabel = nullptr;
		QLabel* VolumeIconLabel = nullptr;
		QSlider* VolumeSlider = nullptr;
		QTimer* AutoHideTimer = nullptr;
		Visindigo::Widgets::LiquidGlassEffect* ControlBarOpacity = nullptr;
		QPropertyAnimation* ControlBarFade = nullptr;
		// 方向键步进：ScrubTimer 周期性地把 ScrubTarget 推给播放器，
		// ScrubClock 记录长按时长以决定每次的步进量。
		QTimer* ScrubTimer = nullptr;
		QElapsedTimer ScrubClock;
		QSet<int> ScrubKeys;
		qint64 ScrubTarget = 0;
		qint64 FrameDurationMs = 33;
		// 最近一帧有效画面。播放结束时 Qt 会清空视频输出，需要靠它把最后一帧推回去。
		QVideoFrame LastVideoFrame;
		// 打开后是否为视频预取首帧（只对视频做，音频没有画面可显示）。
		bool PrimeFirstFrame = false;
		// 首帧预取进行中（此时处于短暂的 Playing 状态，需抑制按钮闪动）。
		bool PrimingActive = false;
		bool ProgressSliderDragging = false;
		// 控制栏的“目标可见状态”。渐出动画结束后才会真正 hide()，
		// 期间 isVisible() 仍为 true，因此不能只看 isVisible() 判断当前状态。
		bool ControlBarShown = false;
		// 远端媒体为播放而落地的临时副本路径；本地文件播放时为空。
		QString TempMediaFile;
	};

	namespace {
		// getNamedFontIcon 内部固定使用 64x64 画布，传 64 可让字形正好填满画布，
		// 之后由控件的 iconSize 等比缩放。
		constexpr int IconRenderSize = 64;
		constexpr int ButtonIconSize = 24;
		constexpr int ButtonSize = 36;
		constexpr int VolumeIconSize = 20;
		constexpr int VolumeSliderWidth = 112;
		constexpr int VolumeBlockSpacing = 6;
		constexpr int VolumeBlockWidth = VolumeIconSize + VolumeBlockSpacing + VolumeSliderWidth;
		constexpr int ControlBarMargin = 8;
		constexpr int ControlBarSideMargin = 10;
		constexpr int VolumeBlockGap = 4;
		constexpr int AutoHideDelayMs = 5000;
		// 方向键长按：每 ScrubRepeatMs 步进一次，步进量在 ScrubRampMs 内
		// 由一帧线性增长到 ScrubMaxStepMs。
		constexpr int ScrubRepeatMs = 60;
		constexpr int ScrubRampMs = 1000;
		constexpr qint64 ScrubMaxStepMs = 5000;
		constexpr qint64 DefaultFrameDurationMs = 33;
		// 临时副本的年龄超过它，就认为是被上次异常退出遗弃的。
		constexpr qint64 TempCacheMaxAgeMs = 24LL * 60 * 60 * 1000;
		const QColor IconColor(0xF0, 0xF0, 0xF0);

		QString mediaCacheDirPath() {
			QString baseDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
			if (baseDir.isEmpty()) {
				baseDir = QDir::tempPath();
			}
			return baseDir + QStringLiteral("/YSSMediaCache");
		}

		// 每个进程只清一次：删掉上次运行遗留的过期临时副本。
		// 本进程正在播放的文件是刚写出来的，不会命中这个年龄阈值。
		void cleanStaleMediaCache() {
			static std::once_flag once;
			std::call_once(once, []() {
				QDir dir(mediaCacheDirPath());
				if (!dir.exists()) {
					return;
				}
				const QDateTime deadline = QDateTime::currentDateTimeUtc().addMSecs(-TempCacheMaxAgeMs);
				const QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
				qint32 removed = 0;
				for (const QFileInfo& entry : entries) {
					if (entry.lastModified().toUTC() > deadline) {
						continue;
					}
					if (QFile::remove(entry.absoluteFilePath())) {
						++removed;
					}
				}
				if (removed > 0) {
					vgDebug << "Removed" << removed << "stale media cache file(s) from" << dir.absolutePath();
				}
			});
		}

		QIcon mediaIcon(const QString& iconName) {
			return VIApp->getNamedFontIcon(iconName, IconRenderSize, { IconColor });
		}

		QString formatTime(qint64 ms) {
			if (ms < 0) {
				ms = 0;
			}
			const qint64 totalSeconds = ms / 1000;
			const qint64 seconds = totalSeconds % 60;
			const qint64 minutes = (totalSeconds / 60) % 60;
			const qint64 hours = totalSeconds / 3600;
			if (hours > 0) {
				return QStringLiteral("%1:%2:%3").arg(hours)
					.arg(minutes, 2, 10, QLatin1Char('0'))
					.arg(seconds, 2, 10, QLatin1Char('0'));
			}
			return QStringLiteral("%1:%2")
				.arg(minutes, 2, 10, QLatin1Char('0'))
				.arg(seconds, 2, 10, QLatin1Char('0'));
		}
	}

	MediaPlayerWidget::MediaPlayerWidget(QWidget* parent)
		: YSSCore::Editor::FileEditWidget(parent)
		, d(new MediaPlayerWidgetPrivate())
	{
		d->Player = new QMediaPlayer(this);
		d->AudioOutput = new QAudioOutput(this);
		d->Player->setAudioOutput(d->AudioOutput);
		// 需要接收空格与方向键，因此本控件要能获得焦点；下面的子控件都设为不接收焦点，
		// 以免方向键被滑块等控件截走。
		setFocusPolicy(Qt::StrongFocus);

		buildUi();
		buildControlBar();

		d->Player->setVideoSink(d->VideoItem->videoSink());
		d->AudioOutput->setVolume(d->VolumeSlider->value() / 100.0);

		// 记住最新一帧有效画面；首帧到达时结束预取并暂停，把画面定格在第一帧。
		connect(d->VideoItem->videoSink(), &QVideoSink::videoFrameChanged, this, [this](const QVideoFrame& frame) {
			if (!frame.isValid()) {
				return;
			}
			d->LastVideoFrame = frame;
			if (d->PrimingActive) {
				finishFirstFramePrime();
			}
		});

		// 控制栏渐入渐出：用不透明度动画实现，动画结束后才真正隐藏，
		// 初始置为透明以便首次显示时也能渐入。
		d->ControlBarOpacity = new Visindigo::Widgets::LiquidGlassEffect(d->ControlBar);
		d->ControlBarOpacity->setEffectTypes(
			Visindigo::Widgets::LiquidGlassEffect::EffectType::Blur
			| Visindigo::Widgets::LiquidGlassEffect::EffectType::NormalDistort
			| Visindigo::Widgets::LiquidGlassEffect::EffectType::RimLight
			| Visindigo::Widgets::LiquidGlassEffect::EffectType::GlobalRim);
		d->ControlBarOpacity->setBorderRadius(8);
		d->ControlBarOpacity->setBlurRadius(20);
		d->ControlBarOpacity->setLiquidDistortRadius(10);
		d->ControlBarOpacity->setColorMask(QColor(0, 0, 0), 0.20);
		d->ControlBarOpacity->setOpacity(0.0);
		d->ControlBar->setGraphicsEffect(d->ControlBarOpacity);
		d->ControlBarFade = new QPropertyAnimation(d->ControlBarOpacity, QByteArrayLiteral("opacity"), this);
		d->ControlBarFade->setEasingCurve(QEasingCurve::InOutQuad);
		connect(d->ControlBarFade, &QPropertyAnimation::finished, this, [this]() {
			if (!d->ControlBarShown) {
				d->ControlBar->hide();
			}
		});

		d->AutoHideTimer = new QTimer(this);
		d->AutoHideTimer->setSingleShot(true);
		d->AutoHideTimer->setInterval(AutoHideDelayMs);
		connect(d->AutoHideTimer, &QTimer::timeout, this, [this]() {
			// 鼠标仍在播放器范围内（含正在拖动进度条）时保持可见，否则渐出隐藏。
			if (d->ProgressSliderDragging || rect().contains(mapFromGlobal(QCursor::pos()))) {
				scheduleAutoHide();
				return;
			}
			setControlBarVisible(false);
		});

		// 方向键长按的周期性步进定时器。
		d->ScrubTimer = new QTimer(this);
		d->ScrubTimer->setInterval(ScrubRepeatMs);
		connect(d->ScrubTimer, &QTimer::timeout, this, [this]() { stepScrub(); });

		connect(d->PlayPauseButton, &QToolButton::clicked, this, [this]() { togglePlayPause(); });
		connect(d->Rewind5Button, &QToolButton::clicked, this, [this]() { seekBy(-5000); });
		connect(d->Rewind1Button, &QToolButton::clicked, this, [this]() { seekBy(-1000); });
		connect(d->Forward1Button, &QToolButton::clicked, this, [this]() { seekBy(1000); });
		connect(d->Forward5Button, &QToolButton::clicked, this, [this]() { seekBy(5000); });

		connect(d->ProgressSlider, &QSlider::sliderPressed, this, [this]() {
			d->ProgressSliderDragging = true;
		});
		connect(d->ProgressSlider, &QSlider::sliderMoved, this, [this](int value) {
			d->Player->setPosition(value);
		});
		connect(d->ProgressSlider, &QSlider::sliderReleased, this, [this]() {
			d->Player->setPosition(d->ProgressSlider->value());
			d->ProgressSliderDragging = false;
		});
		connect(d->VolumeSlider, &QSlider::valueChanged, this, [this](int value) {
			d->AudioOutput->setVolume(value / 100.0);
			updateVolumeIcon();
		});

		connect(d->Player, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
			if (d->ScrubTimer->isActive()) {
				// 方向键步进期间由 stepScrub() 驱动界面：播放器的位置更新是异步的，
				// 跟随它会让进度条来回跳。
				return;
			}
			if (!d->ProgressSliderDragging) {
				d->ProgressSlider->setValue(static_cast<int>(position));
			}
			updateTimeLabel();
		});
		connect(d->Player, &QMediaPlayer::durationChanged, this, [this](qint64 duration) {
			d->ProgressSlider->setRange(0, static_cast<int>(qMax<qint64>(duration, 0)));
			updateTimeLabel();
		});
		connect(d->Player, &QMediaPlayer::metaDataChanged, this, [this]() { updateFrameDuration(); });
		connect(d->Player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state) {
			// 首帧预取期间会出现极短的 Playing 状态，不要让它闪一下暂停图标。
			if (d->PrimingActive) {
				return;
			}
			if (state == QMediaPlayer::PlayingState) {
				// 用户已自行开始播放，不再需要预取首帧。
				d->PrimeFirstFrame = false;
			}
			updatePlayPauseButton();
		});
		connect(d->Player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
			switch (status) {
			case QMediaPlayer::LoadedMedia:
			case QMediaPlayer::BufferedMedia:
				// 仅在停止状态时预取首帧，避免打断用户此刻的播放。
				if (d->PrimeFirstFrame && d->Player->playbackState() == QMediaPlayer::StoppedState) {
					d->PrimeFirstFrame = false;
					d->PrimingActive = true;
					// 逐帧解码只在真正播放时发生，只调用 setPosition() 不会产生画面，
					// 因此这里短暂起播，拿到首帧后立即暂停；期间静音以免出现极短的音频爆音。
					d->AudioOutput->setVolume(0.0);
					d->Player->play();
				}
				break;
			case QMediaPlayer::EndOfMedia:
				// 播放结束后 Qt 会清空视频输出，把缓存的最后一帧推回去，避免画面变黑。
				if (d->LastVideoFrame.isValid()) {
					if (QVideoSink* sink = d->VideoItem->videoSink()) {
						sink->setVideoFrame(d->LastVideoFrame);
					}
				}
				break;
			case QMediaPlayer::InvalidMedia:
				// 加载失败则不会有首帧，结束预取以免音量被静音卡住。
				d->PrimeFirstFrame = false;
				finishFirstFramePrime();
				break;
			default:
				break;
			}
		});
		connect(d->Player, &QMediaPlayer::hasVideoChanged, this, [this](bool hasVideo) {
			showDisplayPage(hasVideo);
		});
		connect(d->Player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString& errorString) {
			// 出错时结束预取，确保音量不会被静音卡住。
			finishFirstFramePrime();
			showPlaybackError(errorString);
		});
		connect(VISTM, &Visindigo::Widgets::ThemeManager::programThemeChanged, this, [this]() {
			// 强调色会随主题变化，控制栏进度条的填充色需要跟着刷新。
			refreshControlBarStyle();
		});

		refreshIcons();
		updateTimeLabel();
		installActivityTracking();
	}

	MediaPlayerWidget::~MediaPlayerWidget() {
		removeTempMediaFile();
		delete d;
	}

	void MediaPlayerWidget::buildUi() {
		QVBoxLayout* root = new QVBoxLayout(this);
		root->setContentsMargins(0, 0, 0, 0);
		root->setSpacing(0);

		d->DisplayArea = new QStackedWidget(this);

		d->AudioPlaceholder = new QLabel(d->DisplayArea);
		d->AudioPlaceholder->setAlignment(Qt::AlignCenter);
		d->AudioPlaceholder->setWordWrap(true);
		d->AudioPlaceholder->setStyleSheet(QStringLiteral("background-color: rgb(24, 24, 24); color: rgb(224, 224, 224);"));

		// 用 QGraphicsView + QGraphicsVideoItem 而不是 QVideoWidget：后者内部创建原生窗口，
		// 会盖住任何叠加在它上面的 Qt 控件，半透明控制栏就无法显示。
		d->VideoScene = new QGraphicsScene(this);
		d->VideoItem = new QGraphicsVideoItem();
		d->VideoItem->setAspectRatioMode(Qt::KeepAspectRatio);
		d->VideoScene->addItem(d->VideoItem);
		d->VideoView = new QGraphicsView(d->VideoScene, d->DisplayArea);
		d->VideoView->setFrameShape(QFrame::NoFrame);
		d->VideoView->setFocusPolicy(Qt::NoFocus);
		d->VideoView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		d->VideoView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		d->VideoView->setBackgroundBrush(QColor(24, 24, 24));
		d->VideoView->setRenderHint(QPainter::SmoothPixmapTransform);

		d->DisplayArea->addWidget(d->AudioPlaceholder);
		d->DisplayArea->addWidget(d->VideoView);
		root->addWidget(d->DisplayArea, 1);
		setLayout(root);
	}

	void MediaPlayerWidget::buildControlBar() {
		d->ControlBar = new QFrame(this);
		d->ControlBar->setObjectName(QStringLiteral("yssMediaPlayerControlBar"));
		d->ControlBar->setFrameShape(QFrame::NoFrame);

		QVBoxLayout* barLayout = new QVBoxLayout(d->ControlBar);
		barLayout->setContentsMargins(10, 6, 10, 6);
		barLayout->setSpacing(2);

		QHBoxLayout* progressRow = new QHBoxLayout();
		progressRow->setSpacing(8);
		d->TimeLabel = new QLabel(d->ControlBar);
		d->TimeLabel->setObjectName(QStringLiteral("yssMediaPlayerTimeLabel"));
		d->ProgressSlider = new QSlider(Qt::Horizontal, d->ControlBar);
		d->ProgressSlider->setRange(0, 0);
		d->ProgressSlider->setFocusPolicy(Qt::NoFocus);
		d->ProgressSlider->setToolTip(VITRL("YSSMediaPlayer::control.progress"));
		progressRow->addWidget(d->TimeLabel);
		progressRow->addWidget(d->ProgressSlider, 1);
		barLayout->addLayout(progressRow);

		QHBoxLayout* buttonRow = new QHBoxLayout();
		buttonRow->setSpacing(2);

		d->ButtonsHost = new QWidget(d->ControlBar);
		QHBoxLayout* buttonsLayout = new QHBoxLayout(d->ButtonsHost);
		buttonsLayout->setContentsMargins(0, 0, 0, 0);
		buttonsLayout->setSpacing(2);
		d->Rewind5Button = createControlButton(QStringLiteral("Rewind"), VITRL("YSSMediaPlayer::control.rewind5"));
		d->Rewind1Button = createControlButton(QStringLiteral("ChevronLeftMed"), VITRL("YSSMediaPlayer::control.rewind1"));
		d->PlayPauseButton = createControlButton(QStringLiteral("Play"), VITRL("YSSMediaPlayer::control.play"));
		d->Forward1Button = createControlButton(QStringLiteral("ChevronRightMed"), VITRL("YSSMediaPlayer::control.forward1"));
		d->Forward5Button = createControlButton(QStringLiteral("FastForward"), VITRL("YSSMediaPlayer::control.forward5"));
		buttonsLayout->addWidget(d->Rewind5Button);
		buttonsLayout->addWidget(d->Rewind1Button);
		buttonsLayout->addWidget(d->PlayPauseButton);
		buttonsLayout->addWidget(d->Forward1Button);
		buttonsLayout->addWidget(d->Forward5Button);

		// 两侧拉伸因子相同且中间只有按钮行一个控件，五个按钮必然严格居中。
		// 音量区不参与该布局，而是贴右边缘单独摆放（见 updateControlBarGeometry），
		// 否则它会占掉右侧空间、把按钮组推离中心。
		buttonRow->addStretch(1);
		buttonRow->addWidget(d->ButtonsHost);
		buttonRow->addStretch(1);
		barLayout->addLayout(buttonRow);

		d->VolumeBlock = new QWidget(d->ControlBar);
		d->VolumeBlock->setFixedSize(VolumeBlockWidth, ButtonSize);
		QHBoxLayout* volumeLayout = new QHBoxLayout(d->VolumeBlock);
		volumeLayout->setContentsMargins(0, 0, 0, 0);
		volumeLayout->setSpacing(VolumeBlockSpacing);
		d->VolumeIconLabel = new QLabel(d->VolumeBlock);
		d->VolumeIconLabel->setFixedSize(VolumeIconSize, VolumeIconSize);
		d->VolumeSlider = new QSlider(Qt::Horizontal, d->VolumeBlock);
		d->VolumeSlider->setRange(0, 100);
		d->VolumeSlider->setValue(100);
		d->VolumeSlider->setFixedWidth(VolumeSliderWidth);
		d->VolumeSlider->setFocusPolicy(Qt::NoFocus);
		d->VolumeSlider->setToolTip(VITRL("YSSMediaPlayer::control.volume"));
		volumeLayout->addStretch(1);
		volumeLayout->addWidget(d->VolumeIconLabel);
		volumeLayout->addWidget(d->VolumeSlider);

		refreshControlBarStyle();
	}

	QToolButton* MediaPlayerWidget::createControlButton(const QString& iconName, const QString& toolTip) {
		QToolButton* button = new QToolButton(d->ButtonsHost);
		button->setObjectName(QStringLiteral("yssMediaPlayerButton"));
		button->setAutoRaise(true);
		button->setCursor(Qt::PointingHandCursor);
		button->setFocusPolicy(Qt::NoFocus);
		button->setFixedSize(ButtonSize, ButtonSize);
		button->setIconSize(QSize(ButtonIconSize, ButtonIconSize));
		button->setIcon(mediaIcon(iconName));
		button->setToolTip(toolTip);
		return button;
	}

	void MediaPlayerWidget::refreshControlBarStyle() {
		// 控制栏浮在视频/占位内容之上，固定使用深色半透明底，因此图标固定用浅色。
		d->ControlBar->setStyleSheet(QStringLiteral(
			"#yssMediaPlayerControlBar { background-color: rgba(18, 18, 18, 35); border-radius: 8px; }"
			"#yssMediaPlayerButton { background: transparent; border: none; border-radius: 6px; }"
			"#yssMediaPlayerButton:hover { background-color: rgba(255, 255, 255, 45); }"
			"#yssMediaPlayerButton:pressed { background-color: rgba(255, 255, 255, 80); }"
			"#yssMediaPlayerTimeLabel { color: rgba(240, 240, 240, 225); }"
		));
		const QColor accent = Visindigo::Widgets::ThemeManager::getPaletteAccentColor();
		const QString accentValue = QStringLiteral("rgb(%1, %2, %3)")
			.arg(accent.red()).arg(accent.green()).arg(accent.blue());
		const QString sliderStyle = QStringLiteral(
			"QSlider { background: transparent; }"
			"QSlider::groove:horizontal { height: 4px; border-radius: 2px; background-color: rgba(255, 255, 255, 70); }"
			"QSlider::sub-page:horizontal { border-radius: 2px; background-color: %1; }"
			"QSlider::handle:horizontal { width: 12px; margin: -5px 0px; border-radius: 6px; background-color: rgba(245, 245, 245, 235); }"
			"QSlider::handle:horizontal:hover { background-color: rgb(255, 255, 255); }"
		).arg(accentValue);
		d->ProgressSlider->setStyleSheet(sliderStyle);
		d->VolumeSlider->setStyleSheet(sliderStyle);
	}

	void MediaPlayerWidget::refreshIcons() {
		d->Rewind5Button->setIcon(mediaIcon(QStringLiteral("Rewind")));
		d->Rewind1Button->setIcon(mediaIcon(QStringLiteral("ChevronLeftMed")));
		d->Forward1Button->setIcon(mediaIcon(QStringLiteral("ChevronRightMed")));
		d->Forward5Button->setIcon(mediaIcon(QStringLiteral("FastForward")));
		updatePlayPauseButton();
		updateVolumeIcon();
	}

	void MediaPlayerWidget::installActivityTracking() {
		// 鼠标落在任何子控件上时父控件收不到 mouseMoveEvent，因此统一用事件过滤器收集活动。
		setMouseTracking(true);
		installEventFilter(this);
		const QList<QWidget*> children = findChildren<QWidget*>();
		for (QWidget* child : children) {
			child->setMouseTracking(true);
			child->installEventFilter(this);
		}
	}

	void MediaPlayerWidget::updateControlBarGeometry() {
		if (d->ControlBar == nullptr) {
			return;
		}
		if (d->ControlBar->layout() != nullptr) {
			d->ControlBar->layout()->activate();
		}
		const int barHeight = d->ControlBar->sizeHint().height();
		const int barWidth = qMax(width() - 2 * ControlBarMargin, 0);
		d->ControlBar->setGeometry(ControlBarMargin, height() - barHeight - ControlBarMargin, barWidth, barHeight);
		updateVolumeBlockGeometry(barWidth);
	}

	void MediaPlayerWidget::updateVolumeBlockGeometry(int barWidth) {
		if (d->VolumeBlock == nullptr || d->ButtonsHost == nullptr) {
			return;
		}
		// 音量区贴在按钮行同一水平线上、靠控制栏右边缘摆放。
		const int volumeX = barWidth - ControlBarSideMargin - d->VolumeBlock->width();
		d->VolumeBlock->setGeometry(volumeX, d->ButtonsHost->y(),
			d->VolumeBlock->width(), d->ButtonsHost->height());
		// 只有在音量区左边缘与按钮行右边缘之间还剩空隙时才显示，避免窗口很窄时互相覆盖。
		const int buttonsRight = d->ButtonsHost->geometry().right();
		d->VolumeBlock->setVisible(volumeX >= buttonsRight + VolumeBlockGap);
	}

	void MediaPlayerWidget::updateVideoGeometry() {
		if (d->VideoView == nullptr || d->VideoView->viewport() == nullptr) {
			return;
		}
		const QRect viewportRect = d->VideoView->viewport()->rect();
		d->VideoScene->setSceneRect(QRectF(viewportRect));
		// 让视频项铺满视口，由 KeepAspectRatio 在项内做黑边适配。
		d->VideoItem->setSize(QSizeF(viewportRect.size()));
	}

	void MediaPlayerWidget::revealControlBar() {
		setControlBarVisible(true);
		scheduleAutoHide();
	}

	void MediaPlayerWidget::setControlBarVisible(bool visible) {
		// 已处于目标状态时直接返回：鼠标每次移动都会调用这里，
		// 不能反复重启动画，否则渐入永远播不完。
		if (d->ControlBarShown == visible && d->ControlBar->isVisible() == visible) {
			return;
		}
		d->ControlBarShown = visible;
		d->ControlBarFade->stop();

		if (visible) {
			updateControlBarGeometry();
			d->ControlBar->raise();
			d->ControlBar->show();
		}

		const int duration = VISTM->getAnimationDuration();
		if (duration <= 0) {
			// 与 ThemeManager 的语义保持一致：未配置动画时长时不做渐变。
			d->ControlBarOpacity->setOpacity(visible ? 1.0 : 0.0);
			if (!visible) {
				d->ControlBar->hide();
			}
			return;
		}
		d->ControlBarFade->setDuration(duration);
		// 从当前不透明度续播，这样在渐出中途把鼠标移回来能平滑地渐入。
		d->ControlBarFade->setStartValue(d->ControlBarOpacity->opacity());
		d->ControlBarFade->setEndValue(visible ? 1.0 : 0.0);
		d->ControlBarFade->start();
	}

	void MediaPlayerWidget::finishFirstFramePrime() {
		if (!d->PrimingActive) {
			return;
		}
		d->PrimingActive = false;
		// 恢复用户设定的音量（预取期间被静音）。
		d->AudioOutput->setVolume(d->VolumeSlider->value() / 100.0);
		d->Player->pause();
	}

	void MediaPlayerWidget::scheduleAutoHide() {
		d->AutoHideTimer->start(AutoHideDelayMs);
	}

	void MediaPlayerWidget::showEvent(QShowEvent* event) {
		YSSCore::Editor::FileEditWidget::showEvent(event);
		setFocus();
		updateControlBarGeometry();
		updateVideoGeometry();
		revealControlBar();
	}

	void MediaPlayerWidget::resizeEvent(QResizeEvent* event) {
		YSSCore::Editor::FileEditWidget::resizeEvent(event);
		updateControlBarGeometry();
		updateVideoGeometry();
	}

	void MediaPlayerWidget::keyPressEvent(QKeyEvent* event) {
		if (event->isAutoRepeat()) {
			// 长按的连续步进由 ScrubTimer 自行驱动，忽略系统的自动重复事件。
			YSSCore::Editor::FileEditWidget::keyPressEvent(event);
			return;
		}
		switch (event->key()) {
		case Qt::Key_Space:
			togglePlayPause();
			revealControlBar();
			event->accept();
			return;
		case Qt::Key_Left:
		case Qt::Key_Right:
			updateScrubKey(event->key(), true);
			event->accept();
			return;
		default:
			break;
		}
		YSSCore::Editor::FileEditWidget::keyPressEvent(event);
	}

	void MediaPlayerWidget::keyReleaseEvent(QKeyEvent* event) {
		if (event->isAutoRepeat()) {
			YSSCore::Editor::FileEditWidget::keyReleaseEvent(event);
			return;
		}
		if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {
			updateScrubKey(event->key(), false);
			event->accept();
			return;
		}
		YSSCore::Editor::FileEditWidget::keyReleaseEvent(event);
	}

	bool MediaPlayerWidget::eventFilter(QObject* watched, QEvent* event) {
		switch (event->type()) {
		case QEvent::MouseButtonPress: {
			const QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			// 视频画面内单击切换播放/暂停；底部控制栏是独立的置顶子控件，
			// 点在它上面的鼠标事件不会落到这里。
			if (mouseEvent->button() == Qt::LeftButton && isVideoAreaObject(watched)) {
				togglePlayPause();
				revealControlBar();
				// 必须消费掉这个事件：QAbstractScrollArea 会把 viewport 收到的鼠标事件
				// 再转发给 view 自己，而两者都在过滤范围内，放行会导致切换两次而互相抵消。
				return true;
			}
			revealControlBar();
			break;
		}
		case QEvent::Enter:
		case QEvent::MouseMove:
		case QEvent::MouseButtonRelease:
		case QEvent::Wheel:
			revealControlBar();
			break;
		default:
			break;
		}
		return YSSCore::Editor::FileEditWidget::eventFilter(watched, event);
	}

	bool MediaPlayerWidget::isVideoAreaObject(const QObject* object) const {
		// 仅视频模式（音频占位页不响应点击），且必须已加载媒体。
		if (d->DisplayArea->currentWidget() != d->VideoView) {
			return false;
		}
		if (d->Player->mediaStatus() == QMediaPlayer::NoMedia) {
			return false;
		}
		return object == d->VideoView || object == d->VideoView->viewport();
	}

	bool MediaPlayerWidget::onOpen(const YSSCore::General::YSSURI& uri, const QByteArray& data) {
		// QMediaPlayer 只接受本地 URL，因此远端 URI 先落一份临时副本再播放。
		QString playablePath;
		if (uri.isLocal()) {
			removeTempMediaFile();
			playablePath = uri.getLocalPath();
		}
		else {
			playablePath = this->materializeTempMediaFile(uri, data);
			if (playablePath.isEmpty()) {
				return false;
			}
		}
		const QFileInfo fileInfo(playablePath);
		if (!fileInfo.exists() || !fileInfo.isFile()) {
			vgErrorF << "Failed to open media file, file does not exist:" << playablePath;
			return false;
		}

		d->AudioPlaceholder->setText(VITRL("YSSMediaPlayer::placeholder.audio").arg(uri.getFileName()));
		const bool isVideo = MediaFormats::isVideoExtension(uri.getFileExt());
		showDisplayPage(isVideo);
		d->ProgressSlider->setRange(0, 0);
		d->ProgressSlider->setValue(0);

		// 换文件时丢弃上一个文件的缓存画面，并重置首帧预取状态。
		d->LastVideoFrame = QVideoFrame();
		d->PrimingActive = false;
		d->PrimeFirstFrame = isVideo;

		// 只加载不播放，由用户点击播放按钮开始；视频会在加载完成后预取首帧用于定格显示。
		d->Player->setSource(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
		updateTimeLabel();
		updateVideoGeometry();
		revealControlBar();
		return true;
	}

	/*!
		\since YSS 0.18.0

		把远端媒体的内容写成临时文件，供 QMediaPlayer 播放。

		扩展名必须保留：QMediaPlayer 靠它挑选解复用器。

		\a uri 媒体文件身份，仅用于生成临时文件名与诊断信息。
		\a data 已经由 \l{YSSCore::General::YSSUniqueFileSystem} 读到的内容。

		\return 临时文件的绝对路径；内容为空或写入失败时返回空字符串。
	*/
	QString MediaPlayerWidget::materializeTempMediaFile(const YSSCore::General::YSSURI& uri, const QByteArray& data) {
		if (data.isEmpty()) {
			vgErrorF << "Remote media file has no content:" << uri.toString();
			return QString();
		}
		removeTempMediaFile();
		cleanStaleMediaCache();
		const QString cacheDir = mediaCacheDirPath();
		if (!QDir().mkpath(cacheDir)) {
			vgErrorF << "Failed to create media cache directory:" << cacheDir;
			return QString();
		}
		const QString ext = uri.getFileExt();
		QString fileName = QUuid::createUuid().toString(QUuid::WithoutBraces);
		if (!ext.isEmpty()) {
			fileName += QStringLiteral(".") + ext;
		}
		const QString path = cacheDir + QStringLiteral("/") + fileName;
		QFile file(path);
		if (!file.open(QIODevice::WriteOnly)) {
			vgErrorF << "Failed to create media cache file:" << path;
			return QString();
		}
		const bool ok = file.write(data) == data.size();
		file.close();
		if (!ok) {
			vgErrorF << "Failed to write media cache file:" << path;
			QFile::remove(path);
			return QString();
		}
		d->TempMediaFile = path;
		return path;
	}

	/*!
		\since YSS 0.18.0

		删除上一次为远端媒体落地的临时副本。没有副本时什么也不做。
	*/
	void MediaPlayerWidget::removeTempMediaFile() {
		if (d->TempMediaFile.isEmpty()) {
			return;
		}
		QFile::remove(d->TempMediaFile);
		d->TempMediaFile.clear();
	}

	bool MediaPlayerWidget::onClose() {
		d->PrimeFirstFrame = false;
		d->PrimingActive = false;
		d->LastVideoFrame = QVideoFrame();
		d->ScrubTimer->stop();
		d->ScrubKeys.clear();
		d->Player->stop();
		d->Player->setSource(QUrl());
		removeTempMediaFile();
		d->ProgressSlider->setRange(0, 0);
		d->ProgressSlider->setValue(0);
		d->AudioPlaceholder->setText(QString());
		showDisplayPage(false);
		updatePlayPauseButton();
		updateTimeLabel();
		return true;
	}

	std::expected<QByteArray, bool> MediaPlayerWidget::onSave(const YSSCore::General::YSSURI& uri) {
		// 媒体播放器为只读视图，不提供保存功能：静默跳过，不当作错误打扰用户。
		Q_UNUSED(uri);
		return std::unexpected(false);
	}

	void MediaPlayerWidget::togglePlayPause() {
		if (d->Player->mediaStatus() == QMediaPlayer::NoMedia) {
			return;
		}
		if (d->Player->playbackState() == QMediaPlayer::PlayingState) {
			d->Player->pause();
		}
		else {
			d->Player->play();
		}
	}

	void MediaPlayerWidget::updateFrameDuration() {
		// "轻按方向键步进一帧"需要知道一帧多长，优先取媒体元数据中的帧率。
		const double fps = d->Player->metaData().value(QMediaMetaData::VideoFrameRate).toDouble();
		if (fps > 1.0 && fps < 1000.0) {
			d->FrameDurationMs = qMax<qint64>(1, static_cast<qint64>(1000.0 / fps + 0.5));
		}
		else {
			d->FrameDurationMs = DefaultFrameDurationMs;
		}
	}

	void MediaPlayerWidget::updateScrubKey(int key, bool pressed) {
		if (d->Player->mediaStatus() == QMediaPlayer::NoMedia) {
			return;
		}
		if (pressed) {
			d->ScrubKeys.insert(key);
			revealControlBar();
			if (!d->ScrubTimer->isActive()) {
				d->ScrubTarget = d->Player->position();
				d->ScrubClock.restart();
				// 立刻走一步，这样轻按一下就是步进一帧。
				stepScrub();
				d->ScrubTimer->start();
			}
			return;
		}
		d->ScrubKeys.remove(key);
		if (!d->ScrubKeys.isEmpty()) {
			return;
		}
		d->ScrubTimer->stop();
		// 收尾落位到目标位置，避免最后一次异步 seek 被丢弃。
		d->Player->setPosition(d->ScrubTarget);
		d->ProgressSlider->setValue(static_cast<int>(d->ScrubTarget));
		updateTimeLabel(d->ScrubTarget);
	}

	void MediaPlayerWidget::stepScrub() {
		int direction = 0;
		if (d->ScrubKeys.contains(Qt::Key_Right)) {
			direction += 1;
		}
		if (d->ScrubKeys.contains(Qt::Key_Left)) {
			direction -= 1;
		}
		if (direction == 0) {
			return;
		}
		const qint64 duration = d->Player->duration();
		// 步进量按长按时长在 0~ScrubRampMs 内从一帧线性增长到 ScrubMaxStepMs。
		const double progress = qBound(0.0, d->ScrubClock.elapsed() / static_cast<double>(ScrubRampMs), 1.0);
		const qint64 step = d->FrameDurationMs
			+ static_cast<qint64>((ScrubMaxStepMs - d->FrameDurationMs) * progress);
		qint64 target = d->ScrubTarget + direction * step;
		if (target < 0) {
			target = 0;
		}
		else if (duration > 0 && target > duration) {
			target = duration;
		}
		d->ScrubTarget = target;
		d->Player->setPosition(d->ScrubTarget);
		// 播放器的位置更新是异步的，这里直接用目标值驱动界面，步进才跟手。
		d->ProgressSlider->setValue(static_cast<int>(d->ScrubTarget));
		updateTimeLabel(d->ScrubTarget);
	}

	void MediaPlayerWidget::seekBy(qint64 deltaMs) {
		if (d->Player->mediaStatus() == QMediaPlayer::NoMedia) {
			return;
		}
		const qint64 duration = d->Player->duration();
		qint64 target = d->Player->position() + deltaMs;
		if (target < 0) {
			target = 0;
		}
		else if (duration > 0 && target > duration) {
			target = duration;
		}
		d->Player->setPosition(target);
	}

	void MediaPlayerWidget::showDisplayPage(bool video) {
		if (video) {
			d->DisplayArea->setCurrentWidget(d->VideoView);
			updateVideoGeometry();
		}
		else {
			d->DisplayArea->setCurrentWidget(d->AudioPlaceholder);
		}
	}

	void MediaPlayerWidget::updatePlayPauseButton() {
		const bool playing = d->Player->playbackState() == QMediaPlayer::PlayingState;
		d->PlayPauseButton->setIcon(mediaIcon(playing ? QStringLiteral("Pause") : QStringLiteral("Play")));
		d->PlayPauseButton->setToolTip(playing
			? VITRL("YSSMediaPlayer::control.pause")
			: VITRL("YSSMediaPlayer::control.play"));
	}

	void MediaPlayerWidget::updateVolumeIcon() {
		const int value = d->VolumeSlider->value();
		QString iconName;
		if (value == 0) {
			iconName = QStringLiteral("Mute");
		}
		else if (value <= 33) {
			iconName = QStringLiteral("Volume0");
		}
		else if (value <= 66) {
			iconName = QStringLiteral("Volume1");
		}
		else if (value < 100) {
			iconName = QStringLiteral("Volume2");
		}
		else {
			iconName = QStringLiteral("Volume3");
		}
		d->VolumeIconLabel->setPixmap(mediaIcon(iconName).pixmap(QSize(VolumeIconSize, VolumeIconSize)));
	}

	void MediaPlayerWidget::updateTimeLabel(qint64 positionOverride) {
		const qint64 position = positionOverride >= 0 ? positionOverride : d->Player->position();
		d->TimeLabel->setText(QStringLiteral("%1 / %2").arg(
			formatTime(position), formatTime(d->Player->duration())));
	}

	void MediaPlayerWidget::showPlaybackError(const QString& errorString) {
		vgErrorF << "Failed to play media file:" << errorString;
		d->AudioPlaceholder->setText(VITRL("YSSMediaPlayer::error.playback").arg(errorString));
		showDisplayPage(false);
	}
}
