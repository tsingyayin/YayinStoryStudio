#include <QtCore/qfileinfo.h>
#include <QtGui/qicon.h>
#include <QtWidgets/qfileiconprovider.h>
#include <algorithm>
#include <memory>
#include <vector>
#include "Editor/MainEditor/UfsTreeModel.h"

namespace YSS::Editor {
	namespace {
		struct UfsTreeNode {
			YSSCore::General::YSSURI Uri;
			QString Name;
			bool IsDir = true;
			bool ChildrenLoaded = false;
			bool Loading = false;
			bool ReloadPending = false;
			bool Watching = false;
			quint64 ListingToken = 0;
			UfsTreeNode* Parent = nullptr;
			std::vector<std::unique_ptr<UfsTreeNode>> Children;
		};

		// 目录优先，其次按名称不区分大小写排序。
		bool entryLess(const YSSCore::General::YSSFileEntry& left, const YSSCore::General::YSSFileEntry& right) {
			const bool leftDir = left.Attributes.isDirectory();
			const bool rightDir = right.Attributes.isDirectory();
			if (leftDir != rightDir) {
				return leftDir;
			}
			return left.Name.compare(right.Name, Qt::CaseInsensitive) < 0;
		}

		UfsTreeNode* findChildNode(UfsTreeNode* node, const YSSCore::General::YSSURI& uri) {
			for (const std::unique_ptr<UfsTreeNode>& child : node->Children) {
				if (child->Uri == uri) {
					return child.get();
				}
				if (UfsTreeNode* found = findChildNode(child.get(), uri)) {
					return found;
				}
			}
			return nullptr;
		}

		void collectLoadedDirs(UfsTreeNode* node, QList<YSSCore::General::YSSURI>& dirs) {
			if (node->ChildrenLoaded) {
				dirs.append(node->Uri);
			}
			for (const std::unique_ptr<UfsTreeNode>& child : node->Children) {
				if (child->IsDir) {
					collectLoadedDirs(child.get(), dirs);
				}
			}
		}
	}

	class UfsTreeModelPrivate {
	public:
		UfsTreeModel* q = nullptr;
		std::unique_ptr<UfsTreeNode> Root;
		QFileIconProvider IconProvider;
		QIcon FolderIcon;
		QIcon FileIcon;
		quint64 NextToken = 0;
		QList<QPair<YSSCore::General::YSSURI, std::function<void(const QModelIndex&)>>> PendingReveals;

		UfsTreeNode* nodeOf(const QModelIndex& index) const {
			if (!index.isValid()) {
				return Root.get();
			}
			return static_cast<UfsTreeNode*>(index.internalPointer());
		}

		int rowOf(const UfsTreeNode* node) const {
			if (node == nullptr || node->Parent == nullptr) {
				return -1;
			}
			int row = 0;
			for (const std::unique_ptr<UfsTreeNode>& child : node->Parent->Children) {
				if (child.get() == node) {
					return row;
				}
				++row;
			}
			return -1;
		}

		UfsTreeNode* findNode(const YSSCore::General::YSSURI& uri) const {
			if (!Root) {
				return nullptr;
			}
			if (Root->Uri == uri) {
				return Root.get();
			}
			return findChildNode(Root.get(), uri);
		}

		QModelIndex indexOfNode(UfsTreeNode* node) const {
			if (node == nullptr || node->Parent == nullptr) {
				return QModelIndex();
			}
			QList<UfsTreeNode*> chain;
			for (UfsTreeNode* current = node; current != nullptr && current->Parent != nullptr;
				current = current->Parent) {
				chain.prepend(current);
			}
			QModelIndex parentIndex;
			for (UfsTreeNode* current : chain) {
				parentIndex = q->createIndex(rowOf(current), 0, current);
			}
			return parentIndex;
		}

