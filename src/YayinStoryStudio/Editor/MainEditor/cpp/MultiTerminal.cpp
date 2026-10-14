#include <QtGui/qaction.h>
#include <QtGui/qicon.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlistwidget.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qtoolbutton.h>
#include <General/TranslationHost.h>
#include <General/VIApplication.h>
#include <Widgets/Terminal.h>
#include <Widgets/ThemeManager.h>
#include "Editor/MainEditor/MultiTerminal.h"
namespace YSS::Editor {
		static void showTerminalContextMenu(MultiTerminal* owner, const QString& name, const QPoint& globalPos) {
			QMenu menu;
			QAction* closeAction = menu.addAction(VITRL("Visindigo::general.close"));
			QAction* closeAllAction = menu.addAction(VITRL("YSS::menu.file.closeAll"));

			closeAction->setEnabled(not name.isEmpty() and name != "builtin");
			QAction* chosen = menu.exec(globalPos);
			if (chosen == closeAction) {
				owner->closeTerminal(name);
			}
			else if (chosen == closeAllAction) {
				owner->closeAll();
			}

			owner->setCurrentTerminal(owner->getCurrentTerminalName());
		}

		static void bindTerminalContextMenu(QWidget* widget, MultiTerminal* owner, const QString& name) {
			widget->setContextMenuPolicy(Qt::CustomContextMenu);
			QObject::connect(widget, &QWidget::customContextMenuRequested, owner, [owner, name, widget](const QPoint& pos) {
				showTerminalContextMenu(owner, name, widget->mapToGlobal(pos));
				});
		}
	
	class MultiTerminalPrivate {
		friend class MultiTerminal;
	protected:
		static MultiTerminal* Instance;
		static QList<Visindigo::Widgets::Terminal*> Terminals; // terminal resource should not be released when q class deleted.
		static QStringList TerminalNames;
		QHBoxLayout* Layout = nullptr;
		QListWidget* TerminalList = nullptr;
		QWidget* ContentArea = nullptr;
		QLabel* Placeholder = nullptr;
		QString CurrentName;

		QIcon terminalCloseIcon() const {
			return VIApp->getFontIcon("\uE711", 64, { VISTM->getPaletteTextColor() });
		}

		void addRow(MultiTerminal* owner, const QString& name, bool closable) {
			QListWidgetItem* item = new QListWidgetItem(TerminalList);
			item->setData(Qt::UserRole, name);
			item->setSizeHint(QSize(0, 28));
			QWidget* row = new QWidget(TerminalList);
			QHBoxLayout* rowLayout = new QHBoxLayout(row);
			rowLayout->setContentsMargins(0, 0, 0, 0);
			rowLayout->setSpacing(2);
			QToolButton* nameButton = new QToolButton(row);
			nameButton->setText(name);
			nameButton->setAutoRaise(true);
			nameButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
			nameButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			nameButton->setStyleSheet("QToolButton { text-align: left; padding-left: 8px; }");
			rowLayout->addWidget(nameButton, 1);
			QObject::connect(nameButton, &QToolButton::clicked, owner, [owner, name]() {
				owner->setCurrentTerminal(name);
				});
			bindTerminalContextMenu(nameButton, owner, name);
			if (closable) {
				QToolButton* closeButton = new QToolButton(row);
				closeButton->setObjectName("TerminalCloseButton");
				closeButton->setAutoRaise(true);
				closeButton->setIcon(terminalCloseIcon());
				closeButton->setToolTip(VITRL("Visindigo::general.close"));
				rowLayout->addWidget(closeButton, 0);
				QObject::connect(closeButton, &QToolButton::clicked, owner, [owner, name]() {
					owner->closeTerminal(name);
					});
				bindTerminalContextMenu(closeButton, owner, name);
			}
			TerminalList->setItemWidget(item, row);
		}

		void rebuildList(MultiTerminal* owner) {
			TerminalList->clear();
			if (VIApp->getVirtualTerminal()) {
				addRow(owner, "builtin", false);
			}
			for (const QString& name : TerminalNames) {
				addRow(owner, name, true);
			}
			if (CurrentName.isEmpty()) {
				TerminalList->clearSelection();
			}
			else {
				selectRow(CurrentName);
			}
		}

		void selectRow(const QString& name) {
			for (int i = 0; i < TerminalList->count(); ++i) {
				QListWidgetItem* item = TerminalList->item(i);
				if (item->data(Qt::UserRole).toString() == name) {
					TerminalList->setCurrentItem(item);
					return;
				}
			}
			TerminalList->clearSelection();
		}

		void refreshRowIcons() {
			const QIcon icon = terminalCloseIcon();
			for (int i = 0; i < TerminalList->count(); ++i) {
				QWidget* row = TerminalList->itemWidget(TerminalList->item(i));
				if (not row) {
					continue;
				}
				if (QToolButton* button = row->findChild<QToolButton*>("TerminalCloseButton")) {
					button->setIcon(icon);
				}
			}
		}

		void setContentWidget(QWidget* widget) {
			if (not widget or ContentArea == widget) {
				return;
			}
			ContentArea->hide();
			Layout->removeWidget(ContentArea);
			ContentArea = widget;
			Layout->insertWidget(0, ContentArea);
			ContentArea->show();
		}

