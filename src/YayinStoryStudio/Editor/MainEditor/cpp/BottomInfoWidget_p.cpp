#include <QtCore/qcoreevent.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qtimer.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qgraphicseffect.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qprogressbar.h>
#include <General/TranslationHost.h>
#include <General/YSSUniqueFileSystem.h>
#include <Widgets/LiquidGlassEffect.h>
#include "Editor/MainEditor/BottomInfoWidget_p.h"
#include "Editor/MainEditor/TextEditConfigOperator.h"

namespace YSS::Editor {
	namespace {
		// 传输监视的刷新间隔。统计本来就是在操作结束时一次性累加的，
		// 没必要刷得更勤。
		constexpr int TransferRefreshIntervalMs = 500;

		/*
			套用与 TextEdit 悬浮窗（HoverInfoWidget / TabCompleterWidget / SuggestionWidget）
			一致的外观：BorderFrame 边框 + 6px 圆角、20px 模糊的液态玻璃，字体缩到 0.8 倍，
			底色半透明。

			那三个控件都是 TextEdit 的子控件，玻璃直接采样父控件背景；底栏这两个是顶层
			弹窗，没有父控件可采样，所以改用 BackgroundPolicy::CustomImage，由
			\l{attachWindowBackground()} 在弹出时补一张窗口截图。
		*/
		Visindigo::Widgets::LiquidGlassEffect* applyGlassStyle(QWidget* widget) {
			auto* glass = new Visindigo::Widgets::LiquidGlassEffect(widget);
			glass->setBorderRadius(6);
			glass->setBlurRadius(20);
			glass->setBackgroundPolicy(Visindigo::Widgets::LiquidGlassEffect::BackgroundPolicy::CustomImage);
			glass->setPositionPolicy(Visindigo::Widgets::LiquidGlassEffect::PositionPolicy::CustomGeometry);
			widget->setGraphicsEffect(glass);
			Visindigo::Widgets::LiquidGlassEffect::applyForegroundAlpha(widget);
			QFont font = widget->font();
			font.setPointSizeF(font.pointSizeF() * 0.8);
			widget->setFont(font);
			return glass;
		}

		/*
			顶层弹窗的玻璃底：抓一张 \a anchor 所在窗口的截图，并告诉效果该取
			\a popup 在截图中的哪一块。grab() 得到的是设备像素，因此按 devicePixelRatio 换算。
		*/
		void attachWindowBackground(Visindigo::Widgets::LiquidGlassEffect* glass,
			QWidget* popup, QWidget* anchor) {
			if (glass == nullptr || popup == nullptr || anchor == nullptr) {
				return;
			}
			QWidget* window = anchor->window();
			if (window == nullptr) {
				return;
			}
			const QPixmap shot = window->grab();
			if (shot.isNull()) {
				return;
			}
			const qreal dpr = shot.devicePixelRatio() > 0 ? shot.devicePixelRatio() : 1.0;
			const QPoint origin = popup->mapTo(window, QPoint(0, 0));
			glass->setBackgroundImage(shot.toImage());
			glass->setCustomGeometry(QRect(
				QPoint(qRound(origin.x() * dpr), qRound(origin.y() * dpr)),
				QSize(qRound(popup->width() * dpr), qRound(popup->height() * dpr))));
		}

		// 把字节数写成便于阅读的形式，基数 1024。
		QString formatBytes(qint64 bytes) {
			static const char* units[] = { "B", "KiB", "MiB", "GiB", "TiB", "PiB" };
			double value = double(bytes);
			int unit = 0;
			while (value >= 1024.0 && unit < 5) {
				value /= 1024.0;
				++unit;
			}
			if (unit == 0) {
				return QStringLiteral("%1 %2").arg(bytes).arg(QLatin1String(units[unit]));
			}
			return QStringLiteral("%1 %2").arg(value, 0, 'f', value < 10.0 ? 2 : 1)
				.arg(QLatin1String(units[unit]));
		}

