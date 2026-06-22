#include "Editor/MainEditor/MultiTerminalTool.h"
#include <Widgets/Terminal.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qpushbutton.h>
#include "Editor/MainEditor/private/StackComponents_p.h"
#include <General/TranslationHost.h>
#include <General/VIApplication.h>
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

		void initData() {
			auto builtinTerminal = VIApp->getVirtualTerminal();
			if (builtinTerminal) {
				builtinTerminal->setContentsMargins(0, 0, 0, 0);
				builtinTerminal->setParent(Instance);
				builtinTerminal->hide();
				TagArea->addStackLabel("builtin", "builtin");
				Instance->setCurrentTerminal("builtin");
			}
			for (int i = 0; i < TerminalNames.size(); i++) {
				auto terminal = Terminals[i];
				auto name = TerminalNames[i];
				terminal->setParent(Instance);
				TagArea->addStackLabel(name, name);
				Instance->setCurrentTerminal(name);
			}
		}
	};

	MultiTerminalTool* MultiTerminalToolPrivate::Instance = nullptr;
	QList<Visindigo::Widgets::Terminal*> MultiTerminalToolPrivate::Terminals = {};
	QStringList MultiTerminalToolPrivate::TerminalNames = {};

	MultiTerminalTool::MultiTerminalTool(QWidget* parent) :Visindigo::Widgets::BorderFrame(parent), d(new MultiTerminalToolPrivate()) {
		MultiTerminalToolPrivate::Instance = this;
		this->setContentsMargins(0, 0, 0, 0);
		d = new MultiTerminalToolPrivate;
		d->Layout = new QHBoxLayout(this);
		d->Layout->setSpacing(0);
		d->Layout->setContentsMargins(0, 0, 0, 0);
		d->TagArea = new StackTagWidget(this, Qt::Vertical);
		d->TagArea->setToolWidgetMode(true);
		d->TagArea->setFixedWidth(160);
		d->CentralArea = new DefaultStackWidgetCentralArea(this);
		d->CentralArea->setText(VITR("YSS::editor.toolWidgetArea.noToolWidget"));
		d->ContentArea = d->CentralArea;

		d->Layout->addWidget(d->CentralArea);
		d->Layout->addWidget(d->TagArea);

		this->setMinimumHeight(200);
		connect(d->TagArea, &StackTagWidget::switchToFile, this, [this](const QString& widgetID) {
			setCurrentTerminal(widgetID);
			});

		connect(d->TagArea, &StackTagWidget::closeFile, this, [this](const QString& widgetID) {
			if (widgetID != "builtin") {
				closeTerminal(widgetID);
				d->TagArea->removeStackLabel(widgetID);
			}
			});

		connect(d->TagArea, &StackTagWidget::closeAllRequested, this, [this]() {
			closeAll();
			});

		//Notice, if we add close dialog for terminal, this logic will be useful again.
		//Currently, closeFile signal from stackTagWidget equals to close terminal directly.
		/*connect(YSSTWM, &YSSCore::Editor::ToolWidgetManager::widgetClosed, this, [this](const QString& widgetID) {
			d->TagArea->removeStackLabel(widgetID);
			});*/

		d->initData();

		setColorfulEnable(true);
		onThemeChanged();
	}
	MultiTerminalTool::~MultiTerminalTool() {
		MultiTerminalToolPrivate::Instance = nullptr;
		// terminal resource should not be released when q class deleted.
		// final released by plugin onProjectClose.
		auto builtinTerminal = VIApp->getVirtualTerminal();
		if (builtinTerminal) {
			builtinTerminal->setContentsMargins(10, 10, 10, 10); // back to default.
			builtinTerminal->setParent(nullptr);
			builtinTerminal->hide();
		}
		for (auto terminal : MultiTerminalToolPrivate::Terminals) {
			terminal->setParent(nullptr);
			terminal->hide(); 
		}
		delete d;
	}
	// This function is nullable. This tool widget not have same survival guarantee with program. 
	// could disappear when it is not used.
	MultiTerminalTool* MultiTerminalTool::getInstance() {
		return MultiTerminalToolPrivate::Instance;
	}
	void MultiTerminalTool::addTerminal(const QString& name, const QString& command, const QDir& workingDir) {
		if (name == "builtin") {
			return;
		}
		auto terminal = new Visindigo::Widgets::Terminal(this);
		terminal->launchExternalProcess(command, {}, workingDir.path());
		MultiTerminalToolPrivate::Terminals.append(terminal);
		MultiTerminalToolPrivate::TerminalNames.append(name);
		d->TagArea->addStackLabel(name, name);
		setCurrentTerminal(name);
	}

	void MultiTerminalTool::closeTerminal(const QString& name) {
		if (name == "builtin") {
			return;
		}
		int index = MultiTerminalToolPrivate::TerminalNames.indexOf(name);
		if (index != -1) {
			auto terminal = MultiTerminalToolPrivate::Terminals[index];
			terminal->close();
			MultiTerminalToolPrivate::Terminals.removeAt(index);
			MultiTerminalToolPrivate::TerminalNames.removeAt(index);
		}
	}

	void MultiTerminalTool::closeAll() {
		for (auto terminal : MultiTerminalToolPrivate::Terminals) {
			terminal->close();
			terminal->deleteLater();
		}
		MultiTerminalToolPrivate::Terminals.clear();
		MultiTerminalToolPrivate::TerminalNames.clear();
	}

	bool MultiTerminalTool::containsTerminal(const QString& name) const {
		return MultiTerminalToolPrivate::TerminalNames.contains(name);
	}

	QStringList MultiTerminalTool::getTerminalNames() const {
		return MultiTerminalToolPrivate::TerminalNames;
	}

	QString MultiTerminalTool::getCurrentTerminalName() const {
		return d->TagArea->getCurrentSelected();
	}

	void MultiTerminalTool::setCurrentTerminal(const QString& name) {
		if (name.isEmpty()) {
			d->ContentArea->hide();
			d->Layout->removeWidget(d->ContentArea);
			d->ContentArea = d->CentralArea;
			d->Layout->insertWidget(0, d->ContentArea);
			d->ContentArea->show();
			return;
		}
		Visindigo::Widgets::Terminal* terminal = nullptr;
		if (name != "builtin") {
			terminal = MultiTerminalToolPrivate::Terminals[MultiTerminalToolPrivate::TerminalNames.indexOf(name)];
		}
		else {
			terminal = VIApp->getVirtualTerminal();
		}
		if (not terminal) return;
		if (d->ContentArea != terminal) {
			d->ContentArea->hide();
			d->Layout->removeWidget(d->ContentArea);
			d->ContentArea = terminal;
			d->Layout->insertWidget(0, d->ContentArea);
			d->ContentArea->show();
		}
		emit currentTerminalChanged(name);
		d->TagArea->setCurrentStackLabel(name);
	}
	void MultiTerminalTool::onThemeChanged() {

	}
	void MultiTerminalTool::resizeEvent(QResizeEvent* event) {

	}
	void MultiTerminalTool::closeEvent(QCloseEvent* event) {
		
	}
}