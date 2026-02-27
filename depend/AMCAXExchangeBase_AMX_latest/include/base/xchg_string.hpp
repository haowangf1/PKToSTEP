
#ifndef __XCHG_STRING_HPP__
#define __XCHG_STRING_HPP__

#ifdef _MSC_VER
#	pragma warning(disable:4786)
#	pragma warning(disable:4800)
#endif

#include "base/xchg_export.hpp"
#include <math.h>
#include "base/xchg_type.hpp"
#include "base/xchg_vector.hpp"

#include <cstdio>
#include <iosfwd>
#include <string>



#ifdef _MSC_VER
#	if (_MSC_VER > 1200)
#		define Xchg_defined__wchar_t
#	endif
#endif
class Xchg_status;
#if ( defined(Xchg_defined__wchar_t) && defined(_NATIVE_WCHAR_T_DEFINED) )
//#pragma message("_NATIVE_WCHAR_T_DEFINED")
#	define w_str _w_str_w
#else
#	define w_str _w_str_us
#endif

#define XCHG_A Xchg_string(L"a")
#define XCHG_AB Xchg_string(L"ab")
#define XCHG_RB Xchg_string(L"rb")
#define XCHG_RBP Xchg_string(L"rb+")
#define XCHG_R Xchg_string(L"r")
#define XCHG_W Xchg_string(L"w")
#define XCHG_WP Xchg_string(L"w+")
#define XCHG_WB Xchg_string(L"wb")
#define XCHG_WBP Xchg_string(L"wb+")
#define XCHG_RW Xchg_string(L"rw")

//! \ingroup base_types
//! \class Xchg_string
//! \brief This is a high level string class.
//!
//!    This class lets you use ASCII or UNICODE strings.
//! It also has some File fonctions


class XCHG_API Xchg_string
{
public:
	typedef size_t Size_t;
protected:
	//! \brief Internal representation of the string
	Xchg_WChar_t					*	m_Str;
	//! \brief compatibility ASCII representation. WARNING this field is not always up to date.
	mutable char				*	m_CStr;
	//! \brief size of m_Str buffer
	//This fields holds in memory the value of wcslen(m_Str). Caution : it may not be updated in case of user manipulation, if so, the Xchg_string is flagged as dirty (see isDirty()).
	//Due to the use of the last byte to hold this flag, any Xchg_string of size > 2^8 will wrongly be considered dirty
	mutable Size_t					m_StrSize;
	//! \brief size of m_CStr buffer
	mutable Size_t					m_CStrSize;
private:
	//! \brief Init
	void init();
	void UpdateLen() const;
protected:
	void _FillFromCharBuffer( const char* inBuffer, const Xchg_Size_t inSize = ( Xchg_Size_t )-1 );

public:
	//! \brief Default destructors
	~Xchg_string();
	//! \brief Default constructor
	Xchg_string();
	//! \brief constructor from a char * (ASCII string)
	//! \param s the ASCII string to convert
	Xchg_string( const char *s );
	Xchg_string( const char* s, const Xchg_Size_t& inCount );
	//! \brief copy constructor
	//! \param s the Xchg_string to copy

	Xchg_string( const Xchg_string & );

#ifndef XCHG_NO_CXX11_RVALUE_REFERENCES
	//! \brief Move constructor
	//! \param s the Xchg_string to move
	Xchg_string( Xchg_string && s ) XCHG_NOEXCEPT : m_Str( s.m_Str ), m_CStr( s.m_CStr ), m_StrSize( s.m_StrSize ), m_CStrSize( s.m_CStrSize )
	{
		s.m_Str = 0;
		s.m_CStr = 0;
		s.m_StrSize = 0;
		s.m_CStrSize = 0;
	}

