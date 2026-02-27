#ifndef UTIL_STL_XCHG_HPP_
#define UTIL_STL_XCHG_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include <iosfwd>
#include <iostream>
#include <type_traits>
#include <utility>

template <typename T1,typename T2>
class Xchg_pair;

void Xchg_ThrowOverflowException();

// #ifdef _DEBUG
// #define _DEBUG_DTK
// #endif

//! \ingroup STL_Classes
//! \class Xchg_greater 
//! \brief Functor comparison object
//!
//!   This class allow you to sort elements in order to put the greater first
//! \remark Template structs don't need XCHG_API because they are instantiated in header files
template<typename T>
struct Xchg_greater
{
	//! \brief Overloaded method comparison
	//! \remarks Use the same schema to make a customized functor
	int operator()(const T& t1,const T& t2) const {return t1>t2;}
};

//! \ingroup STL_Classes
//! \class Xchg_less 
//! \brief Functor comparison object
//! \remarks Default comparison functor.
//!
//!   This class allow you to sort elements in order to put the less first
//! \remark Template structs don't need XCHG_API because they are instantiated in header files
template<typename T>
struct Xchg_less
{
	//! \brief Overloaded method comparison
	//! \remarks Use the same schema to make a customized functor
	int operator()(const T& t1,const T& t2) const {return t1<t2;}
};



//! \brief Swap any type of data.
//! \param a first to swap
//! \param b second to swap
//! \remarks Can swap any types (including pointers)
//! \remarks Reference parameters prevent to get adress, provide an easy using.
//!
//! \b Sample:
//! \code 
//! int a = 5;
//! int b = 7;
//! Xchg_swap(a,b);  // now a=7 and b=5
//! 
//! stuff a = STUFF1;
//! stuff b = STUFF2;
//! Xchg_swap(a,b);  // swapped
//! 
//! stuff* a = PSTUFF1;
//! stuff* b = PSTUFF2;
//! Xchg_swap(a,b);  // swapped
//! 
//! \endcode
template<typename T>
void Xchg_swap( T& a, T& b )
{
	T tmp = std::move( a );
	a = std::move( b );
	b = std::move( tmp );
}

//! \ingroup STL_Classes
//! \class Xchg_vector 
//! \brief This is a high level array class.
//!
//!   This class lets you use optimized arrays.
//! \remark Template classes don't need XCHG_API because they are instantiated in header files
template <typename T>
class Xchg_vector
{
	struct allocator_from_t
	{
		T * operator()( std::pair<T *, Xchg_Size_t> src, Xchg_Size_t n )
		{
			T * out = new T[ n ];
#if !defined( _MSC_VER ) || _MSC_VER >= 1700 // at least vs12
			do_move_if( src.first, src.first + src.second, out, std::integral_constant<bool, std::is_move_assignable<T>::value>() );
#else
			do_move_if( src.first, src.first + src.second, out, std::integral_constant<bool, false>() );
#endif
			return out;
		}
	private:
		void do_move_if( T * begin, T * end, T * dst, std::true_type )
		{
			for( ; begin != end; ( void )( ++begin ), ( void )( ++dst ) )
				*dst = std::move( *begin );
		}
		void do_move_if( T * begin, T * end, T * dst, std::false_type )
		{
			for( ; begin != end; ( void )( ++begin ), ( void )( ++dst ) )
				*dst = *begin;
		}
	};
private:
	T* v;
	Xchg_Size_t nb;
	Xchg_Size_t rsv;
	T* _xchg_doublerealloc(Xchg_Size_t s)
	{
#if 0
		T* ex = v;
		v=new T[s];
		XCHG_ASSERT(s>=nb);
		XCHG_ASSERT(v!=NULL);
		if (s >= nb)
		{
			for (Xchg_Size_t i = 0; i < nb; i++)
				v[i] = ex[i];
		}
		return ex;
#else
		XCHG_ASSERT( s >= nb );
		auto old = v;
		if( s >= nb )
			v = allocator_from_t()( std::make_pair( old, nb ), s );
		else
			v = new T[ s ];
		return old;
#endif
	}
private:
	/*template <typename comp>
	int _partition(int top, int bottom,const comp& C)
	{//http://mathbits.com/mathbits/compsci/Arrays/Quick.htm
		//T x = v[top];       //crash pour des tableaux de 30000+ elts
		assert((top + bottom) / 2 < (int) nb);
		T x = v[ (top + bottom) / 2]; 
		int i = top - 1;
		int j = bottom + 1;
		T temp;
		do
		{
			do      
			{
				j --;
				assert(j<(int)nb);
			}while (C(x,v[j]));//while (C(v[j],x));// //chngt car le tri etait inverse CT le 09-01-07

			do  
			{
				i++;
				assert(i<(int)nb);
			} while (C(v[i],x));//while (C(x,v[i]));// //chngt car le tri etait inverse CT le 09-01-07

			if (i < j)
			{ 
				assert(i<(int)nb);
				assert(j<(int)nb);
				temp = v[i];    // switch elements at positions i and j
				v[i] = v[j];
				v[j] = temp;
			}
		}while (i < j);   
		return j;           // returns middle index 
	}
	template <typename comp>
	void _quicksort(int top, int bottom,const comp& C)
	{
		if (top < bottom)
		{
			int middle = _partition(top, bottom,C);
			assert(middle<bottom);
			_quicksort(top, middle,C);   // sort top partition
			assert(middle+1>top);
			_quicksort(middle+1, bottom,C);    // sort bottom partition
		}
		return;
	}*/
	
