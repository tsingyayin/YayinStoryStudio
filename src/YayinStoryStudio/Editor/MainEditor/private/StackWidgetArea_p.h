#ifndef YSS_Editor_MainWin_StackWidgetArea_p_H
#define YSS_Editor_MainWin_StackWidgetArea_p_H
#include <QtWidgets/qframe.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qtablewidget.h>
#include <Widgets/ThemeManager.h>
namespace YSS::Editor {
	class StackWidgetTagLabel :public QFrame {
		Q_OBJECT;
	private:
		QLabel* TitleLabel;
		QPushButton* PinLabel;
		QPushButton* CloseLabel;
		QFrame* Separator;
		QHBoxLayout* Layout;
		QString FilePath;
		QString NormalStyle;
		QString HoverStyle;
		QString PressedStyle;
		bool Focused = false;
		bool Pinned = false;
		bool Pressed = false;
	signals:
		void clicked(const QString& filePath);
		void pinClicked(const QString& filePath);
		void closeClicked(const QString& filePath);
	public:
		StackWidgetTagLabel(QWidget* parent = nullptr);
		void setText(const QString& text);
		void setFilePath(const QString& filePath);
		QString getFilePath() const;
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

	class StackWidgetTagArea :public QFrame ,public Visindigo::Widgets::ColorfulWidget{
		Q_OBJECT;
	signals:
		void switchToFile(const QString& filePath);
		void closeFile(const QString& filePath);
	private:
		QHBoxLayout* ContentLayout;
		QFrame* ScrollContent;
		QScrollArea* ScrollArea;
		QComboBox* WidgetSelector;
		QHBoxLayout* Layout;
		QList<StackWidgetTagLabel*> Labels;
		QString CurrentSelected;
	public:
		StackWidgetTagArea(QWidget* parent = nullptr);
		virtual ~StackWidgetTagArea();
		void addStackLabel(const QString& filePath);
		void pinStackLabel(const QString& filePath);
		void removeStackLabel(const QString& filePath);
		void setCurrentStackLabel(const QString& filePath);
		QString getCurrentSelected() const;
		void adjustScrollArea();
	public:
		virtual void wheelEvent(QWheelEvent* event) override;
		virtual void onThemeChanged() override;
		virtual void resizeEvent(QResizeEvent* event) override;
	};

	class MessageViewer :public QFrame {
		Q_OBJECT;
	signals:
		void redirectionRequired(const QString& filePath, qint32 lineNumber, qint32 column);
	private:
		QVBoxLayout* Layout;
		QTableWidget* MessageTable;
		QString CurrentFilePath;
	public:
		MessageViewer(QWidget* parent = nullptr);
		void changeCurrentFile(const QString& filePath);
	public slots:
		void onCellClicked(int row, int column);
		void onMessageChanged(const QString& filePath);
		void onMessageChangedForLine(const QString& filePath, qint32 lineNumber);
	};

	class DefaultStackWidgetCentralArea :public QFrame {
		Q_OBJECT;
	private:
		QLabel* ContentLabel;
		QGridLayout* Layout;
	public:
		DefaultStackWidgetCentralArea(QWidget* parent = nullptr);
	};
}
#endif // YSS_Editor_MainWin_StackWidgetArea_p_H
