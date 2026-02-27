#ifndef __XCHG_DIR_HPP__
#define __XCHG_DIR_HPP__
#include "base/xchg_export.hpp"
#include "base/xchg_pnt.hpp"
#include "base/xchg_type.hpp"
#include <iosfwd>

class Xchg_3dMat;
class Xchg_pnt;
//! \ingroup base_types
//! \class Xchg_dir 
//! \brief This is a mathematical direction class.
//!
//!    This class provides all method about direction
class XCHG_API Xchg_dir:public Xchg_pnt
{
public:
	Xchg_dir();
	~Xchg_dir() {}
	Xchg_dir( const Xchg_dir& s );
	Xchg_dir( const Xchg_pnt& pnt );
	Xchg_dir( double a, double b, double c = 0.0 );
	Xchg_dir( const Xchg_pnt&, const Xchg_pnt& );
	Xchg_dir& operator =( const Xchg_dir& );
	Xchg_dir operator+() const;
	Xchg_dir operator-() const;
	Xchg_dir& operator+=( const Xchg_dir& );
	Xchg_dir& operator-=( const Xchg_dir& );
	Xchg_dir& operator+=( const Xchg_pnt& );
	Xchg_dir& operator-=( const Xchg_pnt& );
	Xchg_dir& operator*=( double );
	Xchg_dir& operator/=( double );
	double norm() const;
	int normalize();
	int rotate( const Xchg_dir& Z, double angle );
	double angle_with_Xchg_dir(const Xchg_dir& V2, const Xchg_dir& ref_Z);
	Double64 ComputeAngleByQuadrant( const Xchg_dir& V2, const Xchg_dir& ref_Z ) const;

	void reverse();

	inline Xchg_dir operator* ( double a ) const { return Xchg_dir( _Coords[ 0 ] * a, _Coords[ 1 ] * a, _Coords[ 2 ] * a ); }
	friend XCHG_API Xchg_dir operator* ( double, const Xchg_dir& );
	friend XCHG_API Xchg_dir operator/ ( const Xchg_dir&, double );
	friend XCHG_API Xchg_dir operator/ ( double, const Xchg_dir& );

	inline Xchg_dir operator+( const Xchg_dir& v ) { return Xchg_dir( _Coords[ 0 ] + v._Coords[ 0 ], _Coords[ 1 ] + v._Coords[ 1 ], _Coords[ 2 ] + v._Coords[ 2 ] ); }
	inline Xchg_dir operator-( const Xchg_dir& v ) { return Xchg_dir( _Coords[ 0 ] - v._Coords[ 0 ], _Coords[ 1 ] - v._Coords[ 1 ], _Coords[ 2 ] - v._Coords[ 2 ] ); }

	friend XCHG_API double operator* ( const Xchg_dir&, const Xchg_dir& ); // scalar product - produit scalaire
	friend XCHG_API Xchg_dir operator/ ( const Xchg_dir&, const Xchg_dir& );  // vector product - produit vectoriel
	friend XCHG_API Xchg_dir operator^ ( const Xchg_dir&, const Xchg_dir& );  // vector product - produit vectoriel
	friend XCHG_API double Pxchg_DotProduct( const Xchg_dir&, const Xchg_dir& ); // 
	friend XCHG_API Xchg_dir Pxchg_CrossProduct( const Xchg_dir&, const Xchg_dir& );  // 

	friend XCHG_API Xchg_dir Orthogonal( const Xchg_dir& ); // trouve un vecteur orthogonal
	friend XCHG_API double norm( const Xchg_dir& ); // norme
	friend XCHG_API double normC( const Xchg_dir& ); // norme carree

	friend XCHG_API std::ostream& operator<<( std::ostream& o, const Xchg_dir& d );

	// gestion vecteurs 2D.
	friend XCHG_API Xchg_pnt intersect2d( const Xchg_pnt& p1, const Xchg_dir& d1, const Xchg_pnt& p2, const Xchg_dir& d2 );
	friend XCHG_API Xchg_pnt orthoproj2d( const Xchg_pnt& p1, const Xchg_dir& d1, const Xchg_pnt& p2 );
	friend XCHG_API int onleft( const Xchg_dir&, const Xchg_dir& );
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


	void Cross( const Xchg_dir& right );
	Xchg_dir Crossed( const Xchg_dir& right ) const;
	inline void operator ^= ( const Xchg_dir& right );
	void CrossCross( const Xchg_dir& c1, const Xchg_dir& c2 );
	Xchg_dir CrossCrossed( const Xchg_dir& c1, const Xchg_dir& c2 ) const;

