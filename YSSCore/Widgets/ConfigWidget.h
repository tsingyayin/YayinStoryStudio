#pragma once
#include <QtCore/qobject.h>
#include <QtWidgets/qframe.h>
#include "../Macro.h"

namespace YSSCore::__Private__ {
	class ConfigWidgetPrivate;
}
namespace YSSCore::Widgets {
	class ConfigWidgetPrivate;
	class YSSCoreAPI ConfigWidget :public QFrame {
		friend class YSSCore::__Private__::ConfigWidgetPrivate;
		Q_OBJECT;
	public:
		ConfigWidget(QWidget* parent = nullptr);
		virtual ~ConfigWidget();
		void loadCWJson(const QString& CWJson);
		void resetConfig();
		void saveConfig();
	private:
		YSSCore::__Private__::ConfigWidgetPrivate* d;
	};

}