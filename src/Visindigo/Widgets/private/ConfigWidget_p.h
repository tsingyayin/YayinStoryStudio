#pragma once
#include <QtGui/qcolor.h>
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include "Utility/JsonConfig.h"
class QString;
class QVBoxLayout;
class QWidget;
class QRadioButton;
class QComboBox;
class QLineEdit;
class QTextEdit;
class QLabel;
class QPushButton;
class QHBoxLayout;
namespace Visindigo::Widgets {
	class ConfigWidget;
}

namespace Visindigo::__Private__ {
	class ConfigWidgetPrivate :public QObject {
		Q_OBJECT;
		friend class Visindigo::Widgets::ConfigWidget;
	protected:
		QString TargetConfigPath;
		QString TargetConfigNode;
		QVBoxLayout* Layout = nullptr;
		Visindigo::Utility::JsonConfig Config = Visindigo::Utility::JsonConfig();
		QMap<QComboBox*, QString> ComboBoxDefault;
		QMap<QRadioButton*, bool> RadioButtonDefault;
		QMap<QLineEdit*, QString> LineEditDefault;
		QMap<QTextEdit*, QString> TextEditDefault;
		QMap<QLabel*, QString> ColorDialogDefault;
		QList<QWidget*> SettingsWidget;
		QWidget* ButtonWidget = nullptr;
		QPushButton* ResetButton = nullptr;
		QPushButton* SaveButton = nullptr;
		QHBoxLayout* ButtonLayout = nullptr;
		Visindigo::Widgets::ConfigWidget* self;
		bool IndependentMode = false;
		bool saveNeedRestart = false;
		ConfigWidgetPrivate(Visindigo::Widgets::ConfigWidget* self);
		~ConfigWidgetPrivate();
		void loadCWJson(const QString& json);
		void initConfig();
		void syncConfig();
		void resetConfig();
		void spawnConfig();
		void saveConfig();
		QList<QWidget*> spawnWidget(QList<Visindigo::Utility::JsonConfig>& config);
		QWidget* widgetSpawner(Visindigo::Utility::JsonConfig& config, const QString& parentPath = "");
		QWidget* widgetRouter(const QString& type, const QString& node, Visindigo::Utility::JsonConfig& config, bool readOnly);
		QWidget* widget_QFrame(const QString& node, Visindigo::Utility::JsonConfig& config, bool readOnly);
		QWidget* widget_ComboBox(const QString& node, Visindigo::Utility::JsonConfig& config, bool readOnly);
		QWidget* widget_RadioButton(const QString& node, Visindigo::Utility::JsonConfig& config, bool readOnly);
		QWidget* widget_LineEdit(const QString& node, Visindigo::Utility::JsonConfig& config, bool readOnly);
		QWidget* widget_TextEdit(const QString& node, Visindigo::Utility::JsonConfig& config, bool readOnly);
		QWidget* widget_ColorDialog(const QString& node, Visindigo::Utility::JsonConfig& config, bool readOnly);
	private slots:
		void onComboBoxIndexChanged(int index);
		void onRadioButtonChanged(bool checked);
		void onLineEditTextChanged(QString str);
		void onTextEditTextChanged();
		void onColorChanged(const QColor& clr);
	};
}