#pragma once
#include "../Macro.h"
namespace YSSCore::Utility {
	class DiffUnitPrivate;
	class DiffSequencePrivate;

	class YSSCoreAPI DiffUnit {
		friend class DiffUnitPrivate;
		friend class DiffSequencePrivate;
		friend class DiffSequence;
	public:
		enum class CompareType {
			Equal,
			NotEqual,
			Modified,
		};
		enum class DiffType {
			Unknown,
			None,
			Added,
			Removed,
			Modified,
		};
	public:
		DiffUnit();
		DiffUnit(const DiffUnit& other) = delete;
		DiffUnit(DiffUnit&& other) = delete;
		virtual ~DiffUnit();
		virtual CompareType onCompare(const DiffUnit* other) = 0;
		DiffType getDiffType() const;
	protected:
		DiffUnitPrivate* d;
	};

	class YSSCoreAPI DiffSequence {
	public:
		DiffSequence();
		DiffSequence(const DiffSequence& other) = delete;
		DiffSequence(DiffSequence&& other) = delete;
		~DiffSequence();
		void addDiffUnit(DiffUnit* unit);
		DiffUnit* getDiffUnit(int index) const;
		DiffUnit* operator[](int index) const;
		void clear();
		void refresh();
		void removeDiffUnit(int index);
		int size() const;
		bool isEmpty() const;
		int compareAsOld(const DiffSequence* other) const;
	protected:
		DiffSequencePrivate* d;
	};
}