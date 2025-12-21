#ifndef ASEDevTool_LangServer_ASEHoverInfoProvider_h
#define ASEDevTool_LangServer_ASEHoverInfoProvider_h
#include <Editor/HoverInfoProvider.h>
class ASRuleAdaptor;
class AStoryHoverInfoProvider : public YSSCore::Editor::HoverInfoProvider
{
	Q_OBJECT;
public:
	AStoryHoverInfoProvider(QTextDocument* doc);
	virtual void onMouseHover(const QString& text, int position) override;
private:
	ASRuleAdaptor* RuleAdaptor;
};
#endif // ASEDevTool_LangServer_ASEHoverInfoProvider_h