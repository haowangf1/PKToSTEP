#ifndef _SMART_PTR_HPP_
#define _SMART_PTR_HPP_

#include <iostream>
#include "base/xchg_type.hpp"
#include <iosfwd>

//! \endcond
/** \defgroup ptr SmartPointer Classes
 * Encapsulate other entities for better memory management
 */

 class Xchg_Object;
 class Xchg_Entity;
 class Xchg_Info;
 class Xchg_matrix;
 class Xchg_transfo;
 class Xchg_material;



//! \ingroup ptr
//! \class SmartPtr
//! This the Smart Pointer template<br> 
template <typename T>
class SmartPtr
{
public:
	typedef T	element_type;
	typedef T*	pointer;

protected:
    T * ptr_;     
public:
    inline pointer operator->() const XCHG_NOEXCEPT
	{
        XCHG_ASSERT(ptr_);
        return ptr_; 
    }

	inline element_type & operator*() const XCHG_NOEXCEPT
	{
        XCHG_ASSERT(ptr_);
		return *ptr_;
	}

    explicit operator bool() const { return ptr_ != nullptr; }

    ~SmartPtr();
    SmartPtr();
    SmartPtr(T* p);
    SmartPtr(const SmartPtr<T>& p);

	//Move constructor/assignment
	SmartPtr(SmartPtr && p) XCHG_NOEXCEPT : ptr_(p.ptr_)
	{
		p.ptr_ = nullptr;
	}

	SmartPtr& operator=(SmartPtr && p) XCHG_NOEXCEPT
	{
		if(this != &p)
		{
			release();
			ptr_ = p.ptr_;
			p.ptr_ = nullptr;
		}

		return *this;
	}

	pointer get() const XCHG_NOEXCEPT
	{  return ptr_;  }

    template <typename T2>
    static SmartPtr<T> DtkDynamicCast(const SmartPtr<T2>& p)
    {
        if( p.IsNULL() )
        {
            return NULL;
        }
        else
        {
            return T::DtkDynamicCast(p.operator ->());
        }
    }

    SmartPtr<T>& operator= (const SmartPtr<T>& p) XCHG_NOEXCEPT;

    void Clone(SmartPtr<T>& out) const;
	//! \brief Resets the SmartPtr content.
	void Clear();
    bool IsNULL()  const {return ((!ptr_)?(true):(false));}
    bool IsNotNULL()  const {return ((ptr_)?(true):(false));}

    friend class Xchg_Object;
    friend std::ostream& operator<<(std::ostream& o,const SmartPtr& d)
    {
        if (d.ptr_)
        {
            o <<"<SmartPtr><Adr>" << d.ptr_ << "</Adr><Val>";
            o << *(d.ptr_);
            o << "</Val></SmartPtr>"<<std::endl;
            return o;
        }
        else
            return o<<"<SmartPtr><Adr>" << d.ptr_ << "</Adr></SmartPtr>"<<std::endl;
    }
private:
    void release() XCHG_NOEXCEPT;

};

///////////////////////////////////////////////////////////////////////////


template <typename T>
void SmartPtr<T>::release() XCHG_NOEXCEPT
{
    if(ptr_)
    {
        if (--ptr_->count_ == 0)
        {
            delete ptr_; 
            ptr_ = NULL;
        }
    }
}

template <typename T>
SmartPtr<T>::~SmartPtr()
{
    release();
}
template <typename T>
SmartPtr<T>::SmartPtr()
{
    ptr_ = NULL;
}

template <typename T>
SmartPtr<T>::SmartPtr(T* p):ptr_(p)
{
    if(ptr_)
        ++ptr_->count_;
} 

template <typename T>
SmartPtr<T>::SmartPtr(const SmartPtr<T>& p):ptr_(p.ptr_)
{
    if(ptr_)
        ++ptr_->count_;
}

template <typename T>
SmartPtr<T>& SmartPtr<T>::operator= (const SmartPtr<T>& p) XCHG_NOEXCEPT
{  // NE CHANGEZ PAS L'ORDRE DE CES INSTRUCTIONS!
    // (Cet ordre gere correctement le cas de l'auto-affectation)
    if(p.ptr_)
        ++p.ptr_->count_;
    release();
    ptr_ = p.ptr_;
    return *this;
}

template <typename T>
void SmartPtr<T>::Clone(SmartPtr<T>& res) const
{
    if(ptr_)
    {
        res = T::DtkDynamicCast( ptr_->Clone() );
    }
}

