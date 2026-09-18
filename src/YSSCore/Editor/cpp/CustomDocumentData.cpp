#include <typeindex>
#include <QtCore/qmap.h>
#include <QtGui/qtextdocument.h>
#include <QtGui/qtextobject.h>
#include <General/Log.h>
#include "Editor/CustomDocumentData.h"
#include "Editor/TextEdit.h"
#include "Editor/private/TextEdit_p.h"

/*!
   \class YSSCore::Editor::ICustomDocumentData
   \brief 可挂在文本编辑器某一行上的自定义数据的基类。
   \since YSS 0.17.0
   \inmodule YSSCore

   继承本类，并把对象通过 YSSCore::Editor::TextEdit::setBlockData 交给编辑器，之后就可以按行号取回它。
   挂载、取出、生命周期等使用方式，见 YSSCore::Editor::TextEdit 的“自定义文档数据”一节。
*/

/*!
   \since YSS 0.17.0
   构造函数。
*/
YSSCore::Editor::ICustomDocumentData::ICustomDocumentData() {
}

/*!
   \since YSS 0.17.0
   析构函数。
*/
YSSCore::Editor::ICustomDocumentData::~ICustomDocumentData() {
}

/*!
   \since YSS 0.17.0
   \return 数据当前所在的文本块。
*/
QTextBlock YSSCore::Editor::ICustomDocumentData::getBlock() const {
	return Block;
}

/*!
   \since YSS 0.17.0
   \return 数据当前所在的行号。如果数据所在的块已经不存在，则返回 -1。
*/
qint32 YSSCore::Editor::ICustomDocumentData::getBlockNumber() const {
	return Block.blockNumber();
}

/*!
   \since YSS 0.17.0
   \return 上一次同步时记录的行号。
*/
qint32 YSSCore::Editor::ICustomDocumentData::getLastBlockNumber() const {
	return LastBlockNumber;
}

/*!
   \since YSS 0.17.0
   \return 自上次同步以来，数据所在的行号是否发生了变化。
*/
bool YSSCore::Editor::ICustomDocumentData::isBlockNumberChanged() const {
	return LastBlockNumber != getBlockNumber();
}

/*!
   \since YSS 0.17.0
   把记录的行号同步为数据当前所在的行号；如果行号发生了变化，会调用 onBlockNumberChanged。
*/
void YSSCore::Editor::ICustomDocumentData::syncBlockNumber() {
	qint32 current = getBlockNumber();
	if (LastBlockNumber == current) {
		return;
	}
	LastBlockNumber = current;
	onBlockNumberChanged(current);
}

/*!
   \since YSS 0.17.0
   \a newBlockNumber 数据现在所在的行号。

   当数据所在行的行号发生变化时被调用。默认实现为空。
*/
void YSSCore::Editor::ICustomDocumentData::onBlockNumberChanged(qint32 newBlockNumber) {
}

/*!
   \since YSS 0.17.0
   \a block 数据要挂载到的文本块。

   把数据与文本块绑定，并把当前行号记录为初始值。只应由框架调用。
*/
void YSSCore::Editor::ICustomDocumentData::setBlock(const QTextBlock& block) {
	Block = block;
	LastBlockNumber = Block.blockNumber();
}

namespace YSSCore::Editor {
	/*!
	   挂在文本块上的数据容器。

	   QTextBlock 只允许挂一个 QTextBlockUserData，为了让多个组件的数据共存，
	   这里用一个容器按类型分槽保存它们：容器由框架懒创建并交给块持有（块负责释放），
	   容器析构时释放全部数据对象。
	   除此之外不应有任何人直接操作块上的 userData，否则会破坏其它组件的数据。
	*/
	class CustomDataProxy :public QTextBlockUserData {
	public:
		QMap<std::type_index, ICustomDocumentData*> DataMap;
		virtual ~CustomDataProxy();
	public:
		void attach(ICustomDocumentData* data, QTextBlock block);
	};

	CustomDataProxy::~CustomDataProxy() {
		for (auto it = DataMap.begin(); it != DataMap.end(); ++it) {
			delete it.value();
		}
		DataMap.clear();
	}

	void CustomDataProxy::attach(ICustomDocumentData* data, QTextBlock block) {
		data->setBlock(block);
	}

