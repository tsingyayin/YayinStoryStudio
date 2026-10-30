#ifndef YayinStoryStudio_Editor_PluginLoadFaultDialog_h
#define YayinStoryStudio_Editor_PluginLoadFaultDialog_h
#include <QtCore/qmap.h>
#include <QtWidgets/qdialog.h>
#include <General/PluginManager.h>
class QLabel;
class QPushButton;
class QTableWidget;
namespace YSS::Editor {
	class PluginLoadFaultDialog :public QDialog {
		Q_OBJECT;
	private:
		QLabel* TitleLabel;
		QLabel* SubtitleLabel;
		QLabel* CountLabel;
		QTableWidget* FaultTable;
		QPushButton* ContinueButton;
		QPushButton* QuitButton;
	public:
		static bool isLoadFault(Visindigo::General::PluginManager::LoadPluginResult result);
		static bool hasLoadFault(const QMap<QString, Visindigo::General::PluginManager::LoadPluginResult>& loadResults);
	public:
		PluginLoadFaultDialog(const QMap<QString, Visindigo::General::PluginManager::LoadPluginResult>& loadResults, QWidget* parent = nullptr);
	protected:
		virtual void showEvent(QShowEvent* event) override;
	private:
		void buildUi(const QMap<QString, Visindigo::General::PluginManager::LoadPluginResult>& loadResults);
		void applyStyle();
	};
}
#endif // YayinStoryStudio_Editor_PluginLoadFaultDialog_h