	template <typename comp>
	void _mergesort(T* temp, int left, int right,const comp& C)
	{
		int i, j, k, mid = (left+right)/2;
		if (left == right) return;
		_mergesort(temp, left, mid,C); // la premiere moitie
		_mergesort(temp, mid+1, right,C);// Sort 2nd half
		// operation de fusion. Premierement, copier les deux moities dans temp.
		for (i=left; i<=mid; i++)
			temp[i] = v[i];
		for (j=1; j<=right-mid; j++)
			temp[right-j+1] = v[j+mid];
		// fusionner les deux moities dans array
		for (i=left,j=right,k=left; k<=right; k++)
			if (C(temp[i],temp[j]))
				v[k] = temp[i++];
			else 
				v[k] = temp[j--];
	}

public:
    typedef T value_type;
	typedef T * pointer;
	typedef T const * const_pointer;
	typedef pointer iterator;
	typedef const_pointer const_iterator;

	//! \brief Default constructor
	//! \remarks create an empty array
	Xchg_vector()
	{
		nb=0;
		rsv=0;
		v=NULL;
	}
	Xchg_vector(Xchg_Size_t initreservesize,int resize=0)
	{
		XCHG_ASSERT(initreservesize>0);  // forbidden : at least one element on reserve.
		rsv=initreservesize;
		nb=0;
		if (resize)
			nb = rsv;
		if (rsv>0)
			v=new T[rsv];
		else
			v=NULL;
	}
	~Xchg_vector() 
	{
		if (v!=NULL)
		{
			delete [] v;
			v=NULL;
		}
	}
	//! \brief Returns array of T elements
	//! \remarks You should call size () method to know returned array size.
	const T* GetArray () const
	{
		return this->v;
	}
	T * data() XCHG_NOEXCEPT { return v; }
	T const * data() const XCHG_NOEXCEPT { return v; }
	Xchg_vector(const Xchg_vector<T>& t)
	{
//#ifdef XCHG_DEBUG_STL_TRACE
//		FILE* F = fopen("/stltrace.txt","a");
//		fprintf(F,"Xchg_vector copy constructor size : %d ; elemsize : %d\n",t.size(),sizeof(T));
//		fclose(F);
//#endif
		nb=t.nb;
		rsv=t.nb; // shrink_to_fit while copy
		if (rsv>0)
		{
			XCHG_ASSERT(nb > 0);
			v=new T[nb];
			for(Xchg_Size_t i=0;i<nb;i++)
				v[i] = t.v[i];
		}
		else
			v=NULL;
	}
	Xchg_vector& operator=(const Xchg_vector<T>& t)
	{
		if (&t!=this)
		{
			if (v!=NULL)
			{
				XCHG_ASSERT(rsv>0);
				delete [] v;
			}
			v=NULL;
			nb=t.nb;
			rsv=t.nb;
			if (t.v!=NULL && nb > 0)
			{
				v=new T[nb];
				for(Xchg_Size_t i=0;i<nb;i++)
					v[i] = t.v[i];
			}
		} 
		return *this;
	}
	//! \brief Checks if two vectors are equal.
	//! \param[in] t: The vector to compare with.
	//! \return 1 if equal, 0 otherwise.
	//! \remarks Two vectors are equal if they have the same size and all elements are equal.
	int operator==(const Xchg_vector<T>& t) const
	{
		// Fast path: check size first
		if (size() != t.size())
			return 0;
		
		// Compare elements
		for(Xchg_Size_t i = 0; i < nb; i++)
		{
			if (!(v[i] == t.v[i]))
				return 0;
		}
		return 1;
	}

#ifndef XCHG_NO_CXX11_RVALUE_REFERENCES
	//! \brief Move constructor
	//! \param t the Xchg_vector to move
	Xchg_vector(Xchg_vector<T> && t) XCHG_NOEXCEPT : v(t.v), nb(t.nb), rsv(t.rsv)
	{
		t.v = 0;
		t.nb = 0;
		t.rsv = 0;
	}

