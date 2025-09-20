#pragma once
#include <QtWidgets/qframe.h>
#include "../Macro.h"
// Forward declarations
namespace Visindigo::__Private__ {
	class ConfigWidgetPrivate;
}
namespace Visindigo::Utility {
	class JsonConfig;
}
// Main
namespace Visindigo::Widgets {
	class VisindigoAPI ConfigWidget :public QFrame {
		friend class Visindigo::__Private__::ConfigWidgetPrivate;
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
		Visindigo::Utility::JsonConfig* getConfig();
	private:
		Visindigo::__Private__::ConfigWidgetPrivate* d;
	};
}