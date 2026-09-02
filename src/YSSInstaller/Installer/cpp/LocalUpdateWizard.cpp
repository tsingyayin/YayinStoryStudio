#include "Installer/LocalUpdateWizard.h"
#include "Installer/VersionManager.h"
#include "General/TranslationHost.h"
#include "General/Version.h"
#include "General/Log.h"
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qradiobutton.h>
#include <QtWidgets/qlistwidget.h>
#include <QtWidgets/qtablewidget.h>
#include <QtWidgets/qstackedwidget.h>
#include <QtWidgets/qheaderview.h>
#include <QtWidgets/qabstractitemview.h>
#include <QtWidgets/qlayout.h>
#include <QtGui/qpainter.h>
#include <QtGui/qevent.h>
#include <QtCore/qalgorithms.h>
#include <QtCore/qstring.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <Utility/FileUtility.h>
#include "Installer/InstallerServer.h"
#include <Utility/Console.h>
namespace YSS::Installer {
	class StepIndicator :public QWidget {
	public:
		StepIndicator(const QStringList& steps, QWidget* parent = nullptr)
			: QWidget(parent), steps(steps) {
			setFixedWidth(170);
			setMinimumHeight(steps.size() * rowHeight() + 24);
		}

		void setCurrentStep(int index) {
			if (currentStep != index) {
				currentStep = index;
				update();
			}
		}

	protected:
		QSize sizeHint() const override {
			return QSize(170, steps.size() * rowHeight() + 24);
		}

		void paintEvent(QPaintEvent* event) override {
			Q_UNUSED(event);
			QPainter painter(this);
			painter.setRenderHint(QPainter::Antialiasing);

			const QPalette pal = palette();
			const QColor accent = pal.color(QPalette::Highlight);
			const QColor textColor = pal.color(QPalette::Text);
			const QColor disabledText = pal.color(QPalette::Disabled, QPalette::Text);

			const int circleX = 18;
			const int labelX = 42;
			const int rh = rowHeight();
			const int firstCy = 22;

			for (int i = 0; i < steps.size(); i++) {
				const int cy = firstCy + i * rh;
				// connector
				if (i < steps.size() - 1) {
					QColor lineColor = (i < currentStep) ? accent : disabledText;
					lineColor.setAlpha(120);
					painter.setPen(QPen(lineColor, 2));
					painter.drawLine(circleX, cy + 11, circleX, firstCy + (i + 1) * rh - 11);
				}
				// dot
				const bool active = (i == currentStep);
				const bool done = (i < currentStep);
				const QColor fill = (active || done) ? accent : QColor(0, 0, 0, 0);
				const QColor border = (active || done) ? accent : disabledText;
				painter.setPen(QPen(border, 2));
				painter.setBrush(fill);
				painter.drawEllipse(QPointF(circleX, cy), 9, 9);
				// index
				QFont numFont = font();
				numFont.setPointSizeF(numFont.pointSizeF() * 0.8);
				numFont.setBold(true);
				painter.setFont(numFont);
				painter.setPen(active ? pal.color(QPalette::HighlightedText) : (done ? Qt::white : disabledText));
				painter.drawText(QRect(circleX - 7, cy - 7, 14, 14), Qt::AlignCenter, QString::number(i + 1));
				// tag
				QFont labelFont = font();
				if (active) {
					labelFont.setBold(true);
				}
				painter.setFont(labelFont);
				painter.setPen(active ? accent : (done ? textColor : disabledText));
				painter.drawText(QRect(labelX, cy - rh / 2, width() - labelX - 8, rh), Qt::AlignVCenter | Qt::AlignLeft, steps.at(i));
			}
		}

	private:
		static int rowHeight() { return 44; }
		QStringList steps;
		int currentStep = 0;
	};

