#ifndef __XCHG_PNT_HPP__
#define __XCHG_PNT_HPP__

#include "base/xchg_export.hpp"
#include <fstream>
#include <math.h>
#include "base/xchg_type.hpp" 
#include <cmath>
#include <iosfwd>

class Xchg_pnt;
class Xchg_dir;
class Xchg_string;
class Xchg_status;
class Xchg_3dMat;

//! \ingroup base_types
//! \class Xchg_pnt 
//! \brief This is a mathematical point class.
//!
//!    This class provides all method about points
class XCHG_API Xchg_pnt
{
protected:
	double _Coords[ 3 ];
public:
	//! \brief default constructor
	Xchg_pnt();
	~Xchg_pnt() {}
	//! \brief copy constructor
	Xchg_pnt( const Xchg_pnt& s );
	//! \brief constructor from 3 double
	Xchg_pnt( double a, double b, double c = 0.0 );
	//! \brief constructor from a 3 double array
	Xchg_pnt( const double d[] );
	Xchg_pnt& operator = ( const Xchg_pnt& s );
	//! \brief access to a specified coordinate of the Xchg_pnt
	//! - i = 0 => X coordinate
	//! - i = 1 => Y coordinate
	//! - i = 2 => Z coordinate
	inline double& operator[] ( Size_t i )
	{
		XCHG_ASSERT( i < 3 );
		return _Coords[ i ];
	}
	//! \brief access to a specified coordinate of the Xchg_pnt.
	//! \warning You can't modify the returned value.
	//! - i = 0 => X coordinate
	//! - i = 1 => Y coordinate
	//! - i = 2 => Z coordinate
	inline const double& operator[] ( Size_t i ) const
	{
		XCHG_ASSERT( i < 3 );
		return _Coords[ i ];
	}

	//! \brief access to x coordinate of the Xchg_pnt.
	inline double & x() { return _Coords[ 0 ]; }

	//! \brief access to x coordinate of the Xchg_pnt.
	//! \warning You can't modify the returned value.
	inline const double & x() const { return _Coords[ 0 ]; }

	//! \brief access to y coordinate of the Xchg_pnt.
	inline double & y() { return _Coords[ 1 ]; }

	//! \brief access to y coordinate of the Xchg_pnt.
	//! \warning You can't modify the returned value.
	inline const double & y() const { return _Coords[ 1 ]; }

	//! \brief access to z coordinate of the Xchg_pnt.
	inline double & z() { return _Coords[ 2 ]; }

	//! \brief access to z coordinate of the Xchg_pnt.
	//! \warning You can't modify the returned value.
	inline const double & z() const { return _Coords[ 2 ]; }

	//! \brief Retrieves to the 3 coordinates of the class.
	//! \warning You can't modify the returned value.
	inline void GetXYZ( Double64& outX, Double64& outY, Double64& outZ ) const
	{
		outX = _Coords[ 0 ];
		outY = _Coords[ 1 ];
		outZ = _Coords[ 2 ];
	}

	//! \brief Sets the 3 coordinates of the class.
	inline void SetXYZ( const Double64& inX, const Double64& inY, const Double64& inZ )
	{
		_Coords[ 0 ] = inX;
		_Coords[ 1 ] = inY;
		_Coords[ 2 ] = inZ;
	}

	Xchg_pnt& operator*=( double );
	Xchg_pnt& operator/=( double );
	Xchg_pnt operator-();
	//! \brief translate the Xchg_pnt with the given Xchg_dir
	//! \param a the translation direction
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_pnt a(0.0,0.0,0.0);
	//! Xchg_dir b(15.0,0.0,0.0);
	//! a += b; //Now a is (15.0,0.0,0.0)
	//! \endcode
	Xchg_pnt& operator+=( const Xchg_dir& );
	//! \brief translate (in the opposite direction) the Xchg_pnt with the given Xchg_dir
	//! \param a the translation direction
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_pnt a(0.0,0.0,0.0);
	//! Xchg_dir b(15.0,0.0,0.0);
	//! a -= b; //Now a is (-15.0,0.0,0.0)
	//! \endcode
	Xchg_pnt& operator-=( const Xchg_dir& );
	//! \brief translate  a Xchg_pnt with a Xchg_dir
	//! \param u the point to translate
	//! \param v the translation direction
	//! \return the translated point
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_pnt a(0.0,0.0,0.0);
	//! Xchg_dir b(15.0,0.0,0.0);
	//! Xchg_pnt c;
	//! c = a + b; //Now c is (15.0,0.0,0.0)
	//! \endcode
	friend XCHG_API Xchg_pnt operator+( const Xchg_pnt&, const Xchg_dir& );  // pour barycentre
	//! \brief translate (in the opposite direction) a Xchg_pnt with a Xchg_dir
	//! \param u the point to translate
	//! \param v the translation direction
	//! \return the translated point
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_pnt a(0.0,0.0,0.0);
	//! Xchg_dir b(15.0,0.0,0.0);
	//! Xchg_pnt c;
	//! c = a - b; //Now c is (-15.0,0.0,0.0)
	//! \endcode
	friend XCHG_API Xchg_dir operator-( const Xchg_pnt&, const Xchg_dir& );

