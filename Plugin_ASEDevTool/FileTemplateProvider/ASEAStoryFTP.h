#pragma once
#include <Editor/FileTemplateProvider.h>
#include <Widgets/ConfigWidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLabel;

namespace Visindigo::General {
	class YSSProject;
}
class ASEAStoryFTIW :public Visindigo::Editor::FileTemplateInitWidget
{
	Q_OBJECT;
	Visindigo::Widgets::ConfigWidget* ConfigWidget = nullptr;
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

class ASEAStoryFTP : public Visindigo::Editor::FileTemplateProvider
{
public:
	ASEAStoryFTP(Visindigo::Editor::EditorPlugin* plugin);
	virtual Visindigo::Editor::FileTemplateInitWidget* fileInitWidget(const QString& initPath) override;
};