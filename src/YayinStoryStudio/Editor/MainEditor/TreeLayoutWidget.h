#ifndef YayinStoryStudio_Editor_MainEditor_TreeLayoutWidget_h
#define YayinStoryStudio_Editor_MainEditor_TreeLayoutWidget_h
#include <QtWidgets/qframe.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>
#include <QtCore/qnamespace.h>
#include "Editor/MainEditor/FileEditWidgetArea.h"
#include <Utility/JsonConfig.h>
namespace YSS::Editor {
	class TreeLayoutWidgetPrivate;
	class TreeLayoutWidget :public QFrame {
		Q_OBJECT;
		friend class TreeLayoutWidgetPrivate;
	public:
		static QList<TreeLayoutWidget*> getAllTopLevelLayouts();
		static TreeLayoutWidget* getTopLayoutByID(const QString& topLayoutID);
	public:
		TreeLayoutWidget(QWidget* parent = nullptr, FileEditWidgetArea* firstArea = nullptr);
		virtual ~TreeLayoutWidget();
	public:
		FileEditWidgetArea* createFileEditAreaFirst();
		FileEditWidgetArea* createFileEditAreaLast();
		FileEditWidgetArea* createFileEditAreaAt(int index);
		TreeLayoutWidget* createLayoutFirst();
		TreeLayoutWidget* createLayoutLast();
		TreeLayoutWidget* createLayoutAt(int index);
		TreeLayoutWidget* replaceFileEditAt(int index, FileEditWidgetArea* newArea, bool up);
		TreeLayoutWidget* replaceLayoutAt(int index);
	public:
		FileEditWidgetArea* getFileEditAreaAt(int index) const;
		TreeLayoutWidget* getLayoutAt(int index) const;
		QList<bool> getIsLayoutList() const;
		int getChildCount() const;
		bool isTopLevel() const;
		QString getTopLayoutID() const;
		void setTopLayoutID(const QString& topLayoutID);
		Qt::Orientation getOrientation() const;
		bool isOrientationSelected() const;
		void setOrientation(Qt::Orientation orientation);
		void setChildRatios(const QList<int>& ratios);
	public:
		Visindigo::Utility::JsonConfig saveToJson() const;
		void recoverFromJson(const Visindigo::Utility::JsonConfig& json);
		QList<FileEditWidgetArea*> getAllFileEditAreas() const;
	protected:
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual void dragEnterEvent(QDragEnterEvent* event) override;
		virtual void dragMoveEvent(QDragMoveEvent* event) override;
		virtual void dragLeaveEvent(QDragLeaveEvent* event) override;
		virtual void dropEvent(QDropEvent* event) override;
		virtual bool eventFilter(QObject* watched, QEvent* event) override;
	private:
		TreeLayoutWidgetPrivate* d;
	};
}
#endif // YayinStoryStudio_Editor_MainEditor_TreeLayoutWidget_h
