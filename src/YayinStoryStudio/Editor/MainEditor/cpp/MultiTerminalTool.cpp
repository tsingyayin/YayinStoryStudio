#include "Editor/MainEditor/MultiTerminalTool.h"
#include <Widgets/Terminal.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qpushbutton.h>
namespace YSS::Editor {
	class MultiTerminalToolPrivate {
		friend class MultiTerminalTool;
	protected:
		QList<Visindigo::Widgets::Terminal*> Terminals;
		QStringList TerminalNames;
		QVBoxLayout* Layout;
		QComboBox* TerminalSelector;
		QPushButton* CreateButton;
		static MultiTerminalTool* Instance;
	};

	MultiTerminalTool* MultiTerminalToolPrivate::Instance = nullptr;

	MultiTerminalTool::MultiTerminalTool(QWidget* parent) :Visindigo::Widgets::BorderFrame(parent), d(new MultiTerminalToolPrivate()) {
		MultiTerminalToolPrivate::Instance = this;
		setColorfulEnable(true);
		onThemeChanged();
	}
	MultiTerminalTool::~MultiTerminalTool() {
		MultiTerminalToolPrivate::Instance = nullptr;
		delete d;
	}
	// This function is nullable. This tool widget not have same survival guarantee with program. 
	// could disappear when it is not used.
	MultiTerminalTool* MultiTerminalTool::getInstance() {
		return MultiTerminalToolPrivate::Instance;
	}
	void MultiTerminalTool::addTerminal(const QString& name, const QString& command, const QDir& workingDir) {

	}
	void MultiTerminalTool::switchTo(const QString& name) {

	}
	QStringList MultiTerminalTool::getTerminalNames() const {
		return {};
	}
	void MultiTerminalTool::onThemeChanged() {

	}
	void MultiTerminalTool::resizeEvent(QResizeEvent* event) {

	}
}