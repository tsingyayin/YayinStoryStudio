#ifndef YSS_MainEditor_BottomInfoWidget_h
#define YSS_MainEditor_BottomInfoWidget_h
#include <Widgets/BorderFrame.h>
#include <Editor/DebugServer.h>
#include <Widgets/ThemeManager.h>
#include <QtGui/qtextcursor.h>
#include <tuple>
namespace YSS::Editor{
	class BottomInfoWidgetPrivate;
	class BottomInfoWidget :public Visindigo::Widgets::BorderFrame , public Visindigo::Widgets::ColorfulWidget{
		Q_OBJECT;
	signals:
		void requestEditorFontSizeChange(float scaleFactor);
	public:
		BottomInfoWidget(QWidget* parent = nullptr);
		~BottomInfoWidget();
	public:
		virtual void onThemeChanged() override;
		virtual void resizeEvent(QResizeEvent* event) override;
	public:
		void displayDebugInfo(YSSCore::Editor::DebugServer::DebugAction action, const QString& info);
		void clearDebugInfo();
		void displayDebugProgress(YSSCore::Editor::DebugServer::DebugAction action, qint32 finished, qint32 total);
		void clearDebugProgress();
		void setEditorFontSize(float scaleFactor);
		void displayEditorInfo(qint32 totalLine, qint32 currentLine, qint32 currentColumn, qint32 selected = 0);
		void displayEditorInfo(const QTextCursor& cursor);
		void setEditorInfoEnable(bool enable);
		void displayFileMessageCount(qint32 error, qint32 warning, qint32 info);
		void displayFileMessageCount(const std::tuple<qint32, qint32, qint32>& values);
		void displayGitInfo(qint32 pull, qint32 push, qint32 modified, QStringList branchs, QString currentBranch);
		void setGitInfoEnable(bool enable);
	private:
		BottomInfoWidgetPrivate* d;
	};
}
#endif // YSS_MainEditor_BottomInfoWidget_h
