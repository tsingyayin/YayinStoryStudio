#ifndef Visindigo_Widgets_PluginManageWidget_p_h
#define Visindigo_Widgets_PluginManageWidget_p_h
#include <QtWidgets/qframe.h>
#include <QtCore/qlist.h>
class QLabel;
class QCheckBox;
class QPushButton;
class QGridLayout;
class QHBoxLayout;
class QVBoxLayout;
namespace Visindigo::General {
	class Plugin;
}
namespace Visindigo::__Private__ {
	class Divider :public QFrame {
		Q_OBJECT;
	private:
		QLabel* TitleLabel;
		QLabel* DescriptionLabel;
		QVBoxLayout* Layout;
	public:
		Divider(const QString& title, const QString& description = QString(), QWidget* parent = nullptr);
		~Divider();
	};

	class PluginInfoPanel :public QFrame{
		Q_OBJECT;
	private:
		QLabel* NameLabel;
		QLabel* IconLabel;
		QLabel* AuthorLabel;
		QLabel* VersionLabel;
		QLabel* DateTimeLabel;
		QLabel* DescriptionLabel;
		QLabel* CheckBoxLabel;
		QCheckBox* ActiveCheckBox;
		QPushButton* OpenFolderButton;
		QGridLayout* Layout;
		Visindigo::General::Plugin* Plugin = nullptr;
		bool AsDependency = false;
	public:
		PluginInfoPanel(bool asDependency, QWidget* parent = nullptr);
		~PluginInfoPanel();
		void setPlugin(Visindigo::General::Plugin* plugin);
		virtual void resizeEvent(QResizeEvent* event) override;
	public slots:
		void onActiveStateChanged(bool checked);
		void onOpenFolderButtonClicked();
	};

	class DeactivatePluginInfoPanel :public QFrame {
		Q_OBJECT;
	private:
		QLabel* IDLabel;
		QPushButton* RemoveButton;
		QHBoxLayout* Layout;
	signals:
		void removePlugin(const QString& pluginID);
	public:
		DeactivatePluginInfoPanel(QWidget* parent = nullptr);
		~DeactivatePluginInfoPanel();
		void setPluginID(const QString& pluginID);
	};
}
#endif // Visindigo_Widgets_PluginManageWidget_p_h