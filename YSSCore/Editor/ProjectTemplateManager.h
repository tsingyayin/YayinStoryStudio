#pragma once
#include <QtCore/qobject.h>
#include "../Macro.h"
namespace YSSCore::Editor {
	class YSSCoreAPI ProjectTemplateManager :public QObject {
		Q_OBJECT;
	public:
		static ProjectTemplateManager* getInstance();
	};
};