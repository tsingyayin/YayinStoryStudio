#ifndef Visindigo_VIMacro_h
#define Visindigo_VIMacro_h
#ifdef __cpp_lib_stacktrace
#if __cpp_lib_stacktrace >= 202011L
#define VI_HAS_STD_STACKTRACE
#endif
#endif

#define VINotNull // marker for not nullable
#define VINullable // marker for nullable
#define VIUnstable // marker for a weak pointer
#define VIContent // marker for a string contains content itself
#define VIPath // marker for a string represents a path, not a content
#define VITransferred // marker for a transferred value

#define VInterface class

#define VIAPIClass(name) \
	class name##Private; \
	class VisindigoAPI name

#define VImpl(name)\
	friend class name##Private; \
	private: name##Private* d; \

#define VImplClass(name) \
	class name##Private

#define VIAPI(name) \
	friend class name; \
	private: name* q;

#define p_VIAPICopyable(name)\
	name##Private(const name& other) = default;

#define VICopyable(name)\
	public: \
	name(const name& other); \
	name& operator=(const name& other); \

#define VICopyable_Impl(name)\
	name::name(const name& other){\
		d = new name##Private(*other.d); \
	}\
	name& name::operator=(const name& other) {\
		if (this != &other) { \
			*d = *other.d; \
		} \
		return *this; \
	}

#define VIMoveable(name)\
	public: \
	name(name&& other) noexcept; \
	name& operator=(name&& other) noexcept; \

#define VIMoveable_Impl(name)\
	name::name(name&& other) noexcept : d(other.d) { \
		other.d = nullptr; \
	} \
	name& name::operator=(name&& other) noexcept { \
		if (this != &other) { \
			delete d; \
			d = other.d; \
			other.d = nullptr; \
		} \
		return *this; \
	} \

#define VImplPooled(name)\
	friend class name##Private; \
	private: Visindigo::Utility::PooledPtr<name##Private> d; \

#define VICopyablePooled_Impl(name)\
	name::name(const name& other) {\
		if (other.d) {\
			Visindigo::Utility::ObjectPool<name##Private>* pool = static_cast<Visindigo::Utility::ObjectPool<name##Private>*>(other.d.get_deleter().pool());\
			if (pool) {\
				d = pool->acquire();\
				*d = *other.d;\
			}\
		}\
	}\
	name& name::operator=(const name& other) {\
		if (this != &other) {\
			if (other.d) {\
				Visindigo::Utility::ObjectPool<name##Private>* pool = static_cast<Visindigo::Utility::ObjectPool<name##Private>*>(other.d.get_deleter().pool());\
				if (pool) {\
					if (!d)\
						d = pool->acquire();\
					*d = *other.d;\
				}\
			} else {\
				d.reset();\
			}\
		}\
		return *this;\
	}\

#define VIMoveablePooled_Impl(name)\
	name::name(name&& other) noexcept : d(std::move(other.d)) {\
	}\
	name& name::operator=(name&& other) noexcept {\
		if (this != &other) {\
			d = std::move(other.d);\
		}\
		return *this;\
	}\

#define VI_Delete(ptr)\
	if (ptr) { \
		delete ptr; \
		ptr = nullptr; \
	}

#define VI_DeleteArray(ptr) \
	if (ptr) { \
		delete[] ptr; \
		ptr = nullptr; \
	}

#define VI_DeleteQObject(ptr) \
	if (ptr) { \
		ptr->deleteLater(); \
		ptr = nullptr; \
	}
#endif // Visindigo_VIMacro_h
