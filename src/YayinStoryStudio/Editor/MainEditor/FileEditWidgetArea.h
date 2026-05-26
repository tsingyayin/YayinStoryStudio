#ifndef YSS_Editor_MainWin_StackWidgetArea_h
#define YSS_Editor_MainWin_StackWidgetArea_h
#include <QtWidgets/qframe.h>
#include <QtGui/qtextcursor.h>
#include <Editor/FileEditWidget.h>
#include <Widgets/BorderFrame.h>
namespace YSS::Editor {
	class FileEditWidgetAreaPrivate;
	class FileEditWidgetArea :public Visindigo::Widgets::BorderFrame {
		Q_OBJECT;
	signals:
		void renameRequested(const QString& absOldPath);
		void saveAsRequested(const QString& rawFilePath);
		void currentFileChanged(const QString& filePath);
		void textEditCursorPositionChanged(const QString& filePath, const QTextCursor& cursor);
	public:
		FileEditWidgetArea(QWidget* parent = nullptr);
		virtual ~FileEditWidgetArea();
		void addWidget(YSSCore::Editor::FileEditWidget* widget);
		void closeAll(bool autoGiveup = false);
		void closeSaved();
		void closeWidget(YSSCore::Editor::FileEditWidget* widget);
		void closeWidget(const QString& filePath);
		void setCurrentWidget(YSSCore::Editor::FileEditWidget* widget);
		void setCurrentWidget(const QString& filePath);
		void setCurrentWidget(YSSCore::Editor::FileEditWidget* widget, qint32 lineNumber, qint32 column = 0);
		void setCurrentWidget(const QString& filePath, qint32 lineNumber, qint32 column);
		YSSCore::Editor::FileEditWidget* getCurrentWidget() const;
		QString getCurrentWidgetFilePath() const;
		void setMessageViewerEnable(bool enable);
	public:
		virtual void resizeEvent(QResizeEvent* event) override;
	private:
		FileEditWidgetAreaPrivate* d;
	};
}
#endif // YSS_Editor_MainWin_StackWidgetArea_h