		// 把浮窗摆在 \a anchor 上方，右边缘对齐，避免盖住底栏本身。
		// 宽度限制在 [\a minWidth, \a maxWidth] 之间：内容再长也换行，不会横向撑出一个长条。
		void placeAbove(QWidget* popup, QWidget* anchor, int minWidth, int maxWidth) {
			popup->adjustSize();
			const int width = qBound(minWidth, popup->sizeHint().width(), maxWidth);
			const int height = popup->sizeHint().height();
			popup->setFixedSize(width, height);
			const QPoint anchorPos = anchor->mapToGlobal(QPoint(0, 0));
			popup->move(anchorPos.x() + anchor->width() - width, anchorPos.y() - height);
		}

		// 日志与后端里的操作名保持英文，界面里也直接显示，避免再造一套翻译。
		QString operationName(YSSCore::General::YSSUniqueFileSystem::Operation operation) {
			using Operation = YSSCore::General::YSSUniqueFileSystem::Operation;
			switch (operation) {
			case Operation::Stat: return QStringLiteral("stat");
			case Operation::Read: return QStringLiteral("read");
			case Operation::Write: return QStringLiteral("write");
			case Operation::List: return QStringLiteral("list");
			case Operation::Remove: return QStringLiteral("remove");
			case Operation::Recycle: return QStringLiteral("recycle");
			case Operation::Rename: return QStringLiteral("rename");
			case Operation::Mkdir: return QStringLiteral("mkdir");
			case Operation::Rmdir: return QStringLiteral("rmdir");
			default: return QStringLiteral("unknown");
			}
		}

		// Provider 标识的可读形式：本机后端显示为“本地”，SSH 连接保留 user@host:port。
		QString providerDisplayName(const QString& providerId) {
			if (providerId == QStringLiteral("local")) {
				return VITRL("YSS::editor.bottomInfoWidget.transfer.local");
			}
			return providerId;
		}
	}