		void initData() {
			auto builtinTerminal = VIApp->getVirtualTerminal();
			if (builtinTerminal) {
				builtinTerminal->setContentsMargins(0, 0, 0, 0);
				builtinTerminal->setParent(Instance);
				builtinTerminal->hide();
			}
			for (auto terminal : Terminals) {
				terminal->setParent(Instance);
			}
			rebuildList(Instance);
			if (builtinTerminal) {
				Instance->setCurrentTerminal("builtin");
			}
			for (const QString& name : TerminalNames) {
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

		d->Placeholder = new QLabel(VITRL("YSS::editor.toolWidgetArea.noToolWidget"), this);
		d->Placeholder->setAlignment(Qt::AlignCenter);
		d->ContentArea = d->Placeholder;

		d->TerminalList = new QListWidget(this);
		d->TerminalList->setFixedWidth(160);
		d->TerminalList->setFrameShape(QFrame::NoFrame);
		d->TerminalList->setSelectionMode(QAbstractItemView::SingleSelection);
		d->TerminalList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		d->TerminalList->setContextMenuPolicy(Qt::CustomContextMenu);

		d->Layout->addWidget(d->ContentArea, 1);
		d->Layout->addWidget(d->TerminalList, 0);

		connect(d->TerminalList, &QListWidget::currentItemChanged, this, [this](QListWidgetItem* current, QListWidgetItem*) {
			if (not current) {
				return;
			}
			const QString name = current->data(Qt::UserRole).toString();
			if (name != d->CurrentName) {
				setCurrentTerminal(name);
			}
			});

		connect(d->TerminalList, &QListWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
			// 列表空白处（没有任何终端行的地方）也能关闭全部终端。
			showTerminalContextMenu(this, QString(), d->TerminalList->mapToGlobal(pos));
			});

		connect(VISTM, &Visindigo::Widgets::ThemeManager::programThemeChanged, this, [this](const QString& themeID) {
			d->refreshRowIcons();
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
		if (name == "builtin" or MultiTerminalPrivate::TerminalNames.contains(name)) {
			return;
		}
		auto terminal = new Visindigo::Widgets::Terminal(this);
		terminal->launchExternalProcess(command, {}, workingDir.path());
		MultiTerminalPrivate::Terminals.append(terminal);
		MultiTerminalPrivate::TerminalNames.append(name);
		d->rebuildList(this);
		setCurrentTerminal(name);
	}

	void MultiTerminal::closeTerminal(const QString& name) {
		if (name.isEmpty() or name == "builtin") {
			return;
		}
		int index = MultiTerminalPrivate::TerminalNames.indexOf(name);
		if (index == -1) {
			return;
		}
		auto terminal = MultiTerminalPrivate::Terminals.value(index, nullptr);
		MultiTerminalPrivate::Terminals.removeAt(index);
		MultiTerminalPrivate::TerminalNames.removeAt(index);
		if (terminal) {
			terminal->close();
			terminal->deleteLater();
		}
		const bool closedCurrent = (d->CurrentName == name);
		d->rebuildList(this);
		if (closedCurrent) {
			if (VIApp->getVirtualTerminal()) {
				setCurrentTerminal("builtin");
			}
			else if (MultiTerminalPrivate::TerminalNames.isEmpty()) {
				setCurrentTerminal(QString());
			}
			else {
				setCurrentTerminal(MultiTerminalPrivate::TerminalNames.first());
			}
		}
	}

	void MultiTerminal::closeAll() {
		for (auto terminal : MultiTerminalPrivate::Terminals) {
			if (terminal) {
				terminal->close();
				terminal->deleteLater();
			}
		}
		MultiTerminalPrivate::Terminals.clear();
		MultiTerminalPrivate::TerminalNames.clear();
		d->rebuildList(this);
		setCurrentTerminal(VIApp->getVirtualTerminal() ? QString("builtin") : QString());
	}

	bool MultiTerminal::containsTerminal(const QString& name) const {
		return MultiTerminalPrivate::TerminalNames.contains(name);
	}

	QStringList MultiTerminal::getTerminalNames() const {
		return MultiTerminalPrivate::TerminalNames;
	}

	QString MultiTerminal::getCurrentTerminalName() const {
		return d->CurrentName;
	}

	void MultiTerminal::setCurrentTerminal(const QString& name) {
		if (name.isEmpty()) {
			d->setContentWidget(d->Placeholder);
			d->CurrentName.clear();
			d->TerminalList->clearSelection();
			return;
		}
		Visindigo::Widgets::Terminal* terminal = nullptr;
		if (name == "builtin") {
			terminal = VIApp->getVirtualTerminal();
		}
		else {
			terminal = MultiTerminalPrivate::Terminals.value(MultiTerminalPrivate::TerminalNames.indexOf(name), nullptr);
		}
		if (not terminal) {
			return;
		}
		d->setContentWidget(terminal);
		d->CurrentName = name;
		d->selectRow(name);
		emit currentTerminalChanged(name);
	}

	bool MultiTerminal::onVirtualOpen(const QString& ext, const QString& fileName, const QString& param) {
		if (ext == "YSS.MainEditor.MultiTerminal") {
			return true;
		}
		return false;
	}
}