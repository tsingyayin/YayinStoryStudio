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
			QStringList keys = key.split(':');
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
			QStringList keys = key.split(':');
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

	void GeneralConfigPrivate::changeQ(Visindigo::Utility::GeneralConfig* newQ) {
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
	GeneralConfig::GeneralConfig() {
		d = VPGCMP->getPrivate(this);
	}

	GeneralConfig::GeneralConfig(qint64 value){
		d = VPGCMP->getPrivate(this);
		d->Type = Type::Integer;
		d->Value = value;
	}

	GeneralConfig::GeneralConfig(qreal value) {
		d = VPGCMP->getPrivate(this);
		d->Type = Type::Real;
		memcpy(&(d->Value), &value, sizeof(d->Value));
	}

	GeneralConfig::GeneralConfig(bool value){
		d = VPGCMP->getPrivate(this);
		d->Type = Type::Bool;
		d->Value = value;
	}

	GeneralConfig::GeneralConfig(const QString& value, bool isRef) {
		d = VPGCMP->getPrivate(this);
		d->Type = isRef ? Type::Reference : Type::String;
		d->Value = (qint64)(new QString(value));
	}

	GeneralConfig::GeneralConfig(const GeneralConfigList& value) {
		d = VPGCMP->getPrivate(this);
		d->Type = Type::List;
		d->Value = (qint64)(new GeneralConfigList(value));
	}

	GeneralConfig::GeneralConfig(const GeneralConfigMap& value) {
		d = VPGCMP->getPrivate(this);
		d->Type = Type::Map;
		d->Value = (qint64)(new GeneralConfigMap(value));
	}

	GeneralConfig::~GeneralConfig() { 
		VPGCMP->releasePrivate(d);
	}

	GeneralConfig* GeneralConfig::copyFrom(GeneralConfig* other) {
		GeneralConfig* newConfig = new Visindigo::Utility::GeneralConfig();
		newConfig->d->copyFrom(other->d);
		newConfig->d->changeRoot(newConfig);
		return newConfig;
	}

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

	GeneralConfig* GeneralConfig::getChildNode(const QString& key) {
		auto realD = d->find(key);
		if (realD) {
			return realD->q;
		}
		else {
			return nullptr;
		}
	}

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

	void GeneralConfig::setInt(qint64 value, const QString& key) {
		bool justCreated = false;
		auto	 realD = d->find(key, true, &justCreated);
		if (!justCreated) {
			realD->clear();
		}
		realD->Type = Type::Integer;
		realD->Value = value;
	}

	void GeneralConfig::setDouble(double value, const QString& key) {
		bool justCreated = false;
		auto	 realD = d->find(key, true, &justCreated);
		if (!justCreated) {
			realD->clear();
		}
		realD->Type = Type::Real;
		memcpy(&(realD->Value), &value, sizeof(realD->Value));
	}

	void GeneralConfig::setBool(bool value, const QString& key) {
		bool justCreated = false;
		auto	 realD = d->find(key, true, &justCreated);
		if (!justCreated) {
			realD->clear();
		}
		realD->Type = Type::Bool;
		realD->Value = value;
	}

	void GeneralConfig::setString(const QString& value, const QString& key) {
		bool justCreated = false;
		auto	 realD = d->find(key, true, &justCreated);
		if (!justCreated) {
			realD->clear();
		}
		realD->Type = Type::String;
		realD->Value = (qint64)(new QString(value));
	}

	void GeneralConfig::setRefNode(const QString& value, const QString& key) {
		bool justCreated = false;
		auto	 realD = d->find(key, true, &justCreated);
		if (!justCreated) {
			realD->clear();
		}
		realD->Type = Type::Reference;
		realD->Value = (qint64)(new QString(value));
	}

	void GeneralConfig::setList(const GeneralConfigList& value, const QString& key) {
		bool justCreated = false;
		auto	 realD = d->find(key, true, &justCreated);
		if (!justCreated) {
			realD->clear();
		}
		realD->Type = Type::List;
		if (value.isEmpty()) {
			realD->Value = (qint64)(new GeneralConfigList());
		}
		else {
			realD->Value = (qint64)(new GeneralConfigList(value));
		}
		realD->changeRoot(this->d->Root);
		realD->changeQ(realD->q);
	}

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
		realD->changeQ(realD->q);
	}

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

	bool GeneralConfig::contains(const QString& key) {
		return d->contains(key);
	}

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

	bool GeneralConfig::isNone(const QString& key) {
		auto realD = d->find(key);
		if (realD) {
			return realD->Type == Type::None;
		}
		else {
			return true;
		}
	}

	bool GeneralConfig::isInt(const QString& key) {
		auto realD = d->find(key);
		if (realD) {
			return realD->Type == Type::Integer;
		}
		else {
			return false;
		}
	}

	bool GeneralConfig::isReal(const QString& key) {
		auto realD = d->find(key);
		if (realD) {
			return realD->Type == Type::Real;
		}
		else {
			return false;
		}
	}

	bool GeneralConfig::isString(const QString& key) {
		auto realD = d->find(key);
		if (realD) {
			return realD->Type == Type::String;
		}
		else {
			return false;
		}
	}

	bool GeneralConfig::isList(const QString& key) {
		auto realD = d->find(key);
		if (realD) {
			return realD->Type == Type::List;
		}
		else {
			return false;
		}
	}

	bool GeneralConfig::isMap(const QString& key) {
		auto realD = d->find(key);
		if (realD) {
			return realD->Type == Type::Map;
		}
		else {
			return false;
		}
	}

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