	class AskUpdatePage :public QWidget {
	public:
		QLabel* descriptionLabel = nullptr;
		QRadioButton* autoUpdateRadio = nullptr;
		QRadioButton* keepRadio = nullptr;
		QPushButton* nextButton = nullptr;
		QPushButton* cancelButton = nullptr;

		AskUpdatePage(QWidget* parent = nullptr) :QWidget(parent) {
			QVBoxLayout* layout = new QVBoxLayout(this);
			layout->setSpacing(12);

			descriptionLabel = new QLabel(this);
			descriptionLabel->setWordWrap(true);
			layout->addWidget(descriptionLabel);

			autoUpdateRadio = new QRadioButton(this);
			keepRadio = new QRadioButton(this);
			layout->addWidget(autoUpdateRadio);
			layout->addWidget(keepRadio);
			layout->addStretch();

			QHBoxLayout* buttonLayout = new QHBoxLayout();
			buttonLayout->addStretch();
			cancelButton = new QPushButton(VITRL("YSSInstaller::AutoUpdate.Option_Cancel"), this);
			nextButton = new QPushButton(VITRL("YSSInstaller::AutoUpdate.Option_Next"), this);
			nextButton->setDefault(true);
			buttonLayout->addWidget(cancelButton);
			buttonLayout->addWidget(nextButton);
			layout->addLayout(buttonLayout);

			autoUpdateRadio->setChecked(true);
		}
	};

	class SelectOldVersionPage :public QWidget {
	public:
		QLabel* descriptionLabel = nullptr;
		QTableWidget* table = nullptr;
		QPushButton* backButton = nullptr;
		QPushButton* nextButton = nullptr;
	private:
		QList<InstallerClientData> clients;
	public:
		SelectOldVersionPage(QWidget* parent = nullptr) :QWidget(parent) {
			QVBoxLayout* layout = new QVBoxLayout(this);
			layout->setSpacing(12);

			descriptionLabel = new QLabel(VITRL("YSSInstaller::AutoUpdate.Desc_MultiOldVersion"), this);
			descriptionLabel->setWordWrap(true);
			layout->addWidget(descriptionLabel);

			table = new QTableWidget(0, 3, this);
			table->setHorizontalHeaderLabels({ VITRL("YSSInstaller::AutoUpdate.IWantToUpdate"), 
				VITRL("YSSInstaller::AutoUpdate.Version"), VITRL("YSSInstaller::AutoUpdate.Path") });
			table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
			table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
			table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
			table->verticalHeader()->setVisible(false);
			table->setSelectionMode(QAbstractItemView::NoSelection);
			table->setEditTriggers(QAbstractItemView::NoEditTriggers);
			table->setAlternatingRowColors(true);
			layout->addWidget(table);
			connect(table, &QTableWidget::itemChanged, this, [this](QTableWidgetItem* item) {
				Q_UNUSED(item);
				updateNextButtonState();
			});

			QHBoxLayout* buttonLayout = new QHBoxLayout();
			buttonLayout->addStretch();
			backButton = new QPushButton(VITRL("YSSInstaller::AutoUpdate.BackwardStep"), this);
			nextButton = new QPushButton(VITRL("YSSInstaller::AutoUpdate.Option_Next"), this);
			nextButton->setDefault(true);
			nextButton->setEnabled(false);   // must select at least one old version to update
			buttonLayout->addWidget(backButton);
			buttonLayout->addWidget(nextButton);
			layout->addLayout(buttonLayout);
		}

		void setClients(const QList<InstallerClientData>& clients) {
			this->clients = clients;
			table->setRowCount(clients.size());
			for (int row = 0; row < clients.size(); row++) {
				QTableWidgetItem* checkItem = new QTableWidgetItem();
				checkItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
				checkItem->setCheckState(Qt::Unchecked);
				table->setItem(row, 0, checkItem);
				table->setItem(row, 1, new QTableWidgetItem(clients[row].getProgramVersion()));
				table->setItem(row, 2, new QTableWidgetItem(clients[row].getProgramPath()));
			}
			updateNextButtonState();
		}

