#pragma once
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
class QString;
class QVBoxLayout;
class QWidget;
class QRadioButton;
class QComboBox;
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
		QVBoxLayout* Layout;
		QString getTr(QString raw);
		YSSCore::Utility::JsonConfig* Config = nullptr;
		QMap<QComboBox*, QString> ComboBoxDefault;
		QMap<QRadioButton*, bool> RadioButtonDefault;
		ConfigWidgetPrivate();
		~ConfigWidgetPrivate();
		void loadConfig();
		void saveConfig();
		void backToDefaultConfig();
		QList<QWidget*> spawnWidget(QList<YSSCore::Utility::JsonConfig>& config);
		QWidget* widgetSpawner(YSSCore::Utility::JsonConfig& config, const QString& parentPath = "");
		QWidget* widgetRouter(const QString& type, const QString& node, YSSCore::Utility::JsonConfig& config);
		QWidget* widget_QFrame(const QString& node, YSSCore::Utility::JsonConfig& config);
		QWidget* widget_ComboBox(const QString& node, YSSCore::Utility::JsonConfig& config);
		QWidget* widget_RadioButton(const QString& node, YSSCore::Utility::JsonConfig& config);
	private slots:
		void onComboBoxIndexChanged(int index);
		void onRadioButtonChanged(bool checked);
	};
}