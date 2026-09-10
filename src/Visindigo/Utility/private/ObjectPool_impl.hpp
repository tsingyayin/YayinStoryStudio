#ifndef Visindigo_Utility_private_ObjectPool_impl_h
#define Visindigo_Utility_private_ObjectPool_impl_h

namespace Visindigo::Utility {
	// 构造函数：归一化参数，预分配 initialSize 个空闲对象（受 MaxSize 约束）。
	template <Reusable T>
	ObjectPool<T>::ObjectPool(qint32 initialSize, qint32 minSize, qint32 maxSize) {
		MaxSize = qMax<qint32>(1, maxSize);
		MinSize = qBound<qint32>(0, minSize, MaxSize);
		AllocSize = qMax<qint32>(1, MinSize / 2);
		RecycleSize = MaxSize;
		AutoAdjust = false;
		PeakInUse = 0;
		const qint32 prewarm = qBound<qint32>(0, initialSize, MaxSize);
		Available.reserve(prewarm);
		for (qint32 i = 0; i < prewarm; ++i)
			Available.enqueue(new T());
	}

	// 析构：释放所有空闲对象。借出的对象由各自句柄负责归还/销毁；
	// 池对象须比所有借出的句柄活得更久，否则句柄析构会访问已销毁的池。
	template <Reusable T>
	ObjectPool<T>::~ObjectPool() {
		while (!Available.isEmpty())
			delete Available.dequeue();
	}

	// 借出：优先复用空闲对象；空闲不足且未达上限时按 allocSize 批量补货；
	// 已达上限则借出"临时对象"（不入池、不计 in-use），归还时直接销毁，池不膨胀。
	template <Reusable T>
	typename PooledPtr<T> ObjectPool<T>::acquire() {
		T* obj = nullptr;
		if (!Available.isEmpty()) {
			obj = Available.dequeue(); // FIFO：优先复用最早空闲的对象
			Borrowed.insert(obj);
		} else {
			const qint32 total = (qint32)(Available.size() + Borrowed.size());
			if (total < MaxSize) {
				qint32 batch = AllocSize;
				if (AutoAdjust)
					batch = qMax<qint32>(batch, qMin<qint32>(PeakInUse, MaxSize - total));
				grow(batch);
				obj = Available.dequeue();
				Borrowed.insert(obj);
			} else {
				obj = new T(); // 临时对象：不托管
			}
		}
		PeakInUse = qMax<qint32>(PeakInUse, (qint32)Borrowed.size());
		return Ptr(obj, ObjectPoolDeleter<T>(this)); // 绑定本池：句柄析构自动归还
	}

	// 手动归还：托管对象复位后入空闲池（受上限约束）；临时对象直接销毁；
	// 重复归还（对象仍在空闲池）忽略以防二次入池/误删。调用方须先移交所有权。
	template <Reusable T>
	void ObjectPool<T>::release(T* obj) {
		if (obj == nullptr)
			return;
		const auto it = Borrowed.find(obj);
		if (it != Borrowed.end()) {
			Borrowed.erase(it);
			obj->reset();
			const qint32 total = (qint32)(Available.size() + Borrowed.size());
			if (total >= MaxSize) { // 已达托管上限：不缓存，直接销毁
				delete obj;
				return;
			}
			Available.enqueue(obj);
			trimAvailable(); // 空闲数超过高水位时裁剪
		} else {
			// 不在借出集：对象仍在空闲池 = 重复归还，忽略；否则视为临时对象销毁。
			if (!Available.contains(obj))
				delete obj;
		}
	}

	// 释放空闲对象；toMinSize 为真则保留 minSize 个（保留列表尾部较新者）。
	template <Reusable T>
	void ObjectPool<T>::clear(bool toMinSize) {
		const qint32 keep = toMinSize ? qMin<qint32>(MinSize, (qint32)Available.size()) : 0;
		const qint32 drop = (qint32)Available.size() - keep;
		deleteFirstAvailable(drop);
	}

