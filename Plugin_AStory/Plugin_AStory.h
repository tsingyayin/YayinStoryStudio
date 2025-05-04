#pragma once
#include <Editor/EditorPlugin.h>

class Plugin_AStory : public YSSCore::Editor::EditorPlugin {
	Q_OBJECT;
public:
	Plugin_AStory();
	virtual void onPluginEnable() override;
	virtual void onPluginDisbale() override;
};