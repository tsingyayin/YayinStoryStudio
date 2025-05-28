#include "../DiffSequence.h"
#include <QtCore/qlist.h>
namespace YSSCore::Utility {
	class DiffUnitPrivate {
		friend class DiffUnit;
		friend class DiffSequence;
	protected:
		DiffUnit::DiffType DiffType = DiffUnit::DiffType::Unknown;
	};

	class DiffSequencePrivate {
		friend class DiffSequence;
	protected:
		QList<DiffUnit*> DiffUnitList;
	};

	/*! 
		\class YSSCore::Utility::DiffUnit
		\brief 此类为Yayin Story Studio 提供差异比较的基类。
		\since YSSCore 0.13.0
		\inmodule YSSCore

		DiffUnit是所有差异比较单元的基类，所有差异比较单元都需要继承此类。
		目前来说，DiffUnit主要用于代码差异比较以及文件差异比较，分别用于Git和增量更新。
	*/

	/*!
		\since YSSCore 0.13.0
		DiffUnit的构造函数
	*/
	DiffUnit::DiffUnit() {
		d = new DiffUnitPrivate();
	}
	/*!
		\since YSSCore 0.13.0
		DiffUnit的析构函数
	*/
	DiffUnit::~DiffUnit() {
		delete d;
	}
	/*!
		\since YSSCore 0.13.0
		获得当前DiffUnit的DiffType
	*/
	DiffUnit::DiffType DiffUnit::getDiffType() const
	{
		return d->DiffType;
	}

	/*!
		\class YSSCore::Utility::DiffSequence
		\brief 此类为Yayin Story Studio 提供差异比较的序列。
		\since YSSCore 0.13.0
		\inmodule YSSCore

		DiffSequence承载任何DiffUnit实例的序列，并提供相互比较的功能。
	*/

	/*!
		\since YSSCore 0.13.0
		DiffSequence的构造函数
	*/
	DiffSequence::DiffSequence() {
		d = new DiffSequencePrivate();
	}

	/*!
		\since YSSCore 0.13.0
		DiffSequence的析构函数
	*/
	DiffSequence::~DiffSequence() {
		clear();
		delete d;
	}

	/*!
		\since YSSCore 0.13.0
		\a unit 为DiffUnit实例。
		将DiffUnit实例添加到DiffSequence中。
	*/
	void DiffSequence::addDiffUnit(DiffUnit* unit) {
		d->DiffUnitList.append(unit);
	}

	/*!
		\since YSSCore 0.13.0
		\a index 为索引。
		获取DiffSequence中指定索引的DiffUnit实例。
	*/
	DiffUnit* DiffSequence::getDiffUnit(int index) const {
		if (index < 0 || index >= d->DiffUnitList.size()) {
			return nullptr;
		}
		return d->DiffUnitList.at(index);
	}

	/*!
		\since YSSCore 0.13.0
		\a index 为索引。
		获取DiffSequence中指定索引的DiffUnit实例。getDiffUnit的语法糖。
	*/
	DiffUnit* DiffSequence::operator[](int index) const {
		return getDiffUnit(index);
	}

	/*!
		\since YSSCore 0.13.0
		清空DiffSequence中的所有DiffUnit实例。
	*/
	void DiffSequence::clear() {
		for (DiffUnit* unit : d->DiffUnitList) {
			delete unit;
		}
		d->DiffUnitList.clear();
	}

	/*!
		\since YSSCore 0.13.0
		刷新DiffSequence中的所有DiffUnit实例的DiffType为Unknown。
	*/
	void DiffSequence::refresh() {
		for (DiffUnit* unit : d->DiffUnitList) {
			unit->d->DiffType = DiffUnit::DiffType::Unknown;
		}
	}

	/*!
		\since YSSCore 0.13.0
		\a index 为索引。
		从DiffSequence中移除指定索引的DiffUnit实例。
	*/
	void DiffSequence::removeDiffUnit(int index) {
		if (index < 0 || index >= d->DiffUnitList.size()) {
			return;
		}
		delete d->DiffUnitList.takeAt(index);
	}

	/*!
		\since YSSCore 0.13.0
		获取DiffSequence中DiffUnit实例的数量。
	*/
	int DiffSequence::size() const {
		return d->DiffUnitList.size();
	}

	/*!
		\since YSSCore 0.13.0
		判断DiffSequence是否为空。
	*/
	bool DiffSequence::isEmpty() const {
		return d->DiffUnitList.isEmpty();
	}

	/*!
		\since YSSCore 0.13.0
		\a other 为另一个DiffSequence实例。
		比较当前DiffSequence与另一个DiffSequence的差异。将当前DiffSequence视为旧版本。
	*/
	int DiffSequence::compareAsOld(const DiffSequence* other) const {
		int oIndex = 0;
		int nIndex = 0;
		int oLength = size();
		int nLength = other->size();
		while (oIndex < oLength && nIndex < nLength) {
			for (int y = nIndex; y < nLength; y++) {
				DiffUnit::CompareType type = d->DiffUnitList[oIndex]->onCompare(other->getDiffUnit(y));
				switch (type) {
				case DiffUnit::CompareType::Equal:
					d->DiffUnitList[oIndex]->d->DiffType = DiffUnit::DiffType::None;
					other->getDiffUnit(y)->d->DiffType = DiffUnit::DiffType::None;
					oIndex += 1;
					nIndex = y + 1;
					break;
				case DiffUnit::CompareType::Modified:
					d->DiffUnitList[oIndex]->d->DiffType = DiffUnit::DiffType::Modified;
					other->getDiffUnit(y)->d->DiffType = DiffUnit::DiffType::Modified;
					oIndex += 1;
					nIndex = y + 1;
					break;
				case DiffUnit::CompareType::NotEqual:
					if (y == nLength - 1) {
						d->DiffUnitList[oIndex]->d->DiffType = DiffUnit::DiffType::Removed;
						oIndex += 1;
						break;
					}
				}
			}
		}
		for (DiffUnit* unit : other->d->DiffUnitList) {
			if (unit->d->DiffType == DiffUnit::DiffType::Unknown) {
				unit->d->DiffType = DiffUnit::DiffType::Added;
			}
		}
		return 0;
	}
}

/*!
	\fn YSSCore::Utility::DiffUnit::CompareType YSSCore::Utility::DiffUnit::onCompare(const YSSCore::Utility::DiffUnit* other)
	\a other 为另一个DiffUnit实例。
	\since YSSCore 0.13.0

	必须实现的虚函数，用于具体比较两个DiffUnit实例的差异。
*/