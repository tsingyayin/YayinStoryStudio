#include "Utility/GeneralConfig.h"
#include "Utility/private/GeneralConfig_p.h"
#include "General/Log.h"

namespace Visindigo::__Private__ {
	GeneralConfigPrivate::GeneralConfigPrivate(Visindigo::Utility::GeneralConfig * q) {
		this->q = q;
	}

	GeneralConfigPrivate::~GeneralConfigPrivate() {
		if (Type == Visindigo::Utility::GeneralConfig::String || Type == Visindigo::Utility::GeneralConfig::Reference) {
			QString* ptr = (QString*)Value;
			if (ptr) {
				delete ptr;
			}
		}
		else if (Type == Visindigo::Utility::GeneralConfig::List) {
			auto list = (Visindigo::Utility::GeneralConfigList*)Value;
			for (auto item : *list) {
				delete item;
			}
			delete list;
		}
		else if (Type == Visindigo::Utility::GeneralConfig::Map) {
			auto map = (Visindigo::Utility::GeneralConfigMap*)Value;
			for (auto item : map->values()) {
				delete item;
			}
			delete map;
		}
		Type = Visindigo::Utility::GeneralConfig::None;
		Value = 0;
	}

	void GeneralConfigPrivate::copyFrom(GeneralConfigPrivate* other) {
		this->Type = other->Type;
		if (Type == Visindigo::Utility::GeneralConfig::String || Type == Visindigo::Utility::GeneralConfig::Reference) {
			this->Value = (qint64)(new QString(*(QString*)other->Value));
		}
		else if (Type == Visindigo::Utility::GeneralConfig::List) {
			auto otherList = (Visindigo::Utility::GeneralConfigList*)other->Value;
			auto newList = new Visindigo::Utility::GeneralConfigList();
			for (auto item : *otherList) {
				Visindigo::Utility::GeneralConfig* newItem = Visindigo::Utility::GeneralConfig::copyFrom(item);
				newItem->d->Parent = this->q;
				newList->append(newItem);
			}
			this->Value = (qint64)newList;
		}
		else if (Type == Visindigo::Utility::GeneralConfig::Map) {
			auto otherMap = (Visindigo::Utility::GeneralConfigMap*)other->Value;
			auto newMap = new Visindigo::Utility::GeneralConfigMap();
			for (auto key : otherMap->keys()) {
				Visindigo::Utility::GeneralConfig* newItem = Visindigo::Utility::GeneralConfig::copyFrom((*otherMap)[key]);
				newItem->d->Parent = this->q;
				newMap->insert(key, std::move(newItem));
			}
			this->Value = (qint64)newMap;
		}
		else {
			this->Value = other->Value;
		}
		this->HeadComment = other->HeadComment;
		this->InlineComment = other->InlineComment;
	}

	GeneralConfigPrivate* GeneralConfigPrivate::find(QString key, bool autoCreate, bool* justCreated) {
		if (key.isEmpty()) {
			return this;
		}
		else {
			QStringList keys;
			keys.reserve(5);
			if (key.contains(':')) {
				auto s = key.tokenize(QString(":"));
				for (auto part : s) {
					keys.append(part.toString());
				}
			}
			else {
				keys.append(key);
			}
			return find(&keys, autoCreate, justCreated);
		}
	}
	