	//! \brief Move assignment operator
	//! \param s the Xchg_string to move
	Xchg_string& operator=( Xchg_string && s ) XCHG_NOEXCEPT
	{
		if( this != &s )
		{
			delete[] m_Str;
			delete[] m_CStr;
			m_Str = s.m_Str;
			m_CStr = s.m_CStr;
			m_StrSize = s.m_StrSize;
			m_CStrSize = s.m_CStrSize;

			s.m_Str = 0;
			s.m_CStr = 0;
			s.m_StrSize = 0;
			s.m_CStrSize = 0;
		}

		return *this;
	}
#endif
	//Xchg_string(const int);
	//! \brief constructor from a double
	//! \param dbl_val the double value to convert
	Xchg_string( const double dbl_val );
	Xchg_string( const double dbl_val, Xchg_Int32 inWithMorePrecision );
	//! \brief constructor from a float
	//! \param float_val the float value to convert
	Xchg_string( const float float_val );
	void RawCopyFromASCII( const char * inStrToBeCopied, const Xchg_Size_t inCount = -1 );
	/*************************************/
	//! \brief Retrieve the ASCII conversion string.
	//! \return the ASCII string
	//! \warning You can't modify the returned string !!
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string res;
	//! char * tmp_char;
	//! tmp_char = res.c_str(); //you can't modify the tmp_char value
	//! \endcode
	const char * c_str() const;

	//! \brief Split a Xchg_string into an array of Xchg_string giving a char array
	//! \param inDelimiters the delimiters string. This string contains each characters used to split the string
	//! \param outResults the result array
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string StringToBeSplitted = L"String To Be-Splitted";
	//! Xchg_string Delimiters = L" -";
	//! Xchg_vector< Xchg_string > Result;
	//! StringToBeSplitted.Split( Delimiters, Result);
	//! //Now Result contains 4 elements
	//! //Result.at(0) is equal to L"String"
	//! //Result.at(1) is equal to L"To"
	//! //Result.at(2) is equal to L"Be"
	//! //Result.at(3) is equal to L"Splitted"
	//!
	//! \endcode
	void Split( const Xchg_string& inDelimiters, Xchg_vector<Xchg_string>& outResults ) const;
	void Split( Xchg_WChar_t * inDelimiters, Xchg_vector<Xchg_string>& outResults ) const;

	//! \brief Retrieve the UNICODE string
	//! \return the UNICODE string
	//! \warning You can't modify the returned string !!
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string res;
	//! wchar_t * tmp_char;
	//! tmp_char = res.w_str(); //you can't modify the tmp_char value
	//! \endcode
#if ( defined(Xchg_defined__wchar_t))
	const unsigned short * _w_str_us() const;
	const __wchar_t * _w_str_w() const;
	//! \brief constructor from a wchar_t * (UNICODE string)
	//! \param s the UNICODE string to convert
	Xchg_string( const unsigned short *s );
	//! \brief constructor from a wchar_t * (UNICODE string)
	//! \param s the UNICODE string to convert
	Xchg_string( const __wchar_t *s );
	//! \brief copy the Xchg_string in a char * (UNICODE) string
	//! \param str the buffer string
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1;
	//! wchar_t * tmp;
	//! s1 = L"first string";
	//! tmp = new wchar_t[s1.len() + 1];
	//! s1.get_wchar(tmp); //Now tmp is L"first string"
	//! \endcode
	//! \warning the str string must be allocated before the method call
	void get_wchar( unsigned short * str ) const;
	//! \brief copy the Xchg_string in a char * (UNICODE) string
	//! \param str the buffer string
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1;
	//! wchar_t * tmp;
	//! s1 = L"first string";
	//! tmp = new wchar_t[s1.len() + 1];
	//! s1.get_wchar(tmp); //Now tmp is L"first string"
	//! \endcode
	//! \warning the str string must be allocated before the method call
	void get_wchar( __wchar_t * str ) const;