	double SquareModulus();
	Xchg_dir GetNormal() const;
	double Normalize();
	Xchg_dir Normalized() const;
	Xchg_dir reversed() const;

	inline double Dot( const Xchg_dir& other ) const { return ( *this * other ); }
	inline double operator & ( const Xchg_dir& other ) const { return ( *this * other ); }

	double DotCross( const Xchg_dir& c1, const Xchg_dir& c2 ) const;

	void Multiply( const Xchg_3dMat& matrix );
	inline void operator *= ( const Xchg_3dMat& matrix );
	Xchg_dir Multiplied( const Xchg_3dMat& matrix ) const;
	inline Xchg_dir operator * ( const Xchg_3dMat& matrix ) const;
	void MultiplyNormalize( const Xchg_3dMat& matrix );
	Xchg_dir MultipliedNormalize( const Xchg_3dMat& matrix ) const;
	void PostMultiply( const Xchg_3dMat& matrix );
	Xchg_dir PostMultiplied( const Xchg_3dMat& matrix ) const;
	void MultiplyByTransposed( const Xchg_3dMat& matrix );
	Xchg_dir MultipliedByTransposed( const Xchg_3dMat& matrix ) const;
	double AngleXY( double tolerance = 0.00000001 ) const;
	double Angle( const Xchg_dir& vRef, const Xchg_dir& vAxe, double tolerance = 0.00000001 ) const;

	void    Scale( const Xchg_pnt& p0, double s );
	Xchg_dir Scaled( const Xchg_pnt& p0, double s ) const;
	void    Translate( const Xchg_dir& dir );
	Xchg_dir Translated( const Xchg_dir& dir ) const;
	void	Translate( const Xchg_pnt& p0, const Xchg_pnt& p1 );
	Xchg_dir	Translated( const Xchg_pnt& p0, const Xchg_pnt& p1 ) const;
	void    Transform( const Xchg_3dMat& matrix );
	Xchg_dir Transformed( const Xchg_3dMat& matrix ) const;
	void    Rotate( const Xchg_pnt& p0, const Xchg_3dMat& matrix );
	Xchg_dir Rotated( const Xchg_pnt& p0, const Xchg_3dMat& matrix ) const;
	void    Rotate( const Xchg_dir& dir, double angle, const Xchg_pnt& center );
	Xchg_dir	Rotated( const Xchg_dir& dir, double angle, const Xchg_pnt& center ) const;
	void    Symetry( const Xchg_dir& zpl );
	void    Symetry( const Xchg_pnt& opl, const Xchg_dir& zpl );
	Xchg_dir Symetrised( const Xchg_pnt& opl, const Xchg_dir& zpl ) const;
	void    Affinity( const Xchg_pnt&, const Xchg_dir& xdir, const Xchg_dir& ydir, double[ 3 ] );
	inline void  Affinity( double f[ 3 ] ) { _Coords[ 0 ] *= f[ 0 ]; _Coords[ 1 ] *= f[ 1 ];  _Coords[ 2 ] *= f[ 2 ]; }
	Xchg_dir Affinitised( const Xchg_pnt& p, const Xchg_dir& xdir, const Xchg_dir& ydir, double f[ 3 ] ) const;


	Xchg_dir UnitizedDerivative( const Xchg_dir& thisPrime ) const;
	Xchg_dir UnitizedSecondDerivative( const Xchg_dir& thisPrime, const Xchg_dir& thisSecond ) const;
	Xchg_dir DerivativeOfCrossProduct( const Xchg_dir& other, const Xchg_dir& thisPrime, const Xchg_dir& otherPrime ) const;
	Xchg_dir SecondDerivativeOfCrossProduct( const Xchg_dir& other, const Xchg_dir& thisPrime, const Xchg_dir& otherPrime, const Xchg_dir& thisSecond, const Xchg_dir& otherSecond ) const;
	double DerivativeOfNorm( const Xchg_dir& thisPrime ) const;
	double SecondDerivativeOfNorm( const Xchg_dir& thisPrime, const Xchg_dir& thisSecond ) const;
};


inline void Xchg_dir::operator *= ( const Xchg_3dMat& matrix )
{
	Multiply( matrix );
}

inline Xchg_dir Xchg_dir::operator * ( const Xchg_3dMat& matrix ) const
{
	return Multiplied( matrix );
}

inline void Xchg_dir::operator ^= ( const Xchg_dir& right )
{
	Cross( right );
}


#endif