	GeneralConfigPrivate* GeneralConfigPrivate::find(QStringList* nodes, bool autoCreate, bool* justCreated) {
		if (nodes->isEmpty()) {
			return this;
		}
		GeneralConfigPrivate* p = nullptr;
		if (Type == Visindigo::Utility::GeneralConfig::List)
		{
			bool ok = false;
			int index = nodes->first().toInt(&ok);
			if (ok) {
				auto listPtr = (Visindigo::Utility::GeneralConfigList *)Value;
				if (index >= 0 && index < listPtr->size()) {
					p = listPtr->at(index)->d;
				}
				else if (autoCreate) {
					if (index >= 0 && listPtr) {
						while (listPtr->size() <= index) {
							Visindigo::Utility::GeneralConfig* newItem = new Visindigo::Utility::GeneralConfig();
							newItem->d->Root = this->Root;
							newItem->d->Parent = this->q;
							listPtr->append(newItem);
						}
						Visindigo::Utility::GeneralConfig* target = listPtr->at(index);
						if (justCreated) {
							*justCreated = true;
						}
						if (nodes->size() <= 1) {	
							return target->d;
						}
						else {
							bool nextIsList = false;
							int nextIndex = nodes->at(1).toInt(&ok);
							if (ok) {
								target->setList(Visindigo::Utility::GeneralConfigList());
							}
							else {
								target->setMap(Visindigo::Utility::GeneralConfigMap());
							}
							p = target->d;
						}
					}
				}
			}
			if (p) {
				if (nodes->size() <= 1) {
					return p;
				}
				else {
					nodes->removeFirst();
					return p->find(nodes, autoCreate, justCreated);
				}
			}
			else {
				return nullptr;
			}
		}
		else if (Type == Visindigo::Utility::GeneralConfig::Map)
		{
			auto mapPtr = (Visindigo::Utility::GeneralConfigMap *)Value;
			if (mapPtr->contains(nodes->first())) {
				p = mapPtr->value(nodes->first())->d;
			}
			else if (autoCreate) {
				bool ok = false;
				Visindigo::Utility::GeneralConfig* newItem = new Visindigo::Utility::GeneralConfig();
				newItem->d->Root = this->Root;
				newItem->d->Parent = this->q;
				mapPtr->insert(nodes->first(), std::move(newItem));
				if (justCreated) {
					*justCreated = true;
				}
				if (nodes->size() <= 1) {
					return mapPtr->value(nodes->first())->d;
				}
				else {
					int nextIndex = nodes->at(1).toInt(&ok);
					if (ok) {
						newItem->setList(Visindigo::Utility::GeneralConfigList());
					}
					else {
						newItem->setMap(Visindigo::Utility::GeneralConfigMap());
					}
					mapPtr->insert(nodes->first(), std::move(newItem));
					p = mapPtr->value(nodes->first())->d;
				}
			}
			if (p) {
				if (nodes->size() <= 1) {
					return p;
				}
				else {
					nodes->removeFirst();
					return p->find(nodes, autoCreate, justCreated);
				}
			}
			else {
				return nullptr;
			}
		}
		return nullptr;
	}

	GeneralConfigPrivate* GeneralConfigPrivate::create(const QString& key) {
		if (key.isEmpty()) {
			return this;
		}
		else {
			QStringList keys;
			keys.reserve(5);
			if (key.contains(':')) {
				auto s = key.tokenize(QString(":"));
				for (auto part : s) {
					keys.append(part.toString());
				}
			}
			else {
				keys.append(key);
			}
			return create(&keys);
		}
	}

	GeneralConfigPrivate* GeneralConfigPrivate::create(QStringList* nodes) {
		if (nodes->size() <= 1) {
			if (Type == Visindigo::Utility::GeneralConfig::List) {
				bool ok = false;
				int index = nodes->first().toInt(&ok);
				if (ok) {
					auto listPtr = (Visindigo::Utility::GeneralConfigList *)Value;
					if (index >= 0 && listPtr) {
						if (index < listPtr->size()) {
							return (*listPtr)[index]->d;
						}
						else {
							while (listPtr->size() <= index) {
								Visindigo::Utility::GeneralConfig* newItem = new Visindigo::Utility::GeneralConfig();
								newItem->d->Root = this->Root;
								newItem->d->Parent = this->q;
								listPtr->append(newItem);
							}
							return listPtr->at(index)->d;
						}
					}
				}
				else {
					vgWarningF << "Cannot parse list index:" << nodes->first();
				}
			}
			else if (Type == Visindigo::Utility::GeneralConfig::Map) {
				auto mapPtr = (Visindigo::Utility::GeneralConfigMap *)Value;
				if (mapPtr->contains(nodes->first())) {
					return mapPtr->value(nodes->first())->d;
				}
				else {
					Visindigo::Utility::GeneralConfig* newItem = new Visindigo::Utility::GeneralConfig();
					newItem->d->Root = this->Root;
					newItem->d->Parent = this->q;
					mapPtr->insert(nodes->first(), std::move(newItem));
					return mapPtr->value(nodes->first())->d;
				}
			}
		}

		if (Type == Visindigo::Utility::GeneralConfig::List) {
			QString key = nodes->first();
			QString keyNext = nodes->at(1);
			bool ok = false;
			int index = key.toInt(&ok);
			if (ok) {
				auto listPtr = (Visindigo::Utility::GeneralConfigList *)Value;
				if (index >= 0 && listPtr) {
					if (index < listPtr->size()) {
						auto nextD = listPtr->at(index)->d;
						nodes->removeFirst();
						return nextD->create(nodes);
					}
					else {
						while (listPtr->size() <= index) {
							Visindigo::Utility::GeneralConfig* newItem = new Visindigo::Utility::GeneralConfig();
							newItem->d->Root = this->Root;
							newItem->d->Parent = this->q;
							listPtr->append(newItem);
						}
						Visindigo::Utility::GeneralConfig* target = listPtr->at(index);
						bool nextIsList = false;
						int nextIndex = keyNext.toInt(&ok);
						if (ok) {
							target->setList(Visindigo::Utility::GeneralConfigList());
						}
						else {
							target->setMap(Visindigo::Utility::GeneralConfigMap());
						}
					}
				}
				else {
					vgWarningF << "Cannot parse list index when creating new list node:" << key;
				}
			}
		}
		else if (Type == Visindigo::Utility::GeneralConfig::Map) {
			QString key = nodes->first();
			QString keyNext = nodes->at(1);
			auto mapPtr = (Visindigo::Utility::GeneralConfigMap *)Value;
			if (mapPtr->contains(key)) {
				auto nextD = mapPtr->value(key)->d;
				nodes->removeFirst();
				return nextD->create(nodes);
			}
			else {
				Visindigo::Utility::GeneralConfig* newItem = new Visindigo::Utility::GeneralConfig();
				newItem->d->Root = this->Root;
				newItem->d->Parent = this->q;
				bool ok = false;
				int nextIndex = keyNext.toInt(&ok);
				if (ok) {
					newItem->setList(Visindigo::Utility::GeneralConfigList());
				}
				else {
					newItem->setMap(Visindigo::Utility::GeneralConfigMap());
				}
				mapPtr->insert(key, std::move(newItem));
			}
		}
	}

