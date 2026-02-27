#ifndef __XCHG_VAL_HPP__
#define __XCHG_VAL_HPP__

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_dir.hpp"
#include "base/xchg_pnt.hpp"
#include "base/xchg_rgb.hpp"
#include "base/xchg_status.hpp"
#include "base/xchg_string.hpp"
#include "base/xchg_uuid.hpp"
#include "base/xchg_vector.hpp"
#include <iosfwd>
#include <type_traits>
#include <utility>

enum Xchg_val_type{
    XCHG_VAL_TYPE_UNKNOWN = 0,
    XCHG_VAL_TYPE_INT,
    XCHG_VAL_TYPE_CHAR,
    XCHG_VAL_TYPE_DOUBLE,
    XCHG_VAL_TYPE_STRING,
    XCHG_VAL_TYPE_RGB,
    XCHG_VAL_TYPE_PNT,
    XCHG_VAL_TYPE_DIR,
    XCHG_VAL_TYPE_UUID,
	XCHG_VAL_TYPE_INTARRAY,
	XCHG_VAL_TYPE_BUFFERDATA
};

namespace Xchg_Detail_Val
{
	template <Xchg_val_type v, bool trivial = false> struct Value : std::integral_constant<Xchg_val_type, v> { static bool const is_trivial = trivial; };

	template <typename T = void> struct ValueFor;
	template <> struct ValueFor<Xchg_Char8> : Value<XCHG_VAL_TYPE_CHAR, true> {};
	template <> struct ValueFor<Xchg_Int32> : Value<XCHG_VAL_TYPE_INT, true> {};
	template <> struct ValueFor<Xchg_Double64> : Value<XCHG_VAL_TYPE_DOUBLE, true> {};
	template <> struct ValueFor<Xchg_RGB> : Value<XCHG_VAL_TYPE_RGB> {};
	template <> struct ValueFor<Xchg_pnt> : Value<XCHG_VAL_TYPE_PNT> {};
	template <> struct ValueFor<Xchg_dir> : Value<XCHG_VAL_TYPE_DIR> {};
	template <> struct ValueFor<Xchg_UUID> : Value<XCHG_VAL_TYPE_UUID> {};
	template <> struct ValueFor<Xchg_string> : Value<XCHG_VAL_TYPE_STRING> {};
	template <> struct ValueFor<Xchg_vector<Xchg_Int32>> : Value<XCHG_VAL_TYPE_INTARRAY> {};
	template <> struct ValueFor<Xchg_vector<Xchg_Char8>> : Value<XCHG_VAL_TYPE_BUFFERDATA> {};

	template <typename T> struct Binder : ValueFor<T>
	{
		static T * GetPointer( void * u ) 
		{ 
			return GetPointer( u, std::integral_constant<bool, Binder::is_trivial>() ); 
		}
		static T const * GetPointer( void const * u ) 
		{ 
			return GetPointer( u, std::integral_constant<bool, Binder::is_trivial>() ); 
		}
	private:
		static T * GetPointer( void * u, std::integral_constant<bool, true> ) { return static_cast< T * >( u ); }
		static T * GetPointer( void * u, std::integral_constant<bool, false> ) { return *static_cast< T ** >( u ); }
		static T const * GetPointer( void const * u, std::integral_constant<bool, true> ) { return static_cast< T const * >( u ); }
		static T const * GetPointer( void const * u, std::integral_constant<bool, false> ) { return *static_cast< T * const * >( u ); }
	};
}

class XCHG_API Xchg_Val
{
protected: 
    union {
        Int32 int_val;
        Char8 char_val;
        Double64 double_val;
        Xchg_string * string_val;
        Xchg_RGB * rgb_val;
        Xchg_pnt * pnt_val;
        Xchg_dir * dir_val;
        Xchg_UUID * uuid_val;
        Xchg_vector< Int32 > * tab_val;
		Xchg_vector<Char8> * buffer_val;
    }_val;
    enum Xchg_val_type _type;
    void _init() XCHG_NOEXCEPT;
    void _reset() XCHG_NOEXCEPT;
    void _copy(const Xchg_Val&s);
	void _move( Xchg_Val && ) XCHG_NOEXCEPT;
public:
    Xchg_Val();
    Xchg_Val(const Xchg_Int32 val);
    Xchg_Val(const Xchg_Char8 val);
    Xchg_Val(const Xchg_Double64 val);
	Xchg_Val( const Xchg_Val& val );
	Xchg_Val( Xchg_Val && ) XCHG_NOEXCEPT;
    Xchg_Val(const Xchg_string& val);
	Xchg_Val( Xchg_string && );
    Xchg_Val(const Xchg_RGB & val);
    Xchg_Val(const Xchg_pnt & val);
    Xchg_Val(const Xchg_dir & val);
	Xchg_Val(const Xchg_UUID & val);
	Xchg_Val(const Xchg_vector< Xchg_Int32 > & val);
	Xchg_Val( Xchg_vector<Xchg_Int32> && );
	Xchg_Val( Xchg_vector<Xchg_Char8> bd );
    Xchg_Val& operator = (const Xchg_Val& s);
	Xchg_Val & operator=( Xchg_Val && ) XCHG_NOEXCEPT;
public:
    int         GetInt(Xchg_status & st = Xchg_status::GetDefaultStatus()) const;
    char        GetChar(Xchg_status & st = Xchg_status::GetDefaultStatus()) const;
    double      GetDouble(Xchg_status & st = Xchg_status::GetDefaultStatus()) const;
    Xchg_string  GetString(Xchg_status & st = Xchg_status::GetDefaultStatus()) const;
    Xchg_RGB     GetRgb(Xchg_status & st = Xchg_status::GetDefaultStatus()) const;
    Xchg_pnt     GetPnt(Xchg_status & st = Xchg_status::GetDefaultStatus()) const;
    Xchg_dir     GetDir(Xchg_status & st = Xchg_status::GetDefaultStatus()) const;
	Xchg_UUID    GetUuid(Xchg_status & st = Xchg_status::GetDefaultStatus()) const;
	Xchg_vector< Xchg_Int32 >  GetIntArray(Xchg_status & st = Xchg_status::GetDefaultStatus()) const;
	Xchg_vector<Xchg_Char8> GetBufferData( Xchg_status & st = Xchg_status::GetDefaultStatus() ) const;
public:
	template <typename T> std::pair<bool, T> Get() const
	{
		typedef Xchg_Detail_Val::Binder<T> Binded;
		return ( _type == Binded::value )
			? std::make_pair( true, *Binded::GetPointer( &_val ) )
			: std::make_pair( false, T() );
	}
	template <typename T> T * GetIf()
	{
		typedef Xchg_Detail_Val::Binder<T> Binded;
		return ( _type == Binded::value ) ? Binded::GetPointer( &_val ) : nullptr;
	}
	template <typename T> T const * GetIf() const
	{
		typedef Xchg_Detail_Val::Binder<T> Binded;
		return ( _type == Binded::value ) ? Binded::GetPointer( &_val ) : nullptr;
	}
public:
    Xchg_val_type GetValType() const;
    
    // Comparison operators
    bool operator==(const Xchg_Val& other) const;
    bool operator!=(const Xchg_Val& other) const { return !(*this == other); }
public:
    ~Xchg_Val();
    friend std::ostream& operator<<(std::ostream& o,const Xchg_Val& d);
};




#endif //#ifndef __XCHG_VAL_HPP__
