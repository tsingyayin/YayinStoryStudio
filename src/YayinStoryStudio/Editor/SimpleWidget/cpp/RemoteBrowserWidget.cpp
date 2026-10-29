#include <QtCore/qdir.h>
#include <QtGui/qpalette.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qinputdialog.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qlistwidget.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qtoolbutton.h>
#include <General/Log.h>
#include <General/Plugin.h>
#include <General/TranslationHost.h>
#include <General/VIApplication.h>
#include <General/YSSUniqueFileSystem.h>
#include <Utility/FileUtility.h>
#include <Utility/JsonConfig.h>
#include "Editor/SSHConnection/SSHConnectionWidget.h"
#include "Editor/SimpleWidget/RemoteBrowserWidget.h"
#include "YayinStoryStudio.h"

namespace YSS::Editor {
	namespace {
		QString authenticsKey() {
			return QStringLiteral("SSH.Authentics");
		}

		QStringList loadKnownAuthentics() {
			Visindigo::General::Plugin* plugin = VIApp->getMainPlugin();
			if (plugin == nullptr or plugin->getPluginConfig() == nullptr) {
				return QStringList();
			}
			QStringList authentics = plugin->getPluginConfig()->getStringList(authenticsKey());
			authentics.removeAll(QString());
			return authentics;
		}

		bool parseAuthority(const QString& authority, QString* user, QString* host, qint32* port) {
			const QString trimmed = authority.trimmed();
			const qsizetype at = trimmed.lastIndexOf(QLatin1Char('@'));
			if (at <= 0 or at >= trimmed.size() - 1) {
				return false;
			}
			QString hostPart = trimmed.mid(at + 1);
			qint32 portNumber = 22;
			const qsizetype colon = hostPart.lastIndexOf(QLatin1Char(':'));
			if (colon >= 0) {
				bool ok = false;
				const qint32 parsed = hostPart.mid(colon + 1).toInt(&ok);
				if (!ok or parsed < 1 or parsed > 65535) {
					return false;
				}
				portNumber = parsed;
				hostPart = hostPart.left(colon);
			}
			if (hostPart.isEmpty()) {
				return false;
			}
			*user = trimmed.left(at);
			*host = hostPart;
			*port = portNumber;
			return true;
		}
	}

	class RemoteBrowserWidgetPrivate {
		friend class RemoteBrowserWidget;
	protected:
		RemoteBrowserWidget* q = nullptr;
		QComboBox* AuthenticBox = nullptr;
		QToolButton* AddAuthenticButton = nullptr;
		QPushButton* RefreshButton = nullptr;
		QPushButton* ParentButton = nullptr;
		QPushButton* NewFolderButton = nullptr;
		QLineEdit* PathEdit = nullptr;
		QListWidget* EntryList = nullptr;
		QLabel* StatusLabel = nullptr;
		QStringList SuffixFilters;
		QString ListingUri;

		QString currentAuthentic() const {
			return AuthenticBox->currentText().trimmed();
		}

		YSSCore::General::YSSURI currentUri() const {
			QString user;
			QString host;
			qint32 port = 0;
			if (!parseAuthority(currentAuthentic(), &user, &host, &port)) {
				return YSSCore::General::YSSURI();
			}
			QString path = PathEdit->text().trimmed();
			if (path.isEmpty()) {
				path = QStringLiteral("/");
			}
			return YSSCore::General::YSSURI::fromSsh(user, host, port, path);
		}

		bool matchesFilter(const QString& fileName) const {
			if (SuffixFilters.isEmpty()) {
				return false;
			}
			for (const QString& suffix : SuffixFilters) {
				if (fileName.endsWith(suffix, Qt::CaseInsensitive)) {
					return true;
				}
			}
			return false;
		}

		void refreshAuthentics() {
			const QString current = currentAuthentic();
			QStringList authorities =
				YSSCore::General::YSSUniqueFileSystem::getInstance()->getSshAuthorities();
			for (const QString& authority : loadKnownAuthentics()) {
				if (!authorities.contains(authority)) {
					authorities.append(authority);
				}
			}
			QSignalBlocker blocker(AuthenticBox);
			AuthenticBox->clear();
			AuthenticBox->addItems(authorities);
			if (!current.isEmpty()) {
				AuthenticBox->setEditText(current);
			}
			else if (!authorities.isEmpty()) {
				AuthenticBox->setCurrentIndex(0);
			}
		}

