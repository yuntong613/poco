//
// WeakRefPtr.h
//
// Library: Foundation
// Package: Core
// Module:  WeakRefPtr
//
// Definition of the WeakRefPtr template class.
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//


#ifndef Foundation_WeakRefPtr_INCLUDED
#define Foundation_WeakRefPtr_INCLUDED


#include "Poco/Foundation.h"
#include "Poco/RefCountedObject.h"
#include "Poco/Exception.h"


namespace Poco {


template <class C, class M = Mutex>
class WeakRefPtr
	/// WeakRefPtr is a "smart" pointer for classes implementing
	/// reference counting. WeakRefPtr keeps a "weak" reference
	/// to the object; the referenced object lifetime is not dependent
	/// on the WeakRefPtr reference(s) to it - it will be destroyed
	/// even when those exist.
	/// To enforce referenced object's lifetime duration for as long as
	/// references to it exist, use RefPtr.
{
public:
	WeakRefPtr(): _ptr(0)
		/// Creates null WeakRefPtr
	{
	}

	WeakRefPtr(const WeakRefPtr& weakPtr): _ptr(weakPtr._ptr),
		_pCounter(weakPtr._pCounter)
		/// Creates WeakRefPtr by copying from another WeakRefPtr
		/// and increasing weak count.
	{
		duplicate();
	}

	WeakRefPtr(WeakRefPtr&& weakPtr): _ptr(weakPtr._ptr),
		_pCounter(weakPtr._pCounter)
		/// Creates WeakRefPtr by moving from another WeakRefPtr.
	{
		weakPtr._ptr = 0;
		weakPtr._pCounter = 0;
	}

	template <class Other>
	WeakRefPtr(const WeakRefPtr<Other>& weakPtr): _ptr(const_cast<Other*>(weakPtr._ptr)),
		_pCounter(weakPtr._pCounter)
		/// Creates WeakRefPtr by copying from another WeakRefPtr of different
		/// type. Pointer held by another WeakRefPtr must be convertible to pointer
		/// held by this WeakRefPtr.
	{
		duplicate();
	}

	template <class Other>
	WeakRefPtr(WeakRefPtr<Other>&& weakPtr): _ptr(weakPtr._ptr),
		_pCounter(weakPtr._pCounter)
		/// Creates WeakRefPtr by moving from another WeakRefPtr.
		/// Pointer held by another WeakRefPtr must be convertible to
		/// pointer held by this WeakRefPtr.
	{
		weakPtr._ptr = 0;
		weakPtr._pCounter = 0;
	}

	WeakRefPtr(const RefPtr<C>& autoPtr): _ptr(const_cast<RefPtr<C>&>(autoPtr).get()),
		_pCounter(_ptr ? _ptr->counter() : 0)
		/// Creates WeakRefPtr by copying from a RefPtr and
		/// increasing the weak count.
	{
		duplicate();
	}

	template <class Other>
	WeakRefPtr(const RefPtr<Other>& autoPtr): _ptr(const_cast<RefPtr<C>&>(autoPtr).get()),
		_pCounter(_ptr ? _ptr->counter() : 0)
		/// Creates WeakRefPtr by copying from a RefPtr of different
		/// type and increasing the weak count. Pointer held by the RefPtr
		/// must be convertible to pointer held by this WeakRefPtr.
	{
		duplicate();
	}

	WeakRefPtr(RefPtr<C>&& autoPtr): _ptr(autoPtr.get()),
			_pCounter(_ptr ? _ptr->counter() : 0)
		/// Creates WeakRefPtr by moving from a RefPtr and
		/// increasing the weak count.
		///
		/// Moved-from RefPtr count is decreased and the
		/// pointer is set to zero.
	{
		autoPtr.reset();
		duplicate();
	}

	template <class Other>
	WeakRefPtr(RefPtr<Other>&& autoPtr): _ptr(autoPtr.get()),
			_pCounter(_ptr ? _ptr->counter() : 0)
		/// Creates WeakRefPtr by moving from a RefPtr of different
		/// type. Pointer held by the RefPtr must be convertible to
		/// pointer held by this WeakRefPtr.
		///
		/// Moved-from RefPtr count is decreased and the
		/// pointer is set to zero.
	{
		autoPtr.reset();
		duplicate();
	}

	~WeakRefPtr()
		/// Releases the weak count and destroys the WeakRefPtr.
	{
		release();
	}

	WeakRefPtr& operator = (const WeakRefPtr<C>& weakPtr)
		/// Assigns WeakRefPtr to this WeakRefPtr by copying
		/// from another WeakRefPtr, releases the weak count
		/// to previously held pointer and increases the
		/// weak count to the newly held one.
	{
		if (&weakPtr != this)
		{
			release();
			_ptr = const_cast<C*>(weakPtr.get());
			_pCounter = _ptr ? _ptr->counter() : 0;
			duplicate();
		}
		return *this;
	}

	WeakRefPtr& operator = (WeakRefPtr<C>&& weakPtr)
		/// Assigns WeakRefPtr to this WeakRefPtr by moving
		/// from another WeakRefPtr, releases the weak count
		/// to previously held pointer and increases the
		/// weak count to the newly held one.
	{
		if (&weakPtr != this)
		{
			release();
			_ptr = const_cast<C*>(weakPtr.get());
			_pCounter = _ptr ? _ptr->counter() : 0;
			duplicate();
			weakPtr._ptr = 0;
			weakPtr._pCounter = 0;
		}
		return *this;
	}

	template <class Other>
	WeakRefPtr& operator = (const WeakRefPtr<Other>& weakPtr)
		/// Assigns WeakRefPtr to this WeakRefPtr by copying from
		/// another WeakRefPtr of different type, releases the weak
		/// count to previously held pointer and increases the
		/// weak count to the newly held one.
		/// Pointer to the type held by the RefPtr must be
		/// convertible to pointer to the type held by this
		/// WeakRefPtr.
	{
		if (&weakPtr != this)
		{
			release();
			_ptr = const_cast<Other*>(weakPtr.get());
			_pCounter = _ptr ? _ptr->counter() : 0;
			duplicate();
		}
		return *this;
	}

	template <class Other>
	WeakRefPtr& operator = (WeakRefPtr<Other>&& weakPtr)
		/// Assigns WeakRefPtr to this WeakRefPtr by moving from
		/// another WeakRefPtr of different type, releases the weak
		/// count to previously held pointer and increases the
		/// weak count to the newly held one.
		/// Pointer to the type held by the RefPtr must be
		/// convertible to pointer to the type held by this
		/// WeakRefPtr.
	{
		if (&weakPtr != this)
		{
			release();
			_ptr = const_cast<Other*>(weakPtr.get());
			_pCounter = _ptr ? _ptr->counter() : 0;
			weakPtr._ptr = 0;
			weakPtr._pCounter = 0;
			duplicate();
		}
		return *this;
	}

	WeakRefPtr& operator = (const RefPtr<C>& autoPtr)
		/// Assigns RefPtr to this WeakRefPtr by copying from
		/// another RefPtr, releases the weak count to
		/// previously held pointer and increases the weak
		/// count to the newly held one.
	{
		if (autoPtr.get() != _ptr)
		{
			release();
			_ptr = const_cast<C*>(autoPtr.get());
			_pCounter = _ptr ? _ptr->counter() : 0;
			duplicate();
		}
		return *this;
	}

	WeakRefPtr& operator = (RefPtr<C>&& autoPtr)
		/// Assigns RefPtr to this WeakRefPtr by moving from
		/// another RefPtr, releases the weak count to
		/// previously held pointer and increases the weak
		/// count to the newly held one.
		///
		/// Moved-from RefPtr count is decreased and the
		/// pointer is set to zero.
	{
		if (autoPtr.get() != _ptr)
		{
			release();
			_ptr = const_cast<C*>(autoPtr.get());
			_pCounter = _ptr ? _ptr->counter() : 0;
			autoPtr.reset();
			duplicate();
		}
		return *this;
	}

	template <class Other>
	WeakRefPtr& operator = (const RefPtr<Other>& autoPtr)
		/// Assigns RefPtr to this WeakRefPtr by copying from
		/// another RefPtr of different type, releases the weak
		/// count to previously held pointer and increases the
		/// weak count to the newly held one.
		/// Pointer to the type held by the RefPtr must be
		/// convertible to pointer to the type held by this
		/// WeakRefPtr.
	{
		if (&autoPtr.get() != _ptr)
		{
			release();
			_ptr = const_cast<Other*>(autoPtr.get());
			_pCounter = _ptr ? _ptr->counter() : 0;
			duplicate();
		}
		return *this;
	}

	template <class Other>
	WeakRefPtr& operator = (RefPtr<Other>&& autoPtr)
		/// Assigns RefPtr to this WeakRefPtr by moving from
		/// another RefPtr of different type, releases the weak
		/// count to previously held pointer and increases the
		/// weak count to the newly held one.
		/// Pointer to the type held by the RefPtr must be
		/// convertible to pointer to the type held by this
		/// WeakRefPtr.
		///
		/// Moved-from RefPtr count is decreased and the
		/// pointer is set to zero.
	{
		if (&autoPtr.get() != _ptr)
		{
			release();
			_ptr = const_cast<Other*>(autoPtr.get());
			_pCounter = _ptr ? _ptr->counter() : 0;
			autoPtr.reset();
			duplicate();
		}
		return *this;
	}

	C* operator -> ()
	{
		if (good()) return _ptr;
		throw InvalidAccessException("WeakRefPtr::operator -> ()");
	}

	const C* operator -> () const
	{
		if (good()) return _ptr;
		throw InvalidAccessException("WeakRefPtr::operator -> () const");
	}

	C& operator * ()
	{
		if (good()) return *_ptr;
		throw InvalidAccessException("WeakRefPtr::operator * ()");
	}

	const C& operator * () const
	{
		if (good()) return *_ptr;
		throw InvalidAccessException("WeakRefPtr::operator * () const");
	}

	C* get()
	{
		if (good()) return _ptr;
		return 0;
	}

	const C* get() const
	{
		if (good()) return _ptr;
		return 0;
	}

	operator C* ()
	{
		if (good()) return _ptr;
		return 0;
	}

	operator const C* () const
	{
		if (good()) return _ptr;
		return 0;
	}

	bool operator ! () const
	{
		if (good()) return _ptr == 0;
		return false;
	}

	bool isNull() const
	{
		if (good()) return (_pCounter == 0 || _ptr == 0);
		return true;
	}

	bool operator == (const WeakRefPtr& ptr) const
	{
		if      (good()   &&  ptr) return *_ptr == *ptr;
		else if (isNull() && !ptr) return true;
		return false;
	}

	bool operator == (const C* ptr) const
	{
		if      (good()   &&  ptr) return *_ptr == *ptr;
		else if (isNull() && !ptr) return true;
		return false;
	}

	bool operator == (C* ptr) const
	{
		if      (good()   &&  ptr) return *_ptr == *ptr;
		else if (isNull() && !ptr) return true;
		return false;
	}

	bool operator != (const WeakRefPtr& ptr) const
	{
		if      (good()   &&  ptr) return *_ptr != *ptr;
		else if (isNull() && !ptr) return false;
		return true;
	}

	bool operator != (const C* ptr) const
	{
		if      (good()   &&  ptr) return *_ptr != *ptr;
		else if (isNull() && !ptr) return false;
		return true;
	}

	bool operator != (C* ptr) const
	{
		if      (good()   &&  ptr) return *_ptr != *ptr;
		else if (isNull() && !ptr) return false;
		return true;
	}

	int referenceCount()
	{
		return good() ? _pCounter->count() : 0;
	}

	RefPtr<C> lock() const
	{
		good(); // to set _ptr to 0, if needed
		return RefPtr<C>(_ptr, true);
	}

	void duplicate()
	{
		if (_pCounter)  _pCounter = _pCounter->duplicateWeak();
		if (!_pCounter) _ptr = 0;
	}

	void release()
	{
		if (_pCounter)  _pCounter = _pCounter->releaseWeak();
		if (!_pCounter) _ptr = 0;
	}

private:

	bool good() const
	{
		_ptr = _pCounter ? ((_pCounter->count() > 0) ? _ptr : 0) : 0;
		return _ptr != 0;
	}

	mutable C*         _ptr = 0;
	WeakRefCounter<M>* _pCounter = 0;
};


} // namespace Poco


#endif // Foundation_WeakRefPtr_INCLUDED
