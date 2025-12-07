#pragma once
#include <Editor/DebugServer.h>

class DS_ASEPrivate;
class DS_ASE :public YSSCore::Editor::DebugServer {
public:
	DS_ASE(YSSCore::Editor::EditorPlugin* plugin);
	virtual ~DS_ASE();
	virtual void onBuild() override;
	virtual void onClear() override;
	virtual void onDebugStart() override;
	virtual void onPause() override;
	virtual void onContinue() override;
	virtual void onStop(bool resume = false) override;
	virtual void onRun() override;
	virtual void nextStep() override;
	virtual void nextProcess() override;
	virtual QWidget* getDebugSettingsWidget(QWidget* parent = nullptr) override;
public slots:
	void onASEStdOutput();
	void onASEErrOutput();
	void ASEStdInput(const QString& input);
private:
	DS_ASEPrivate* d;
};