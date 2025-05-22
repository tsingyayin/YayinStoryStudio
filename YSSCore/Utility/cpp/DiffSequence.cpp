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
	DiffUnit::DiffUnit() {
		d = new DiffUnitPrivate();
	}
	DiffUnit::~DiffUnit() {
		delete d;
	}
	DiffUnit::DiffType DiffUnit::getDiffType() const
	{
		return d->DiffType;
	}

	DiffSequence::DiffSequence() {
		d = new DiffSequencePrivate();
	}
	DiffSequence::~DiffSequence() {
		clear();
		delete d;
	}
	void DiffSequence::addDiffUnit(DiffUnit* unit) {
		d->DiffUnitList.append(unit);
	}
	DiffUnit* DiffSequence::getDiffUnit(int index) const {
		if (index < 0 || index >= d->DiffUnitList.size()) {
			return nullptr;
		}
		return d->DiffUnitList.at(index);
	}
	DiffUnit* DiffSequence::operator[](int index) const {
		return getDiffUnit(index);
	}
	void DiffSequence::clear() {
		for (DiffUnit* unit : d->DiffUnitList) {
			delete unit;
		}
		d->DiffUnitList.clear();
	}
	void DiffSequence::refresh() {
		for (DiffUnit* unit : d->DiffUnitList) {
			unit->d->DiffType = DiffUnit::DiffType::Unknown;
		}
	}
	void DiffSequence::removeDiffUnit(int index) {
		if (index < 0 || index >= d->DiffUnitList.size()) {
			return;
		}
		delete d->DiffUnitList.takeAt(index);
	}
	int DiffSequence::size() const {
		return d->DiffUnitList.size();
	}
	bool DiffSequence::isEmpty() const {
		return d->DiffUnitList.isEmpty();
	}
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