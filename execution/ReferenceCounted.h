#ifndef __ExecutionReferenceCounted_h__
#define __ExecutionReferenceCounted_h__

#include "MessageException.h"
#include "ExecutionAtomic.h"

namespace exec {

	class ReferenceCounted {
		public:
			enum RetainAction {
				Retain,
				DoNotRetain
			};
			template<typename ReferenceCountedType>
			class Ptr {
				public:
					Ptr(ReferenceCountedType *ptr= NULL, RetainAction action=Retain);
					Ptr(const Ptr &other);
					~Ptr();
					ReferenceCountedType *operator->();
					const ReferenceCountedType *operator->() const;
					Ptr &assign(ReferenceCountedType *ptr= NULL, RetainAction action=Retain);
					Ptr &operator=(const Ptr &other);
					operator bool() const;
					bool valid() const;
				private:
					ReferenceCountedType	*_ptr;
					void _retain();
					void _release();
			};
			ReferenceCounted(int32_t initialCount= 1);
			int32_t references();
			int32_t retain();
			void release();
		protected:
			virtual ~ReferenceCounted();
			virtual void free();
		private:
			AtomicNumber	_references;
	};

	inline ReferenceCounted::ReferenceCounted(int32_t initialCount)
		:_references(initialCount) {
	}
	inline int32_t ReferenceCounted::references() {
		return _references.value();
	}
	inline int32_t ReferenceCounted::retain() {
		//printf("+%08x:%d\n", this, references());
		return _references.valueAfterIncrement();
	}
	inline void ReferenceCounted::release() {
		//printf("-%08x:%d\n", this, references());
		int32_t	refs= _references.valueAfterDecrement();

		if(refs <= 0) {
			free();
		}
	}
	inline ReferenceCounted::~ReferenceCounted() {
	}
	inline void ReferenceCounted::free() {
		delete this;
	}
	template<typename ReferenceCountedType>
	ReferenceCounted::Ptr<ReferenceCountedType>::Ptr(ReferenceCountedType *ptr, ReferenceCounted::RetainAction action)
		:_ptr(ptr) {
		if(Retain == action) {
			_retain();
		}
	}
	template<typename ReferenceCountedType>
	ReferenceCounted::Ptr<ReferenceCountedType>::Ptr(const Ptr &other)
		:_ptr(other._ptr) {
		_retain();
	}
	template<typename ReferenceCountedType>
	ReferenceCounted::Ptr<ReferenceCountedType>::~Ptr() {
		_release();
	}
	template<typename ReferenceCountedType>
	ReferenceCountedType *ReferenceCounted::Ptr<ReferenceCountedType>::operator->() {
		ThrowMessageExceptionIfNULL(_ptr);
		return _ptr;
	}
	template<typename ReferenceCountedType>
	const ReferenceCountedType *ReferenceCounted::Ptr<ReferenceCountedType>::operator->() const {
		ThrowMessageExceptionIfNULL(_ptr);
		return _ptr;
	}
	template<typename ReferenceCountedType>
	ReferenceCounted::Ptr<ReferenceCountedType> &ReferenceCounted::Ptr<ReferenceCountedType>::assign(ReferenceCountedType *ptr, ReferenceCounted::RetainAction action) {
		if(_ptr != ptr) {
			_release();
			_ptr= ptr;
			if(Retain == action) {
				_retain();
			}
		}
		return *this;
	}
	template<typename ReferenceCountedType>
	ReferenceCounted::Ptr<ReferenceCountedType> &ReferenceCounted::Ptr<ReferenceCountedType>::operator=(const ReferenceCounted::Ptr<ReferenceCountedType>::Ptr &other) {
		if(this != &other) {
			return assign(other._ptr);
		}
		return *this;
	}
	template<typename ReferenceCountedType>
	ReferenceCounted::Ptr<ReferenceCountedType>::operator bool() const {
		return valid();
	}
	template<typename ReferenceCountedType>
	bool ReferenceCounted::Ptr<ReferenceCountedType>::valid() const {
		return NULL != _ptr;
	}
	template<typename ReferenceCountedType>
	void ReferenceCounted::Ptr<ReferenceCountedType>::_retain() {
		if(valid()) {
			_ptr->retain();
		}
	}
	template<typename ReferenceCountedType>
	void ReferenceCounted::Ptr<ReferenceCountedType>::_release() {
		if(valid()) {
			_ptr->release();
		}
	}
}

#endif // __ReferenceCounted_h__