		void updateNextButtonState() {
			bool anyChecked = false;
			for (int row = 0; row < table->rowCount(); row++) {
				QTableWidgetItem* checkItem = table->item(row, 0);
				if (checkItem != nullptr && checkItem->checkState() == Qt::Checked) {
					anyChecked = true;
					break;
				}
			}
			nextButton->setEnabled(anyChecked);
		}

		QList<InstallerClientData> selectedClients() const {
			QList<InstallerClientData> result;
			for (int row = 0; row < table->rowCount(); row++) {
				QTableWidgetItem* checkItem = table->item(row, 0);
				if (checkItem != nullptr && checkItem->checkState() == Qt::Checked) {
					result.append(clients.at(row));
				}
			}
			return result;
		}
	};

	class UpdateOptionPage :public QWidget {
	public:
		QLabel* descriptionLabel = nullptr;
		QCheckBox* autoDeleteCheck = nullptr;
		QCheckBox* autoLaunchCheck = nullptr;
		QPushButton* backButton = nullptr;
		QPushButton* nextButton = nullptr;

		UpdateOptionPage(QWidget* parent = nullptr) :QWidget(parent) {
			QVBoxLayout* layout = new QVBoxLayout(this);
			layout->setSpacing(12);

			descriptionLabel = new QLabel(VITRL("YSSInstaller::AutoUpdate.Desc_UpdateOption"), this);
			descriptionLabel->setWordWrap(true);
			layout->addWidget(descriptionLabel);

			autoDeleteCheck = new QCheckBox(VITRL("YSSInstaller::AutoUpdate.Option_AutoDelete"), this);
			autoLaunchCheck = new QCheckBox(VITRL("YSSInstaller::AutoUpdate.Option_AutoLaunch"), this);
			autoDeleteCheck->setChecked(false);
			autoLaunchCheck->setChecked(true);
			layout->addWidget(autoDeleteCheck);
			layout->addWidget(autoLaunchCheck);
			layout->addStretch();

			QHBoxLayout* buttonLayout = new QHBoxLayout();
			buttonLayout->addStretch();
			backButton = new QPushButton(VITRL("YSSInstaller::AutoUpdate.BackwardStep"), this);
			nextButton = new QPushButton(VITRL("YSSInstaller::AutoUpdate.Option_Next"), this);
			nextButton->setDefault(true);
			buttonLayout->addWidget(backButton);
			buttonLayout->addWidget(nextButton);
			layout->addLayout(buttonLayout);
		}
	};

	class CloseProgramPage :public QWidget {
	public:
		QLabel* descriptionLabel = nullptr;
		QLabel* runningTitleLabel = nullptr;
		QListWidget* runningList = nullptr;
		QPushButton* backButton = nullptr;
		QPushButton* retryButton = nullptr;

		CloseProgramPage(QWidget* parent = nullptr) :QWidget(parent) {
			QVBoxLayout* layout = new QVBoxLayout(this);
			layout->setSpacing(12);

			descriptionLabel = new QLabel(VITRL("YSSInstaller::AutoUpdate.Desc_CloseProgram"), this);
			descriptionLabel->setWordWrap(true);
			layout->addWidget(descriptionLabel);

			runningTitleLabel = new QLabel(VITRL("YSSInstaller::AutoUpdate.StillRunning"), this);
			layout->addWidget(runningTitleLabel);

			runningList = new QListWidget(this);
			runningList->setSelectionMode(QAbstractItemView::NoSelection);
			layout->addWidget(runningList);

			QHBoxLayout* buttonLayout = new QHBoxLayout();
			buttonLayout->addStretch();
			backButton = new QPushButton(VITRL("YSSInstaller::AutoUpdate.BackwardStep"), this);
			retryButton = new QPushButton(VITRL("YSSInstaller::AutoUpdate.Option_Retry"), this);
			retryButton->setDefault(true);
			buttonLayout->addWidget(backButton);
			buttonLayout->addWidget(retryButton);
			layout->addLayout(buttonLayout);
		}

