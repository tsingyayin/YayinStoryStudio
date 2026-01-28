#pragma once

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
			d = other.d; \
			other.d = nullptr; \
		} \
		return *this; \
	} \

#define VI_Singleton(name) \
	public: static name* getInstance();

#define VI_Singleton_Definition(name) \
	static name* Instance;

#define VI_Sigleton_Init(name) \
	name* name##Private::Instance = nullptr;

#define VI_Singleton_Impl(name) \
	name* name::getInstance() { \
		if (name##Private::Instance == nullptr) { \
			name##Private::Instance = new name(); \
		} \
		return name##Private::Instance; \
	}

#define VI_Singleton_Construct(name) \
	if (name##Private ::Instance == nullptr) { \
		name##Private::Instance = this; \
	} else { \
		return; \
	}

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