		void refreshListing() {
			EntryList->clear();
			const YSSCore::General::YSSURI uri = currentUri();
			if (!uri.isValid()) {
				StatusLabel->setText(VITRL("YSS::project.locate.remoteNotConnected"));
				emit q->currentUriChanged(YSSCore::General::YSSURI());
				return;
			}
			if (!YSSCore::General::YSSUniqueFileSystem::getInstance()->isReady(uri)) {
				// 未连接时不去触发列目录请求，免得统一文件系统发出的“需要凭据”信号把连接页弹出来。
				StatusLabel->setText(VITRL("YSS::project.locate.remoteNotConnected"));
				emit q->currentUriChanged(YSSCore::General::YSSURI());
				return;
			}
			StatusLabel->setText(VITRL("YSS::project.locate.remoteLoading"));
			const QString requested = uri.toString();
			ListingUri = requested;
			YSSCore::General::YSSFileSystemReply* reply =
				YSSCore::General::YSSUniqueFileSystem::getInstance()->list(uri);
			if (reply == nullptr) {
				StatusLabel->setText(VITRL("YSS::project.locate.remoteNotConnected"));
				return;
			}
			QObject::connect(reply, &YSSCore::General::YSSFileSystemReply::finished, q,
				[this, reply, requested]() {
					if (ListingUri != requested) {
						return;
					}
					const YSSCore::General::YSSFileError error = reply->getError();
					if (error.isError()) {
						if (error.Type == YSSCore::General::YSSFileError::ErrorType::NeedsCredentials) {
							StatusLabel->setText(VITRL("YSS::project.locate.remoteNotConnected"));
						}
						else {
							StatusLabel->setText(error.toString());
						}
						emit q->currentUriChanged(YSSCore::General::YSSURI());
						return;
					}
					QStringList directories;
					QStringList files;
					for (const YSSCore::General::YSSFileEntry& entry : reply->getEntries()) {
						if (entry.Attributes.isDirectory()) {
							directories.append(entry.Name);
						}
						else if (matchesFilter(entry.Name)) {
							files.append(entry.Name);
						}
					}
					directories.sort(Qt::CaseInsensitive);
					files.sort(Qt::CaseInsensitive);
					for (const QString& name : directories) {
						QListWidgetItem* item = new QListWidgetItem(
							QStringLiteral("[ %1 ]").arg(name), EntryList);
						item->setData(Qt::UserRole, name);
						item->setData(Qt::UserRole + 1, true);
					}
					for (const QString& name : files) {
						QListWidgetItem* item = new QListWidgetItem(name, EntryList);
						item->setData(Qt::UserRole, name);
						item->setData(Qt::UserRole + 1, false);
					}
					StatusLabel->setText((directories.isEmpty() and files.isEmpty())
						? VITRL("YSS::project.locate.remoteEmpty") : QString());
					emit q->currentUriChanged(currentUri());
					emit q->selectionChanged();
					});
		}

		void navigateTo(const QString& path) {
			QString normalized = path.trimmed();
			if (normalized.isEmpty()) {
				normalized = QStringLiteral("/");
			}
			if (!normalized.startsWith(QLatin1Char('/'))) {
				normalized.prepend(QLatin1Char('/'));
			}
			PathEdit->setText(normalized);
			refreshListing();
		}

		void onEntryActivated() {
			QListWidgetItem* item = EntryList->currentItem();
			if (item == nullptr) {
				return;
			}
			const QString name = item->data(Qt::UserRole).toString();
			if (item->data(Qt::UserRole + 1).toBool()) {
				QString path = PathEdit->text().trimmed();
				if (!path.endsWith(QLatin1Char('/'))) {
					path += QLatin1Char('/');
				}
				navigateTo(path + name);
				return;
			}
			const YSSCore::General::YSSURI base = currentUri();
			if (base.isValid()) {
				emit q->fileActivated(base.resolve(name));
			}
		}

