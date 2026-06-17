#include "Editor/MainEditor/MultiTerminalTool.h"
#include <Widgets/Terminal.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qpushbutton.h>
#include "Editor/MainEditor/private/StackComponents_p.h"

namespace YSS::Editor {
	class MultiTerminalToolPrivate {
		friend class MultiTerminalTool;
	protected:
		static MultiTerminalTool* Instance;
		static QList<Visindigo::Widgets::Terminal*> Terminals; // terminal resource should not be released when q class deleted.
		static QStringList TerminalNames;
		QHBoxLayout* Layout;
		QPushButton* CreateButton;
		StackTagWidget* TagArea;
		QWidget* ContentArea = nullptr;
		DefaultStackWidgetCentralArea* CentralArea;
	};

	MultiTerminalTool* MultiTerminalToolPrivate::Instance = nullptr;
	QList<Visindigo::Widgets::Terminal*> MultiTerminalToolPrivate::Terminals = {};
	QStringList MultiTerminalToolPrivate::TerminalNames = {};

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
	void MultiTerminalTool::closeEvent(QCloseEvent* event) {
		
	}
}