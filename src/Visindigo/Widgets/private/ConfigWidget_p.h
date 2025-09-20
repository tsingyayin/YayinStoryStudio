#pragma once
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
class QString;
class QVBoxLayout;
class QWidget;
class QRadioButton;
class QComboBox;
class QLineEdit;

namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::Widgets {
	class ConfigWidget;
}
namespace Visindigo::__Private__ {
	class ConfigWidgetPrivate :public QObject {
		Q_OBJECT;
		friend class Visindigo::Widgets::ConfigWidget;
	protected:
		QString TargetConfigPath;
		QVBoxLayout* Layout = nullptr;
		Visindigo::Utility::JsonConfig* Config = nullptr;
		QMap<QComboBox*, QString> ComboBoxDefault;
		QMap<QRadioButton*, bool> RadioButtonDefault;
		QMap<QLineEdit*, QString> LineEditDefault;
		QList<QWidget*> Settings;
		Visindigo::Widgets::ConfigWidget* self;
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
		QWidget* widgetRouter(const QString& type, const QString& node, Visindigo::Utility::JsonConfig& config);
		QWidget* widget_QFrame(const QString& node, Visindigo::Utility::JsonConfig& config);
		QWidget* widget_ComboBox(const QString& node, Visindigo::Utility::JsonConfig& config);
		QWidget* widget_RadioButton(const QString& node, Visindigo::Utility::JsonConfig& config);
		QWidget* widget_LineEdit(const QString& node, Visindigo::Utility::JsonConfig& config);
	private slots:
		void onComboBoxIndexChanged(int index);
		void onRadioButtonChanged(bool checked);
		void onLineEditTextChanged(QString str);
	};
}