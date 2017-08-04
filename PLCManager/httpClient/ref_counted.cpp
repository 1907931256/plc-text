#include "ref_counted.h"

bool RefCountedBase::HasOneRef() const {
	return ref_count_ == 1;
}

RefCountedBase::RefCountedBase() : ref_count_(0) {
	in_dtor_ = false;
}

RefCountedBase::~RefCountedBase() {
}

void RefCountedBase::AddRef() const {
	ref_count_.ref();
}

bool RefCountedBase::Release() const {
	if (!ref_count_.deref()) {
		in_dtor_ = true;
		return true;
	}
	return false;
}