	int cmp( const unsigned short *s2 ) const;
	int cmp( const __wchar_t *s2 ) const;
	int ncmp( const unsigned short *s2, const int count ) const;
	int ncmp( const __wchar_t *s2, const int count ) const;
	int icmp( const unsigned short *s2 ) const;
	int icmp( const __wchar_t *s2 ) const;
	int nicmp( const unsigned short *s2, const size_t size ) const;
	int nicmp( const __wchar_t *s2, const size_t size ) const;
#else
	const Xchg_WChar_t * w_str() const;
	Xchg_string( const Xchg_WChar_t *s );
	//! \brief copy the Xchg_string in a char * (UNICODE) string
	//! \param str the buffer string
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1;
	//! wchar_t * tmp;
	//! s1 = L"first string";
	//! tmp = new wchar_t[s1.len() + 1];
	//! s1.get_wchar(tmp); //Now tmp is L"first string"
	//! \endcode
	//! \warning the str string must be allocated before the method call
	void get_wchar( Xchg_WChar_t * str ) const;
	int cmp( const Xchg_WChar_t *s2 ) const;
	int ncmp( const Xchg_WChar_t *s2, const int count ) const;
	int icmp( const Xchg_WChar_t *s2 ) const;
	int nicmp( const Xchg_WChar_t *s2, const size_t size ) const;
#endif

	//! \brief affectation operator from a Xchg_string
	//! \param s the affected value
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string res("this is a sample");
	//! Xchg_string tmp;
	//! tmp = res; //Now tmp is "this is a sample"
	//! \endcode
	Xchg_string& operator = ( const Xchg_string& s );
	//! \brief affectation operator from a char (ASCII) string
	//! \param s the affected value
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string tmp;
	//! tmp = "this is a sample"; //Now tmp is "this is a sample"
	//! \endcode
	Xchg_string& operator = ( const char* s );

	//! \brief affectation operator from a float
	//! \param fltval the affected value (a conversion is made)
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string tmp;
	//! tmp = (float)1.0; //Now tmp is "1.0"
	//! \endcode
	Xchg_string& operator = ( const float fltval );
	//! \brief affectation operator from a double
	//! \param dbl_val the affected value (a conversion is made)
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string tmp;
	//! tmp = (double)1.0; //Now tmp is "1.0"
	//! \endcode
	Xchg_string& operator = ( const double dbl_val );

	//! \brief access to a specified letter in the Xchg_string
	//! \param i the position of the wanted letter
	//! \return the letter
	Xchg_WChar_t& operator[] ( int );
	const Xchg_WChar_t& operator[] ( int i ) const;

	//! \brief File Utility : retrieve the file size
	//! \return The file size if success 0 else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1;
	//! Xchg_Size_t size;
	//! s1 = L"C:\\dir\\dir2\\filename.extension";
	//! size = s1.FileSize();
	//! \endcode
	Xchg_Size_t FileSize() const;
	Xchg_status ToDtkInt32( Xchg_Int32& outRes ) const;
	Xchg_status ToDtkDouble64( Xchg_Double64& outRes ) const;

	//! \brief Converts the Xchg_string to UTF8 string.
	//! \return The UTF8 String allocated with 'new'. Please use 'delete []' to free it.
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string tmp = L"30"; //special ascii string.
	//! char * UTF8Str = tmp.ToUTF8String(); //UTF8Str is ''
	//! delete [] UTF8Str; UTF8Str = NULL;
	//! \endcode
	char * ToUTF8String() const;

	Xchg_string Substring( const Xchg_Size_t& inStartIndex, const Xchg_Size_t& inLength ) const;
	//! \brief clear string data
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string tmp = L"one_text";
	//! tmp.clear(); //Now tmp is ""
	//! \endcode
	void clear();

	//! \brief affectation operator from a int
	//! \param integer the affected value (a conversion is made)
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string tmp;
	//! tmp.convert_from_int(1); //Now tmp is "1"
	//! \endcode
	void convert_from_int( const int integer, int force_unsigned_int = 0 );
	void ConvertFromUTF8String( const char * inUTF8String );

	/*utils functions*/
	Xchg_bool is_NULL() const;
	Xchg_bool is_not_NULL() const;
	//! \brief Retrieve the length of the Xchg_string
	//! \return the Xchg_string length
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string res;
	//! int length;
	//! length = res.len();
	//! \endcode
	int len() const;