	inline Xchg_pnt operator* ( double a ) const { return Xchg_pnt( _Coords[ 0 ] * a, _Coords[ 1 ] * a, _Coords[ 2 ] * a ); }
	friend XCHG_API Xchg_pnt operator* ( double, const Xchg_pnt& );
	friend XCHG_API Xchg_pnt operator/ ( const Xchg_pnt&, double );
	friend XCHG_API Xchg_pnt operator/ ( double, const Xchg_pnt& );

	//! \brief test if 2 Xchg_pnt are equal
	//! \param u the first Xchg_pnt
	//! \param v the second Xchg_pnt
	//! \return 0 if not equals != 0 else
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_pnt a(0.0,0.0,0.0);
	//! Xchg_pnt b(15.0,0.0,0.0);
	//! ...
	//! if(a == b)
	//! {...}
	//! \endcode
	friend XCHG_API int operator==( const Xchg_pnt&, const Xchg_pnt& );
	//! \brief test if 2 Xchg_pnt are different
	//! \param u the first Xchg_pnt
	//! \param v the second Xchg_pnt
	//! \return 0 if equals != 0 else
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_pnt a(0.0,0.0,0.0);
	//! Xchg_pnt b(15.0,0.0,0.0);
	//! ...
	//! if(a != b)
	//! {...}
	//! \endcode
	friend XCHG_API int operator!=( const Xchg_pnt&, const Xchg_pnt& );

	//! \brief test if 2 Xchg_pnt are equal - with a tolerance -
	//! \param pnt2 the Xchg_pnt to compare
	//! \param tol the tolerance to compare
	//! \return 0 if not equals != 0 else
	//!
	//! \b Sample:
	//! \code 
	//! Xchg_pnt a(0.0,0.0,0.0);
	//! Xchg_pnt b(15.0,0.0,0.0);
	//! ...
	//! if(a.is_equal(b, 0.1))
	//! {...}
	//! \endcode
	int IsEqual( const Xchg_pnt& inPointToCompare, const double inTolerance ) const;
	//  Xchg_pnt operator=(double d[]); // inutile

	inline static void ProcessMiddlePoint( const Xchg_pnt& inPntA, const Xchg_pnt& inPntB, Xchg_pnt& outMiddlePnt )
	{
		for( Size_t i = 0; i < 3; i++ )
			outMiddlePnt[ i ] = ( inPntA[ i ] + inPntB[ i ] ) / 2.0;
	}

	inline static Double64 ProcessDistance( const Xchg_pnt& inPoint1, const Xchg_pnt& inPoint2 )
	{
		Double64 dist = 0.0;
		Size_t i;

		for( i = 0; i < 3; i++ )
			dist += ( inPoint1[ i ] - inPoint2[ i ] ) * ( inPoint1[ i ] - inPoint2[ i ] );

		return static_cast< Double64 >( sqrt( dist ) );
	}

	/*	void SetInvalid();
		Xchg_bool IsInvalid()const;*/

	friend XCHG_API std::ostream& operator<<( std::ostream& o, const Xchg_pnt& d );

	friend XCHG_API double Argument( const Xchg_pnt& O, const Xchg_dir& x, const Xchg_dir& y, const Xchg_pnt& P );
	friend XCHG_API double Argument( const Xchg_pnt& O, const Xchg_dir& x, const Xchg_pnt& P );
	friend XCHG_API double Argument( const Xchg_pnt& P );

	double Solve( const Xchg_dir& col0, const Xchg_dir& col1, const Xchg_dir& col2, const Xchg_dir& col3 );
	double Solve( const Xchg_3dMat& matrix, const Xchg_dir& col3 );
	void Multiply( const Xchg_3dMat& matrix );
	inline void operator *= ( const Xchg_3dMat& matrix );
	Xchg_pnt Multiplied( const Xchg_3dMat& matrix ) const;
	inline Xchg_pnt operator * ( const Xchg_3dMat& matrix ) const;
	void PostMultiply( const Xchg_3dMat& matrix );
	Xchg_pnt PostMultiplied( const Xchg_3dMat& matrix ) const;