	//! \brief Move assignment operator
	//! \param t the Xchg_vector to move
	Xchg_vector<T>& operator=(Xchg_vector<T> && t) XCHG_NOEXCEPT
	{
		if(this != &t)
		{
			delete [] v;
			v = t.v;
			nb = t.nb;
			rsv = t.rsv;
			t.v = 0;
			t.nb = 0;
			t.rsv = 0;
		}

		return *this;
	}
#endif

	int operator<(const Xchg_vector<T>& t) const
	{
		Xchg_Size_t i;
		if (size()<t.size())
			return 1;
		if (t.size()<size())
			return 0;
		for(i=0;i<size();i++)
		{
			if ((v[i]<t.v[i]))
				return 1;
			if ((t.v[i]<v[i]))
				return 0;
		}
		return 0;
	}
	//! \brief Resets the Xchg_vector content.
	//! \remarks If the Xchg_vector has pointers, theses ones aren't deleted.
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_vector<int> tab; //we construct an array of int.
	//! tab.push_back(0); //we insert '0'
	//! tab.push_back(2); //we insert '2'
	//! tab.push_back(1); //we insert '1'
	//! Now tab has 3 elements...
	//! tab.clear(); //Now tab hasn't any element.
	//! \endcode
	void clear(int no_delete=0) 
	{
		if (v==NULL)
			return;
		if (no_delete)
		{
			nb=0;
			return;
		}
		delete [] v;
		nb=0;
		rsv=0;
		v=NULL;
	}
	//! \brief Accesses the ith element - like a classic array -.
	//! \param[in] k : The element position.
	//! \warning There isn't any check for the element position validity !!!
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_vector<int> tab; //we construct an array of int.
	//! tab.push_back(0); //we insert '0'
	//! tab.push_back(2); //we insert '2'
	//! tab.push_back(1); //we insert '1'
	//! // tab is {0,2,1}
	//! int elt = tab[2]; //elt is equal to 1
	//! tab[1] = 5; // tab is {0,5,1}
	//! tab[5] = 5; // Error !!! The program will crash :(
	//! \endcode
	inline T& operator[](Xchg_Size_t k)
	{
#ifdef _DEBUG_DTK
		if (k>=nb)
			Xchg_ThrowOverflowException();
#endif
		return v[k];
	}
	inline const T& operator[](Xchg_Size_t k) const
	{
#ifdef _DEBUG_DTK
		if (k>=nb)
			Xchg_ThrowOverflowException();
#endif
		return v[k];
	}
	inline T& at(Xchg_Size_t k) 
	{
		return operator[](k);
	}
	inline const T& at(Xchg_Size_t k) const 
	{
		return operator[](k);
	} 
	//! \brief Inserts an element at the end of the array.
	//! \param[in] x: The element to insert.
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_vector<int> tab; //we construct an array of int.
	//! tab.push_back(0); //we insert '0'
	//! tab.push_back(2); //we insert '2'
	//! tab.push_back(1); //we insert '1'
	//! Now tab is {0,2,1}...
	//! \endcode
	void push_back(const T& x) 
	{
		if (nb==rsv)
		{
			if (nb==0)
			{
				rsv=1;
				v = new T[1];
				v[nb++]=x;
			}
			else
			{
				rsv*=2;
				// Save x before realloc in case x references an element in the current array
				T to_add(x);  // Copy construct (more efficient than default construct + assign)
				T* ex = _xchg_doublerealloc(rsv);
				v[ nb++ ] = std::move(to_add);  // Move the saved copy
				delete [] ex;
			}
		}
		else
			v[nb++]=x;
	}

#ifndef XCHG_NO_CXX11_RVALUE_REFERENCES
	//! \brief Inserts an element by moving it at the end of the array.
	//! \param[in] x: The element to insert.
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_vector<int> tab; //we construct an array of int.
	//! tab.push_back(0); //we insert '0'
	//! tab.push_back(2); //we insert '2'
	//! tab.push_back(1); //we insert '1'
	//! Now tab is {0,2,1}...
	//! \endcode
	void push_back(T&& x) 
	{
		if (nb==rsv)
		{
			if (nb==0)
			{
				rsv=1;
				v = new T[1];
				v[nb++]=std::move(x);
			}
			else
			{
				rsv*=2;
				// Move x into a temporary to ensure safety
				T to_add(std::move(x));  // Move construct
				T* ex = _xchg_doublerealloc(rsv);
				v[nb++] = std::move(to_add);  // Move into new array
				delete [] ex;
			}
		}
		else
			v[nb++]=std::move(x);
	}

