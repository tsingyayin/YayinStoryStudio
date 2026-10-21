#ifndef YayinStoryStudio_Editor_MainEditor_BottomInfoWidget_p_h
#define YayinStoryStudio_Editor_MainEditor_BottomInfoWidget_p_h
#include <QtCore/qlist.h>
#include <Widgets/BorderFrame.h>

class QCheckBox;
class QComboBox;
class QLabel;
class QProgressBar;
class QTimer;
class QVBoxLayout;

namespace Visindigo::Widgets {
	class LiquidGlassEffect;
}

namespace YSS::Editor {
	/*
		TextEdit 的补全/内联提示设置。

		底栏空间有限，这些选项收进一个贴着底栏向上弹出的独立浮窗里。浮窗是
		Qt::Popup 顶层窗口，没有父对象，生命周期由调用方负责。

		外观与 TextEdit 里的悬浮窗（HoverInfo / TabCompleter）一致：BorderFrame 边框
		+ LiquidGlassEffect 毛玻璃。那两者是子控件，可以直接采样父控件背景；顶层弹窗
		没有父控件，因此每次弹出时自己抓一张所在窗口的截图作为玻璃底。
	*/
	class EditorHintSettingsPopup : public Visindigo::Widgets::BorderFrame {
	public:
		EditorHintSettingsPopup();
		// 贴着 \a anchor 的底边向上弹出，右边缘对齐。
		void popupFor(QWidget* anchor);
	private:
		QComboBox* CompleterLevelComboBox = nullptr;
		QCheckBox* InlayHintCheckBox = nullptr;
		Visindigo::Widgets::LiquidGlassEffect* Glass = nullptr;
	};

	/*
		统一文件系统的传输监视。

		显示成功读写的累计量（合计 + 按 Provider 分项）、当前正在执行的操作及其
		进度、以及排队中的任务数。浮窗可见时按固定间隔从 UniqueFileSystem 拉取
		快照，隐藏后停止轮询。

		外观同样对标 TextEdit 的悬浮窗，见 \l{EditorHintSettingsPopup}。
	*/
	class TransferMonitorPopup : public Visindigo::Widgets::BorderFrame {
	public:
		TransferMonitorPopup();
		// 贴着 \a anchor 的底边向上弹出，右边缘对齐。
		void popupFor(QWidget* anchor);
		// 立即刷新一次显示内容。
		void refresh();
	protected:
		virtual void showEvent(QShowEvent* event) override;
		virtual void hideEvent(QHideEvent* event) override;
	private:
		QLabel* TotalLabel = nullptr;
		QVBoxLayout* ProviderLayout = nullptr;
		QList<QLabel*> ProviderRows;
		QLabel* CurrentLabel = nullptr;
		QProgressBar* CurrentProgress = nullptr;
		QLabel* QueuedLabel = nullptr;
		QTimer* RefreshTimer = nullptr;
		Visindigo::Widgets::LiquidGlassEffect* Glass = nullptr;

		void refreshProviders();
		void refreshTasks();
	};
}
#endif // YayinStoryStudio_Editor_MainEditor_BottomInfoWidget_p_h