	EditorHintSettingsPopup::EditorHintSettingsPopup()
		: Visindigo::Widgets::BorderFrame(nullptr) {
		setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
		// 半透明底色要真正透出下方的玻璃，顶层窗口必须显式声明。
		setAttribute(Qt::WA_TranslucentBackground);
		setObjectName("EditorHintSettingsPopup");
		Glass = applyGlassStyle(this);
		auto* layout = new QVBoxLayout(this);
		layout->setContentsMargins(8, 5, 8, 6);
		layout->setSpacing(4);

		auto* completerLevelRow = new QHBoxLayout();
		completerLevelRow->setContentsMargins(0, 0, 0, 0);
		completerLevelRow->setSpacing(8);
		completerLevelRow->addWidget(new QLabel(VITRL("YSS::editor.textEdit.completerLevel"), this));
		completerLevelRow->addStretch(1);
		CompleterLevelComboBox = new QComboBox(this);
		completerLevelRow->addWidget(CompleterLevelComboBox);
		layout->addLayout(completerLevelRow);

		InlayHintCheckBox = new QCheckBox(VITRL("YSS::editor.textEdit.inlayHintVisible"), this);
		InlayHintCheckBox->setChecked(TextEditConfigOperator::getInlayHintVisible());
		layout->addWidget(InlayHintCheckBox);

		CompleterLevelComboBox->addItem(VITRL("YSS::editor.textEdit.tabCompleterLevel.none"), YSSCore::Editor::TabCompleterItem::CompleterLevel::None);
		CompleterLevelComboBox->addItem(VITRL("YSS::editor.textEdit.tabCompleterLevel.few"), YSSCore::Editor::TabCompleterItem::CompleterLevel::Few);
		CompleterLevelComboBox->addItem(VITRL("YSS::editor.textEdit.tabCompleterLevel.some"), YSSCore::Editor::TabCompleterItem::CompleterLevel::Some);
		CompleterLevelComboBox->addItem(VITRL("YSS::editor.textEdit.tabCompleterLevel.many"), YSSCore::Editor::TabCompleterItem::CompleterLevel::Many);
		CompleterLevelComboBox->addItem(VITRL("YSS::editor.textEdit.tabCompleterLevel.all"), YSSCore::Editor::TabCompleterItem::CompleterLevel::All);
		qint32 index = -1;
		const auto configLevel = TextEditConfigOperator::getCompleterLevel();
		for (qint32 i = 0; i < CompleterLevelComboBox->count(); i++) {
			if (CompleterLevelComboBox->itemData(i).value<YSSCore::Editor::TabCompleterItem::CompleterLevel>() == configLevel) {
				index = i;
				break;
			}
		}
		if (index == -1) {
			index = 4; // Default to "All" if not found
		}
		CompleterLevelComboBox->setCurrentIndex(index);

		connect(CompleterLevelComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int newIndex) {
			if (newIndex < 0) {
				return;
			}
			const auto level = static_cast<YSSCore::Editor::TabCompleterItem::CompleterLevel>(
				CompleterLevelComboBox->itemData(newIndex).toInt());
			TextEditConfigOperator::setCompleterLevel(level, true);
			});
		connect(InlayHintCheckBox, &QCheckBox::toggled, this, [](bool checked) {
			TextEditConfigOperator::setInlayHintVisible(checked, true);
			});
	}

	void EditorHintSettingsPopup::popupFor(QWidget* anchor) {
		if (anchor == nullptr) {
			return;
		}
		// 每次弹出都重新读一次配置，避免别处改动后这里显示过期状态。
		InlayHintCheckBox->setChecked(TextEditConfigOperator::getInlayHintVisible());
		placeAbove(this, anchor, 220, 360);
		attachWindowBackground(Glass, this, anchor);
		show();
	}

	TransferMonitorPopup::TransferMonitorPopup()
		: Visindigo::Widgets::BorderFrame(nullptr) {
		setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
		setAttribute(Qt::WA_TranslucentBackground);
		setObjectName("TransferMonitorPopup");
		Glass = applyGlassStyle(this);
		auto* layout = new QVBoxLayout(this);
		layout->setContentsMargins(8, 5, 8, 6);
		layout->setSpacing(4);

		TotalLabel = new QLabel(this);
		QFont totalFont = TotalLabel->font();
		// 浮窗整体字体已缩到 0.8 倍，这里乘 2.0 让总计回到默认字号的约 1.6 倍。
		totalFont.setPointSizeF(totalFont.pointSizeF() * 2.0);
		totalFont.setBold(true);
		TotalLabel->setFont(totalFont);
		TotalLabel->setToolTip(VITRL("YSS::editor.bottomInfoWidget.transfer.tooltip"));
		layout->addWidget(TotalLabel);

		ProviderLayout = new QVBoxLayout();
		ProviderLayout->setContentsMargins(0, 0, 0, 0);
		ProviderLayout->setSpacing(2);
		layout->addLayout(ProviderLayout);

		auto* separator = new QFrame(this);
		separator->setFrameShape(QFrame::HLine);
		separator->setFrameShadow(QFrame::Sunken);
		layout->addWidget(separator);

		CurrentLabel = new QLabel(this);
		CurrentLabel->setWordWrap(true);
		layout->addWidget(CurrentLabel);

		CurrentProgress = new QProgressBar(this);
		CurrentProgress->setTextVisible(false);
		CurrentProgress->setMinimum(0);
		CurrentProgress->setMaximum(100);
		layout->addWidget(CurrentProgress);

		QueuedLabel = new QLabel(this);
		layout->addWidget(QueuedLabel);

		RefreshTimer = new QTimer(this);
		RefreshTimer->setInterval(TransferRefreshIntervalMs);
		connect(RefreshTimer, &QTimer::timeout, this, [this]() { refresh(); });
	}

	void TransferMonitorPopup::popupFor(QWidget* anchor) {
		if (anchor == nullptr) {
			return;
		}
		// 先刷新再摆位置，否则第一次弹出会按旧的尺寸算高度。
		refresh();
		placeAbove(this, anchor, 260, 420);
		attachWindowBackground(Glass, this, anchor);
		show();
	}

	void TransferMonitorPopup::showEvent(QShowEvent* event) {
		QFrame::showEvent(event);
		RefreshTimer->start();
	}

	void TransferMonitorPopup::hideEvent(QHideEvent* event) {
		RefreshTimer->stop();
		QFrame::hideEvent(event);
	}

	void TransferMonitorPopup::refresh() {
		using YSSCore::General::YSSUniqueFileSystem;
		YSSUniqueFileSystem* ufs = YSSUniqueFileSystem::getInstance();
		const auto [readBytes, writtenBytes] = ufs->getTransferredBytes();
		// ↑ 写出（上传），↓ 读入（下载）。
		TotalLabel->setText(QStringLiteral("%1 %2 ↑ / %3 ↓")
			.arg(VITRL("YSS::editor.bottomInfoWidget.transfer.title"))
			.arg(formatBytes(writtenBytes))
			.arg(formatBytes(readBytes)));
		refreshProviders();
		refreshTasks();
	}

	void TransferMonitorPopup::refreshProviders() {
		using YSSCore::General::YSSUniqueFileSystem;
		YSSUniqueFileSystem* ufs = YSSUniqueFileSystem::getInstance();
		const QList<YSSCore::General::YSSFileSystemTransferStats> stats =
			ufs->getTransferredBytesByProvider();

		// 本机与所有已登记的 SSH 连接都列出来（没有流量显示 0），
		// 之后再补上只存在于统计里的历史 Provider（连接已被移除）。
		QStringList providerIds;
		providerIds.append(QStringLiteral("local"));
		for (const QString& authority : ufs->getSshAuthorities()) {
			if (!providerIds.contains(authority, Qt::CaseInsensitive)) {
				providerIds.append(authority);
			}
		}
		for (const auto& item : stats) {
			if (!providerIds.contains(item.ProviderId, Qt::CaseInsensitive)) {
				providerIds.append(item.ProviderId);
			}
		}

		// 行数很少变动，复用已有标签，避免每次刷新都重建控件。
		while (ProviderRows.size() < providerIds.size()) {
			auto* row = new QLabel(this);
			row->setContentsMargins(8, 0, 0, 0);
			ProviderLayout->addWidget(row);
			ProviderRows.append(row);
		}
		for (int i = 0; i < ProviderRows.size(); ++i) {
			if (i >= providerIds.size()) {
				ProviderRows[i]->hide();
				continue;
			}
			const QString& providerId = providerIds[i];
			qint64 written = 0;
			qint64 read = 0;
			for (const auto& item : stats) {
				if (QString::compare(item.ProviderId, providerId, Qt::CaseInsensitive) == 0) {
					written = item.BytesWritten;
					read = item.BytesRead;
					break;
				}
			}
			ProviderRows[i]->setText(QStringLiteral("%1    %2    %3")
				.arg(providerDisplayName(providerId))
				.arg(formatBytes(written))
				.arg(formatBytes(read)));
			ProviderRows[i]->setToolTip(providerId);
			ProviderRows[i]->show();
		}
	}

	void TransferMonitorPopup::refreshTasks() {
		using YSSCore::General::YSSUniqueFileSystem;
		YSSUniqueFileSystem* ufs = YSSUniqueFileSystem::getInstance();
		const QList<YSSCore::General::YSSFileSystemTask> tasks = ufs->getTasks();
		const YSSCore::General::YSSFileSystemTask* running = nullptr;
		for (const auto& task : tasks) {
			if (task.isRunning()) {
				running = &task;
				break;
			}
		}
		if (running == nullptr) {
			CurrentLabel->setText(VITRL("YSS::editor.bottomInfoWidget.transfer.idle"));
			CurrentProgress->setVisible(false);
		}
		else {
			CurrentLabel->setText(VITRL("YSS::editor.bottomInfoWidget.transfer.current")
				.arg(running->Uri.toString())
				.arg(operationName(running->Operation)));
			const qreal percent = running->progressPercent();
			CurrentProgress->setVisible(true);
			if (percent < 0) {
				// 总长度未知（非读写操作，或后端没给出尺寸）：显示忙碌指示。
				CurrentProgress->setMaximum(0);
			}
			else {
				CurrentProgress->setMaximum(100);
				CurrentProgress->setValue(qRound(percent));
			}
		}

		const int queued = ufs->getQueuedTaskCount();
		if (queued > 0) {
			QueuedLabel->setText(VITRL("YSS::editor.bottomInfoWidget.transfer.queued").arg(queued));
		}
		else {
			QueuedLabel->setText(VITRL("YSS::editor.bottomInfoWidget.transfer.queuedNone"));
		}
	}
}
