#include "Editor/MainEditor/ToolWidgetArea.h"
#include "Editor/MainEditor/private/StackComponents_p.h"
namespace YSS::Editor {
	class ToolWidgetAreaPrivate {
		friend class ToolWidgetArea;
	protected:
		StackTagWidget* TagArea;
		QFrame* ContentArea = nullptr;
		QVBoxLayout* Layout;
		DefaultStackWidgetCentralArea* CentralArea;
	};
}