	bool GeneralConfigPrivate::contains(const QString& key) {
		auto realD = find(key);
		return realD != nullptr;
	}

	void GeneralConfigPrivate::changeRoot(Visindigo::Utility::GeneralConfig* newRoot) {
		Root = newRoot;
		if (Type == Visindigo::Utility::GeneralConfig::List) {
			auto listPtr = (Visindigo::Utility::GeneralConfigList *)Value;
			for (auto item : *listPtr) {
				item->d->changeRoot(newRoot);
			}
		}
		else if (Type == Visindigo::Utility::GeneralConfig::Map) {
			auto mapPtr = (Visindigo::Utility::GeneralConfigMap *)Value;
			for (auto item : mapPtr->values()) {
				item->d->changeRoot(newRoot);
			}
		}
	}

	void GeneralConfigPrivate::changeParent(Visindigo::Utility::GeneralConfig* newQ) {
		q = newQ;
		if (Type == Visindigo::Utility::GeneralConfig::List) {
			auto listPtr = (Visindigo::Utility::GeneralConfigList *)Value;
			for (auto item : *listPtr) {
				item->d->Parent = newQ;
			}
		}
		else if (Type == Visindigo::Utility::GeneralConfig::Map) {
			auto mapPtr = (Visindigo::Utility::GeneralConfigMap *)Value;
			for (auto item : mapPtr->values()) {
				item->d->Parent = newQ;
			}
		}
	}

	void GeneralConfigPrivate::clear() {
		if (Type == Visindigo::Utility::GeneralConfig::String || Type == Visindigo::Utility::GeneralConfig::Reference) {
			QString* ptr = (QString*)Value;
			if (ptr) {
				delete ptr;
			}
		}
		else if (Type == Visindigo::Utility::GeneralConfig::List) {
			auto list = (Visindigo::Utility::GeneralConfigList*)Value;
			for (auto item : *list) {
				delete item;
			}
			delete list;
		}
		else if (Type == Visindigo::Utility::GeneralConfig::Map) {
			auto map = (Visindigo::Utility::GeneralConfigMap*)Value;
			for (auto item : map->values()) {
				delete item;
			}
			delete map;
		}
		Type = Visindigo::Utility::GeneralConfig::None;
		Value = 0;
	}
}