		void setRunningClients(const QList<InstallerClientData>& clients) {
			runningList->clear();
			for (const InstallerClientData& client : clients) {
				runningList->addItem(QString("%1  (%2)").arg(client.getProgramVersion(), client.getProgramPath()));
			}
		}
	};

	class ReadyUpdatePage :public QWidget {
	public:
		QLabel* descriptionLabel = nullptr;
		QPushButton* cancelButton = nullptr;
		QPushButton* okButton = nullptr;

		ReadyUpdatePage(QWidget* parent = nullptr) :QWidget(parent) {
			QVBoxLayout* layout = new QVBoxLayout(this);
			layout->setSpacing(12);

			descriptionLabel = new QLabel(VITRL("YSSInstaller::AutoUpdate.Desc_Ready"), this);
			descriptionLabel->setWordWrap(true);
			layout->addWidget(descriptionLabel);
			layout->addStretch();

			QHBoxLayout* buttonLayout = new QHBoxLayout();
			buttonLayout->addStretch();
			cancelButton = new QPushButton(VITRL("YSSInstaller::AutoUpdate.Option_Cancel"), this);
			okButton = new QPushButton(VITRL("YSSInstaller::AutoUpdate.Option_OK"), this);
			okButton->setDefault(true);
			buttonLayout->addWidget(cancelButton);
			buttonLayout->addWidget(okButton);
			layout->addLayout(buttonLayout);
		}
	};

	class UpdateFinishedPage :public QWidget {
	public:
		QLabel* titleLabel = nullptr;
		QLabel* autoLaunchGroupLabel = nullptr;
		QWidget* autoLaunchList = nullptr;
		QVBoxLayout* autoLaunchListLayout = nullptr;
		QPushButton* confirmButton = nullptr;
		QList<QRadioButton*> radioButtons;
	private:
		QList<InstallerClientData> clients;
	public:
		UpdateFinishedPage(QWidget* parent = nullptr) :QWidget(parent) {
			QVBoxLayout* layout = new QVBoxLayout(this);
			layout->setSpacing(12);

			titleLabel = new QLabel(VITRL("YSSInstaller::AutoUpdate.Desc_Finished"), this);
			QFont titleFont = titleLabel->font();
			titleFont.setPointSize(titleFont.pointSize() + 5);
			titleFont.setBold(true);
			titleLabel->setFont(titleFont);
			titleLabel->setAlignment(Qt::AlignCenter);
			layout->addWidget(titleLabel);
			layout->addSpacing(8);

			autoLaunchGroupLabel = new QLabel(VITRL("YSSInstaller::AutoUpdate.Desc_ChooseAutoLaunch"), this);
			autoLaunchGroupLabel->setWordWrap(true);
			autoLaunchGroupLabel->setVisible(false);
			layout->addWidget(autoLaunchGroupLabel);

			autoLaunchList = new QWidget(this);
			autoLaunchListLayout = new QVBoxLayout(autoLaunchList);
			autoLaunchListLayout->setContentsMargins(0, 0, 0, 0);
			autoLaunchListLayout->setSpacing(4);
			autoLaunchList->setVisible(false);
			layout->addWidget(autoLaunchList);
			layout->addStretch();

			QHBoxLayout* buttonLayout = new QHBoxLayout();
			buttonLayout->addStretch();
			confirmButton = new QPushButton(VITRL("YSSInstaller::AutoUpdate.Option_Confirm"), this);
			confirmButton->setDefault(true);
			buttonLayout->addWidget(confirmButton);
			layout->addLayout(buttonLayout);
		}