	//! \brief concat the Xchg_string with the Xchg_string given in parameter
	//! \param s2 the Xchg_string to concat
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,s2;
	//! s1 = L"first string";
	//! s2 = L"second string";
	//! s1.cat(s2); //Now s1 is L"first stringsecond string"
	//! \endcode
	//! \remarks this method is similar to operator '+'
	void cat( const Xchg_string &s2 );

	//! \brief Converts the Xchg_string to Upper case.
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1 = L"My String";
	//! s1.ToUpper();
	//! //Now the s1 is L"MY STRING"
	//! \endcode
	void ToUpper();

	//! \brief Converts the Xchg_string to Lower case.
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1 = L"My String";
	//! s1.ToLower();
	//! //Now the s1 is L"my string"
	//! \endcode
	void ToLower();

	//! \brief compare the Xchg_string with the string given in parameter
	//! \param s2 : Xchg_string to be compared with
	//! \return  > 0 if this is greater than s2, 0 if they are identical, < 0 else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,s2;
	//! ...
	//! if(s1.cmp(s2) != 0)
	//! {
	//! ...
	//! }
	//! \endcode
	//! \remark this method is similar to '==', '<', '>', '>=', '<=', '!=' operators
	int cmp( const Xchg_string &s2 ) const;
	//! \brief compare the count first character of the Xchg_string with the string given in parameter
	//! \param s2 : Xchg_string to be compared with
	//! \param count : the number of characters to be compared
	//! \return  > 0 if this is greater than s2, 0 if they are identical, < 0 else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,s2;
	//! ...
	//! if(s1.cmp(s2,3) != 0)
	//! {
	//! ...
	//! }
	//! \endcode
	int ncmp( const Xchg_string &s2, const int count ) const;
	//! \brief Perform a lowercase comparison of strings
	//! \param s2 : Xchg_string to be compared with
	//! \return  > 0 if this is greater than s2, 0 if they are identical, < 0 else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,s2;
	//! ...
	//! if(s1.cmp(s2,3) != 0)
	//! {
	//! ...
	//! }
	//! \endcode
	int icmp( const Xchg_string &s2 ) const;
	//! \brief Compare characters of two strings without regard to case.
	//! \param s2 : Xchg_string to be compared with
	//! \param size : the number of characters to be compared
	//! \return  > 0 if this is greater than s2, 0 if they are identical, < 0 else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,s2;
	//! ...
	//! if(s1.cmp(s2,3) != 0)
	//! {
	//! ...
	//! }
	//! \endcode
	int nicmp( const Xchg_string &s2, const size_t size ) const;
	//! \brief copy the Xchg_string given in parameter in the Xchg_string
	//! \param s2 the Xchg_string to be copied
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,s2;
	//! s1 = L"first string";
	//! s2 = L"second string";
	//! s1.cpy(s2); //Now s1 is L"second string"
	//! \endcode
	//! \remarks this method is similar to operator '='
	void cpy( Xchg_string s2 );
	//! \brief copy a number of characters of the Xchg_string in an other Xchg_string
	//! \param s2 the Xchg_string to be copied
	//! \param size the number of character to be copied
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,s2;
	//! s2 = L"first string";
	//! s1.ncpy(s2,4); //Now s1 is L"firs"
	//! \endcode
	void ncpy( const Xchg_string & s2, size_t size );
	//! \brief find the position of a substring into a Xchg_string
	//! \param s1 the searched substring
	//! \return the position of the first character of the substring if success -1 else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,s2;
	//! int pos;
	//! s1 = L"first string";
	//! s2 = L"str";
	//! pos = s1.find_substring(s2); //Now pos is equal to 6
	//! \endcode
	int find_substring( const Xchg_string &s1 ) const;
	//! \brief find the position of a character into a Xchg_string
	//! \param car the searched character (UNICODE or ASCII form)
	//! \return the position of the first occurrence of car if success -1 else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1;
	//! int pos;
	//! s1 = L"first string";
	//! pos = s1.find_first(L's'); //Now pos is equal to 3
	//! \endcode
	int find_first( int character ) const;
	//! \brief find the position of a character into a Xchg_string
	//! \param car the searched character (UNICODE or ASCII form)
	//! \return the position of the last occurrence of car if success -1 else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1;
	//! int pos;
	//! s1 = L"first string";
	//! pos = s1.find_last(L's'); //Now pos is equal to 6
	//! \endcode
	int find_last( int character ) const;
	//! \brief retrieve the left part of the Xchg_string from a position
	//! \param pos : start position
	//! \return the left part of the Xchg_string if success or a NULL Xchg_string else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,s2;
	//! s1 = L"first string";
	//! s2 = s1.left(3); //Now s2 is equal to L"firs"
	//! \endcode
	//! \remark the pivot character is included in the resulted Xchg_string
	Xchg_string left( int pos ) const;
	//! \brief retrieve the right part of the Xchg_string from a position
	//! \param pos : start position
	//! \return the right part of the Xchg_string if success or a NULL Xchg_string else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,s2;
	//! s1 = L"first string";
	//! s2 = s1.right(3); //Now s2 is equal to L"st string"
	//! \endcode
	//! \remark the pivot character is included in the resulted Xchg_string
	Xchg_string right( int pos ) const;
	//! \brief retrieve the left part of the Xchg_string from a position
	//! \param pos : start position
	//! \return the left part of the Xchg_string if success or a NULL Xchg_string else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,s2;
	//! s1 = L"first string";
	//! s2 = s1.left_exclusive(3); //Now s2 is equal to L"fir"
	//! \endcode
	//! \remark the pivot character is NOT included in the resulted Xchg_string
	Xchg_string left_exclusive( int pos ) const;
	void LeftExclusive( const int& pos, Xchg_string& outRes ) const;
	//! \brief retrieve the right part of the Xchg_string from a position
	 //! \param pos : start position
	 //! \return the right part of the Xchg_string if success or a NULL Xchg_string else
	 //!
	 //! \b Sample:
	 //! \code
	 //! Xchg_string s1,s2;
	 //! s1 = L"first string";
	 //! s2 = s1.right_exclusive(3); //Now s2 is equal to L"t string"
	 //! \endcode
	 //! \remark the pivot character is NOT included in the resulted Xchg_string
	Xchg_string right_exclusive( int pos ) const;
	void RightExclusive( const int& pos, Xchg_string& outRes ) const;

