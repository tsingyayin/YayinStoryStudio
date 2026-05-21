#ifndef YSS_Editor_MainWin_StackWidgetArea_p_H
#define YSS_Editor_MainWin_StackWidgetArea_p_H
#include <QtWidgets/qframe.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qtablewidget.h>
#include <QtWidgets/qmenu.h>
#include <Widgets/ThemeManager.h>
namespace YSS::Editor {
	class StackTag :public QFrame {
		friend class StackTagWidget;
		friend class StackTagWidgetPrivate;
		Q_OBJECT;
	private:
		QLabel* TitleLabel;
		QHBoxLayout* Layout;
		QString FilePath;
		QString NormalStyle;
		QString HoverStyle;
		QString PressedStyle;
		bool toolWidgetMode = false;
		bool Focused = false;
		bool Pinned = false;
		bool Pressed = false;
	protected:
		QToolButton* PinLabel;
		QToolButton* CloseLabel;
		QAction* ActionClose;
		QAction* ActionPin;
		QAction* ActionReload;
		QAction* ActionRename;
		QAction* ActionSaveAs;
		QAction* ActionShowInExplorer;
		QAction* ActionCloseAll;
		QAction* ActionCloseSaved;
	signals:
		void clicked(const QString& filePath);
		void pinClicked(const QString& filePath);
		void closeClicked(const QString& filePath);
		void renameRequested(const QString& filePath);
		void saveAsRequested(const QString& filePath);
		void closeAllRequested();
		void closeSavedRequested();
	public:
		StackTag(QWidget* parent = nullptr, bool toolWidgetMode = false);
		void setText(const QString& text);
		void setFilePath(const QString& filePath);
		QString getFilePath() const;
		QString getText() const;
		void setFocusOn(bool focus);
		bool isFocusOn() const;
		void setPinned(bool pinned);
		bool isPinned() const;
		void setStyleSheet(const QString& normal, const QString& hover, const QString& pressed);
	public:
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseReleaseEvent(QMouseEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual void enterEvent(QEnterEvent* event) override;
		virtual void leaveEvent(QEvent* event) override;
	};

	class StackTagWidget :public QFrame, public Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
	signals:
		void switchToFile(const QString& filePath);
		void closeFile(const QString& filePath);
		void renameRequested(const QString& filePath);
		void saveAsRequested(const QString& filePath);
		void closeAllRequested();
		void closeSavedRequested();
	private:
		QHBoxLayout* ContentLayout;
		QFrame* ScrollContent;
		QScrollArea* ScrollArea;
		QComboBox* WidgetSelector;
		QHBoxLayout* Layout;
		QList<StackTag*> Labels;
		QString CurrentSelected;
		bool ToolWidgetMode = false;
	public:
		StackTagWidget(QWidget* parent = nullptr);
		virtual ~StackTagWidget();
		void addStackLabel(const QString& filePath, const QString& displayName = QString());
		void changeStackLabel(const QString& oldFilePath, const QString& newFilePath, const QString& newDisplayName = QString());
		void pinStackLabel(const QString& filePath);
		void removeStackLabel(const QString& filePath);
		void setCurrentStackLabel(const QString& filePath);
		QString getCurrentSelected() const;
		void adjustScrollArea();
		void setFileChanged(const QString& filePath);
		void cancelFileChanged(const QString& filePath);
		bool containsStackLabel(const QString& filePath) const;
		bool isStackLabelPinned(const QString& filePath) const;
		void setToolWidgetMode(bool toolWidgetMode);
		bool isToolWidgetMode() const;
	public:
		virtual void wheelEvent(QWheelEvent* event) override;
		virtual void onThemeChanged() override;
		virtual void resizeEvent(QResizeEvent* event) override;
	private:
		void applyIcon(StackTag* label, const QColor& textColor, const QColor& accentColor);
	};

	class DefaultStackWidgetCentralArea :public QFrame {
		Q_OBJECT;
	private:
		QLabel* ContentLabel;
		QGridLayout* Layout;
	public:
		DefaultStackWidgetCentralArea(QWidget* parent = nullptr);
		void setText(const QString& text);
	};
}
#endif // YSS_Editor_MainWin_StackWidgetArea_p_H