		void setAutoLaunchSelection(const QList<InstallerClientData>& clients, bool visible) {
			qDeleteAll(radioButtons);
			radioButtons.clear();
			this->clients = clients;
			if (visible) {
				for (const InstallerClientData& client : clients) {
					QRadioButton* radio = new QRadioButton(
						QString("%1  (%2)").arg(client.getProgramVersion(), client.getProgramPath()),
						autoLaunchList);
					autoLaunchListLayout->addWidget(radio);
					radioButtons.append(radio);
				}
				if (!radioButtons.isEmpty()) {
					radioButtons.first()->setChecked(true);
				}
				autoLaunchGroupLabel->setVisible(true);
				autoLaunchList->setVisible(true);
			}
			else {
				autoLaunchGroupLabel->setVisible(false);
				autoLaunchList->setVisible(false);
			}
		}

		InstallerClientData selectedAutoLaunchClient() const {
			for (int i = 0; i < radioButtons.size(); i++) {
				if (radioButtons.at(i)->isChecked() && i < clients.size()) {
					return clients.at(i);
				}
			}
			return InstallerClientData();
		}
	};

	class LocalUpdateWizardPrivate {
		friend class LocalUpdateWizard;
	protected:
		InstallerClientData clientData;
		QList<InstallerClientData> oldClients;
		QList<InstallerClientData> selectedTargets;
		bool viaSelection = false;
		bool autoDelete = true;
		bool autoLaunch = false;

		QStackedWidget* stack = nullptr;
		AskUpdatePage* askPage = nullptr;
		SelectOldVersionPage* selectPage = nullptr;
		UpdateOptionPage* optionPage = nullptr;
		CloseProgramPage* closePage = nullptr;
		ReadyUpdatePage* readyPage = nullptr;
		UpdateFinishedPage* finishedPage = nullptr;
		StepIndicator* stepIndicator = nullptr;
		InstallerClientData autoLaunchTarget;
	};

