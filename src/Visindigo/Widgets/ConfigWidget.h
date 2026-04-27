#pragma once
#include <QtGui/qcolor.h>
#include <QtWidgets/qframe.h>
#include "../VICompileMacro.h"
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
		void reseted();
		void saved();
		void radioButtonChanged(const QString& node, bool checked);
		void comboBoxIndexChanged(const QString& node, int index, QString data);
		void lineEditTextChanged(const QString& node, const QString& text);
		void textEditTextChanged(const QString& node, const QString& text);
		void colorChanged(const QString& node, const QColor& clr);
	public:
		ConfigWidget(QWidget* parent = nullptr);
		virtual ~ConfigWidget();
		void setTargetConfig(const QString& path, const QString& node = "", const QString& fileType = "json");
		void loadCWJson(const QString& CWJson);
		void resetConfig();
		void syncConfig();
		void saveConfig();
		void setLineEditText(const QString& node, const QString& text);
		void setComboBoxIndex(const QString& node, int index);
		void setRadioButtonChecked(const QString& node, bool checked);
		void setIndependentMode(bool independent, bool saveNeedRestart = false);
		bool isIndependentMode() const;
		bool isSaveNeedRestart() const;
		Visindigo::Utility::JsonConfig* getConfig();
	private:
		Visindigo::__Private__::ConfigWidgetPrivate* d;
	};
}