	//! \brief Constructs an element in-place at the end of the array.
	//! \param[in] args: Arguments to forward to the constructor of T.
	//! \remarks Constructs the element directly, reducing temporary object creation.
	//!
	//! \b Sample:
	//! \code 
	//! struct Point { int x, y; Point(int a, int b) : x(a), y(b) {} };
	//! Xchg_vector<Point> vec;
	//! vec.emplace_back(10, 20); // Constructs Point(10, 20) and moves it into the vector
	//! \endcode
	template<typename... Args>
	void emplace_back(Args&&... args)
	{
		// Construct temporary and move it in - safer with current array allocation strategy
		push_back(T(std::forward<Args>(args)...));
	}
#endif

	//! \brief Removes the last element.
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_vector<int> tab; //we construct an array of int.
	//! tab.push_back(0); //we insert '0'
	//! tab.push_back(2); //we insert '2'
	//! tab.push_back(1); //we insert '1'
	//! Now tab is {0,2,1}...
	//! \endcode
	void pop_back()
	{
		XCHG_ASSERT(nb!=0);
		if( nb == 0 ) {	Xchg_ThrowOverflowException(); }
		nb--;
	}  // extent : liberation a 1/3.

	//! \brief Returns the size of the array.
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_vector<int> tab; //we construct an array of int.
	//! tab.push_back(0); //we insert '0'
	//! tab.push_back(2); //we insert '2'
	//! tab.push_back(1); //we insert '1'
	//! int size = tab.size(); //size is equal to 3
	//! \endcode
	Xchg_Size_t size() const XCHG_NOEXCEPT
	{
		return nb;
	}

	//! \brief Checks whether the container is empty.
	//! \return true if the container is empty, false otherwise.
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_vector<int> tab;
	//! if (tab.empty()) {
	//!     std::cout << "Vector is empty" << std::endl;
	//! }
	//! tab.push_back(1);
	//! if (!tab.empty()) {
	//!     std::cout << "Vector has " << tab.size() << " elements" << std::endl;
	//! }
	//! \endcode
	bool empty() const XCHG_NOEXCEPT
	{
		return nb == 0;
	}

	//! \brief Returns the size of the storage space currently allocated for the array, expressed in terms of elements.
	//!
	//! \b This capacity is not necessarily equal to the size. It can be equal or greater.
	//! \b The capacity of the Xchg_vector can be explicitly altered by calling member Xchg_vector::reserve.
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_vector<int> tab; //we construct an array of int.
	//! tab.reserve(5); //we reserve 5 element of type int.
	//! tab.push_back(0); //we insert '0'
	//! tab.push_back(2); //we insert '2'
	//! tab.push_back(1); //we insert '1'
	//! Xchg_Size_t size = tab.size(); //size is equal to 3
	//! Xchg_Size_t capacity = tab.capacity(); //capacity is equal to 5
	//! \endcode
	Xchg_Size_t capacity() const XCHG_NOEXCEPT
	{
		return rsv;
	}

	//! \brief Return the elements at the end of the array
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_vector<int> L; //we construct an array of int.
	//! L.push_back(5);
	//! L.push_back(35);
	//! L.push_back(8);  // array is now : 5 - 35 - 8
	//! int a = L.back();   // a is 8
	//! \endcode
	T& back() XCHG_NOEXCEPT
	{
		XCHG_ASSERT(nb!=0);
		return v[nb - 1];
	}
	//! \brief Return the elements at the end of the array
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_vector<int> L; //we construct an array of int.
	//! L.push_back(5);
	//! L.push_back(35);
	//! L.push_back(8);  // array is now : 5 - 35 - 8
	//! int a = L.back();   // a is 8
	//! \endcode
	const T& back() const XCHG_NOEXCEPT
	{
		XCHG_ASSERT(nb!=0);
		return v[nb - 1];
	}
	//! \brief Return the elements at the beginning of the array
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_vector<int> L; //we construct a list of array.
	//! L.push_back(5);
	//! L.push_back(35);
	//! L.push_back(8);  // array is now : 5 - 35 - 8
	//! int a = L.front();   // a is 5
	//! \endcode
	T& front() XCHG_NOEXCEPT
	{
		XCHG_ASSERT(nb!=0);
		return v[0];
	}
	//! \brief Return the elements at the beginning of the array
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_vector<int> L; //we construct a list of array.
	//! L.push_back(5);
	//! L.push_back(35);
	//! L.push_back(8);  // array is now : 5 - 35 - 8
	//! int a = L.front();   // a is 5
	//! \endcode
	const T& front() const XCHG_NOEXCEPT
	{
		XCHG_ASSERT(nb!=0);
		return v[0];
	}