	//! \brief Removes all occurrences of a given char at the beginning - leading - or the end - trailing - into the Xchg_string
	//! \param inCharToBeTrimmed the searched Xchg_WChar_t
	//! \param inTrimLeadingCharacters XCHG_TRUE If you want to trim leading characters. XCHG_FALSE otherwise. XCHG_TRUE by default.
	//! \param inTrimTrailingCharacters XCHG_TRUE If you want to trim leading characters. XCHG_FALSE otherwise. XCHG_TRUE by default.
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1;
	//! s1 = L" Test Test Test ";
	//! s1.TrimCharacters( ' ', XCHG_TRUE, XCHG_FALSE ); //Trim only leading ' ' characters
	//! //Now the string equals L"Test Test Test "
	//! \endcode
	void TrimCharacters( const Xchg_WChar_t inCharToBeTrimmed,
						 const Xchg_bool inTrimLeadingCharacters = XCHG_TRUE,
						 const Xchg_bool inTrimTrailingCharacters = XCHG_TRUE );
	//! \brief Removes all occurrences of a given char at the beginning - leading - and the end - trailing - into the Xchg_string
	//! \param inCharToBeTrimmed the searched Xchg_WChar_t
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1;
	//! s1 = L" Test Test Test ";
	//! s1.TrimLeadingCharacters( ' ' );
	//! //Now the string equals L"Test Test Test "
	//! \endcode
	inline void TrimLeadingCharacters( const Xchg_WChar_t inCharToBeTrimmed )
	{
		TrimCharacters( inCharToBeTrimmed, XCHG_TRUE, XCHG_FALSE );
	}
	//! \brief Removes all occurrences of a given char at the end - trailing - into the Xchg_string
	//! \param inCharToBeTrimmed the searched Xchg_WChar_t
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1;
	//! s1 = L" Test Test Test ";
	//! s1.TrimTrailingCharacters( ' ' );
	//! //Now the string equals L" Test Test Test"
	//! \endcode
	inline void TrimTrailingCharacters( const Xchg_WChar_t inCharToBeTrimmed )
	{
		TrimCharacters( inCharToBeTrimmed, XCHG_FALSE, XCHG_TRUE );
	}
	//! \brief Removes all occurrences of a given char at the beginning - leading - and the end - trailing - into the Xchg_string
	//! \param inCharToBeTrimmed the searched Xchg_WChar_t
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1;
	//! s1 = L" Test Test Test ";
	//! s1.TrimLeadingAndTrailingCharacters( ' ' );
	//! //Now the string equals L"Test Test Test"
	//! \endcode
	inline void TrimLeadingAndTrailingCharacters( const Xchg_WChar_t inCharToBeTrimmed )
	{
		TrimCharacters( inCharToBeTrimmed, XCHG_TRUE, XCHG_TRUE );
	}
	//! \brief Counts all the occurrences of a given substring into the Xchg_string
	//! \param substring the searched substring
	//! \return the occurrences number of the substring.
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1;
	//! Xchg_string s2;
	//! int nb;
	//! s1 = L"first string. Second string";
	//! s2 = L"string";
	//! nb = s1.count_substring_occurrences(s2); //Now nb is equal to 2
	//! \endcode
	int count_substring_occurrences( const Xchg_string &substring ) const;
	//! \brief Counts all the occurrences of a given character into the Xchg_string
	//! \param car the searched substring
	//! \return the occurrences number of the car.
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1;
	//! int nb;
	//! s1 = L"occurrences";
	//! nb = s1.count_char_occurrences('c'); //Now nb is equal to 3
	//! \endcode
	int count_char_occurrences( const int car ) const;