		void onParentClicked() {
			QString path = PathEdit->text().trimmed();
			if (path.isEmpty() or path == QStringLiteral("/")) {
				return;
			}
			while (path.endsWith(QLatin1Char('/'))) {
				path.chop(1);
			}
			const qsizetype slash = path.lastIndexOf(QLatin1Char('/'));
			path = slash <= 0 ? QStringLiteral("/") : path.left(slash);
			navigateTo(path);
		}

		void onNewFolderClicked() {
			const YSSCore::General::YSSURI base = currentUri();
			if (!base.isValid()) {
				return;
			}
			bool ok = false;
			const QString rawName = QInputDialog::getText(q,
				VITRL("YSS::project.locate.newFolderDialog.title"),
				VITRL("YSS::project.locate.newFolderDialog.text"),
				QLineEdit::Normal, QString(), &ok);
			if (!ok) {
				return;
			}
			const QString name = Visindigo::Utility::FileUtility::toLegalFileName(rawName).trimmed();
			if (name.isEmpty()) {
				return;
			}
			const YSSCore::General::YSSURI target = base.resolve(name);
			YSSCore::General::YSSFileSystemReply* reply =
				YSSCore::General::YSSUniqueFileSystem::getInstance()->mkdir(target);
			if (reply == nullptr) {
				QMessageBox::warning(q, VITRL("YSS::project.locate.newFolderDialog.failedTitle"),
					VITRL("YSS::project.locate.newFolderDialog.failedMessage").arg(target.toString()));
				return;
			}
			QObject::connect(reply, &YSSCore::General::YSSFileSystemReply::finished, q,
				[this, reply, target]() {
					const YSSCore::General::YSSFileError error = reply->getError();
					if (error.isError()
						and error.Type != YSSCore::General::YSSFileError::ErrorType::AlreadyExists) {
						QMessageBox::warning(q, VITRL("YSS::project.locate.newFolderDialog.failedTitle"),
							VITRL("YSS::project.locate.newFolderDialog.failedMessage").arg(target.toString()));
						return;
					}
					refreshListing();
					});
		}