	LocalUpdateWizard::LocalUpdateWizard(const InstallerClientData& clientData, QWidget* parent) : QWidget(parent) {
		d = new LocalUpdateWizardPrivate();
		d->clientData = clientData;
		setWindowTitle(VITRL("YSSInstaller::AutoUpdate.Title"));
		setMinimumSize(560, 420);

		QList<InstallerClientData> records = VersionManager::getInstance()->getClientRecords();
		for (const InstallerClientData& record : records) {
			if (record.getProgramPath() != clientData.getProgramPath()) {
				d->oldClients.append(record);
			}
		}

		QString newVersion = clientData.getProgramVersion();
		QString newPath = clientData.getProgramPath();

		d->stack = new QStackedWidget(this);
		d->askPage = new AskUpdatePage();
		d->selectPage = new SelectOldVersionPage();
		d->optionPage = new UpdateOptionPage();
		d->closePage = new CloseProgramPage();
		d->readyPage = new ReadyUpdatePage();
		d->finishedPage = new UpdateFinishedPage();
		d->stack->addWidget(d->askPage);
		d->stack->addWidget(d->selectPage);
		d->stack->addWidget(d->optionPage);
		d->stack->addWidget(d->closePage);
		d->stack->addWidget(d->readyPage);
		d->stack->addWidget(d->finishedPage);

		d->stepIndicator = new StepIndicator({
			VITRL("YSSInstaller::AutoUpdate.Step_Confirm"),
			VITRL("YSSInstaller::AutoUpdate.Step_Select"),
			VITRL("YSSInstaller::AutoUpdate.Step_Option"),
			VITRL("YSSInstaller::AutoUpdate.Step_Check"),
			VITRL("YSSInstaller::AutoUpdate.Step_Update")
			}, this);

		QWidget* rightPanel = new QWidget(this);
		QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
		rightLayout->setContentsMargins(0, 0, 0, 0);
		rightLayout->addWidget(d->stack);

		QHBoxLayout* mainLayout = new QHBoxLayout(this);
		mainLayout->setContentsMargins(16, 16, 16, 16);
		mainLayout->setSpacing(16);
		mainLayout->addWidget(d->stepIndicator);
		mainLayout->addWidget(rightPanel, 1);

		connect(d->stack, &QStackedWidget::currentChanged, this, [this](int index) {
			d->stepIndicator->setCurrentStep(qMin(index, 4));
		});

		d->askPage->descriptionLabel->setText(
			VITRL("YSSInstaller::AutoUpdate.Desc_CanUpdate").arg(newVersion, newPath));
		d->askPage->autoUpdateRadio->setText(VITRL("YSSInstaller::AutoUpdate.Option_AutoUpdate").arg(newVersion));
		d->askPage->keepRadio->setText(VITRL("YSSInstaller::AutoUpdate.Option_Keep").arg(newVersion));

		connect(d->askPage->nextButton, &QPushButton::clicked, this, [this]() {
			if (d->askPage->autoUpdateRadio->isChecked()) {
				if (d->oldClients.size() > 1) {
					d->selectPage->setClients(d->oldClients);
					d->viaSelection = true;
					d->stack->setCurrentWidget(d->selectPage);
				}
				else {
					d->selectedTargets = d->oldClients;
					d->viaSelection = false;
					d->stack->setCurrentWidget(d->optionPage);
				}
			}
			else {
				emit asIndependent(d->clientData);
				close();
			}
		});
		connect(d->askPage->cancelButton, &QPushButton::clicked, this, [this]() {
			close();
		});

		connect(d->selectPage->backButton, &QPushButton::clicked, this, [this]() {
			d->stack->setCurrentWidget(d->askPage);
		});

		connect(d->selectPage->nextButton, &QPushButton::clicked, this, [this]() {
			QList<InstallerClientData> selected = d->selectPage->selectedClients();
			if (selected.isEmpty()) {
				vgWarning << "No client selected to update.";
				return;
			}
			d->selectedTargets = selected;
			d->stack->setCurrentWidget(d->optionPage);
		});

		connect(d->optionPage->backButton, &QPushButton::clicked, this, [this]() {
			QWidget* previous = d->viaSelection
				? static_cast<QWidget*>(d->selectPage)
				: static_cast<QWidget*>(d->askPage);
			d->stack->setCurrentWidget(previous);
		});

		connect(d->optionPage->nextButton, &QPushButton::clicked, this, [this]() {
			d->autoDelete = d->optionPage->autoDeleteCheck->isChecked();
			d->autoLaunch = d->optionPage->autoLaunchCheck->isChecked();
			QList<InstallerClientData> Clients = d->selectedTargets;
			Clients.append(d->clientData);
			QList<bool> runningStates = checkProgramStillRunning(Clients);
			bool someRunning = std::any_of(runningStates.begin(), runningStates.end(), [](bool state) { return state; });
			if (someRunning) {
				QList<InstallerClientData> runningClients;
				for (int i = 0; i < Clients.size(); i++) {
					if (runningStates[i]) {
						runningClients.append(Clients[i]);
					}
				}
				d->closePage->setRunningClients(runningClients);
				d->stack->setCurrentWidget(d->closePage);
			}
			else {
				d->stack->setCurrentWidget(d->readyPage);
			}
		});

		connect(d->closePage->backButton, &QPushButton::clicked, this, [this]() {
			d->stack->setCurrentWidget(d->optionPage);
		});

		connect(d->closePage->retryButton, &QPushButton::clicked, this, [this]() {
			QList<InstallerClientData> Clients = d->selectedTargets;
			Clients.append(d->clientData);
			QList<bool> runningStates = checkProgramStillRunning(Clients);
			bool someRunning = std::any_of(runningStates.begin(), runningStates.end(), [](bool state) { return state; });
			if (someRunning) {
				QList<InstallerClientData> runningClients;
				for (int i = 0; i < Clients.size(); i++) {
					if (runningStates[i]) {
						runningClients.append(Clients[i]);
					}
				}
				d->closePage->setRunningClients(runningClients);
			}
			else {
				d->stack->setCurrentWidget(d->readyPage);
			}
		});

		connect(d->readyPage->cancelButton, &QPushButton::clicked, this, [this]() {
			close();
		});

		connect(d->readyPage->okButton, &QPushButton::clicked, this, [this]() {
			updateProgram(d->clientData, d->selectedTargets);
			onUpdateFinished();
		});

		connect(d->finishedPage->confirmButton, &QPushButton::clicked, this, [this]() {
			if (d->autoLaunch) {
				if (d->selectedTargets.size() == 1) {
					d->autoLaunchTarget = d->selectedTargets.first();
				}
				else {
					d->autoLaunchTarget = d->finishedPage->selectedAutoLaunchClient();
				}
				onAutoLaunch();
			}
			close();
		});

		d->stack->setCurrentWidget(d->askPage);
	}