	// 取出块上已有的容器。块上没挂东西、或者挂的是别人的数据时返回 nullptr。
	static CustomDataProxy* getProxy(QTextBlock block) {
		if (not block.isValid()) {
			return nullptr;
		}
		return dynamic_cast<CustomDataProxy*>(block.userData());
	}

	// 取出或创建块上的容器。块上已经有别人的数据时放弃，不去破坏它。
	static CustomDataProxy* ensureProxy(QTextBlock block) {
		if (not block.isValid()) {
			return nullptr;
		}
		CustomDataProxy* proxy = getProxy(block);
		if (proxy != nullptr) {
			return proxy;
		}
		if (block.userData() != nullptr) {
			vgWarningF << "The block" << block.blockNumber() << "already has foreign user data, give up storing custom data.";
			return nullptr;
		}
		proxy = new CustomDataProxy();
		block.setUserData(proxy);
		return proxy;
	}
}

namespace YSSCore::Editor {
	/*!
	   \fn template<typename T> T* TextEdit::getBlockData(qint32 blockNumber) const
	   \since YSS 0.17.0
	   \a blockNumber 行号，从 0 开始。

	   \return 指定行上挂着的 T 类型数据；该行不存在、或者该行没挂 T 类型的数据时返回空指针。

	   类型在编译期已知时用这个重载：不必自己给出类型标识，返回值也已经是 T*。
	*/

	/*!
	   \fn ICustomDocumentData* TextEdit::getBlockData(qint32 blockNumber, const std::type_index& type) const
	   \since YSS 0.17.0
	   \a blockNumber 行号，从 0 开始。
	   \a type 数据的类型标识，用 std::type_index(typeid(YourDataType)) 得到。

	   \return 指定行上挂着的数据。如果该行不存在，或者这一行没挂这种类型的数据，则返回空指针。

	   取到之后可以直接转换成你自己继承 ICustomDocumentData 的类型使用。
	   类型在编译期已知时，直接用带类型参数的模板重载即可，不必自己构造类型标识。
	   \warning 返回的指针由编辑器管理，不要手动删除它。
	*/
	ICustomDocumentData* TextEdit::getBlockData(qint32 blockNumber, const std::type_index& type) const {
		QTextBlock block = d->Text->document()->findBlockByNumber(blockNumber);
		CustomDataProxy* proxy = getProxy(block);
		if (proxy == nullptr) {
			return nullptr;
		}
		auto it = proxy->DataMap.constFind(type);
		if (it == proxy->DataMap.constEnd()) {
			return nullptr;
		}
		ICustomDocumentData* data = it.value();
		data->syncBlockNumber();
		return data;
	}

	/*!
	   \fn template<typename T> bool TextEdit::hasBlockData(qint32 blockNumber) const
	   \since YSS 0.17.0
	   \a blockNumber 行号，从 0 开始。

	   \return 指定行上是否挂着 T 类型的数据。
	*/

	/*!
	   \fn bool TextEdit::hasBlockData(qint32 blockNumber, const std::type_index& type) const
	   \since YSS 0.17.0
	   \a blockNumber 行号，从 0 开始。
	   \a type 数据的类型标识。

	   \return 指定行上是否挂着这种类型的数据。
	   类型在编译期已知时改用带类型参数的模板重载。
	*/
	bool TextEdit::hasBlockData(qint32 blockNumber, const std::type_index& type) const {
		return getBlockData(blockNumber, type) != nullptr;
	}

	/*!
	   \fn template<typename T> void TextEdit::setBlockData(qint32 blockNumber, T* data)
	   \since YSS 0.17.0
	   \a blockNumber 行号，从 0 开始。
	   \a data 要挂上去的数据，类型为 T。

	   类型在编译期已知时用这个重载：不必自己给出类型标识，其余行为与下面那个重载一致。
	*/

