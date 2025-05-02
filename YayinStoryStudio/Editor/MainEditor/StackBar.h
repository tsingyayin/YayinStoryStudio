#pragma once
#include <QtWidgets>
#include "StackBarLabel.h"
namespace YSS::Editor {
	class StackBar :public QWidget {
		Q_OBJECT;
	signals:
		void stackBarLabelChanged(StackBarLabel* label);
	private:
		QScrollArea* ScrollArea;
		QHBoxLayout* Layout;
		QWidget* ScrollAreaWidget;
		QList<StackBarLabel*> Labels;
		QList<QString> OpenedFiles;
		QMap<QString, StackBarLabel*> LabelMap;
		StackBarLabel* ActiveLabel = nullptr;
	public:
		StackBar(QWidget* parent);
		bool isLabelOpened(const QString& filePath);
		bool addLabel(StackBarLabel* label);
		void focusOn(const QString& filePath);
		void focus(StackBarLabel* label);
		void removeLabel(StackBarLabel* label);
	private:
		virtual void resizeEvent(QResizeEvent* event) override;
	};

	
}