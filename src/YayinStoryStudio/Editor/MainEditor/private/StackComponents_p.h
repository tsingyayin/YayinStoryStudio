#ifndef YayinStoryStudio_Editor_MainEditor_private_StackComponents_p_h
#define YayinStoryStudio_Editor_MainEditor_private_StackComponents_p_h
#include <General/YSSURI.h>
#include <QtCore/qpointer.h>
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
#include <QtGui/qicon.h>
#include <Widgets/ThemeManager.h>
namespace YSSCore::Editor {
	class FileEditWidget;
}
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
		QLabel* ModifiedLabel;
		QHBoxLayout* Layout;
		YSSCore::General::YSSURI Uri;
		bool Hovering = false;
		bool Focused = false;
		bool AreaCurrent = false;
		bool Pinned = false;
		bool Pressed = false;
		bool Modified = false;
		qint32 AppliedFixedWidth = -1;
		QPoint PressedPos;
		StackTagWidget* StayInWidget = nullptr;
		// 用 QPointer 而不是裸指针：FileEditWidget 由 FileServerManager 在 fileClosed
		// 之后 deleteLater()，标签有可能比它活得久。QPointer 在对象销毁时自动置空，
		// 于是 getFileEditWidgets() 永远不会返回悬空指针。
		QPointer<YSSCore::Editor::FileEditWidget> FileEditWidget;
		QIcon ModifiedIcon;
		void updateToolTip();
		void updateFixedWidth();
		void updateModifiedIndicator();
		void updateButtonVisibility();
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
		QAction* ActionCompareAsOld;
		QAction* ActionCompareAsNew;
	signals:
		void clicked(const YSSCore::General::YSSURI& uri);
		void pinClicked(const YSSCore::General::YSSURI& uri);
		void closeClicked(const YSSCore::General::YSSURI& uri);
		void renameRequested(const YSSCore::General::YSSURI& uri);
		void saveAsRequested(const YSSCore::General::YSSURI& uri);
		void closeAllRequested();
		void closeSavedRequested();
		void saveRequested(const YSSCore::General::YSSURI& uri);
	public:
		static const QString stackTagDragMimeType;
		static bool canAcceptDrag(const QMimeData* mimeData);
		static StackTag* draggedTag(const QMimeData* mimeData);
		static StackTagWidget* draggedTagArea(const QMimeData* mimeData);
	public:
		StackTag(QWidget* parent = nullptr, Qt::Orientation orientation = Qt::Horizontal);
		void setStayInWidget(StackTagWidget* widget);
		StackTagWidget* getStayInWidget() const;
		void setText(const QString& text);
		void setModified(bool modified);
		bool isModified() const;
		void setUri(const YSSCore::General::YSSURI& uri);
		YSSCore::General::YSSURI getUri() const;
		QString getText() const;
		void setFileEditWidget(YSSCore::Editor::FileEditWidget* widget);
		YSSCore::Editor::FileEditWidget* getFileEditWidget() const;
		void setFocusOn(bool focus);
		bool isFocusOn() const;
		void setAreaCurrent(bool areaCurrent);
		bool isAreaCurrent() const;
		void setPinned(bool pinned);
		bool isPinned() const;
	public:
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseReleaseEvent(QMouseEvent* event) override;
		virtual void mouseMoveEvent(QMouseEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual void enterEvent(QEnterEvent* event) override;
		virtual void leaveEvent(QEvent* event) override;
		virtual void contextMenuEvent(QContextMenuEvent* event) override;
		virtual void paintEvent(QPaintEvent* event) override;
	};

	class StackTagWidget :public QFrame, public Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
	signals:
		void switchToFile(const YSSCore::General::YSSURI& uri);
		void closeFile(const YSSCore::General::YSSURI& uri);
		void renameRequested(const YSSCore::General::YSSURI& uri);
		void saveAsRequested(const YSSCore::General::YSSURI& uri);
		void closeAllRequested();
		void closeSavedRequested();
		void saveRequested(const YSSCore::General::YSSURI& uri);
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
		YSSCore::General::YSSURI CurrentSelected;
		FileEditWidgetArea* Area = nullptr;
		bool UseGlobalFocus = false;
		YSSCore::General::YSSURI GlobalCurrentUri;
	public:
		StackTagWidget(QWidget* parent = nullptr, Qt::Orientation orientation = Qt::Horizontal);
		virtual ~StackTagWidget();
		void setArea(FileEditWidgetArea* area);
		FileEditWidgetArea* getArea() const;
		void addStackLabel(const YSSCore::General::YSSURI& uri, const QString& displayName = QString(), YSSCore::Editor::FileEditWidget* widget = nullptr);
		void changeStackLabel(const YSSCore::General::YSSURI& oldUri, const YSSCore::General::YSSURI& newUri, const QString& newDisplayName = QString());
		void pinStackLabel(const YSSCore::General::YSSURI& uri);
		void setStackLabelPinned(const YSSCore::General::YSSURI& uri, bool pinned);
		void removeStackLabel(const YSSCore::General::YSSURI& uri);
		void setCurrentStackLabel(const YSSCore::General::YSSURI& uri);
		void setUseGlobalFocus(bool use);
		void setGlobalCurrentUri(const YSSCore::General::YSSURI& uri);
		YSSCore::General::YSSURI getCurrentSelected() const;
		void adjustScrollArea();
		void setFileChanged(const YSSCore::General::YSSURI& uri);
		void cancelFileChanged(const YSSCore::General::YSSURI& uri);
		bool containsStackLabel(const YSSCore::General::YSSURI& uri) const;
		bool isStackLabelPinned(const YSSCore::General::YSSURI& uri) const;
		QList<YSSCore::Editor::FileEditWidget*> getFileEditWidgets() const;
	public:
		virtual void wheelEvent(QWheelEvent* event) override;
		virtual void onThemeChanged() override;
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual void dragEnterEvent(QDragEnterEvent* event) override;
		virtual void dragMoveEvent(QDragMoveEvent* event) override;
		virtual void dropEvent(QDropEvent* event) override;
	private:
		void applyIcon(StackTag* label, const QColor& textColor, const QColor& accentColor);
		StackTag* findLabel(const YSSCore::General::YSSURI& uri) const;
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
#endif // YayinStoryStudio_Editor_MainEditor_private_StackComponents_p_h
