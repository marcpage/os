#ifndef __ReferenceCounted_h__
#define __ReferenceCounted_h__

#include "Exception.h"
#include "AtomicInteger.h"

namespace exec {

	/** The parent of a reference counted object.
		The default behavior is that instances are allocated with
			new() and when the reference count reaches zero, delete()
			will be called automatically on this object.
	*/
	class ReferenceCounted {
		public:
			/** What Retain action to perform.
			*/
			enum RetainAction {
				Retain,	///< Do increment the reference count
				DoNotRetain	///< Do not increment the reference count
			};
			/** A templated "Smart Pointer" for a reference counted item.
				Typically, in your subclass you will define something like this:
					typedef ReferenceCounted::Ptr<MyRefObj> Ptr;
				@todo Should we implement operator*? I think not as it helps protect against
						having non-smart pointers running around
			*/
			template<typename ReferenceCountedType>
			class Ptr {
				public:
					/// Holds on to an actual pointer and reference counts it
					Ptr(ReferenceCountedType *ptr= NULL, RetainAction action=Retain);
					/// Copies another pointer, incrementing the reference count
					Ptr(const Ptr &other);
					/// Decrements the reference count
					~Ptr();
					/// Dereference the "Smart Pointer"
					ReferenceCountedType *operator->();
					/// const dereference the "Smart Pointer"
					const ReferenceCountedType *operator->() const;
					/// Dereference the "Smart Pointer"
					ReferenceCountedType &operator*();
					/// const dereference the "Smart Pointer"
					const ReferenceCountedType &operator*() const;
					/// Assign another actual pointer to this smart pointer
					Ptr &assign(ReferenceCountedType *ptr= NULL, RetainAction action=Retain);
					/// Copy the given pointer
					Ptr &operator=(const Ptr &other);
					/// Is it non-NULL
					operator bool() const;
					/// Is it non-NULL
					bool valid() const;
					operator ReferenceCountedType*();
					operator const ReferenceCountedType*() const;
				private:
					ReferenceCountedType	*_ptr; ///< The actual reference counted pointer, or NULL
					void _retain();	///< Handle retain without worrying about NULL
					void _release();	///< Handle release without worrying about NULL
			};
			/// Default reference count is 1
			ReferenceCounted(int32_t initialCount= 1);
			/// Get the number of references to this object
			int32_t references();
			/// Increment the reference count
			int32_t retain();
			/// Decrement the reference count, and call free() if reference count is zero
			void release();
		protected:
			/// only called from delete() in the free() call
			virtual ~ReferenceCounted();
			/// defaults to calling delete on this, but can be overridden
			virtual void free();
		private:
			AtomicInteger	_references;	///< The number of references to this object
	};