	//! \brief Replaces all occurrences of a character in a string with a new character.
	//! \param old_char : character to be replaced
	//! \param new_char : character to replace
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1;
	//! s1 = L"first string";
	//! s1.replace(L's', L't'); // Now s1 is equal to L"firtt ttring"
	//! \endcode
	int replace( const int& old_char, const int& new_char );
	//! \brief Removes all occurrences of a character in a string.
	//! \param removed_char : character to be removed
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1;
	//! s1 = L"first string";
	//! s1.removechar(L'r'); // Now s1 is equal to L"fist sting"
	//! \endcode
	int removechar( const int& removed_char );

	//! \brief File Utility : Retrieves the drive in Xchg_string form
	//! \return the drive string
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,res;
	//! s1 = L"C:\\dir\\dir2\\filename.extension";
	//! res = s1.drive(); //result is L"C:"
	//! \endcode
	Xchg_string drive() const;
	//! \brief File Utility : Retrieves the path in Xchg_string form
	//! \return the path string
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,res;
	//! s1 = L"C:\\dir\\dir2\\filename.extension";
	//! res = s1.path(); //result is L"\dir\dir2\"
	//! \endcode
	Xchg_string path() const;
	//! \brief File Utility : Retrieves the filename in Xchg_string form
	//! \return the filename string
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,res;
	//! s1 = L"C:\\dir\\dir2\\filename.extension";
	//! res = s1.filename(); //result is L"filename"
	//! \endcode
	Xchg_string filename() const;
	//! \brief File Utility : Retrieves the extension in Xchg_string form
	//! \return the extension string
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,res;
	//! s1 = L"C:\\dir\\dir2\\filename.extension";
	//! res = s1.extension(); //result is L"extension"
	//! \endcode
	Xchg_string extension() const;
	int test_extension( const Xchg_string &ext ) const;