	//! \brief Resizes the array.
	//! \param[in] n: The new size.
	//! \param[in] t: The filling element.
	//! \remarks If the new size is lower than the old one, the last elements are removed.
	//! \remarks If the new size is greater than the old one, inserts t in the new elements.
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_vector<int> tab; //we construct an array of int.
	//! tab.push_back(0); //we insert '0'
	//! tab.push_back(2); //we insert '2'
	//! tab.push_back(1); //we insert '1'
	//! tab.resize(2); //Now tab is {0,2}
	//! tab.resize(5,6); //Now tab is {0,2,6,6,6}
	//! \endcode
	void resize(Xchg_Size_t n, const T& t) 
	{
		if (n<=nb) 
		{
			nb=n;
			return;
		} 
		if (n<=0) 
		{
			clear();
			return;
		}
		
		// Check if we can use existing capacity
		if (n <= rsv)
		{
			// No reallocation needed, just fill new elements
			for(Xchg_Size_t i=nb; i<n; i++) 
				v[i]=t;
			nb = n;
			return;
		}
		
		// Need to reallocate
		if (v==NULL)
		{
			XCHG_ASSERT(rsv==0 && n>0);
			v = new T[n];
		}
		else
			delete [] _xchg_doublerealloc(n);
		
		// Fill new elements
		for(Xchg_Size_t i=nb; i<n; i++) 
			v[i]=t;
		nb=rsv=n;
	}
	void resize(Xchg_Size_t n) 
	{
		if (n<=nb) 
		{
			nb=n;
			return;
		} 
		if (n<=0) 
		{
			clear();
			return;
		}
		
		// Use existing capacity if available
		if (n <= rsv)
		{
			// No reallocation needed, elements already default-constructed
			nb = n;
			return;
		}
		
		// Need to reallocate
		if (v==NULL)
		{
			XCHG_ASSERT(rsv==0);
			v = new T[n];
		}
		else
			delete [] _xchg_doublerealloc(n);

		nb=rsv=n;
	}
	//! \brief Sorts the array.
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_vector<int> tab; //we construct an array of int.
	//! tab.push_back(0); //we insert '0'
	//! tab.push_back(2); //we insert '2'
	//! tab.push_back(1); //we insert '1'
	//! //tab is {0,2,1}
	//! tab.sort();
	//! //tab is {0,1,2}
	//! \endcode
	void swap(const Xchg_Size_t inA, const Xchg_Size_t inB) 
	{
#ifdef _DEBUG_DTK
		if ( inA >= nb || inB >= nb )
			Xchg_ThrowOverflowException();
#endif
		Xchg_swap(v[inA], v[inB]);
	}

	//! \brief Removes the first element with the value a
	//! \param[in] a: The value of the element that should be removed
	//! \remarks : The order of the elements of the tab won't be conserved, the last element is moved to the old position of the removed element
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_vector<int> tab; //we construct an array of int.
	//! tab.push_back(0); //we insert '0'
	//! tab.push_back(1); //we insert '1'
	//! tab.push_back(2); //we insert '2'
	//! tab.push_back(3); //we insert '3'
	//! tab.remove(1); //Now tab is {0,3,2}
	//! tab.remove(2); //Now tab is {0,3}
	//! \endcode
	void remove( const T& a)
	{
		int pos = find( a );
		if( pos >= 0)
		{
            swap( pos, nb - 1 );
			pop_back();
		}
	}
	//! \brief Swap two elements.
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_vector<int> tab; //we construct an array of int.
	//! tab.push_back(0); //we insert '0'
	//! tab.push_back(2); //we insert '2'
	//! tab.push_back(1); //we insert '1'
	//! //tab is {0,2,1}
	//! tab.swap(tab[0],tab[1]);
	//! //tab is {2,0,1}
	//! \endcode
	void sort()
	{
		sort(Xchg_less<T>());
	}
	//! \brief Sorts the array with custom sorting.
	//! \param[in] C: Comparison Functor
	//! \remarks Comparison functor need to be instanciated with default : use (). See sample.
	//! \remarks Use a custom functor or one of the two internal functors available : Xchg_less (default) and Xchg_greater
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_vector<int> tab; //we construct an array of int.
	//! tab.push_back(0); //we insert '0'
	//! tab.push_back(2); //we insert '2'
	//! tab.push_back(1); //we insert '1'
	//! //tab is {0,2,1}
	//! tab.sort(Xchg_greater<int>());
	//! //tab is {2,1,0}
	//! \endcode
	template <typename comp>
	void sort(const comp& C)
	{
		if(nb > 0 )
		{
			//_quicksort((int)0,(int)(nb-1),C);
			T* tmp = new T[nb];
			_mergesort(tmp,(int)0,(int)(nb-1),C);
			delete [] tmp;
		}
	}