namespace Visindigo::Utility {
	/*!
		\class Visindigo::Utility::GeneralConfig
		\brief 一种通用的配置数据存储结构，支持多种数据类型和嵌套结构。
		\ingroup GeneralConfig
		\inmodule Visindigo
		\since Visindigo 0.13.0

		GeneralConfig类用于存储和管理各种类型的配置数据。
		
		它支持整数、实数、布尔值、字符串、列表和映射等多种数据类型，并允许嵌套使用这些类型来构建复杂的配置结构。
		GeneralConfig类提供了方便的方法来设置和获取不同类型的数据，以及检查节点的类型和存在性。

		它与Qt提供的Json相关对象有完全不同的设计理念。QtJson设施（包括Visindigo由此封装的
		Visindigo::Utility::JsonConfig等）均以值类型形成树，并在内部恰当位置使用隐式内存共享。

		而GeneralConfig内部则完全以指针形式形成树。这使得它们至少有如下区别：
		\list
		\li 1. GeneralConfig的每一个节点明确直到其父对象和根对象都是谁，因此GeneralConfig兼容类似YAML的引用概念。
		\li 2. GeneralConfig理念上不喜欢复制，因此也不提供复制和移动构造。如果使用GeneralConfig::copyFrom静态方法，则会递归地创建一个全新的树结构。
				新的树结构与原树结构没有任何共享的节点。新的树结构的根节点和所有子节点的父节点均正确指向新的树结构。
				而在QtJson设施中，复制和移动是非常常见的操作，只有在发生修改时才会进行实际的数据复制。
		\li 3. QtJson设施不喜欢指针，因此访问深层节点的数据通常很麻烦，需要递归的拆解对象。而设置深层节点通常
				也需要递归的创建中间节点。而GeneralConfig则使用指针，提供了方便的方法，可以直接通过键路径访问或创建深层节点。
				不过同样的，访问深层节点的指针应该被视为弱指针，即用即弃，不应该长期保存。
		\endlist

		\section1 我在什么时候应该使用GeneralConfig？
		如果你的配置数据结构复杂，包含多种数据类型，并且需要频繁地访问和修改深层节点，GeneralConfig
		相比Visindigo::Utility::JsonConfig是个更高性能的选择。它在密集写时耗时约为JsonConfig的
		20~30%，在密集读时耗时约为JsonConfig的10~20%。就算不用于配置文件场景，仅用于在运行时
		传递一些复杂的参数结构，GeneralConfig也是一个不错的选择。

		但当前GeneralConfig配套的Json解析器（参见Visindigo::Utility::GeneralConfigParser）的性能
		并不理想，在密集构造-解析-析构测试，以及密集序列化测试中的耗时均为JsonConifg的4~5倍。
		因此如果你需要频繁的从Json文本加载配置，或者频繁地将配置序列化为Json文本，而不需要
		频繁访问和修改深层节点，那么JsonConfig可能是更好的选择。也许未来GeneralConfigParser的性能会有所提升。

		值得指出的是，虽然JsonConfig的读写性能差于GeneralConfig，但由于读写开销相比于
		解析、序列化开销实在不是同一个数量级（深度为7时，JsonConfig的读开销是解析开销
		的约三十分之一，GeneralConfig的读写开销是解析开销的约五百分之一），因此目前
		GeneralConfig在读写上节约的开销远远少于解析/序列化时浪费的开销，总的来说暂时还是
		选择JsonConfig更为合适一点。

		但如果你需要在不同类型的配置存储格式之间进行转换（例如Json、YAML、XML等），
		那使用GenralConfig是本框架目前的唯一解。

		\section1 内存管理
		虽然GeneralConfig没有使用隐式内存共享，但由于其节点均为指针形式，直接在堆上频繁通过new
		分配内存的性能并不理想。因此Visindigo内部使用了一个Private类的内存池。这个池对用户完全
		封闭，用户无需关心其存在与否。不过目前该池不是很完善，它不会释放任何内存给操作系统，
		且每次需要申请内存时，都是按1024个Private对象的块进行分配。因此如果你的应用程序
		频繁创建和销毁大量GeneralConfig节点，那么该池会长期维持一大块内存占用。这会在未来的
		版本中得到改善。
	*/

	/*!
		\enum Visindigo::Utility::GeneralConfig::Type
		\since Visindigo 0.13.0

		\value None 空值
		\value Integer 整数
		\value Real 实数
		\value Bool 布尔量
		\value String 字符串
		\value List 列表
		\value Map 映射
		\value Reference 引用
		\value Number 数字，Real的别名
		\value Array 数组，List的别名
		\value Dict 字典，Map的别名
		\value Null 空，None的别名
	
		表达节点的类型
	*/

	/*!
		\enum Visindigo::Utility::GeneralConfig::StringFormat
		\since Visindigo 0.13.0

		\value Stream 流，指满足语法的前提下，不考虑可读性，一般用于传输的输出格式。
		\value Formatted 已格式化，指满足语法的前提下，最大程度考虑可读性的输出格式。

		表达输出的序列化策略
	*/

	/*!
		\since Visindigo 0.13.0

		默认构造函数，创建一个类型为None的空节点。
	*/
	GeneralConfig::GeneralConfig() {
		d = VPGCMP->getPrivate(this);
	}

	/*!
		\since Visindigo 0.13.0
		使用整数值构造一个节点。
	*/
	GeneralConfig::GeneralConfig(qint64 value){
		d = VPGCMP->getPrivate(this);
		d->Type = Type::Integer;
		d->Value = value;
	}

	/*!
		\since Visindigo 0.13.0
		使用实数值构造一个节点。
	*/
	GeneralConfig::GeneralConfig(qreal value) {
		d = VPGCMP->getPrivate(this);
		d->Type = Type::Real;
		memcpy(&(d->Value), &value, sizeof(d->Value));
	}

