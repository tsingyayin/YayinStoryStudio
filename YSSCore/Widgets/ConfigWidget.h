#pragma once
#include <QtWidgets/qframe.h>
#include "../Macro.h"
// Forward declarations
namespace YSSCore::__Private__ {
	class ConfigWidgetPrivate;
}
namespace YSSCore::Utility {
	class JsonConfig;
}
// Main
namespace YSSCore::Widgets {
	class YSSCoreAPI ConfigWidget :public QFrame {
		friend class YSSCore::__Private__::ConfigWidgetPrivate;
		Q_OBJECT;
	signals:
		void radioButtonChanged(const QString& node, bool checked);
		void comboBoxIndexChanged(const QString& node, int index, QString data);
		void lineEditTextChanged(const QString& node, const QString& text);
	public:
		ConfigWidget(QWidget* parent = nullptr);
		virtual ~ConfigWidget();
		void loadCWJson(const QString& CWJson);
		void resetConfig();
		void saveConfig();
		void setLineEditText(const QString& node, const QString& text);
		void setComboBoxIndex(const QString& node, int index);
		void setRadioButtonChecked(const QString& node, bool checked);
		YSSCore::Utility::JsonConfig* getConfig();
	private:
		YSSCore::__Private__::ConfigWidgetPrivate* d;
	};
}