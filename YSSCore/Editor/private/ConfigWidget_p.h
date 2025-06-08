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

namespace YSSCore::Utility {
	class JsonConfig;
}
namespace YSSCore::Editor {
	class ConfigWidget;
}
namespace YSSCore::__Private__ {
	class ConfigWidgetPrivate :public QObject{
		Q_OBJECT;
		friend class YSSCore::Editor::ConfigWidget;
	protected:
		QString TargetConfigPath;
		QVBoxLayout* Layout = nullptr;
		YSSCore::Utility::JsonConfig* Config = nullptr;
		QMap<QComboBox*, QString> ComboBoxDefault;
		QMap<QRadioButton*, bool> RadioButtonDefault;
		QMap<QLineEdit*, QString> LineEditDefault;
		QList<QWidget*> Settings;
		QWidget* self;
		ConfigWidgetPrivate(QWidget* self);
		~ConfigWidgetPrivate();
		void loadCWJson(const QString& json);
		void initConfig();
		void syncConfig();
		void resetConfig();
		void spawnConfig();
		void saveConfig();
		QList<QWidget*> spawnWidget(QList<YSSCore::Utility::JsonConfig>& config);
		QWidget* widgetSpawner(YSSCore::Utility::JsonConfig& config, const QString& parentPath = "");
		QWidget* widgetRouter(const QString& type, const QString& node, YSSCore::Utility::JsonConfig& config);
		QWidget* widget_QFrame(const QString& node, YSSCore::Utility::JsonConfig& config);
		QWidget* widget_ComboBox(const QString& node, YSSCore::Utility::JsonConfig& config);
		QWidget* widget_RadioButton(const QString& node, YSSCore::Utility::JsonConfig& config);
		QWidget* widget_LineEdit(const QString& node, YSSCore::Utility::JsonConfig& config);
	private slots:
		void onComboBoxIndexChanged(int index);
		void onRadioButtonChanged(bool checked);
		void onLineEditTextChanged(QString str);
	};
}