	//! \brief File Utility : Retrieves the full path in Xchg_string form
	//! \return the full path string
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,res;
	//! s1 = L"C:\\dir\\dir2\\filename.extension";
	//! s1.FullPath( res ); //res == L"C:/dir/dir2/"
	//! \endcode
	//! \remark the full path '\\' are converted into '/'.
	void FullPath( Xchg_string& outFullPath ) const;

	//! \brief File Utility : Retrieves the filename extension in Xchg_string form
	//! \return the filename extension string
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,res;
	//! s1 = L"C:\\dir\\dir2\\filename.extension";
	//! s1.FileNameExtension( res ); //res == L"filename.extension"
	//! \endcode
	void FileNameExtension( Xchg_string& outFileNameExtension ) const;

	//int test_extension(const char *ext);
	//int test_extension(const wchar_t * ext);
	//! \brief File Utility : Open a file with the given rights
	//! \param inRights the given rights (in ASCII, UNICODE or Xchg_string form
	//! - you can use define such as 'XCHG_A' or 'XCHG_WP' for example - )
	//! \return the FILE * pointer if success or NULL else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1;
	//! FILE * res;
	//! s1 = L"C:\\dir\\dir2\\filename.extension";
	//! res = s1.OpenFile(XCHG_WP);
	//! if(res)
	//!        fclose(res); // We close the file
	//! \endcode
	FILE * OpenFile( const Xchg_string &inRights ) const;

	//! \brief File Utility : Delete a file
	//! \return 0 if success -1 else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1;
	//! FILE * res;
	//! s1 = L"C:\\dir\\dir2\\filename.extension";
	//! res = s1.OpenFile(XCHG_WP);
	//! if(res)
	//! {
	//!        fclose(res); // We close the file
	//!        s1.unlink(); // We delete the file
	//! }
	//! \endcode
	int unlink() const;
	//! \brief File Utility : Create a Directory
	//! \return 0 if success -1 else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1;
	//! FILE * res;
	//! s1 = L"C:\\dir\\dir2\\";
	//! res = s1.mkdir();
	//! \endcode
	int mkdir() const;
	int rmdir() const;
	//! \brief File Utility : tests if a file exists
	//! \return true if file exists false else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1;
	//! Xchg_bool res;
	//! s1 = L"C:\\dir\\dir2\\filename.extension";
	//! res = s1.existe_fichier();
	//! if(res)
	//! {
	//!        s1.unlink(); // We delete the file
	//! }
	//! \endcode
	Xchg_bool existe_fichier() const;

	//! \brief File Utility : retrieve The OS Path Separator as char.
	//! \return '\\' on Windows/Cygwin, '/' otherwise.
	inline static const char PathSeparatorChar()
	{
# if defined(_WIN32) || defined(__CYGWIN__) // Windows default, including MinGW and Cygwin
		return '\\';
# else
		return '/';
# endif
	}
	//! \brief File Utility : Retrieves The OS Path Separator as Xchg_WChar_t.
	//! \return L'\\' on Windows/Cygwin, L'/' otherwise.
	inline static const Xchg_WChar_t PathSeparatorWChar()
	{
# if defined(_WIN32) || defined(__CYGWIN__) // Windows default, including MinGW and Cygwin
		return L'\\';
# else
		return L'/';
# endif
	}
	//! \brief File Utility : Retrieves The OS Path Separator as Xchg_string.
	//! \return "\\" on Windows/Cygwin, "/" otherwise.
	inline static const Xchg_string PathSeparatorString()
	{
# if defined(_WIN32) || defined(__CYGWIN__) // Windows default, including MinGW and Cygwin
		return L"\\";
# else
		return "/";
# endif
	}

	//! \brief File Utility : Fixes path separator consistency.
	//! It lets you replace the '\' or '/' by the OS needed separator.
	//! \remark It uses the PathSeparatorChar method to get the correct path separator.
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1;
	//! s1 = L"\\dir/dir2\\filename.extension";
	//! s1.FixPathSeparator();
	//! On Windows the path is now L"\\dir\\dir2\\filename.extension"
	//! On Linux/MacOS the path is now L"/dir/dir2/filename.extension"
	//! \endcode
	void FixPathSeparator();