	int find(const T& e) const
	{
		for(Xchg_Size_t i=0;i<nb;i++) 
			if (v[i]==e) 
				return (int)i;
		return -1;
	}
	template <typename comp>
	int find(const T& e,const comp& C) const
	{
		for(Xchg_Size_t i=0;i<nb;i++) 
			if (!C(v[i],e) && !C(e,v[i])) 
				return (int)i;
		return -1;
	}
	//Xchg_vector& operator+=(const Xchg_vector& T) {V.resize(V.size()+T.V.size());V.insert(V.end(),T.V.begin(),T.V.end());return *this;}
	Xchg_vector& operator+=(const Xchg_vector& t) 
	{
		if (t.nb==0) // empty 2nd
			return *this;
		
		Xchg_Size_t new_size = nb + t.nb;
		
		// Only reallocate if current capacity is insufficient
		if (new_size > rsv)
		{
			rsv = new_size;
			if (v==NULL)
				v = new T[rsv];
			else
				delete [] _xchg_doublerealloc(rsv);
		}
		
		// Copy elements from t
		for(Xchg_Size_t i=0;i<t.nb;i++) 
			v[i+nb] = t.v[i];
		nb = new_size;
		return *this;
	}

	Xchg_vector& operator+=( Xchg_vector&& t )
	{
		if( t.nb == 0 )
			return *this;
		
		Xchg_Size_t new_size = nb + t.nb;
		
		// Only reallocate if current capacity is insufficient
		if( new_size > rsv )
		{
			rsv = new_size;
			if( v == nullptr )
				v = new T[ rsv ];
			else
				delete[] _xchg_doublerealloc( rsv );
		}
		
		// Move elements from t
		for( Xchg_Size_t i = 0; i < t.nb; i++ )
			v[ i + nb ] = std::move( t.v[ i ] );
		nb = new_size;
		return *this;
	}

	Xchg_vector subtab(Xchg_Size_t a,Xchg_Size_t b) 
	{
		XCHG_ASSERT(a<nb && b<nb && b>=a);
		Xchg_vector<T> res;
		res.resize(b-a+1);
		for(Xchg_Size_t i=0;i<res.rsv;i++) 
			res.v[i] = v[a+i];
		return res;
	}
	//! \brief Requests that the vector capacity be at least enough to contain n elements.
	//! \param[in] n: Minimum capacity for the vector.
	//! \remarks If n is greater than the current capacity, the function causes the container to reallocate.
	//! \remarks If n is less than or equal to the current capacity, the function does nothing.
	//! \remarks This function has no effect on the vector size and cannot alter its elements.
	//!
	//! \b Performance note:
	//! \b - If n <= capacity(): Zero cost, no allocation
	//! \b - If n > capacity(): Reallocation cost O(size())
	void reserve(Xchg_Size_t n) 
	{
		// If requested capacity is already available, do nothing
		if (n<=rsv) 
			return; 
		
		// First allocation
		if (v==NULL)
		{
			XCHG_ASSERT(rsv==0);
			XCHG_ASSERT(nb==0);
			v = new T[n];
			rsv = n;
			return;
		}
		
		// Reallocate to larger capacity
		rsv = n;
		delete [] _xchg_doublerealloc(rsv);
	}
	T min_element() const
	{
		if (nb==0) 
			return T();
		T min = v[0];
		for(Xchg_Size_t i=0;i<nb;i++) 
			if (v[i]<min) 
				min=v[i];
		return min;
	}
	T max_element() const
	{
		if (nb==0) 
			return T();
		T max = v[0];
		for(Xchg_Size_t i=0;i<nb;i++) 
			if (v[i]>max) 
				max=v[i];
		return max;
	}
	Xchg_Size_t min_ithelement()
	{
		if (nb==0) 
			return 0;
		Xchg_Size_t imin = 0;
		T min = v[0];
		for(Xchg_Size_t i=0;i<nb;i++) 
			if (v[i]<min) 
			{
				min=v[i];
				imin = i;
			}
			return imin;
	}
	Xchg_Size_t max_ithelement()
	{
		if (nb==0) 
			return 0;
		Xchg_Size_t imax = 0;
		T max = v[0];
		for(Xchg_Size_t i=0;i<nb;i++) 
			if (v[i]>max) 
			{
				max=v[i];
				imax = i;
			}
			return imax;
	}

