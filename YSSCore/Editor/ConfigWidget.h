#pragma once
#include <QtCore/qobject.h>
#include <QtWidgets/qframe.h>
#include "../Macro.h"

namespace YSSCore::Editor {
	class YSSCoreAPI ConfigWidget :public QFrame {
		Q_OBJECT;
	public:
		ConfigWidget(QWidget* parent = nullptr);
		~ConfigWidget();
		void setConfig(const QString& jsonStr);
	};

}