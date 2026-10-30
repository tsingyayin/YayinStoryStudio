#ifndef YayinStoryStudio_Editor_MainEditor_UfsTreeModel_h
#define YayinStoryStudio_Editor_MainEditor_UfsTreeModel_h
#include <General/YSSURI.h>
#include <General/YSSUniqueFileSystem.h>
#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qstringlist.h>
#include <functional>

namespace YSS::Editor {
	class UfsTreeModelPrivate;

	/*
		基于 YSSUniqueFileSystem 的目录树模型。

		节点身份是 YSSURI，目录在展开时才异步列目录，因此本机与 SSH 项目共用同一套
		视图代码。模型本身不做任何写操作，重命名/删除/复制交给调用方，完成后调用
		refreshDirectory() 或 refreshAll()。
	*/
	class UfsTreeModel :public QAbstractItemModel {
		Q_OBJECT;
	public:
		explicit UfsTreeModel(QObject* parent = nullptr);
		virtual ~UfsTreeModel() override;
	public:
		void setRoot(const YSSCore::General::YSSURI& root);
		YSSCore::General::YSSURI getRoot() const;
		YSSCore::General::YSSURI uriOf(const QModelIndex& index) const;
		bool isDirectory(const QModelIndex& index) const;
		// 已经列出来的子项名称；未加载时返回空列表。
		QStringList knownChildNames(const YSSCore::General::YSSURI& dir) const;
		bool isLoaded(const YSSCore::General::YSSURI& dir) const;
	public:
		void refreshAll();
		void refreshDirectory(const YSSCore::General::YSSURI& dir);
		// 保证 uri 所在路径被逐层展开；失败时回调收到无效 index。
		void reveal(const YSSCore::General::YSSURI& uri, std::function<void(const QModelIndex&)> done);
	public:
		virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
		virtual QModelIndex parent(const QModelIndex& child) const override;
		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
		virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
		virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
		virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
		virtual bool canFetchMore(const QModelIndex& parent) const override;
		virtual void fetchMore(const QModelIndex& parent) override;
	signals:
		void directoryFailed(const YSSCore::General::YSSURI& dir, const YSSCore::General::YSSFileError& error);
	private:
		friend class UfsTreeModelPrivate;
		UfsTreeModelPrivate* d;
	};
}
#endif // YayinStoryStudio_Editor_MainEditor_UfsTreeModel_h