	friend std::ostream& operator<<(std::ostream& o,const Xchg_vector& d)
	{
		o<<"<Tab><Size>"<<d.size()<<"</Size>"<<std::endl;
		Xchg_Size_t i;
		for(i=0;i<d.size();i++)
		{
			o<<"<Elem_"<<i<<">"<< d[i] <<"</Elem_"<<i<<">"<<std::endl;
		}
		o << "</Tab>"<<std::endl;
		return o;
	}
	Xchg_Size_t GetSize() const
	{
		return 2*sizeof(Xchg_Size_t) + sizeof(T*) + rsv * sizeof(T);
	}
	void reduce()
	{
		if (rsv>nb)
		{
			rsv = nb;
			if (rsv==0)
				rsv = 1;
			delete [] _xchg_doublerealloc(rsv);
		}
	}

	//! \brief Requests the removal of unused capacity.
	//! \remarks This is a non-binding request to reduce capacity() to size().
	//! \remarks It may cause reallocation and invalidate iterators.
	//! \remarks If size() is 0, capacity becomes 0 and memory is freed.
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_vector<int> vec;
	//! vec.reserve(10000);      // capacity = 10000
	//! vec.push_back(1);
	//! vec.push_back(2);        // size = 2, capacity = 10000
	//! vec.shrink_to_fit();     // size = 2, capacity = 2 (or 0 if empty)
	//! // Unused capacity is released
	//! \endcode
	void shrink_to_fit()
	{
		if (rsv <= nb)
			return;  // Already tight or empty
		
		if (nb == 0)
		{
			// Free all memory
			if (v != NULL)
			{
				delete [] v;
				v = NULL;
			}
			rsv = 0;
			return;
		}
		
		// Reallocate to exact size
		T* old_v = v;
		rsv = nb;
		v = new T[rsv];
		
		// Move elements to new array
#if !defined( _MSC_VER ) || _MSC_VER >= 1700
		for(Xchg_Size_t i = 0; i < nb; i++)
			v[i] = std::move(old_v[i]);
#else
		for(Xchg_Size_t i = 0; i < nb; i++)
			v[i] = old_v[i];
#endif
		
		delete [] old_v;
	}
	void reverse(){Xchg_Size_t i;for(i=0;i<nb/2;i++) Xchg_swap(v[i],v[nb-i-1]);}

	iterator begin() XCHG_NOEXCEPT
	{
		return v;
	}
	const_iterator begin() const XCHG_NOEXCEPT
	{
		return v;
	}
	const_iterator cbegin() const XCHG_NOEXCEPT
	{
		return v;
	}
	iterator end() XCHG_NOEXCEPT
	{
		return v + nb;
	}
	const_iterator end() const XCHG_NOEXCEPT
	{
		return v + nb;
	}
	const_iterator cend() const XCHG_NOEXCEPT
	{
		return v + nb;
	}
};



template <typename T1,typename T2>
class XCHG_API Xchg_pair
{
public:
   T1 first;
   T2 second;
   Xchg_pair() {}
   Xchg_pair(const T1& a,const T2& b) {first = a; second = b;}

   template <typename Arg, typename Arg2> Xchg_pair( Arg&& a, Arg2&& b ) : first( std::forward<Arg>( a ) ), second( std::forward<Arg2>( b ) )
   {
   }

   template <typename Pair> Xchg_pair( Pair&& p )
	: first(std::forward<Pair>(p).first), second(std::forward<Pair>(p).second)
   {
   }

   friend std::ostream& operator<<(std::ostream& o,const Xchg_pair& d)
   {
      o<<"<Pair><First>" << d.first <<"</First><Second>" << d.second << "</Second></Pair>" <<std::endl;
      return o;
   }
   int operator<(const Xchg_pair<T1,T2>& t) const
   {
		if (first<t.first)  
			return 1;
		if (first>t.first)  
			return 0;
		return second<t.second;
     }
   int operator==(const Xchg_pair<T1,T2>& t) const
   {
	   return (first == t.first) && (second == t.second);
   }
};


//! \brief Constructs a pair object with its first element set to x and its second element set to y.
//! \param x first element
//! \param y second element
//! \remarks The template types can be implicitly deduced from the arguments passed to make_Dtkpair.
//! \remarks pair objects can be constructed from other pair objects containing different types, if the respective types are implicitly convertible.
//!
//! \b Sample:
//! \code 
//! Xchg_pair <int,int> foo;
//! Xchg_pair <int,int> bar;
//! 
//! foo = make_Dtkpair (10,20);
//! bar = make_Dtkpair (10.5,'A'); // ok: implicit conversion from pair<double,char>
//! 
//! output:
//! foo: 10, 20
//! bar: 10, 65
//! 
//! \endcode
template <typename T1, typename T2>
Xchg_pair<T1, T2> make_Dtkpair( T1&& x, T2&& y )
{
	return Xchg_pair<T1, T2>( std::forward<T1>( x ), std::forward<T2>( y ) );
}




