#ifndef YayinStoryStudio_Editor_PluginLoadFaultDialog_h
#define YayinStoryStudio_Editor_PluginLoadFaultDialog_h
#include <QtCore/qmap.h>
#include <QtWidgets/qdialog.h>
#include <General/PluginManager.h>
class QLabel;
class QTableWidget;
class QDialogButtonBox;
namespace YSS::Editor {
	class PluginLoadFaultDialog :public QDialog {
		Q_OBJECT;
	private:
		QLabel* PromptLabel;
		QTableWidget* FaultTable;
		QDialogButtonBox* ButtonBox;
	public:
		static bool isLoadFault(Visindigo::General::PluginManager::LoadPluginResult result);
		static bool hasLoadFault(const QMap<QString, Visindigo::General::PluginManager::LoadPluginResult>& loadResults);
	public:
		PluginLoadFaultDialog(const QMap<QString, Visindigo::General::PluginManager::LoadPluginResult>& loadResults, QWidget* parent = nullptr);
	};
}
#endif // YayinStoryStudio_Editor_PluginLoadFaultDialog_h