	/**
		@param initialCount	The initial number of references
	*/
	inline ReferenceCounted::ReferenceCounted(int32_t initialCount)
		:_references(initialCount) {;
	}
	/**
		@return	The number of references to the object
	*/
	inline int32_t ReferenceCounted::references() {;
		return _references.value();
	}
	/** Increments the reference count.
		@return The number of references after we increment the count
	*/
	inline int32_t ReferenceCounted::retain() {;
		//printf("+%08x:%d\n", this, references());
		return _references.valueAfterIncrement();
	}
	/** Decrements the reference count.
		@todo For speed improvement, we don't have to decrement if references == 1,
				we can just set it to zero and free.
	*/
	inline void ReferenceCounted::release() {;
		//printf("-%08x:%d\n", this, references());
		int32_t	refs= _references.valueAfterDecrement();

		if(refs <= 0) {
			free();
		}
	}
	/** Does nothing.
	*/
	inline ReferenceCounted::~ReferenceCounted() {;}
	/** Calls delete() on this.
		Override this method if you allocate your objects some other way.
		For example, if you allocated some memory with malloc(), and then explicitly called
		the constructor, in here you would explicitly call the destructor and then call free().
	*/
	inline void ReferenceCounted::free() {;
		delete this;
	}
	/**
		@param ptr		The reference counted pointer to manage. Can be NULL
		@param action	If Retain then the reference count will be incremented.
							Note the reference count on the pointer <b>will</b> be
							decremented at some point (either before being assigned
							another value, or upon zero reference count).
		@todo TEST when action == DoNotRetain
	*/
	template<typename ReferenceCountedType>
	ReferenceCounted::Ptr<ReferenceCountedType>::Ptr(ReferenceCountedType *ptr, ReferenceCounted::RetainAction action)
		:_ptr(ptr) {;
		if(Retain == action) {
			_retain();
		}
	}
	/**
		@param other	The "Smart Pointer" to copy. Note that the pointer is copied
							and not the smart object. Both other and *this will point
							to the same Reference Counted object.
	*/
	template<typename ReferenceCountedType>
	ReferenceCounted::Ptr<ReferenceCountedType>::Ptr(const Ptr &other)
		:_ptr(other._ptr) {;
		_retain();
	}
	/** Calls release on the actual pointer (if not NULL).
	*/
	template<typename ReferenceCountedType>
	ReferenceCounted::Ptr<ReferenceCountedType>::~Ptr() {;
		_release();
	}
	/** Accesses members of the reference counted object.
		@return The referenced member.
		@throw msg::MessageException if the pointer is NULL.
		@todo TEST dereferencing a NULL Ptr
	*/
	template<typename ReferenceCountedType>
	ReferenceCountedType *ReferenceCounted::Ptr<ReferenceCountedType>::operator->() {;
		ThrowMessageExceptionIfNULL(_ptr);
		return _ptr;
	}
	/** Accesses members of the reference counted object.
		@return The referenced member.
		@throw msg::MessageException if the pointer is NULL.
		@todo TEST dereferencing a NULL Ptr
		@todo TEST const dereference
	*/
	template<typename ReferenceCountedType>
	const ReferenceCountedType *ReferenceCounted::Ptr<ReferenceCountedType>::operator->() const {;
		ThrowMessageExceptionIfNULL(_ptr);
		return _ptr;
	}
	/** Accesses the reference counted object.
		@return The referenced object.
		@throw msg::MessageException if the pointer is NULL.
		@todo TEST
	*/
	template<typename ReferenceCountedType>
	ReferenceCountedType &ReferenceCounted::Ptr<ReferenceCountedType>::operator*() {;
		ThrowMessageExceptionIfNULL(_ptr);
		return *_ptr;
	}
	/** Accesses the reference counted object.
		@return The referenced object.
		@throw msg::MessageException if the pointer is NULL.
		@todo TEST
	*/
	template<typename ReferenceCountedType>
	const ReferenceCountedType &ReferenceCounted::Ptr<ReferenceCountedType>::operator*() const {;
		ThrowMessageExceptionIfNULL(_ptr);
		return *_ptr;
	}
	/** Assigns a new reference counted pointer to the smart pointer.
		If the "Smart Pointer" is not NULL, the value will be Released() before the assignment.
		@param ptr		The pointer to now reference.
		@param action	If Retain, the ptr will be retained, otherwise just assigned.
		@todo TEST when action == DoNotRetain
	*/
	template<typename ReferenceCountedType>
	ReferenceCounted::Ptr<ReferenceCountedType> &ReferenceCounted::Ptr<ReferenceCountedType>::assign(ReferenceCountedType *ptr, ReferenceCounted::RetainAction action) {;
		if(_ptr != ptr) {
			_release();
			_ptr= ptr;
			if(Retain == action) {
				_retain();
			}
		}
		return *this;
	}
	/** Changes the pointer to point to a new object.
		If the "Smart Pointer" is not NULL, the value will be Released() before the assignment.
		@param other	The object to assign
		@return			Reference to *this
	*/
	template<typename ReferenceCountedType>
	ReferenceCounted::Ptr<ReferenceCountedType> &ReferenceCounted::Ptr<ReferenceCountedType>::operator=(const ReferenceCounted::Ptr<ReferenceCountedType>::Ptr &other) {;
		if(this != &other) {
			return assign(other._ptr, Retain);
		}
		return *this;
	}
	/** Support of if(pointer) {} or while(pointer) {}.
		@return true if the pointer is not NULL.
	*/
	template<typename ReferenceCountedType>
	ReferenceCounted::Ptr<ReferenceCountedType>::operator bool() const {;
		return (valid());
	}
	/**
		@return true if the pointer is not NULL.
	*/
	template<typename ReferenceCountedType>
	bool ReferenceCounted::Ptr<ReferenceCountedType>::valid() const {;
		return (NULL != _ptr);
	}
	template<typename ReferenceCountedType>
	ReferenceCounted::Ptr<ReferenceCountedType>::operator ReferenceCountedType*() {;
		return _ptr;
	}
	template<typename ReferenceCountedType>
	ReferenceCounted::Ptr<ReferenceCountedType>::operator const ReferenceCountedType*() const {;
		return _ptr;
	}
	/** NULL safe retain.
	*/
	template<typename ReferenceCountedType>
	void ReferenceCounted::Ptr<ReferenceCountedType>::_retain() {;
		if(valid()) {
			_ptr->retain();
		}
	}
	/** NULL safe release.
	*/
	template<typename ReferenceCountedType>
	void ReferenceCounted::Ptr<ReferenceCountedType>::_release() {;
		if(valid()) {
			_ptr->release();
		}
	}
}

#endif // __ReferenceCounted_h__