		QIcon iconOf(const UfsTreeNode* node) const {
			if (node->IsDir) {
				return FolderIcon;
			}
			const QIcon icon = IconProvider.icon(QFileInfo(node->Name));
			return icon.isNull() ? FileIcon : icon;
		}

		void requestListing(UfsTreeNode* node, bool force);
		void applyEntries(UfsTreeNode* node, const QList<YSSCore::General::YSSFileEntry>& entries);
		void processReveals();
	};

	void UfsTreeModelPrivate::requestListing(UfsTreeNode* node, bool force) {
		if (node == nullptr || !node->IsDir) {
			return;
		}
		if (node->Loading) {
			if (force) {
				node->ReloadPending = true;
			}
			return;
		}
		if (node->ChildrenLoaded && !force) {
			return;
		}
		node->Loading = true;
		const quint64 token = ++NextToken;
		node->ListingToken = token;
		const YSSCore::General::YSSURI dirUri = node->Uri;
		YSSCore::General::YSSFileSystemReply* reply =
			YSSCore::General::YSSUniqueFileSystem::getInstance()->list(dirUri);
		if (reply == nullptr) {
			node->Loading = false;
			node->ChildrenLoaded = true;
			YSSCore::General::YSSFileError error;
			error.Type = YSSCore::General::YSSFileError::ErrorType::NotSupported;
			error.Message = QStringLiteral("No filesystem backend accepts this path");
			error.Detail = dirUri.toString();
			emit q->directoryFailed(dirUri, error);
			return;
		}
		QObject::connect(reply, &YSSCore::General::YSSFileSystemReply::succeeded, q,
			[this, reply, dirUri, token]() {
				UfsTreeNode* self = this->findNode(dirUri);
				if (self == nullptr || self->ListingToken != token) {
					return;
				}
				self->Loading = false;
				self->ChildrenLoaded = true;
				this->applyEntries(self, reply->getEntries());
				// 本机后端支持监听时顺手挂上，外部改动也能自动刷新（QFileSystemModel 以前白送的能力）。
				if (!self->Watching) {
					YSSCore::General::YSSFileError watchError;
					self->Watching = YSSCore::General::YSSUniqueFileSystem::getInstance()->watchDirectory(dirUri, &watchError);
				}
				this->processReveals();
				if (self->ReloadPending) {
					self->ReloadPending = false;
					this->requestListing(self, true);
				}
			});
		QObject::connect(reply, &YSSCore::General::YSSFileSystemReply::failed, q,
			[this, dirUri, token](const YSSCore::General::YSSFileError& error) {
				UfsTreeNode* self = this->findNode(dirUri);
				if (self == nullptr || self->ListingToken != token) {
					return;
				}
				self->Loading = false;
				// 列目录失败也标记为已加载，避免视图反复触发重试；手动刷新可以再来一次。
				self->ChildrenLoaded = true;
				emit q->directoryFailed(dirUri, error);
				this->processReveals();
			});
	}

