#ifndef Plugin_YSSMediaPlayer_YSS_MediaPlayerWidget_h
#define Plugin_YSSMediaPlayer_YSS_MediaPlayerWidget_h
#include <Editor/FileEditWidget.h>

class QEvent;
class QKeyEvent;
class QFrame;
class QGraphicsScene;
class QGraphicsVideoItem;
class QGraphicsView;
class QLabel;
class QSlider;
class QStackedWidget;
class QTimer;
class QToolButton;
class QAudioOutput;
class QMediaPlayer;

namespace YSS::MediaPlayer {

	class MediaPlayerWidgetPrivate;

	/// 基于 Qt Multimedia 的通用媒体播放器，音频与视频共用同一套界面。
	///
	/// 视频通过 QGraphicsVideoItem 渲染（非原生窗口），因此底部控制栏能够以半透明浮层
	/// 叠加在内容之上；播放音频时使用 QLabel 占位显示播放状态。
	///
	/// 控制栏：上方为时间与播放进度，下方为回退 5s、回退 1s、播放/暂停、前进 1s、
	/// 前进 5s 五个图标按钮（居中），右侧为音量图标与音量条。
	/// 控制栏以半透明浮层形式叠在内容之上，鼠标离开播放器区域超过 5 秒后渐出隐藏，
	/// 鼠标再次进入或移动时渐入显示；渐变时长取 ThemeManager 的动画时长设置。
	///
	/// 快捷键：视频画面内单击（不含底部浮层）或空格键切换播放/暂停；
	/// 按住左右方向键回退/前进，步进量随长按时长在 1 秒内由 1 帧线性增长到 5 秒。
	class MediaPlayerWidget : public YSSCore::Editor::FileEditWidget {
		Q_OBJECT
	public:
		explicit MediaPlayerWidget(QWidget* parent = nullptr);
		virtual ~MediaPlayerWidget() override;
	public:
		virtual bool onOpen(const QString& filePath) override;
		virtual bool onClose() override;
		virtual bool onSave(const QString& filePath = "") override;
	protected:
		virtual void showEvent(QShowEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual void keyPressEvent(QKeyEvent* event) override;
		virtual void keyReleaseEvent(QKeyEvent* event) override;
		virtual bool eventFilter(QObject* watched, QEvent* event) override;
	private:
		void buildUi();
		void buildControlBar();
		QToolButton* createControlButton(const QString& iconName, const QString& toolTip);
		void refreshControlBarStyle();
		void refreshIcons();
		void installActivityTracking();
		void updateControlBarGeometry();
		void updateVolumeBlockGeometry(int barWidth);
		void updateVideoGeometry();
		void revealControlBar();
		void setControlBarVisible(bool visible);
		void finishFirstFramePrime();
		void scheduleAutoHide();
		void togglePlayPause();
		bool isVideoAreaObject(const QObject* object) const;
		void updateScrubKey(int key, bool pressed);
		void stepScrub();
		void updateFrameDuration();
		void seekBy(qint64 deltaMs);
		void showDisplayPage(bool video);
		void updatePlayPauseButton();
		void updateVolumeIcon();
		void updateTimeLabel(qint64 positionOverride = -1);
		void showPlaybackError(const QString& errorString);
	private:
		MediaPlayerWidgetPrivate* d;
	};

} // namespace YSS::MediaPlayer
#endif // Plugin_YSSMediaPlayer_YSS_MediaPlayerWidget_h
