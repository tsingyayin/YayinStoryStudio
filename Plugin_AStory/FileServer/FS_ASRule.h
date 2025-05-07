#pragma once
#include <Editor/FileServer.h>
#include <Editor/FileEditWidget.h>
#include <QVBoxLayout>
class ASRuleFileServer : public YSSCore::Editor::FileServer {
public:
	ASRuleFileServer();
	~ASRuleFileServer();
	YSSCore::Editor::FileEditWidget* createFileEditWidget() override;
};

class ASRuleFileEditWidget : public YSSCore::Editor::FileEditWidget {
public:
	ASRuleFileEditWidget(QWidget* parent = nullptr);
	~ASRuleFileEditWidget();
	virtual QString getFilePath() const;
	virtual QString getFileName() const;
	virtual bool openFile(const QString& path);
	virtual bool onClose();
	virtual bool onSave(const QString& path = "");
	virtual bool onSaveAs(const QString& path = "");
	virtual bool onReload();
private:
	QString FilePath;
	QVBoxLayout* Layout;
private:
	void generateContent(QTextStream* ts);
};