	/*!
		\since Visindigo 0.13.0
		使用布尔值构造一个节点。
	*/
	GeneralConfig::GeneralConfig(bool value){
		d = VPGCMP->getPrivate(this);
		d->Type = Type::Bool;
		d->Value = value;
	}

	/*!
		\since Visindigo 0.13.0
		使用字符串值构造一个节点。如果isRef为true，则构造一个引用节点。
	*/
	GeneralConfig::GeneralConfig(const QString& value, bool isRef) {
		d = VPGCMP->getPrivate(this);
		d->Type = isRef ? Type::Reference : Type::String;
		d->Value = (qint64)(new QString(value));
	}

	/*!
		\since Visindigo 0.13.0
		使用列表值构造一个节点。
	*/
	GeneralConfig::GeneralConfig(const GeneralConfigList& value) {
		d = VPGCMP->getPrivate(this);
		d->Type = Type::List;
		d->Value = (qint64)(new GeneralConfigList(value));
		if (d->Value) {
			auto listPtr = (GeneralConfigList*)d->Value;
			if (listPtr->size() == 0) {
				listPtr->reserve(7);
				return;
			}
			else {
				d->changeRoot(d->Root);
				d->changeParent(this);
			}
		}
	}

	/*!
		\since Visindigo 0.13.0
		使用映射值构造一个节点。
	*/
	GeneralConfig::GeneralConfig(const GeneralConfigMap& value) {
		d = VPGCMP->getPrivate(this);
		d->Type = Type::Map;
		d->Value = (qint64)(new GeneralConfigMap(value));
		if (d->Value) {
			auto mapPtr = (GeneralConfigMap*)d->Value;
			if (mapPtr->size() == 0) {
				return;
			}
			else {
				d->changeRoot(d->Root);
				d->changeParent(this);
			}
		}
	}

	/*!
		\since Visindigo 0.13.0
		析构函数，递归地释放节点及其子节点占用的内存。
	*/
	GeneralConfig::~GeneralConfig() { 
		VPGCMP->releasePrivate(d);
	}

