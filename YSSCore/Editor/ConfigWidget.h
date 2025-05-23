#pragma once
#include <QtCore/qobject.h>
#include <QtWidgets/qframe.h>
#include "../Macro.h"

namespace YSSCore::__Private__ {
	class ConfigWidgetPrivate;
}
namespace YSSCore::Editor {
	class ConfigWidgetPrivate;
	class YSSCoreAPI ConfigWidget :public QFrame {
		friend class YSSCore::__Private__::ConfigWidgetPrivate;
		Q_OBJECT;
	public:
		ConfigWidget(QWidget* parent = nullptr);
		virtual ~ConfigWidget();
		bool setConfig(const QString& jsonPath);
	private:
		YSSCore::__Private__::ConfigWidgetPrivate* d;
	};

}