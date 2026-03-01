#ifndef YayinStoryPlayer_StoryLine_StoryAction_h
#define YayinStoryPlayer_StoryLine_StoryAction_h
#include <QtCore/qtypes.h>
class QString;

namespace YSP {
	class StoryAction {
	public:
		StoryAction() {};
		qint32 getRawIndex() const;
		QString getRawContent() const;
		bool isRunning() const;
		bool isActionDependOnPrevious() const;
		void setActionDependOnPrevious(bool depend);
	public:
		virtual qint32 getActionFrames() const;
		virtual void onActionStart() {};
		virtual void onActionRunning(qint32 frameIndex) {};
		virtual void onActionEnd() {};
	};
}
#endif // YayinStoryPlayer_StoryLine_StoryAction_h
