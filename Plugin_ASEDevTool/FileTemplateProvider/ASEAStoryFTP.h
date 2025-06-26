#pragma once
#include <Editor/FileTemplateProvider.h>
#include <Widgets/ConfigWidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLabel;

namespace YSSCore::General {
	class YSSProject;
}
class ASEAStoryFTIW :public YSSCore::Editor::FileTemplateInitWidget
{
	Q_OBJECT;
public:
	ASEAStoryFTIW(QWidget* parent = nullptr);

private:
};

class ASEAStoryFTP : public YSSCore::Editor::FileTemplateProvider
{
public:
	ASEAStoryFTP(YSSCore::Editor::EditorPlugin* plugin);
	virtual YSSCore::Editor::FileTemplateInitWidget* fileInitWidget() override;
};