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
	YSSCore::Widgets::ConfigWidget* ConfigWidget = nullptr;
	QLabel* WhereLabel = nullptr;
	QPushButton* CreateButton = nullptr;
	QVBoxLayout* Layout = nullptr;
	QHBoxLayout* ButtonLayout = nullptr;
	QString FilePath;
	QString FileName;
public:
	ASEAStoryFTIW(const QString& initFolder, QWidget* parent = nullptr);
	virtual void resizeEvent(QResizeEvent* event) override;
public slots:
	void onLineEditTextChanged(const QString& node, const QString& text);
	void onCreateButtonClicked();
	QStringList initFileV2_05_22_1A();
private:
	void refreshWhereLabel();
};

class ASEAStoryFTP : public YSSCore::Editor::FileTemplateProvider
{
public:
	ASEAStoryFTP(YSSCore::Editor::EditorPlugin* plugin);
	virtual YSSCore::Editor::FileTemplateInitWidget* fileInitWidget(const QString& initPath) override;
};