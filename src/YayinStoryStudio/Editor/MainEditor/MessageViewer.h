#ifndef YSS_Editor_MainWin_MessageViewer_h
#define YSS_Editor_MainWin_MessageViewer_h
#include <QtWidgets/qframe.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qtablewidget.h>

namespace YSS::Editor {
	class MessageViewer :public QFrame {
		Q_OBJECT;
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
}
#endif // YSS_Editor_MainWin_MessageViewer_h
