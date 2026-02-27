#ifndef __EXCHANGE_UUID_HPP__
#define __EXCHANGE_UUID_HPP__

#include "base/xchg_export.hpp"
#include <iosfwd>
#include "base/xchg_type.hpp"

	
class XCHG_API Xchg_UUID
{
private:
	Int32   _vals[ 4 ];
public:
	Xchg_UUID();
	Xchg_UUID( const Xchg_UUID & s );
	Xchg_UUID( const Int32( &inVals )[ 4 ] );
	Xchg_UUID( const Int32 inVal1, const Int32 inVal2, const Int32 inVal3, const Int32 inVal4 );
	Int32& operator []( Size_t pos );
	const Int32& operator []( Size_t pos ) const;
	void Increment();
	int CompareTo( const Xchg_UUID& inUuid ) const;
	static void GenerateRandomUUID( Xchg_UUID& outRes );
	void clear();
	bool IsNull() const;
	friend inline int operator < ( const Xchg_UUID &s1, const Xchg_UUID &s2 );
	bool operator ==( const Xchg_UUID& s1 ) const;
	friend std::ostream& operator <<( std::ostream &inOutStream, const Xchg_UUID &inVal );
};


typedef Xchg_UUID Xchg_UUID;

#endif