	LocalUpdateWizard::~LocalUpdateWizard() {
		delete d;
	}

	QList<bool> LocalUpdateWizard::checkProgramStillRunning(const QList<InstallerClientData>& targets) {
		QList<bool> rtn;
		for (const InstallerClientData& target : targets) {
			rtn.append(InstallerServer::getInstance()->isClientStillRunning(target));
		}
		return rtn;
	}

	void LocalUpdateWizard::onUpdateFinished() {
		bool needAutoLaunchSelection = d->autoLaunch && d->selectedTargets.size() > 1;
		d->finishedPage->setAutoLaunchSelection(d->selectedTargets, needAutoLaunchSelection);
		d->stack->setCurrentWidget(d->finishedPage);
	}

	bool LocalUpdateWizard::updateProgram(const InstallerClientData& from, const QList<InstallerClientData>& targets) {
		QStringList files = {
			"Visindigo.dll", "Qt6Core.dll", "Qt6Gui.dll", "Qt6Widgets.dll", "Qt6Network.dll", "Qt6Sql.dll",
			"Qt6Svg.dll", "dbghelp.dll", "icuuc.dll", "opengl32sw.dll", "7za.exe", "YSSInstaller.exe",
			"YayinStoryStudio.exe", "YSSCore.dll",
			"user_data/themes/template/yss.vst",
		};
		QStringList folders = {
			"user_data/plugins"
		};
		QDir fromPath = QFileInfo(from.getProgramPath()).absoluteDir();
		QList<QDir> toPath;
		for (auto t : targets) {
			toPath.append(QFileInfo(t.getProgramPath()).absoluteDir());
		}
		QList<InstallerClientData> targetList = targets;
		qint32 i = 0;
		for (auto t : toPath) {
			for (auto f : files) {
				Visindigo::Utility::FileUtility::copyFile(fromPath.absolutePath() + "/" + f,
					t.absolutePath() + "/" + f, false, true);
			}
			for (auto f : folders) {
				Visindigo::Utility::FileUtility::copyDir(fromPath.absolutePath() + "/" + f,
					t.absolutePath() + "/" + f, false, true);
			}
			targetList[i].setProgramVersion(from.getProgramVersion());
			VersionManager::getInstance()->updateClientRecord(targetList[i]);
			i++;
		}

		if (d->autoDelete) {
			Visindigo::Utility::FileUtility::deleteDir(fromPath.absolutePath(), { 
				fromPath.absolutePath() + "/user_data/repos",
				fromPath.absolutePath() + "/user_data/third_party",
				});
		}
		return true;
	}

	void LocalUpdateWizard::onAutoLaunch() {
		if (not d->autoLaunchTarget.getProgramPath().isEmpty()) {
			QString exePath = d->autoLaunchTarget.getProgramPath();
			Visindigo::Utility::Console::exec(QString("start \"\" \"%1\"").arg(exePath.replace('/', '\\')));
		}
	}
}