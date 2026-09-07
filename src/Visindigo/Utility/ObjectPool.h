#ifndef Visindigo_Utility_ObjectPool_h
#define Visindigo_Utility_ObjectPool_h

#include <concepts>
#include <memory>
#include <unordered_set>
#include <QtCore/qglobal.h>
#include <QtCore/qqueue.h>
#include "VICompileMacro.h"
// Main
namespace Visindigo::Utility {
	template <typename T>
	concept Reusable = requires(T & obj) {
		{ obj.reset() } -> std::same_as<void>;
	};

	template<Reusable T> class ObjectPool;

	template<typename T>
	struct ObjectPoolDeleter {
	public:
		ObjectPoolDeleter() = default; 
		explicit ObjectPoolDeleter(void* pool) : Pool(pool) {}
		void* pool() const noexcept { return Pool; }
		void operator()(T* obj) const {
			if (Pool) {
				static_cast<ObjectPool<T>*>(Pool)->release(obj);
			}
			else 
			{
				delete obj;
			}
		}
	private:
		void* Pool = nullptr;
	};

	template <Reusable T>
	class ObjectPool {
	public:
		ObjectPool(qint32 initialSize = 20, qint32 minSize = 10, qint32 maxSize = 100);
		~ObjectPool();
	public:
		std::unique_ptr<T, ObjectPoolDeleter<T>> acquire();
		void release(T* obj);
		void clear(bool toMinSize = false);
		qint32 getCurrentSize() const;
		qint32 getAvailableSize() const;
		qint32 getInUseSize() const;
		void setMinSize(qint32 minSize);
		void setMaxSize(qint32 maxSize);
		void setAllocSize(qint32 allocSize);
		void setRecycleSize(qint32 recycleSize);
		void setAutoAdjust(bool autoAdjust);
		qint32 getMinSize() const;
		qint32 getMaxSize() const;
		qint32 getAllocSize() const;
		qint32 getRecycleSize() const;
		bool isAutoAdjust() const;
	private:
		void grow(qint32 count);
		void trimAvailable();
		void deleteFirstAvailable(qint32 count);
	private:
		QQueue<T*> Available;
		std::unordered_set<T*> Borrowed;
		qint32 MinSize;
		qint32 MaxSize;
		qint32 AllocSize;
		qint32 RecycleSize;
		qint32 PeakInUse;
		bool AutoAdjust;
	};
	template <typename T>
	using PooledPtr = std::unique_ptr<T, ObjectPoolDeleter<T>>;
}

#include "private/ObjectPool_impl.hpp"
#endif // Visindigo_Utility_ObjectPool_h
