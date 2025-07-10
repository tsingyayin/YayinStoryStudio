#pragma once
#include <Editor/FileServer.h>
#include <Editor/FileEditWidget.h>
#include <Editor/EditorPlugin.h>
#include <QVBoxLayout>
class ASRuleFileServer : public YSSCore::Editor::FileServer {
public:
	ASRuleFileServer(YSSCore::Editor::EditorPlugin* plugin);
	~ASRuleFileServer();
	YSSCore::Editor::FileEditWidget* onCreateFileEditWidget() override;
};

class ASRuleFileEditWidget : public YSSCore::Editor::FileEditWidget {
public:
	ASRuleFileEditWidget(QWidget* parent = nullptr);
	~ASRuleFileEditWidget();
	virtual bool onOpen(const QString& path);
	virtual bool onClose();
	virtual bool onSave(const QString& path = "");
	virtual bool onReload();
private:
	QVBoxLayout* Layout;
private:
	void generateContent(QTextStream* ts);
};