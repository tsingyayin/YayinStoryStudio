#ifndef YSS_MainEditor_TreeLayoutWidget_h
#define YSS_MainEditor_TreeLayoutWidget_h
#include <QtWidgets/qsplitter.h>
#include "Editor/MainEditor/FileEditWidgetArea.h"
#include <Utility/JsonConfig.h>
namespace YSS::Editor {
	class TreeLayoutWidgetPrivate;
	class TreeLayoutWidget :public QSplitter {
		Q_OBJECT;
	public:
		TreeLayoutWidget(Qt::Orientation orientation, QWidget* parent = nullptr) {};
		virtual ~TreeLayoutWidget() {};
	public:
		FileEditWidgetArea* createFileEditAreaFirst();
		FileEditWidgetArea* createFileEditAreaLast();
		FileEditWidgetArea* createFileEditAreaAt(int index);
		TreeLayoutWidget* replaceFileEdittAt(int index, FileEditWidgetArea* newArea, bool up); // replace raw area with a new layout, include raw one and a new one, up means the new area is above the raw one
		TreeLayoutWidget* replaceLayoutAt(int index); //replace raw layout with it`s child area. The layout must only have one area left, otherwise it will do nothing and return nullptr.
		TreeLayoutWidget* createLayoutFirst();
		TreeLayoutWidget* createLayoutLast();
		TreeLayoutWidget* createLayoutAt(int index);
	public:
		Visindigo::Utility::JsonConfig saveToJson() const;
		void recoverFromJson(const Visindigo::Utility::JsonConfig& json);
		QList<FileEditWidgetArea*> getAllFileEditAreas() const;
	private:
		TreeLayoutWidgetPrivate* d;
	};
}
#endif // YSS_MainEditor_TreeLayoutWidget_h