	//! \brief concat two Xchg_string
	//! \param s1 the first Xchg_string to concat
	//! \param s2 the second Xchg_string to concat
	//! \return the resulting Xchg_string
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,s2,res;
	//! s1 = L"first string";
	//! s2 = L"second string";
	//! res = s1 + s2; //Now res is L"first stringsecond string"
	//! \endcode
	friend Xchg_string operator + ( const Xchg_string &s1, const Xchg_string &s2 );

	//! \brief concat an int to the Xchg_string (convert the int to Xchg_string)
	//! \param integer the int to concat
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,res;
	//! int val;
	//! s1 = L"first string";
	//! val = 2;
	//! s1.add_int(val); //Now res is L"first string2"
	//! \endcode
	void add_int( const int integer, int force_unsigned_int = 0 );

	void Merge( const Xchg_string &s2 );
	Xchg_string& operator += ( const Xchg_string &s2 );

	//! \brief compare two Xchg_string
	//! \param s1 the first Xchg_string to compare
	//! \param s2 the second Xchg_string to compare
	//! \return true if they are identical false else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,s2;
	//! ...
	//! if(s1 == s2)
	//! {
	//! ...
	//! }
	//! \endcode
	friend XCHG_API bool operator == ( const Xchg_string &s1, const Xchg_string &s2 );
	//! \brief compare two Xchg_string
	//! \param s1 the first Xchg_string to compare
	//! \param s2 the second Xchg_string to compare
	//! \return true if s1 is greater than s2 false else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,s2;
	//! ...
	//! if(s1 > s2)
	//! {
	//! ...
	//! }
	//! \endcode
	friend XCHG_API bool operator > ( const Xchg_string &s1, const Xchg_string &s2 );
	//! \brief compare two Xchg_string
	//! \param s1 the first Xchg_string to compare
	//! \param s2 the second Xchg_string to compare
	//! \return true if s1 is smaller than s2 false else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,s2;
	//! ...
	//! if(s1 < s2)
	//! {
	//! ...
	//! }
	//! \endcode
	friend XCHG_API bool operator < ( const Xchg_string &s1, const Xchg_string &s2 );
	//! \brief compare two Xchg_string
	//! \param s1 the first Xchg_string to compare
	//! \param s2 the second Xchg_string to compare
	//! \return true if s1 is greater or equal than s2 false else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,s2;
	//! ...
	//! if(s1 >= s2)
	//! {
	//! ...
	//! }
	//! \endcode
	friend XCHG_API bool operator >= ( const Xchg_string &s1, const Xchg_string &s2 );
	//! \brief compare two Xchg_string
	//! \param s1 the first Xchg_string to compare
	//! \param s2 the second Xchg_string to compare
	//! \return true if s1 is smaller or equal than s2 false else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,s2;
	//! ...
	//! if(s1 <= s2)
	//! {
	//! ...
	//! }
	//! \endcode
	friend XCHG_API bool operator <= ( const Xchg_string &s1, const Xchg_string &s2 );
	//! \brief compare two Xchg_string
	//! \param s1 the first Xchg_string to compare
	//! \param s2 the second Xchg_string to compare
	//! \return true if s1 is different of s2 false else
	//!
	//! \b Sample:
	//! \code
	//! Xchg_string s1,s2;
	//! ...
	//! if(s1 != s2)
	//! {
	//! ...
	//! }
	//! \endcode
	friend XCHG_API bool operator != ( const Xchg_string &s1, const Xchg_string &s2 );
	friend XCHG_API std::ostream& operator<<( std::ostream&, const Xchg_string& );



};

namespace Xchg_StringUtility
{
	std::string ToUtf8( Xchg_string const & );

	Xchg_string FromUtf8( char const * );
}


#endif // __XCHG_STRING_HPP__

