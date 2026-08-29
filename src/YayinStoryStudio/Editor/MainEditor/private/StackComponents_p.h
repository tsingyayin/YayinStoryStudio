#ifndef YSS_Editor_MainWin_StackWidgetArea_p_H
#define YSS_Editor_MainWin_StackWidgetArea_p_H
#include <QtWidgets/qframe.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qtablewidget.h>
#include <QtWidgets/qlistview.h>
#include <QtGui/qstandarditemmodel.h>
#include <QtGui/qaction.h>
#include <QtGui/qdrag.h>
#include <Widgets/ThemeManager.h>
namespace YSS::Editor {
	class FileEditWidgetArea;
	class StackTagWidget;
	class TreeLayoutWidget;
	class StackTag :public QFrame {
		friend class StackTagWidget;
		friend class StackTagWidgetPrivate;
		Q_OBJECT;
	private:
		Qt::Orientation Orientation;
		QLabel* TitleLabel;
		QHBoxLayout* Layout;
		QString FilePath;
		bool Hovering = false;
		bool Focused = false;
		bool Pinned = false;
		bool Pressed = false;
		QPoint PressedPos;
		StackTagWidget* StayInWidget = nullptr;
		void updateToolTip();
	protected:
		QToolButton* PinLabel;
		QToolButton* CloseLabel;
		QAction* ActionClose;
		QAction* ActionPin;
		QAction* ActionReload;
		QAction* ActionRename;
		QAction* ActionSave;
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
		void saveRequested(const QString& filePath);
	public:
		static const QString stackTagDragMimeType;
		static bool canAcceptDrag(const QMimeData* mimeData);
	public:
		StackTag(QWidget* parent = nullptr, Qt::Orientation orientation = Qt::Horizontal);
		void setStayInWidget(StackTagWidget* widget);
		StackTagWidget* getStayInWidget() const;
		void setText(const QString& text);
		void setFilePath(const QString& filePath);
		QString getFilePath() const;
		QString getText() const;
		void setFocusOn(bool focus);
		bool isFocusOn() const;
		void setPinned(bool pinned);
		bool isPinned() const;
	public:
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseReleaseEvent(QMouseEvent* event) override;
		virtual void mouseMoveEvent(QMouseEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual void enterEvent(QEnterEvent* event) override;
		virtual void leaveEvent(QEvent* event) override;
		virtual void paintEvent(QPaintEvent* event) override;
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
		void saveRequested(const QString& filePath);
	private:
		Qt::Orientation Orientation;
		QBoxLayout* ContentLayout;
		QWidget* ScrollContent;
		QScrollArea* ScrollArea;
		QToolButton* WidgetSelector;
		QFrame* WidgetSelectorPopup;
		QListView* WidgetSelectorList;
		QStandardItemModel* WidgetSelectorModel;
		QBoxLayout* Layout;
		QList<StackTag*> Labels;
		QString CurrentSelected;
		FileEditWidgetArea* Area = nullptr;
	public:
		StackTagWidget(QWidget* parent = nullptr, Qt::Orientation orientation = Qt::Horizontal);
		virtual ~StackTagWidget();
		void setArea(FileEditWidgetArea* area);
		FileEditWidgetArea* getArea() const;
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
	public:
		virtual void wheelEvent(QWheelEvent* event) override;
		virtual void onThemeChanged() override;
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual void dragEnterEvent(QDragEnterEvent* event) override;
		virtual void dragMoveEvent(QDragMoveEvent* event) override;
		virtual void dropEvent(QDropEvent* event) override;
	private:
		void applyIcon(StackTag* label, const QColor& textColor, const QColor& accentColor);
		StackTag* findLabel(const QString& filePath) const;
		void showWidgetSelectorPopup();
	};

	class StackTagDrag :public QDrag {
	public:
		StackTagDrag(StackTag* source, QWidget* dragSource);
		virtual ~StackTagDrag() override;
	private:
		StackTag* Source;
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
