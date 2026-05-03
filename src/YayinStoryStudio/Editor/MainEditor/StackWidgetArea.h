#ifndef YSS_Editor_MainWin_StackWidgetArea_h
#define YSS_Editor_MainWin_StackWidgetArea_h
#include <QtWidgets/qframe.h>
#include <Editor/FileEditWidget.h>

namespace YSS::Editor {
	class StackWidgetAreaPrivate;
	class StackWidgetArea :public QFrame {
		Q_OBJECT;
	signals:
		void renameRequested(const QString& absOldPath);
		void saveAsRequested(const QString& rawFilePath);
	public:
		StackWidgetArea(QWidget* parent = nullptr);
		virtual ~StackWidgetArea();
		void addWidget(YSSCore::Editor::FileEditWidget* widget);
		void closeAll();
		void closeWidget(YSSCore::Editor::FileEditWidget* widget);
		void closeWidget(const QString& filePath);
		void setCurrentWidget(YSSCore::Editor::FileEditWidget* widget);
		void setCurrentWidget(const QString& filePath);
		void setCurrentWidget(YSSCore::Editor::FileEditWidget* widget, qint32 lineNumber, qint32 column = 0);
		void setCurrentWidget(const QString& filePath, qint32 lineNumber, qint32 column);
		YSSCore::Editor::FileEditWidget* getCurrentWidget() const;
		void setMessageViewerEnable(bool enable);
	public:
		virtual void resizeEvent(QResizeEvent* event) override;
	private:
		StackWidgetAreaPrivate* d;
	};
}
#endif // YSS_Editor_MainWin_StackWidgetArea_h