	void MultiplyByTransposed( const Xchg_3dMat& matrix );
	Xchg_pnt MultipliedByTransposed( const Xchg_3dMat& matrix ) const;

	double Project( const Xchg_pnt& org, const Xchg_dir& zdir, const Xchg_dir& zPrj );
	inline Xchg_pnt Projected( const Xchg_pnt& org, const Xchg_dir& zdir, const Xchg_dir& zPrj ) const;
	double Project( const Xchg_pnt& org, const Xchg_dir& zdir );
	inline Xchg_pnt Projected( const Xchg_pnt& org, const Xchg_dir& zdir ) const;
	double Project( const Xchg_dir& zdir, double d, const Xchg_dir& zPrj );
	inline Xchg_pnt Projected( const Xchg_dir& zdir, double d, const Xchg_dir& zPrj ) const;
	double Project( const Xchg_dir& zdir, double d );
	inline Xchg_pnt Projected( const Xchg_dir& zdir, double d ) const;
	void   ProjectOnLine( const Xchg_pnt& org, const Xchg_dir& dir );
	inline Xchg_pnt ProjectedOnLine( const Xchg_pnt& org, const Xchg_dir& dir ) const;

	void    Scale( const Xchg_pnt& p0, double s );
	Xchg_pnt Scaled( const Xchg_pnt& p0, double s ) const;
	void    Translate( const Xchg_dir& dir );
	Xchg_pnt Translated( const Xchg_dir& dir ) const;
	void	Translate( const Xchg_pnt& p0, const Xchg_pnt& p1 );
	Xchg_pnt	Translated( const Xchg_pnt& p0, const Xchg_pnt& p1 ) const;
	void    Transform( const Xchg_3dMat& matrix );
	Xchg_pnt Transformed( const Xchg_3dMat& matrix ) const;
	void    Rotate( const Xchg_pnt& p0, const Xchg_3dMat& matrix );
	Xchg_pnt Rotated( const Xchg_pnt& p0, const Xchg_3dMat& matrix ) const;
	void    Rotate( const Xchg_dir& dir, double angle, const Xchg_pnt& center );
	Xchg_pnt	Rotated( const Xchg_dir& dir, double angle, const Xchg_pnt& center ) const;
	void    Symetry( const Xchg_dir& zpl );
	void    Symetry( const Xchg_pnt& opl, const Xchg_dir& zpl );
	Xchg_pnt Symetrised( const Xchg_pnt& opl, const Xchg_dir& zpl ) const;
	void    Affinity( const Xchg_pnt&, const Xchg_dir& xdir, const Xchg_dir& ydir, double f[ 3 ] );
	inline void  Affinity( double f[ 3 ] ) { _Coords[ 0 ] *= f[ 0 ]; _Coords[ 1 ] *= f[ 1 ];  _Coords[ 2 ] *= f[ 2 ]; }
	Xchg_pnt Affinitised( const Xchg_pnt& p, const Xchg_dir& xdir, const Xchg_dir& ydir, double f[ 3 ] ) const;
};

inline void Xchg_pnt::operator *= ( const Xchg_3dMat& matrix )
{
	Multiply( matrix );
}

inline Xchg_pnt Xchg_pnt::operator * ( const Xchg_3dMat& matrix ) const
{
	return Multiplied( matrix );
}

inline Xchg_pnt Xchg_pnt::Projected( const Xchg_pnt& org, const Xchg_dir& zdir, const Xchg_dir& zprj ) const
{
	Xchg_pnt tmp = *this;
	tmp.Project( org, zdir, zprj );
	return tmp;
}

inline Xchg_pnt Xchg_pnt::Projected( const Xchg_pnt& org, const Xchg_dir& zdir ) const
{
	Xchg_pnt tmp = *this;
	tmp.Project( org, zdir );
	return tmp;
}

inline Xchg_pnt Xchg_pnt::Projected( const Xchg_dir& zdir, double d, const Xchg_dir& zprj ) const
{
	Xchg_pnt tmp = *this;
	tmp.Project( zdir, d, zprj );
	return tmp;
}

inline Xchg_pnt Xchg_pnt::Projected( const Xchg_dir& zdir, double d ) const
{
	Xchg_pnt tmp = *this;
	tmp.Project( zdir, d );
	return tmp;
}

inline Xchg_pnt Xchg_pnt::ProjectedOnLine( const Xchg_pnt& org, const Xchg_dir& dir ) const
{
	Xchg_pnt tmp = *this;
	tmp.ProjectOnLine( org, dir );
	return tmp;
}

#endif