namespace dtk
{
	//! \brief Test condition on all elements in range, and returns true if pred returns true for all the elements in the range [first,last] or if the range is empty, and false otherwise.
	//! \param first Input iterator to the initial position in a sequence
	//! \param last Input iterator to the final position in a sequence
	//! \param Pred Unary function that accepts an element in the range as argument and returns a value convertible to bool. 
	//! \remarks Note that invalid parameters cause undefined behavior.
	template<class InputIterator, class Predicate>
	bool all_of(InputIterator first, InputIterator last, Predicate Pred)
	{	// test if all elements satisfy _Pred
		for (; first != last; ++first)
			if (!Pred(*first))
				return false;
		return true;
	}

	//! \brief Test condition on all elements in range, and returns true if pred returns true for all the elements in the range [first,last] or if the range is empty, and false otherwise.
	//! \param first Input iterator to the initial position in a sequence
	//! \param last Input iterator to the final position in a sequence
	//! \param Pred Unary function that accepts an element in the range as argument and returns a value convertible to bool. 
	//! \remarks Note that invalid parameters cause undefined behavior.
	//! \remarks Specialized version for Xchg_vector.
	template<typename T, class Predicate>
	bool all_of(const Xchg_vector<T> & tab, Predicate Pred)
	{
		for(Xchg_Size_t i = 0; i < tab.size(); ++i)
			if(!Pred(tab[i]))
				return false;
		return true;
	}

	//! \brief Test if any element in range fulfills condition, returns true if pred returns true for any of the elements in the range [first,last], and false otherwise or if the range is empty.
	//! \param first Input iterator to the initial position in a sequence
	//! \param last Input iterator to the final position in a sequence
	//! \param Pred Unary function that accepts an element in the range as argument and returns a value convertible to bool. 
	//! \remarks Note that invalid parameters cause undefined behavior. 
	template<class InputIterator, class Predicate>
	bool any_of(InputIterator first, InputIterator last, Predicate Pred)
	{
		for(; first != last; ++first)
			if(Pred(*first))
				return true;
		return false;
	}

	//! \brief Test if any element in range fulfills condition, returns true if pred returns true for any of the elements in the range [first,last], and false otherwise or if the range is empty.
	//! \param first Input iterator to the initial position in a sequence
	//! \param last Input iterator to the final position in a sequence
	//! \param Pred Unary function that accepts an element in the range as argument and returns a value convertible to bool. 
	//! \remarks Note that invalid parameters cause undefined behavior.
	//! \remarks Specialized version for Xchg_vector.
	template<typename T, class Predicate>
	bool any_of(const Xchg_vector<T> & tab, Predicate Pred)
	{
		for(Xchg_Size_t i = 0; i < tab.size(); ++i)
			if(Pred(tab[i]))
				return true;
		return false;
	}

	//! \brief Test if no elements fulfill condition, returns true if pred returns false for all the elements in the range [first,last] or if the range is empty, and false otherwise.
	//! \param first Input iterator to the initial position in a sequence
	//! \param last Input iterator to the final position in a sequence
	//! \param Pred Unary function that accepts an element in the range as argument and returns a value convertible to bool. 
	//! \remarks Note that invalid parameters cause undefined behavior.
	template<class InputIterator, class Predicate>
	bool none_of(InputIterator first, InputIterator last, Predicate Pred)
	{
		for (; first != last; ++first)
			if (Pred(*first))
				return false;
		return true;
	}

	//! \brief Test if no elements fulfill condition, returns true if pred returns false for all the elements in the range [first,last] or if the range is empty, and false otherwise.
	//! \param first Input iterator to the initial position in a sequence
	//! \param last Input iterator to the final position in a sequence
	//! \param Pred Unary function that accepts an element in the range as argument and returns a value convertible to bool. 
	//! \remarks Note that invalid parameters cause undefined behavior.
	//! \remarks Specialized version for Xchg_vector.
	template<typename T, class Predicate>
	bool none_of(const Xchg_vector<T> & tab, Predicate Pred)
	{
		for(Xchg_Size_t i = 0; i < tab.size(); ++i)
			if(Pred(tab[i]))
				return false;
		return true;
	}
}  // end of namespace dtk



#endif  // of ifndef UTIL_STL_XCHG_HPP_
