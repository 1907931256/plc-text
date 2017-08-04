#ifndef REF_COUNTED
#define REF_COUNTED

#include "qatomic.h"

class RefCountedBase {
public:
	bool HasOneRef() const;

protected:
	RefCountedBase();
	~RefCountedBase();

	void AddRef() const;

	// Returns true if the object should self-delete.
	bool Release() const;

private:
	mutable QAtomicInt ref_count_;
	mutable bool in_dtor_;
};

template <class T, typename Traits> class RefCounted;

template<typename T>
struct DefaultRefCountedThreadSafeTraits {
	static void Destruct(const T* x) {
		RefCounted<T,
			DefaultRefCountedThreadSafeTraits>::DeleteInternal(x);
	}
};

template <class T, typename Traits = DefaultRefCountedThreadSafeTraits<T> >
class RefCounted : public RefCountedBase {
public:
	RefCounted() {}

	void AddRef() const {
		RefCountedBase::AddRef();
	}

	void Release() const {
		if (RefCountedBase::Release()) {
			Traits::Destruct(static_cast<const T*>(this));
		}
	}

protected:
	~RefCounted() {}

private:
	friend struct DefaultRefCountedThreadSafeTraits<T>;
	static void DeleteInternal(const T* x) { delete x; }
};

#endif