	void UfsTreeModelPrivate::applyEntries(UfsTreeNode* node,
		const QList<YSSCore::General::YSSFileEntry>& entries) {
		QList<YSSCore::General::YSSFileEntry> sorted;
		sorted.reserve(entries.size());
		for (const YSSCore::General::YSSFileEntry& entry : entries) {
			if (entry.Name.isEmpty() or entry.Name == QLatin1String(".") or entry.Name == QLatin1String("..")) {
				continue;
			}
			if (entry.Attributes.hasField(YSSCore::General::YSSFileAttributes::Field::IsHidden)
				and entry.Attributes.isHidden()) {
				continue;
			}
			sorted.append(entry);
		}
		std::sort(sorted.begin(), sorted.end(), entryLess);

		const QModelIndex parentIndex = indexOfNode(node);

		// 先删掉已经不存在的项，从后往前删，行号不会错位。
		for (int row = static_cast<int>(node->Children.size()) - 1; row >= 0; --row) {
			const QString name = node->Children[row]->Name;
			bool stillThere = false;
			for (const YSSCore::General::YSSFileEntry& entry : sorted) {
				if (entry.Name == name) {
					stillThere = true;
					break;
				}
			}
			if (!stillThere) {
				q->beginRemoveRows(parentIndex, row, row);
				node->Children.erase(node->Children.begin() + row);
				q->endRemoveRows();
			}
		}

		// 再按顺序补齐缺失项，保留已有节点（连同它们的子树与展开状态）。
		for (int row = 0; row < static_cast<int>(sorted.size()); ++row) {
			const YSSCore::General::YSSFileEntry& entry = sorted.at(row);
			if (row < static_cast<int>(node->Children.size()) && node->Children[row]->Name == entry.Name) {
				node->Children[row]->IsDir = entry.Attributes.isDirectory();
				continue;
			}
			q->beginInsertRows(parentIndex, row, row);
			auto child = std::make_unique<UfsTreeNode>();
			child->Parent = node;
			child->Name = entry.Name;
			child->Uri = node->Uri.resolve(entry.Name);
			child->IsDir = entry.Attributes.isDirectory();
			node->Children.insert(node->Children.begin() + row, std::move(child));
			q->endInsertRows();
		}
	}

	void UfsTreeModelPrivate::processReveals() {
		for (int i = PendingReveals.size() - 1; i >= 0; --i) {
			const QPair<YSSCore::General::YSSURI, std::function<void(const QModelIndex&)>> item = PendingReveals.at(i);
			if (UfsTreeNode* node = findNode(item.first)) {
				PendingReveals.removeAt(i);
				item.second(indexOfNode(node));
				continue;
			}
			UfsTreeNode* parentNode = findNode(item.first.parent());
			if (parentNode == nullptr) {
				continue;
			}
			if (!parentNode->ChildrenLoaded) {
				requestListing(parentNode, false);
				continue;
			}
			PendingReveals.removeAt(i);
			item.second(QModelIndex());
		}
	}

	UfsTreeModel::UfsTreeModel(QObject* parent) :QAbstractItemModel(parent) {
		d = new UfsTreeModelPrivate();
		d->q = this;
		d->FolderIcon = d->IconProvider.icon(QFileIconProvider::Folder);
		d->FileIcon = d->IconProvider.icon(QFileIconProvider::File);
	}

	UfsTreeModel::~UfsTreeModel() {
		delete d;
	}

	void UfsTreeModel::setRoot(const YSSCore::General::YSSURI& root) {
		beginResetModel();
		d->Root = std::make_unique<UfsTreeNode>();
		d->Root->Uri = root;
		d->Root->Name = root.getFileName();
		if (d->Root->Name.isEmpty()) {
			d->Root->Name = root.toString();
		}
		d->Root->IsDir = true;
		d->PendingReveals.clear();
		endResetModel();
		d->requestListing(d->Root.get(), true);
	}

	YSSCore::General::YSSURI UfsTreeModel::getRoot() const {
		return d->Root ? d->Root->Uri : YSSCore::General::YSSURI();
	}

	YSSCore::General::YSSURI UfsTreeModel::uriOf(const QModelIndex& index) const {
		UfsTreeNode* node = d->nodeOf(index);
		return node ? node->Uri : YSSCore::General::YSSURI();
	}

	bool UfsTreeModel::isDirectory(const QModelIndex& index) const {
		UfsTreeNode* node = d->nodeOf(index);
		return node ? node->IsDir : false;
	}

	QStringList UfsTreeModel::knownChildNames(const YSSCore::General::YSSURI& dir) const {
		QStringList names;
		UfsTreeNode* node = d->findNode(dir);
		if (node == nullptr) {
			return names;
		}
		names.reserve(node->Children.size());
		for (const std::unique_ptr<UfsTreeNode>& child : node->Children) {
			names.append(child->Name);
		}
		return names;
	}