	template <Reusable T>
	qint32 ObjectPool<T>::getCurrentSize() const {
		return (qint32)(Available.size() + Borrowed.size());
	}

	template <Reusable T>
	qint32 ObjectPool<T>::getAvailableSize() const {
		return (qint32)Available.size();
	}

	template <Reusable T>
	qint32 ObjectPool<T>::getInUseSize() const {
		return (qint32)Borrowed.size();
	}

	template <Reusable T>
	void ObjectPool<T>::setMinSize(qint32 minSize) {
		MinSize = qBound<qint32>(0, minSize, MaxSize);
	}

	template <Reusable T>
	void ObjectPool<T>::setMaxSize(qint32 maxSize) {
		MaxSize = qMax<qint32>(1, maxSize);
		if (MinSize > MaxSize)
			MinSize = MaxSize;
		// 收紧上限后：空闲过多则裁剪（借出对象不受影响）。
		const qint32 keep = qMax<qint32>(0, MaxSize - (qint32)Borrowed.size());
		const qint32 avail = (qint32)Available.size();
		if (avail > keep)
			deleteFirstAvailable(avail - keep);
	}

	template <Reusable T>
	void ObjectPool<T>::setAllocSize(qint32 allocSize) {
		AllocSize = qMax<qint32>(1, allocSize);
	}

	template <Reusable T>
	void ObjectPool<T>::setRecycleSize(qint32 recycleSize) {
		RecycleSize = qMax<qint32>(MinSize, recycleSize);
	}

	template <Reusable T>
	void ObjectPool<T>::setAutoAdjust(bool autoAdjust) {
		AutoAdjust = autoAdjust;
	}

	template <Reusable T>
	qint32 ObjectPool<T>::getMinSize() const {
		return MinSize;
	}

	template <Reusable T>
	qint32 ObjectPool<T>::getMaxSize() const {
		return MaxSize;
	}

	template <Reusable T>
	qint32 ObjectPool<T>::getAllocSize() const {
		return AllocSize;
	}

	template <Reusable T>
	qint32 ObjectPool<T>::getRecycleSize() const {
		return RecycleSize;
	}

	template <Reusable T>
	bool ObjectPool<T>::isAutoAdjust() const {
		return AutoAdjust;
	}

	// 空闲不足时批量新建 count 个对象加入空闲池（不超过 MaxSize 上限）。
	// 注意：对象一律以 new 创建，因此回收侧统一用 delete 释放（不可改成 malloc + placement new，
	// 否则对 malloc 块内部的元素调用 delete 会破坏堆）。
	template <Reusable T>
	void ObjectPool<T>::grow(qint32 count) {
		const qint32 total = (qint32)(Available.size() + Borrowed.size());
		const qint32 room = qMax<qint32>(0, MaxSize - total);
		count = qMin<qint32>(count, room);
		for (qint32 i = 0; i < count; ++i) {
			Available.enqueue(new T());
		}
	}

	// 空闲数超过高水位 RecycleSize 时，裁剪回目标下限：
	// 默认保留 minSize 个；开启 autoAdjust 时按近期借用峰值保留（不低于 minSize）。
	template <Reusable T>
	void ObjectPool<T>::trimAvailable() {
		const qint32 avail = (qint32)Available.size();
		if (avail <= RecycleSize)
			return;
		qint32 target = MinSize;
		if (AutoAdjust)
			target = qBound<qint32>(MinSize, PeakInUse, RecycleSize);
		if (avail <= target)
			return;
		deleteFirstAvailable(avail - target);
	}

	// 从空闲队列队首删除 count 个对象（保留队尾较新对象）。
	template <Reusable T>
	void ObjectPool<T>::deleteFirstAvailable(qint32 count) {
		const qint32 n = qMax<qint32>(0, qMin<qint32>(count, (qint32)Available.size()));
		for (qint32 i = 0; i < n; ++i)
			delete Available.dequeue();
	}
}
#endif // Visindigo_Utility_private_ObjectPool_impl_h
