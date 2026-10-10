#include <QtCore/qmetaobject.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qheaderview.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qtablewidget.h>
#include <General/PluginManager.h>
#include <General/TranslationHost.h>
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
		using Visindigo::General::PluginManager;
		this->setWindowTitle(VITRL("YSS::pluginLoadFaultDialog.title"));

		PromptLabel = new QLabel(VITRL("YSS::pluginLoadFaultDialog.message"), this);
		PromptLabel->setWordWrap(true);

		FaultTable = new QTableWidget(this);
		FaultTable->setColumnCount(2);
		FaultTable->setHorizontalHeaderLabels({
			VITRL("YSS::pluginLoadFaultDialog.pluginID"),
			VITRL("YSS::pluginLoadFaultDialog.reason")
			});
		FaultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
		FaultTable->setSelectionMode(QAbstractItemView::NoSelection);
		FaultTable->setWordWrap(true);
		FaultTable->verticalHeader()->setVisible(false);
		FaultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
		FaultTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

		const QMetaEnum resultEnum = QMetaEnum::fromType<PluginManager::LoadPluginResult>();
		for (auto it = loadResults.begin(); it != loadResults.end(); ++it) {
			if (not isLoadFault(it.value())) {
				continue;
			}
			const QString keyBase = "YSS::pluginLoadResult." + QString::fromLatin1(resultEnum.valueToKey(static_cast<int>(it.value()))) + ".";
			const int row = FaultTable->rowCount();
			FaultTable->insertRow(row);
			FaultTable->setItem(row, 0, new QTableWidgetItem(it.key()));
			FaultTable->setItem(row, 1, new QTableWidgetItem(VITR(keyBase + "title") + "\n" + VITR(keyBase + "message")));
		}
		FaultTable->resizeRowsToContents();

		ButtonBox = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No, this);
		ButtonBox->button(QDialogButtonBox::Yes)->setText(VITRL("YSS::pluginLoadFaultDialog.continue"));
		ButtonBox->button(QDialogButtonBox::No)->setText(VITRL("YSS::pluginLoadFaultDialog.quit"));
		ButtonBox->button(QDialogButtonBox::Yes)->setDefault(true);
		connect(ButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
		connect(ButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->addWidget(PromptLabel);
		layout->addWidget(FaultTable);
		layout->addWidget(ButtonBox);

		this->resize(600, 380);
	}
}