		void applyStyle() {
			const QPalette pal = q->palette();
			const QColor window = pal.color(QPalette::Window);
			const QColor base = pal.color(QPalette::Base);
			const QColor text = pal.color(QPalette::WindowText);
			QColor accent = pal.color(QPalette::Highlight);
			if (!accent.isValid()) {
				accent = pal.color(QPalette::Accent);
			}
			const QColor onAccent = pal.color(QPalette::HighlightedText);
			const bool lightTheme = window.lightness() > 128;
			const QColor cardBg = lightTheme ? base : window.lighter(115);
			const QColor insetBg = lightTheme ? window.darker(104) : base;
			const QColor border = lightTheme ? window.darker(120) : window.lighter(140);
			const auto blend = [](const QColor& from, const QColor& to, qreal ratio) {
				return QColor::fromRgbF(
					from.redF() * (1.0 - ratio) + to.redF() * ratio,
					from.greenF() * (1.0 - ratio) + to.greenF() * ratio,
					from.blueF() * (1.0 - ratio) + to.blueF() * ratio);
				};
			const QColor dim = blend(text, cardBg, 0.45);
			const QColor accentFill = lightTheme ? accent : accent.darker(118);
			QString qss = QStringLiteral(R"QSS(
QComboBox#remoteAuthenticBox, QLineEdit#remotePathEdit {
	background: @inset;
	border: 1px solid @border;
	border-radius: 6px;
	padding: 6px 10px;
	min-height: 22px;
	color: @text;
}
QComboBox#remoteAuthenticBox:focus, QLineEdit#remotePathEdit:focus { border: 1px solid @accent; }
QComboBox#remoteAuthenticBox QLineEdit { background: transparent; border: none; padding: 0; }
QComboBox#remoteAuthenticBox::drop-down { border: none; width: 20px; }
QComboBox#remoteAuthenticBox QAbstractItemView {
	background: @card;
	border: 1px solid @border;
	color: @text;
	selection-background-color: @accent;
	selection-color: @onAccent;
}
QToolButton#remoteAddButton {
	background: @inset;
	border: 1px solid @border;
	border-radius: 6px;
	color: @text;
	font-size: 16px;
	font-weight: 600;
	min-width: 34px;
	min-height: 30px;
}
QToolButton#remoteAddButton:hover { border-color: @accent; color: @accent; }
QListWidget#remoteEntryList {
	background: @inset;
	border: 1px solid @border;
	border-radius: 8px;
	color: @text;
	padding: 4px;
}
QListWidget#remoteEntryList::item { padding: 5px 8px; border-radius: 4px; }
QListWidget#remoteEntryList::item:selected { background: @accent; color: @onAccent; }
QLabel#remoteStatusLabel { color: @dim; }
QPushButton {
	background: @inset;
	border: 1px solid @border;
	border-radius: 6px;
	color: @text;
	padding: 6px 16px;
}
QPushButton:hover { border-color: @accent; }
QPushButton:disabled { color: @dim; }
)QSS");
			qss.replace(QLatin1String("@text"), text.name());
			qss.replace(QLatin1String("@dim"), dim.name());
			qss.replace(QLatin1String("@card"), cardBg.name());
			qss.replace(QLatin1String("@inset"), insetBg.name());
			qss.replace(QLatin1String("@border"), border.name());
			qss.replace(QLatin1String("@accentFill"), accentFill.name());
			qss.replace(QLatin1String("@accent"), accent.name());
			qss.replace(QLatin1String("@onAccent"), onAccent.name());
			q->setStyleSheet(qss);
		}

		void buildUi() {
			auto* layout = new QVBoxLayout(q);
			layout->setContentsMargins(0, 0, 0, 0);
			layout->setSpacing(6);

			auto* authenticRow = new QHBoxLayout();
			authenticRow->setSpacing(6);
			AuthenticBox = new QComboBox(q);
			AuthenticBox->setEditable(true);
			AuthenticBox->setObjectName("remoteAuthenticBox");
			AddAuthenticButton = new QToolButton(q);
			AddAuthenticButton->setText(QStringLiteral("+"));
			AddAuthenticButton->setToolTip(VITRL("YSS::project.locate.addAuthentic"));
			AddAuthenticButton->setObjectName("remoteAddButton");
			RefreshButton = new QPushButton(VITRL("YSS::project.locate.refresh"), q);
			authenticRow->addWidget(AuthenticBox, 1);
			authenticRow->addWidget(AddAuthenticButton);
			authenticRow->addWidget(RefreshButton);
			layout->addLayout(authenticRow);

			auto* pathRow = new QHBoxLayout();
			pathRow->setSpacing(6);
			ParentButton = new QPushButton(VITRL("YSS::project.locate.up"), q);
			PathEdit = new QLineEdit(q);
			PathEdit->setText(QStringLiteral("/"));
			PathEdit->setObjectName("remotePathEdit");
			NewFolderButton = new QPushButton(VITRL("YSS::project.locate.newFolder"), q);
			pathRow->addWidget(ParentButton);
			pathRow->addWidget(PathEdit, 1);
			pathRow->addWidget(NewFolderButton);
			layout->addLayout(pathRow);

			EntryList = new QListWidget(q);
			EntryList->setObjectName("remoteEntryList");
			EntryList->setMinimumHeight(200);
			layout->addWidget(EntryList, 1);

			StatusLabel = new QLabel(q);
			StatusLabel->setObjectName("remoteStatusLabel");
			StatusLabel->setWordWrap(true);
			layout->addWidget(StatusLabel);

			QObject::connect(AddAuthenticButton, &QToolButton::clicked, q, [this]() {
				SSHConnectionWidget* ssh = SSHConnectionWidget::getInstance();
				ssh->setWindowModality(Qt::ApplicationModal);
				ssh->show();
				ssh->raise();
				ssh->activateWindow();
				});
			QObject::connect(RefreshButton, &QPushButton::clicked, q, [this]() {
				refreshAuthentics();
				refreshListing();
				});
			QObject::connect(ParentButton, &QPushButton::clicked, q, [this]() { onParentClicked(); });
			QObject::connect(NewFolderButton, &QPushButton::clicked, q, [this]() { onNewFolderClicked(); });
			QObject::connect(PathEdit, &QLineEdit::returnPressed, q, [this]() { navigateTo(PathEdit->text()); });
			QObject::connect(EntryList, &QListWidget::itemActivated, q, [this]() { onEntryActivated(); });
			QObject::connect(EntryList, &QListWidget::itemSelectionChanged, q, [this]() {

				emit q->selectionChanged();
				});
			QObject::connect(AuthenticBox, &QComboBox::currentTextChanged, q, [this](const QString&) {
				navigateTo(QStringLiteral("/"));
				});
			QObject::connect(SSHConnectionWidget::getInstance(), &SSHConnectionWidget::connectionReady, q,
				[this](const QString& authority) {
					refreshAuthentics();
					if (currentAuthentic() == authority) {
						refreshListing();
					}
				});
			applyStyle();
			refreshAuthentics();
		}
	};

	/*!
		\class YSS::Editor::RemoteBrowserWidget
		\brief 浏览远端 SSH 目录并选择目标位置的控件.
		\since YSS 0.18.0
		\inmodule YayinStoryStudio

		控件顶部是连接身份选择与刷新按钮，中间是路径栏，下面是当前目录的子项列表。
		连接身份下拉框合并了统一文件系统里已连接的连接与主插件配置里 \c{SSH.Authentics}
		记录的历史连接；右侧的加号可以直接打开 SSH 连接页建立新连接。

		默认只列出子文件夹。用 \l{setFileSuffixFilters()} 给出后缀后，匹配的文件也会出现在
		列表里，双击这类文件会发出 \l{fileActivated()}。

		本控件只负责展示与选择，不会写盘；需要建目录时会通过 \l{YSSUniqueFileSystem::mkdir()}
		提交请求。
	*/

	/*!
		\since YSS 0.18.0
		\a parent 父组件

		构造函数
	*/
	RemoteBrowserWidget::RemoteBrowserWidget(QWidget* parent)
		: QWidget(parent) {
		d = new RemoteBrowserWidgetPrivate;
		d->q = this;
		d->buildUi();
	}

	/*!
		\since YSS 0.18.0

		析构函数
	*/
	RemoteBrowserWidget::~RemoteBrowserWidget() {
		delete d;
	}

	/*!
		\since YSS 0.18.0
		\a suffixes 需要显示的文件后缀，例如 \c{"yssp"}；传空列表则只显示文件夹。

		设置列表里需要显示的文件后缀。比较时不区分大小写，按结尾匹配，因此
		\c{"yssproj.json"} 这类多段后缀同样可用。
	*/
	void RemoteBrowserWidget::setFileSuffixFilters(const QStringList& suffixes) {
		d->SuffixFilters = suffixes;
	}

	/*!
		\since YSS 0.18.0

		return 当前目录的 URI；连接身份无效时返回无效 URI。
	*/
	YSSCore::General::YSSURI RemoteBrowserWidget::getCurrentUri() const {
		return d->currentUri();
	}

	/*!
		\since YSS 0.18.0

		return 当前目录的路径文本（以 \c{/} 开头）。
	*/
	QString RemoteBrowserWidget::getCurrentPath() const {
		return d->PathEdit->text().trimmed();
	}

	/*!
		\since YSS 0.18.0

		return 列表里当前选中的文件 URI；没有选中文件（或选中的是文件夹）时返回无效 URI。
	*/
	YSSCore::General::YSSURI RemoteBrowserWidget::getSelectedFileUri() const {
		const QList<QListWidgetItem*> selected = d->EntryList->selectedItems();
		QListWidgetItem* item = selected.isEmpty() ? d->EntryList->currentItem() : selected.first();
		if (item == nullptr or item->data(Qt::UserRole + 1).toBool()) {
			return YSSCore::General::YSSURI();
		}
		const YSSCore::General::YSSURI base = d->currentUri();
		if (!base.isValid()) {
			return YSSCore::General::YSSURI();
		}
		return base.resolve(item->data(Qt::UserRole).toString());
	}

	/*!
		\since YSS 0.18.0
		\a path 目标目录路径

		跳转到 \a path 并刷新列表。传入相对路径时按根目录下的路径处理。
	*/
	void RemoteBrowserWidget::navigateTo(const QString& path) {
		d->navigateTo(path);
	}

	/*!
		\since YSS 0.18.0

		重新读取当前目录。
	*/
	void RemoteBrowserWidget::refresh() {
		d->refreshListing();
	}

	/*!
		\since YSS 0.18.0

		重新读取连接身份列表。
	*/
	void RemoteBrowserWidget::refreshAuthentics() {
		d->refreshAuthentics();
	}
}
