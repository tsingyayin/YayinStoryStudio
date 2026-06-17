#ifndef YSS_MainEditor_MultiTerminalTool_h
#define YSS_MainEditor_MultiTerminalTool_h
#include <QtCore/qdir.h>
#include <QtWidgets/qwidget.h>
#include <Widgets/ThemeManager.h>
#include <Widgets/BorderFrame.h>
#include <Widgets/Terminal.h>

namespace YSS::Editor {
	class MultiTerminalToolPrivate;
	class MultiTerminalTool :public Visindigo::Widgets::BorderFrame, public Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
	public:
		MultiTerminalTool(QWidget* parent = nullptr);
		~MultiTerminalTool();
		static MultiTerminalTool* getInstance();
		void addTerminal(const QString& name, const QString& command, const QDir& workingDir = QDir::homePath());
		void switchTo(const QString& name);
		QStringList getTerminalNames() const;
	public:
		virtual void onThemeChanged() override;
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual void closeEvent(QCloseEvent* event) override;
	private:
		MultiTerminalToolPrivate* d;
	};
}
#endif // YSS_MainEditor_MultiTerminalTool_h