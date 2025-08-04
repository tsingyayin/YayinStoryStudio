#pragma once
#include <Editor/ProjectTemplateProvider.h>
#include <Widgets/ConfigWidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLabel;

namespace Visindigo::General {
	class YSSProject;
}
class ASEAStoryPTIW :public Visindigo::Editor::ProjectTemplateInitWidget
{
	Q_OBJECT;
	Visindigo::Widgets::ConfigWidget* ConfigWidget;
	QLabel* WhereLabel;
	QPushButton* CreateButton;
	QVBoxLayout* Layout;
	QHBoxLayout* ButtonLayout;
	QString ProjectPath;
	QString ProjectName;
public:
	ASEAStoryPTIW(QWidget* parent = nullptr);
	virtual void resizeEvent(QResizeEvent* event) override;
public slots:
	void onLineEditTextChanged(const QString& node, const QString& str);
	void onCreateButtonClicked();
	void initResourceV2_05_22_1A(Visindigo::General::YSSProject* project);
private:
	void refreshWhereLabel();
};

class ASEAStoryPTP : public Visindigo::Editor::ProjectTemplateProvider
{
public:
	ASEAStoryPTP(Visindigo::Editor::EditorPlugin* plugin);
	virtual Visindigo::Editor::ProjectTemplateInitWidget* projectInitWidget() override;
};