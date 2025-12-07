#pragma once
#include <Editor/ProjectTemplateProvider.h>
#include <Widgets/ConfigWidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLabel;

namespace YSSCore::General {
	class YSSProject;
}
class ASEAStoryPTIW :public YSSCore::Editor::ProjectTemplateInitWidget
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
	void initResourceV2_05_22_1A(YSSCore::General::YSSProject* project);
private:
	void refreshWhereLabel();
};

class ASEAStoryPTP : public YSSCore::Editor::ProjectTemplateProvider
{
public:
	ASEAStoryPTP(YSSCore::Editor::EditorPlugin* plugin);
	virtual YSSCore::Editor::ProjectTemplateInitWidget* projectInitWidget() override;
};