	/*!
		\since Visindigo 0.13.0
		静态方法，递归地复制一个GeneralConfig节点及其子节点，返回新的节点指针。
	*/
	GeneralConfig* GeneralConfig::copyFrom(GeneralConfig* other) {
		GeneralConfig* newConfig = new Visindigo::Utility::GeneralConfig();
		newConfig->d->copyFrom(other->d);
		newConfig->d->changeRoot(newConfig);
		return newConfig;
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		\a def 默认值，当节点不存在或类型不匹配时返回该值。
		\a ok 可选输出参数，指示操作是否成功。发生任何错误时均设置为false。
		获取指定键路径下的整数值。如果节点不存在或类型不匹配，则返回默认值def。
	*/
	qint64 GeneralConfig::getInt(const QString& key, qint64 def, bool* ok) {
		auto realD = d->find(key);
		if (realD) {
			if (realD->Type != Type::Integer) {
				vgWarningF << "Type mismatch when getting int value from GeneralConfig:" << key;
				if (ok) {
					*ok = false;
				}
				return def;
			}
			if (ok) {
				*ok = true;
			}
			return realD->Value;
		}
		else {
			if (ok) {
				*ok = false;
			}
			return def;
		}
	}
	
	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		\a def 默认值，当节点不存在或类型不匹配时返回该值。
		\a ok 可选输出参数，指示操作是否成功。发生任何错误时均设置为false。
		获取指定键路径下的实数值（双浮点数）。如果节点不存在或类型不匹配，则返回默认值def。
	*/
	qreal GeneralConfig::getReal(const QString& key, qreal def, bool* ok) {
		auto realD = d->find(key);
		if (realD) {
			if (realD->Type != Type::Real) {
				vgWarningF << "Type mismatch when getting real value from GeneralConfig:" << key;
				if (ok) {
					*ok = false;
				}
				return def;
			}
			qreal rtn;
			memcpy(&rtn, &(d->Value), sizeof(d->Value));
			if (ok) {
				*ok = true;
			}
			return rtn;
		}
		else {
			if (ok) {
				*ok = false;
				return def;
			}
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		\a def 默认值，当节点不存在或类型不匹配时返回该值。
		\a ok 可选输出参数，指示操作是否成功。发生任何错误时均设置为false。
		获取指定键路径下的布尔值。如果节点不存在或类型不匹配，则返回默认值def。
	*/
	bool GeneralConfig::getBool(const QString& key, bool def, bool* ok) {
		auto realD = d->find(key);
		if (realD) {
			if (realD->Type != Type::Bool) {
				vgWarningF << "Type mismatch when getting bool value from GeneralConfig:" << key;
				if (ok) {
					*ok = false;
				}
				return def;
			}
			if (ok) {
				*ok = true;
			}
			return realD->Value;
		}
		else {
			if (ok) {
				*ok = false;
			}
			return def;
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		\a def 默认值，当节点不存在或类型不匹配时返回该值。
		\a ok 可选输出参数，指示操作是否成功。发生任何错误时均设置为false。
		获取指定键路径下的字符串值。如果节点不存在或类型不匹配，则返回默认值def。
	*/
	QString GeneralConfig::getString(const QString& key, const QString& def, bool* ok) {
		auto realD = d->find(key);
		if (realD) {
			if (realD->Type != Type::String) {
				vgWarningF << "Type mismatch when getting string value from GeneralConfig:" << key;
				if (ok) {
					*ok = false;
				}
				return def;
			}
			QString* rtn = (QString*)realD->Value;
			if (ok) {
				*ok = (rtn != nullptr);
			}
			if (rtn) {
				return *rtn;
			}
			else {
				return def;
			}
		}
		else {
			if (ok) {
				*ok = false;
			}
			return def;
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		\a def 默认值，当节点不存在或类型不匹配时返回该值。
		\a ok 可选输出参数，指示操作是否成功。发生任何错误时均设置为false。
		获取指定键路径下的引用节点的目标路径字符串值。如果节点不存在或类型不匹配，则返回默认值def。
	*/
	QString GeneralConfig::getRefNode(const QString& key, const QString& def, bool* ok) {
		auto realD = d->find(key);
		if (realD) {
			if (realD->Type != Type::Reference) {
				vgWarningF << "Type mismatch when getting reference node value from GeneralConfig:" << key;
				if (ok) {
					*ok = false;
				}
				return def;
			}
			QString* rtn = (QString*)realD->Value;
			if (ok) {
				*ok = (rtn != nullptr);
			}
			if (rtn) {
				return *rtn;
			}
			else {
				return def;
			}
		}
		else {
			if (ok) {
				*ok = false;
			}
			return def;
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		\a def 默认值，当节点不存在或类型不匹配时返回该值。
		\a ok 可选输出参数，指示操作是否成功。发生任何错误时均设置为false。
		获取指定键路径下的列表值。如果节点不存在或类型不匹配，则返回默认值def。

		请注意，不要将返回的列表值长期保存，请将其视为弱指针，即用即弃。
		如果有必要长期保存，请使用GeneralConfig::copyFrom方法复制对应的节点。
	*/
	GeneralConfigList GeneralConfig::getList(const QString& key, const GeneralConfigList& def, bool* ok) {
		auto realD = d->find(key);
		if (realD) {
			if (realD->Type != Type::List) {
				vgWarningF << "Type mismatch when getting list value from GeneralConfig:" << key << ". Should be type" << (int)realD->Type;
				if (ok) {
					*ok = false;
				}
				return def;
			}
			auto rtn = (GeneralConfigList*)realD->Value;
			if (ok) {
				*ok = (rtn != nullptr);
			}
			if (rtn) {
				return *rtn;
			}
			else {
				return def;
			}
		}
		else {
			if (ok) {
				*ok = false;
			}
			return def;
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		\a def 默认值，当节点不存在或类型不匹配时返回该值。
		\a ok 可选输出参数，指示操作是否成功。发生任何错误时均设置为false。
		获取指定键路径下的映射值。如果节点不存在或类型不匹配，则返回默认值def。
		请注意，不要将返回的映射值长期保存，请将其视为弱指针，即用即弃。
		如果有必要长期保存，请使用GeneralConfig::copyFrom方法复制对应的节点。
	*/
	GeneralConfigMap GeneralConfig::getMap(const QString& key, const GeneralConfigMap& def, bool* ok) {
		auto realD = d->find(key);
		if (realD) {
			if (realD->Type != Type::Map) {
				vgWarningF << "Type mismatch when getting map value from GeneralConfig:" << key;
				if (ok) {
					*ok = false;
				}
				return def;
			}
			auto rtn = (GeneralConfigMap*)realD->Value;
			if (ok) {
				*ok = (rtn != nullptr);
			}
			if (rtn) {
				return *rtn;
			}
			else {
				return def;
			}
		}
		else {
			if (ok) {
				*ok = false;
			}
			return def;
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		\a ok 可选输出参数，指示操作是否成功。发生任何错误时均设置为false。
		\a isHeadComment 指示是否获取头注释（true）还是行内注释（false）。
		获取指定键路径下节点的注释字符串。如果节点不存在，则返回空字符串。
	*/
	QString GeneralConfig::getComment(const QString& key, bool* ok, bool isHeadComment) {
		auto realD = d->find(key);
		if (realD) {
			if (isHeadComment) {
				if (ok) {
					*ok = true;
				}
				return realD->HeadComment;
			}
			else {
				if (ok) {
					*ok = true;
				}
				return realD->InlineComment;
			}
		}
		else {
			if (ok) {
				*ok = false;
			}
			return QString();
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		获取指定键路径下的子节点指针。如果节点不存在，则返回nullptr。

		这也是个应该即用即弃的弱指针。如果有必要长期保存，请使用GeneralConfig::copyFrom方法复制对应的节点。
	*/
	GeneralConfig* GeneralConfig::getChildNode(const QString& key) {
		auto realD = d->find(key);
		if (realD) {
			return realD->q;
		}
		else {
			return nullptr;
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		\a ok 可选输出参数，指示操作是否成功。发生任何错误时均设置为false。
		获取指定键路径下节点的所有子节点的键列表。如果节点不存在或类型不匹配，则返回空列表。

		如果类型为Map，则返回映射的所有键；如果类型为List，则返回索引字符串列表（"0"、"1"等）。
	*/
	QStringList GeneralConfig::getKeys(const QString& key, bool* ok) {
		auto realD = d->find(key);
		if (realD) {
			if (realD->Type == Type::Map) {
				auto mapPtr = (GeneralConfigMap*)realD->Value;
				QStringList rtn = mapPtr->keys();
				if (ok) {
					*ok = true;
				}
				return rtn;
			}else if (realD->Type == Type::List) {
				auto listPtr = (GeneralConfigList*)realD->Value;
				QStringList rtn;
				for (int i = 0; i < listPtr->size(); i++) {
					rtn.append(QString::number(i));
				}
				if (ok) {
					*ok = true;
				}
				return rtn;
			}
			else {
				vgWarningF << "Type mismatch when getting keys from GeneralConfig:" << key;
				if (ok) {
					*ok = false;
				}
				return QStringList();
			}
		}
		else {
			if (ok) {
				*ok = false;
			}
			return QStringList();
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		设置指定键路径下的整数值。如果节点不存在，则创建新节点；如果节点已存在，则覆盖其值和类型。
	*/
	void GeneralConfig::setInt(qint64 value, const QString& key) {
		bool justCreated = false;
		auto	 realD = d->find(key, true, &justCreated);
		if (!justCreated) {
			realD->clear();
		}
		realD->Type = Type::Integer;
		realD->Value = value;
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		设置指定键路径下的实数值（双浮点数）。如果节点不存在，则创建新节点；如果节点已存在，则覆盖其值和类型。
	*/
	void GeneralConfig::setDouble(double value, const QString& key) {
		bool justCreated = false;
		auto	 realD = d->find(key, true, &justCreated);
		if (!justCreated) {
			realD->clear();
		}
		realD->Type = Type::Real;
		memcpy(&(realD->Value), &value, sizeof(realD->Value));
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		设置指定键路径下的布尔值。如果节点不存在，则创建新节点；如果节点已存在，则覆盖其值和类型。
	*/
	void GeneralConfig::setBool(bool value, const QString& key) {
		bool justCreated = false;
		auto	 realD = d->find(key, true, &justCreated);
		if (!justCreated) {
			realD->clear();
		}
		realD->Type = Type::Bool;
		realD->Value = value;
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		设置指定键路径下的字符串值。如果节点不存在，则创建新节点；如果节点已存在，则覆盖其值和类型。
	*/
	void GeneralConfig::setString(const QString& value, const QString& key) {
		bool justCreated = false;
		auto	 realD = d->find(key, true, &justCreated);
		if (!justCreated) {
			realD->clear();
		}
		realD->Type = Type::String;
		realD->Value = (qint64)(new QString(value));
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		设置指定键路径下的引用节点的目标路径字符串值。如果节点不存在，则创建新节点；如果节点已存在，则覆盖其值和类型。
	*/
	void GeneralConfig::setRefNode(const QString& value, const QString& key) {
		bool justCreated = false;
		auto	 realD = d->find(key, true, &justCreated);
		if (!justCreated) {
			realD->clear();
		}
		realD->Type = Type::Reference;
		realD->Value = (qint64)(new QString(value));
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		设置指定键路径下的列表值。如果节点不存在，则创建新节点；如果节点已存在，则覆盖其值和类型。

		此操作传入的列表被视为所有权转移，即被设置到节点后，调用者不应再使用该列表。
		列表中的每一个对象的根节点指针和父节点指针均被更新为当前节点的根节点和当前节点。
	*/
	void GeneralConfig::setList(const GeneralConfigList& value, const QString& key) {
		bool justCreated = false;
		auto	 realD = d->find(key, true, &justCreated);
		if (!justCreated) {
			realD->clear();
		}
		realD->Type = Type::List;
		if (value.isEmpty()) {
			realD->Value = (qint64)(new GeneralConfigList());
			((GeneralConfigList*)(realD->Value))->reserve(7);
		}
		else {
			realD->Value = (qint64)(new GeneralConfigList(value));
		}
		realD->changeRoot(this->d->Root);
		realD->changeParent(realD->q);
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		设置指定键路径下的映射值。如果节点不存在，则创建新节点；如果节点已存在，则覆盖其值和类型。

		此操作传入的映射被视为所有权转移，即被设置到节点后，调用者不应再使用该映射。
		映射中的每一个对象的根节点指针和父节点指针均被更新为当前节点的根节点和当前节点。
	*/
	void GeneralConfig::setMap(const GeneralConfigMap& value, const QString& key) {
		bool justCreated = false;
		auto	 realD = d->find(key, true, &justCreated);
		if (!justCreated) {
			realD->clear();
		}
		realD->Type = Type::Map;
		if (value.isEmpty()) {
			realD->Value = (qint64)(new GeneralConfigMap());
		}
		else {
			realD->Value = (qint64)(new GeneralConfigMap(value));
		}
		realD->changeRoot(this->d->Root);
		realD->changeParent(realD->q);
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		\a isHeadComment 指示是否设置头注释（true）还是行内注释（false）。
		设置指定键路径下节点的注释字符串。如果节点不存在，则创建新节点。
	*/
	void GeneralConfig::setComment(const QString& value, const QString& key, bool isHeadComment) {
		bool justCreated = false;
		auto	 realD = d->find(key, true, &justCreated);
		if (isHeadComment) {
			realD->HeadComment = value;
		}
		else {
			realD->InlineComment = value;
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		检查指定键路径下的节点是否存在。
	*/
	bool GeneralConfig::contains(const QString& key) {
		return d->contains(key);
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		\a refNodeGetFinalType 指示如果节点为引用节点，是否获取其引用目标节点的最终类型。
		获取指定键路径下节点的类型。如果节点不存在，则返回Type::None。
		如果refNodeGetFinalType为true且节点为引用节点，则返回其引用目标节点的类型。
	*/
	GeneralConfig::Type GeneralConfig::getNodeType(const QString& key, bool refNodeGetFinalType) {
		auto realD = d->find(key);
		if (realD) {
			if (refNodeGetFinalType && realD->Type == Type::Reference) {
				QString refNodePath = *(QString*)realD->Value;
				auto refD = d->Root->d->find(refNodePath);
				if (refD) {
					return refD->Type;
				}
				else {
					vgWarningF << "Reference node not found when getting final type:" << refNodePath;
					return Type::None;
				}
			}
			else {
				return realD->Type;
			}
		}
		else {
			return Type::None;
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		检查指定键路径下的节点是否为None类型。如果节点不存在，则视为None类型。
	*/
	bool GeneralConfig::isNone(const QString& key) {
		auto realD = d->find(key);
		if (realD) {
			return realD->Type == Type::None;
		}
		else {
			return true;
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		检查指定键路径下的节点是否为布尔类型。
	*/
	bool GeneralConfig::isInt(const QString& key) {
		auto realD = d->find(key);
		if (realD) {
			return realD->Type == Type::Integer;
		}
		else {
			return false;
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		检查指定键路径下的节点是否为实数类型。
	*/
	bool GeneralConfig::isReal(const QString& key) {
		auto realD = d->find(key);
		if (realD) {
			return realD->Type == Type::Real;
		}
		else {
			return false;
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		检查指定键路径下的节点是否为布尔类型。
	*/
	bool GeneralConfig::isString(const QString& key) {
		auto realD = d->find(key);
		if (realD) {
			return realD->Type == Type::String;
		}
		else {
			return false;
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		检查指定键路径下的节点是否为列表类型。
	*/
	bool GeneralConfig::isList(const QString& key) {
		auto realD = d->find(key);
		if (realD) {
			return realD->Type == Type::List;
		}
		else {
			return false;
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		检查指定键路径下的节点是否为映射类型。
	*/
	bool GeneralConfig::isMap(const QString& key) {
		auto realD = d->find(key);
		if (realD) {
			return realD->Type == Type::Map;
		}
		else {
			return false;
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a key 指定键路径，使用句点分隔各级节点名称。为空字符串表示当前节点。
		检查指定键路径下的节点是否为引用类型。
	*/
	bool GeneralConfig::isReference(const QString& key) {
		auto realD = d->find(key);
		if (realD) {
			return realD->Type == Type::Reference;
		}
		else {
			return false;
		}
	}
}