//! \brief Resets the SmartPtr content.
template <typename T>
void SmartPtr<T>::Clear()
{
	release();
	ptr_ = NULL;
}

template <class T1, class T2>
inline bool operator==(const SmartPtr<T1> &x, const SmartPtr<T2> &y) XCHG_NOEXCEPT
{  return x.get() == y.get(); }

template <class T1, class T2>
inline bool operator!=(const SmartPtr<T1> &x, const SmartPtr<T2> &y) XCHG_NOEXCEPT
{  return !(x == y); }

template <class T1, class T2>
inline bool operator<(const SmartPtr<T1> &x, const SmartPtr<T2> &y) XCHG_NOEXCEPT
{  return x.get() < y.get(); }

template <class T1, class T2>
inline bool operator<=(const SmartPtr<T1> &x, const SmartPtr<T2> &y) XCHG_NOEXCEPT
{  return !(y < x); }

template <class T1, class T2>
inline bool operator>(const SmartPtr<T1> &x, const SmartPtr<T2> &y) XCHG_NOEXCEPT
{  return y < x; }

template <class T1, class T2>
inline bool operator>=(const SmartPtr<T1> &x, const SmartPtr<T2> &y) XCHG_NOEXCEPT
{  return !(x < y); }


template <class T>
inline bool operator==(const SmartPtr<T> &x, std::nullptr_t) XCHG_NOEXCEPT
{  return !x;  }

template <class T>
inline bool operator==(std::nullptr_t, const SmartPtr<T> &x) XCHG_NOEXCEPT
{  return !x;  }

template <class T>
inline bool operator!=(const SmartPtr<T> &x, std::nullptr_t) XCHG_NOEXCEPT
{  return !!x;  }

template <class T>
inline bool operator!=(std::nullptr_t, const SmartPtr<T> &x) XCHG_NOEXCEPT
{  return !!x;  }

template <class T>
inline bool operator<(const SmartPtr<T> &x, nullptr_t) XCHG_NOEXCEPT
{  return x.get() < typename SmartPtr<T>::pointer();  }

template <class T>
inline bool operator<(std::nullptr_t, const SmartPtr<T> &x) XCHG_NOEXCEPT
{  return typename SmartPtr<T>::pointer() < x.get();  }

template <class T>
inline bool operator>(const SmartPtr<T> &x, std::nullptr_t) XCHG_NOEXCEPT
{  return x.get() > typename SmartPtr<T>::pointer();  }

template <class T>
inline bool operator>(std::nullptr_t, const SmartPtr<T> &x) XCHG_NOEXCEPT
{  return typename SmartPtr<T>::pointer() > x.get();  }

template <class T>
inline bool operator<=(const SmartPtr<T> &x, std::nullptr_t) XCHG_NOEXCEPT
{  return !(x > std::nullptr_t());  }

template <class T>
inline bool operator<=(std::nullptr_t, const SmartPtr<T> &x) XCHG_NOEXCEPT
{  return !(std::nullptr_t() > x);  }

template <class T>
inline bool operator>=(const SmartPtr<T> &x, std::nullptr_t) XCHG_NOEXCEPT
{  return !(x < std::nullptr_t());  }

template <class T>
inline bool operator>=(std::nullptr_t, const SmartPtr<T> &x) XCHG_NOEXCEPT
{  return !(std::nullptr_t() < x);  }

//! \ingroup ptr
//! \typedef ObjectPtr 
//! \brief Handles a Xchg_Object object
typedef SmartPtr<Xchg_Object> ObjectBasePtr;

//! \ingroup ptr
//! \typedef Xchg_EntityPtr 
//! \brief Handles a Xchg_Entity object
typedef SmartPtr<Xchg_Entity> Xchg_EntityPtr;

//! \ingroup ptr
//! \typedef Xchg_matrixPtr 
//! \brief Handles a Xchg_matrix object
//! \sa Xchg_dimension
typedef SmartPtr<Xchg_matrix> Xchg_matrixPtr;

//! \ingroup ptr
//! \typedef Xchg_transfoPtr 
//! \brief Handles a Xchg_transfo object
//! \sa Xchg_dimension
typedef SmartPtr<Xchg_transfo> Xchg_transfoPtr;

//! \ingroup ptr
//! \typedef Xchg_InfoPtr 
//! \brief Handles a Xchg_Info object
//! \sa Xchg_Info
typedef SmartPtr<Xchg_Info> Xchg_InfoPtr;

typedef SmartPtr<Xchg_material>  Xchg_MaterialPtr;


#endif


