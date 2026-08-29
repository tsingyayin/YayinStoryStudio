#include "Editor/MainEditor/MultiTerminal.h"
#include <Widgets/Terminal.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qpushbutton.h>
#include "Editor/MainEditor/private/StackComponents_p.h"
#include <General/TranslationHost.h>
#include <General/VIApplication.h>
namespace YSS::Editor {
	class MultiTerminalPrivate {
		friend class MultiTerminal;
	protected:
		static MultiTerminal* Instance;
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

	MultiTerminal* MultiTerminalPrivate::Instance = nullptr;
	QList<Visindigo::Widgets::Terminal*> MultiTerminalPrivate::Terminals = {};
	QStringList MultiTerminalPrivate::TerminalNames = {};

	MultiTerminalVFS::MultiTerminalVFS(YSSCore::Editor::EditorPlugin* plugin) :
		YSSCore::Editor::FileServer("YSS Built-in Multi Terminal", "cn.yxgeneral.yss_builtin.multiTerminalVFS", plugin) {
		setEditorType(YSSCore::Editor::FileServer::BuiltInEditor);
		setSupportedFileExts({ "YSS.MainEditor.MultiTerminal" });
		setAsVitrualFileServer(true);
		setPreferredOrientation(YSSCore::Editor::FileServer::Horizontal_Wide);
		setListAsTool(true);
		setToolNickname("i18n:YSS::editor.multiTerminal.title");
	}

	YSSCore::Editor::FileEditWidget* MultiTerminalVFS::onCreateFileEditWidget() {
		return new MultiTerminal();
	}

	MultiTerminal::MultiTerminal(QWidget* parent) :YSSCore::Editor::FileEditWidget(parent) {
		MultiTerminalPrivate::Instance = this;
		this->setContentsMargins(0, 0, 0, 0);
		d = new MultiTerminalPrivate;
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

		d->initData();
	}
	MultiTerminal::~MultiTerminal() {
		MultiTerminalPrivate::Instance = nullptr;
		// terminal resource should not be released when q class deleted.
		// final released by plugin onProjectClose.
		auto builtinTerminal = VIApp->getVirtualTerminal();
		if (builtinTerminal) {
			builtinTerminal->setContentsMargins(10, 10, 10, 10); // back to default.
			builtinTerminal->setParent(nullptr);
			builtinTerminal->hide();
		}
		for (auto terminal : MultiTerminalPrivate::Terminals) {
			terminal->setParent(nullptr);
			terminal->hide();
		}
		delete d;
	}
	// This function is nullable. This tool widget not have same survival guarantee with program. 
	// could disappear when it is not used.
	MultiTerminal* MultiTerminal::getInstance() {
		return MultiTerminalPrivate::Instance;
	}
	void MultiTerminal::addTerminal(const QString& name, const QString& command, const QDir& workingDir) {
		if (name == "builtin") {
			return;
		}
		auto terminal = new Visindigo::Widgets::Terminal(this);
		terminal->launchExternalProcess(command, {}, workingDir.path());
		MultiTerminalPrivate::Terminals.append(terminal);
		MultiTerminalPrivate::TerminalNames.append(name);
		d->TagArea->addStackLabel(name, name);
		setCurrentTerminal(name);
	}

	void MultiTerminal::closeTerminal(const QString& name) {
		if (name == "builtin") {
			return;
		}
		int index = MultiTerminalPrivate::TerminalNames.indexOf(name);
		if (index != -1) {
			auto terminal = MultiTerminalPrivate::Terminals[index];
			terminal->close();
			MultiTerminalPrivate::Terminals.removeAt(index);
			MultiTerminalPrivate::TerminalNames.removeAt(index);
		}
	}

	void MultiTerminal::closeAll() {
		for (auto terminal : MultiTerminalPrivate::Terminals) {
			terminal->close();
			terminal->deleteLater();
		}
		MultiTerminalPrivate::Terminals.clear();
		MultiTerminalPrivate::TerminalNames.clear();
	}

	bool MultiTerminal::containsTerminal(const QString& name) const {
		return MultiTerminalPrivate::TerminalNames.contains(name);
	}

	QStringList MultiTerminal::getTerminalNames() const {
		return MultiTerminalPrivate::TerminalNames;
	}

	QString MultiTerminal::getCurrentTerminalName() const {
		return d->TagArea->getCurrentSelected();
	}

	void MultiTerminal::setCurrentTerminal(const QString& name) {
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
			terminal = MultiTerminalPrivate::Terminals[MultiTerminalPrivate::TerminalNames.indexOf(name)];
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

	bool MultiTerminal::onVirtualOpen(const QString& ext, const QString& fileName, const QString& param) {
		if (ext == "YSS.MainEditor.MultiTerminal") {
			return true;
		}
		return false;
	}
}