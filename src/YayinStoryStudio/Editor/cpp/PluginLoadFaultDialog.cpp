#include <QtCore/qmetaobject.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qframe.h>
#include <QtWidgets/qheaderview.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qtablewidget.h>
#include <General/PluginManager.h>
#include <General/TranslationHost.h>
#include <Widgets/ThemeManager.h>
#include "Editor/PluginLoadFaultDialog.h"

namespace YSS::Editor {
	/*!
		\since YayinStoryStudio 0.17.0
		\a result 插件的加载结果。

		return 该结果是否属于"插件加载失败"。

		Success是加载成功；Deactivated表示插件被用户手动禁用，属于用户的正常选择，
		两者都不会被视为加载失败。其余结果都表示插件未能按预期加载。
	*/
	bool PluginLoadFaultDialog::isLoadFault(Visindigo::General::PluginManager::LoadPluginResult result) {
		using Visindigo::General::PluginManager;
		return result != PluginManager::LoadPluginResult::Success
			&& result != PluginManager::LoadPluginResult::Deactivated;
	}

	/*!
		\since YayinStoryStudio 0.17.0
		\a loadResults 插件的加载结果映射。

		return \a loadResults 中是否存在插件加载失败的条目。若不存在，则无需显示本对话框。
	*/
	bool PluginLoadFaultDialog::hasLoadFault(const QMap<QString, Visindigo::General::PluginManager::LoadPluginResult>& loadResults) {
		for (auto it = loadResults.begin(); it != loadResults.end(); ++it) {
			if (isLoadFault(it.value())) {
				return true;
			}
		}
		return false;
	}

	PluginLoadFaultDialog::PluginLoadFaultDialog(const QMap<QString, Visindigo::General::PluginManager::LoadPluginResult>& loadResults, QWidget* parent)
		:QDialog(parent) {
		this->setWindowTitle(VITRL("YSS::pluginLoadFaultDialog.title"));
		this->setMinimumWidth(620);
		this->buildUi(loadResults);
	}

	void PluginLoadFaultDialog::buildUi(
		const QMap<QString, Visindigo::General::PluginManager::LoadPluginResult>& loadResults) {
		using Visindigo::General::PluginManager;

		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setContentsMargins(20, 18, 20, 16);
		layout->setSpacing(12);
		this->setLayout(layout);

		TitleLabel = new QLabel(VITRL("YSS::pluginLoadFaultDialog.title"), this);
		TitleLabel->setObjectName(QStringLiteral("faultTitleLabel"));
		layout->addWidget(TitleLabel);

		SubtitleLabel = new QLabel(VITRL("YSS::pluginLoadFaultDialog.message"), this);
		SubtitleLabel->setObjectName(QStringLiteral("faultSubtitleLabel"));
		SubtitleLabel->setWordWrap(true);
		layout->addWidget(SubtitleLabel);

		QFrame* card = new QFrame(this);
		card->setObjectName(QStringLiteral("faultCard"));
		QVBoxLayout* cardLayout = new QVBoxLayout(card);
		cardLayout->setContentsMargins(16, 14, 16, 16);
		cardLayout->setSpacing(8);
		card->setLayout(cardLayout);
		layout->addWidget(card, 1);

		FaultTable = new QTableWidget(card);
		FaultTable->setObjectName(QStringLiteral("faultTable"));
		FaultTable->setColumnCount(2);
		FaultTable->setHorizontalHeaderLabels({
			VITRL("YSS::pluginLoadFaultDialog.pluginID"),
			VITRL("YSS::pluginLoadFaultDialog.reason")
			});
		FaultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
		FaultTable->setSelectionMode(QAbstractItemView::NoSelection);
		FaultTable->setShowGrid(false);
		FaultTable->setWordWrap(true);
		FaultTable->setFrameShape(QFrame::NoFrame);
		FaultTable->verticalHeader()->setVisible(false);
		FaultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
		FaultTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
		cardLayout->addWidget(FaultTable, 1);

		CountLabel = new QLabel(card);
		CountLabel->setObjectName(QStringLiteral("faultCountLabel"));
		cardLayout->addWidget(CountLabel);

		const QMetaEnum resultEnum = QMetaEnum::fromType<PluginManager::LoadPluginResult>();
		int faultCount = 0;
		for (auto it = loadResults.begin(); it != loadResults.end(); ++it) {
			if (not isLoadFault(it.value())) {
				continue;
			}
			++faultCount;
			const QString keyBase = "YSS::pluginLoadResult."
				+ QString::fromLatin1(resultEnum.valueToKey(static_cast<int>(it.value()))) + ".";
			const int row = FaultTable->rowCount();
			FaultTable->insertRow(row);
			FaultTable->setItem(row, 0, new QTableWidgetItem(it.key()));
			FaultTable->setItem(row, 1, new QTableWidgetItem(VITR(keyBase + "title") + "\n" + VITR(keyBase + "message")));
		}
		FaultTable->resizeRowsToContents();
		CountLabel->setText(VITRL("YSS::pluginLoadFaultDialog.count").arg(faultCount));

		QHBoxLayout* buttonLayout = new QHBoxLayout();
		buttonLayout->setSpacing(8);
		QuitButton = new QPushButton(VITRL("YSS::pluginLoadFaultDialog.quit"), this);
		QuitButton->setObjectName(QStringLiteral("faultQuitButton"));
		QuitButton->setProperty("dialogRole", "secondary");
		QuitButton->setCursor(Qt::PointingHandCursor);
		ContinueButton = new QPushButton(VITRL("YSS::pluginLoadFaultDialog.continue"), this);
		ContinueButton->setObjectName(QStringLiteral("faultContinueButton"));
		ContinueButton->setProperty("dialogRole", "primary");
		ContinueButton->setCursor(Qt::PointingHandCursor);
		ContinueButton->setDefault(true);
		buttonLayout->addStretch();
		buttonLayout->addWidget(QuitButton);
		buttonLayout->addWidget(ContinueButton);
		layout->addLayout(buttonLayout);

		connect(ContinueButton, &QPushButton::clicked, this, &QDialog::accept);
		connect(QuitButton, &QPushButton::clicked, this, &QDialog::reject);

		this->resize(680, 420);
	}

	void PluginLoadFaultDialog::showEvent(QShowEvent* event) {
		this->applyStyle();
		QDialog::showEvent(event);
	}

	void PluginLoadFaultDialog::applyStyle() {
		setStyleSheet(VISTMGT("YSS::Editor.PluginLoadFault", this));
	}
}
