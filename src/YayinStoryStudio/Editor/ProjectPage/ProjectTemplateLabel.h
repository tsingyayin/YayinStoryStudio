#pragma once
#include <QFrame>
#include <Editor/ProjectTemplateProvider.h>
class QLabel;
class QHBoxLayout;
class QGridLayout;

namespace YSS::ProjectPage {
	class ProjectTemplateLabel :public QFrame {
		Q_OBJECT;
	private:
		QLabel* TemplateIcon = nullptr;
		QLabel* TemplateName = nullptr;
		QLabel* TemplateDescription = nullptr;
		QList<QLabel*> TagLabels;
		QHBoxLayout* TagsLayout = nullptr;
		QGridLayout* Layout = nullptr;
	public:
		ProjectTemplateLabel(QWidget* parent = nullptr);
	};
}