	bool UfsTreeModel::isLoaded(const YSSCore::General::YSSURI& dir) const {
		UfsTreeNode* node = d->findNode(dir);
		return node != nullptr && node->ChildrenLoaded;
	}

	void UfsTreeModel::refreshAll() {
		if (!d->Root) {
			return;
		}
		QList<YSSCore::General::YSSURI> dirs;
		collectLoadedDirs(d->Root.get(), dirs);
		for (const YSSCore::General::YSSURI& dir : dirs) {
			refreshDirectory(dir);
		}
	}

	void UfsTreeModel::refreshDirectory(const YSSCore::General::YSSURI& dir) {
		if (UfsTreeNode* node = d->findNode(dir)) {
			d->requestListing(node, true);
		}
	}

	void UfsTreeModel::reveal(const YSSCore::General::YSSURI& uri,
		std::function<void(const QModelIndex&)> done) {
		if (!d->Root || !uri.isValid()) {
			done(QModelIndex());
			return;
		}
		if (UfsTreeNode* node = d->findNode(uri)) {
			done(d->indexOfNode(node));
			return;
		}
		d->PendingReveals.append(qMakePair(uri, std::move(done)));
		d->requestListing(d->Root.get(), false);
		d->processReveals();
	}

	QModelIndex UfsTreeModel::index(int row, int column, const QModelIndex& parent) const {
		UfsTreeNode* parentNode = d->nodeOf(parent);
		if (parentNode == nullptr || row < 0 || row >= static_cast<int>(parentNode->Children.size())) {
			return QModelIndex();
		}
		return createIndex(row, column, parentNode->Children[row].get());
	}

	QModelIndex UfsTreeModel::parent(const QModelIndex& child) const {
		UfsTreeNode* node = d->nodeOf(child);
		if (node == nullptr || node->Parent == nullptr || node->Parent == d->Root.get()) {
			return QModelIndex();
		}
		const int row = d->rowOf(node->Parent);
		if (row < 0) {
			return QModelIndex();
		}
		return createIndex(row, 0, node->Parent);
	}

	int UfsTreeModel::rowCount(const QModelIndex& parent) const {
		if (parent.isValid() && parent.column() != 0) {
			return 0;
		}
		UfsTreeNode* node = d->nodeOf(parent);
		return node ? static_cast<int>(node->Children.size()) : 0;
	}

	int UfsTreeModel::columnCount(const QModelIndex& parent) const {
		Q_UNUSED(parent);
		return 1;
	}

	QVariant UfsTreeModel::data(const QModelIndex& index, int role) const {
		if (!index.isValid()) {
			return QVariant();
		}
		UfsTreeNode* node = d->nodeOf(index);
		if (node == nullptr) {
			return QVariant();
		}
		switch (role) {
		case Qt::DisplayRole:
			return node->Name;
		case Qt::DecorationRole:
			return d->iconOf(node);
		case Qt::ToolTipRole:
			return node->Uri.toString();
		case Qt::UserRole:
			return QVariant::fromValue(node->Uri);
		default:
			return QVariant();
		}
	}

	Qt::ItemFlags UfsTreeModel::flags(const QModelIndex& index) const {
		if (!index.isValid()) {
			return Qt::NoItemFlags;
		}
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	bool UfsTreeModel::hasChildren(const QModelIndex& parent) const {
		UfsTreeNode* node = d->nodeOf(parent);
		if (node == nullptr || !node->IsDir) {
			return false;
		}
		if (!node->ChildrenLoaded) {
			return true;
		}
		return !node->Children.empty();
	}

	bool UfsTreeModel::canFetchMore(const QModelIndex& parent) const {
		UfsTreeNode* node = d->nodeOf(parent);
		return node != nullptr && node->IsDir && !node->ChildrenLoaded && !node->Loading;
	}

	void UfsTreeModel::fetchMore(const QModelIndex& parent) {
		UfsTreeNode* node = d->nodeOf(parent);
		d->requestListing(node, false);
	}
}