	/*!
	   \fn void TextEdit::setBlockData(qint32 blockNumber, const std::type_index& type, ICustomDocumentData* data)
	   \since YSS 0.17.0
	   \a blockNumber 行号，从 0 开始。
	   \a type 数据的类型标识，用 std::type_index(typeid(YourDataType)) 得到。
	   \a data 要挂上去的数据，类型必须与 \a type 对应。

	   把数据挂到指定行上。同一行同一种类型只允许存在一个数据对象，重复设置会先释放旧对象。
	   数据交给编辑器之后，它的生命周期就由编辑器管理，不要再手动删除它，也不要把同一个对象挂到多行上。
	   如果 \a data 与已经挂着的是同一个对象，则什么也不做。
	*/
	void TextEdit::setBlockData(qint32 blockNumber, const std::type_index& type, ICustomDocumentData* data) {
		if (data == nullptr) {
			return;
		}
		QTextBlock block = d->Text->document()->findBlockByNumber(blockNumber);
		CustomDataProxy* proxy = ensureProxy(block);
		if (proxy == nullptr) {
			return;
		}
		auto it = proxy->DataMap.find(type);
		if (it != proxy->DataMap.end()) {
			if (it.value() == data) {
				return;
			}
			delete it.value();
		}
		proxy->DataMap.insert(type, data);
		proxy->attach(data, block);
	}

	/*!
	   \fn template<typename T> void TextEdit::removeBlockData(qint32 blockNumber)
	   \since YSS 0.17.0
	   \a blockNumber 行号，从 0 开始。

	   释放并移除指定行上挂着的 T 类型数据。
	*/

	/*!
	   \fn void TextEdit::removeBlockData(qint32 blockNumber, const std::type_index& type)
	   \since YSS 0.17.0
	   \a blockNumber 行号，从 0 开始。
	   \a type 数据的类型标识。

	   释放并移除指定行上挂着的这种类型的数据。如果该行没有这种类型的数据，则什么也不做。
	   类型在编译期已知时改用带类型参数的模板重载。
	*/
	void TextEdit::removeBlockData(qint32 blockNumber, const std::type_index& type) {
		QTextBlock block = d->Text->document()->findBlockByNumber(blockNumber);
		CustomDataProxy* proxy = getProxy(block);
		if (proxy == nullptr) {
			return;
		}
		auto it = proxy->DataMap.find(type);
		if (it == proxy->DataMap.end()) {
			return;
		}
		delete it.value();
		proxy->DataMap.erase(it);
	}

	/*!
	   \fn template<typename T> void TextEdit::removeAllBlockData()
	   \since YSS 0.17.0

	   释放并移除整个文档中所有 T 类型的数据。
	*/

	/*!
	   \fn void TextEdit::removeAllBlockData(const std::type_index& type)
	   \since YSS 0.17.0
	   \a type 数据的类型标识。

	   释放并移除整个文档中所有这种类型的数据，常用于整篇重新解析前的清理。
	   类型在编译期已知时改用带类型参数的模板重载。
	*/
	void TextEdit::removeAllBlockData(const std::type_index& type) {
		QTextDocument* document = d->Text->document();
		for (QTextBlock block = document->begin(); block.isValid(); block = block.next()) {
			CustomDataProxy* proxy = getProxy(block);
			if (proxy == nullptr) {
				continue;
			}
			auto it = proxy->DataMap.find(type);
			if (it == proxy->DataMap.end()) {
				continue;
			}
			delete it.value();
			proxy->DataMap.erase(it);
		}
	}

	/*!
	   \fn template<typename T> QMap<qint32, T*> TextEdit::getAllBlockData() const
	   \since YSS 0.17.0

	   \return 整个文档中所有 T 类型的数据，以行号为键。

	   类型在编译期已知时用这个重载：拿到的就是一份 T* 的映射，不用再转换。
	*/

	/*!
	   \fn QMap<qint32, ICustomDocumentData*> TextEdit::getAllBlockData(const std::type_index& type) const
	   \since YSS 0.17.0
	   \a type 数据的类型标识。

	   \return 整个文档中所有这种类型的数据，以行号为键。

	   返回的是一份快照，键是取数据那一刻的行号；数据本身仍然由编辑器管理，不要手动删除。
	*/
	QMap<qint32, ICustomDocumentData*> TextEdit::getAllBlockData(const std::type_index& type) const {
		QMap<qint32, ICustomDocumentData*> result;
		QTextDocument* document = d->Text->document();
		for (QTextBlock block = document->begin(); block.isValid(); block = block.next()) {
			CustomDataProxy* proxy = getProxy(block);
			if (proxy == nullptr) {
				continue;
			}
			auto it = proxy->DataMap.constFind(type);
			if (it == proxy->DataMap.constEnd()) {
				continue;
			}
			ICustomDocumentData* data = it.value();
			data->syncBlockNumber();
			result.insert(block.blockNumber(), data);
		}
		return result;
	}
}
