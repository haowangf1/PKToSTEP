#ifndef __XCHG_TRANSFO_HPP__
#define __XCHG_TRANSFO_HPP__

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_define.hpp"
#include "base/xchg_objectbase.hpp"
#include "base/xchg_pnt.hpp"
#include "base/xchg_dir.hpp"
#include "base/xchg_matrix.hpp"
#include "base/xchg_smartptr.hpp"


//! \ingroup base_types
//! \class Xchg_transfo 
//! \brief This is the Transformation dedicated class.
//! \brief A transformation a matrix is a basis : it contain a point, 3 vectors X,Y,Z, and a global scale.
//! \brief Note that the implementation allow you to unnormalize the vectors if really needed (distortion matrix), but it is suggested that the X,Y,Z vectors are normalized, and the scale parameter define the global scale
//!
//!	This class lets you use transformation.
class XCHG_API Xchg_transfo : public Xchg_Object
{
protected:
	enum { _typeID = XCHG_TYPE_TRANSFO }; 
	Xchg_dir v[3];  // X, Y, Z direction vectors
	Xchg_pnt o;     // Origin point
	Double64 sc;   // Global scale
	
	void _init();
	void _copy(const Xchg_transfo& s);
	void _reset();
	
	friend class SmartPtr<Xchg_transfo>;
	virtual Xchg_Object* Clone() const override { return new Xchg_transfo(*this); }

public:
	static SmartPtr<Xchg_transfo> create();
	static SmartPtr<Xchg_transfo> create(const Xchg_transfo&);
	
	//! \brief default constructor
	Xchg_transfo();
	//! \brief copy constructor
	Xchg_transfo(const Xchg_transfo& s);
	//! \brief move constructor
	Xchg_transfo(Xchg_transfo&& s) XCHG_NOEXCEPT;
	//! \brief create a transfo from a double*
	//! \param[in] d array of double
	//! \param[in] typefill method to fill (see below)
	//! \brief 0 (default) : 13 elements : Xx,Xy,Xz,Yx,Yy,Yz,Zx,Zy,Zz,Ox,Oy,Oz,GlobalScale
	Xchg_transfo(Double64* d, Xchg_Int32 typefill = 0);
	//! \brief constructor from matrix
	//! \param[in] m the matrix to be copied
	//! \warning m must be a 4x4 or 4x3 Xchg_matrix
	Xchg_transfo(const Xchg_matrix& m);
	//! \brief Full featured constructor
	//! \param[in] X the X direction.
	//! \param[in] Y the Y direction.
	//! \param[in] Z the Z direction.
	//! \param[in] O the Origin.
	//! \param[in] scale the scale factor : default is 1.0
	Xchg_transfo(const Xchg_dir& X, const Xchg_dir& Y, const Xchg_dir& Z, const Xchg_pnt& O, const Double64& scale = 1.0);
	//! \brief Destructor
	~Xchg_transfo();
	
	//! \brief assignment operators
	Xchg_transfo& operator=(const Xchg_transfo& s);
	Xchg_transfo& operator=(Xchg_transfo&& s) XCHG_NOEXCEPT;
	
	//! \brief Compare 2 Xchg_transfo matrix
	//! \param[in] in the Xchg_transfo to test with this.
	//! \return 1 if equal, 0 if not.
	int operator==(const Xchg_transfo& in) const;
	//! \brief Is transformation Identity
	//! \return 1 if is identity 0 if not.
	//!
	int is_identity() const;

	//! \brief Is normalized
	//! \return 1 if rotation vector are normalized 0 if not.
	//!
	Xchg_bool IsNormalized( const Double64 &inTol = XCHG_TOLERANCE ) const;

	//! \brief Is direct
	//! \return 1 if transformation is direct 0 if not.
	//!
	Xchg_bool IsDirect() const;

	//! \brief Obsolete : use getScale instead.
	//!
	double scale() const;
	//! \brief Retrieves the scale factor.
	//! \return the scale factor.
	//!
	const double& getScale() const;
	//! \brief Set a new scale
	//! \param[in] scale the new scale.
	//! \return the scale factor (get/set).
	//!
	void setScale( Double64 scale );
	//Xchg_pnt getO(); // deprecated.
	//Xchg_dir getVect(int n,bool normalize=false);  // deprecated.
	//! \brief Return the X vector
	//! \return the X Xchg_dir
	//!
	const Xchg_dir& getXdir() const;
	//! \brief Return the Y vector
	//! \return the Y Xchg_dir
	//!
	const Xchg_dir& getYdir() const;
	//! \brief Return the Z vector
	//! \return the Z Xchg_dir
	//!
	const Xchg_dir& getZdir() const;
	//! \brief Return the O center point of the Xchg_transfo (the center O of the basis)
	//! \return the O Xchg_pnt
	//!
	const Xchg_pnt& getOrigin() const;
	//! \brief Return the n component of the X vector
	//! \param[in] n coordinate of the X vector to return.
	//! \return the n component of the X vector
	//!
	const double& getXcomponent( Xchg_Int32 n ) const;
	//! \brief Return the n component of the Y vector
	//! \param[in] n coordinate of the Y vector to return.
	//! \return the n component of the Y vector
	//!
	const double& getYcomponent( Xchg_Int32 n ) const;
	//! \brief Return the n component of the Z vector
	//! \param[in] n coordinate of the Z vector to return.
	//! \return the n component of the Z vector
	//!
	const double& getZcomponent( Xchg_Int32 n ) const;
	//! \brief Return the n component of the O center point
	//! \param[in] n coordinate of the O center point
	//! \return the n component of the O center point
	//!
	const double& getOcomponent( Xchg_Int32 n ) const;
	//! \brief Set a new X vector
	//! \param[in] X the new X vector
	//!
	void setXdir( const Xchg_dir& X );
	//! \brief Set a new Y vector
	//! \param[in] Y the new Y vector
	//!
	void setYdir( const Xchg_dir& Y );
	//! \brief Set a new Z vector
	//! \param[in] Z the new Z vector
	//!
	void setZdir( const Xchg_dir& Z );
	//! \brief Set a new O center point
	//! \param[in] O the new O point
	//!
	void setOrigin( const Xchg_pnt& O );
	//! \brief Set a new X vector by giving its components
	//! \param[in] x the x component of the X vector
	//! \param[in] y the y component of the X vector
	//! \param[in] z the z component of the X vector
	//!
	void setXdir( Double64 x, Double64 y, Double64 z );
	//! \brief Set a new Y vector by giving its components
	//! \param[in] x the x component of the Y vector
	//! \param[in] y the y component of the Y vector
	//! \param[in] z the z component of the Y vector
	//!
	void setYdir( Double64 x, Double64 y, Double64 z );
	//! \brief Set a new Z vector by giving its components
	//! \param[in] x the x component of the Z vector
	//! \param[in] y the y component of the Z vector
	//! \param[in] z the z component of the Z vector
	//!
	void setZdir( Double64 x, Double64 y, Double64 z );
	//! \brief Set a new O center point by giving its components
	//! \param[in] x the x component of the O center point
	//! \param[in] y the y component of the O center point
	//! \param[in] z the z component of the O center point
	//!
	void setOrigin( Double64 x, Double64 y, Double64 z );
	//! \brief Inverse the current Matrix
	//! \return 0 is success, -1 otherwise
	//!
	int inverse();
	//! \brief Allow to multiply 2 Xchg_transfo (matrix multiplication)
	//! \param[in] t1 first matrix to multiply
	//! \param[in] t2 second matrix to multiply
	//! \return the product t1*t2
	//!
	friend XCHG_API Xchg_transfo operator*( const Xchg_transfo&, const Xchg_transfo& );
	//! \brief Allow to multiply a Xchg_transfo by a point
	//! \param[in] M matrix to multiply
	//! \param[in] P point to multiply by
	//! \return the point M*P
	//!
	friend XCHG_API Xchg_pnt operator*( const Xchg_transfo& M, const Xchg_pnt& P );
	//! \brief Allow to multiply a Xchg_transfo by a vector
	//! \param[in] M matrix to multiply
	//! \param[in] P vector to multiply by
	//! \return the vector M*P
	//!
	friend XCHG_API Xchg_dir operator*( const Xchg_transfo& M, const Xchg_dir& P );
	//! \brief Multiply a matrix by a scalar number
	//! \brief whole datas are multiplied.
	//! \param[in] M matrix to multiply
	//! \param[in] d scalar
	//! \return the vector M*d
	//!
	friend XCHG_API Xchg_transfo operator*( const Xchg_transfo& M, const Double64 d );
	//! \brief Divide a matrix by a scalar number
	//! \brief whole datas are divided.
	//! \param[in] M matrix to divide
	//! \param[in] d scalar
	//! \return the vector M/d
	//!
	friend XCHG_API Xchg_transfo operator/( Xchg_transfo& M, Double64 d );
	//! \brief Allow to add 2 Xchg_transfo (matrix addition)
	//! \param[in] t1 first matrix to add
	//! \param[in] t2 second matrix to add
	//! \return the sum t1+t2
	//!
	friend XCHG_API Xchg_transfo operator+( const Xchg_transfo& t1, const Xchg_transfo& t2 );
	//! \brief Allow to substracte 2 Xchg_transfo (matrix difference)
	//! \param[in] t1 first matrix to substracte
	//! \param[in] t2 second matrix to substracte
	//! \return the difference t1-t2
	//!
	friend XCHG_API Xchg_transfo operator-( const Xchg_transfo& t1, const Xchg_transfo& t2 );
	//friend Xchg_transfo operator+=(Xchg_transfo& t1,const Xchg_transfo& t2);  // deprecated
	//friend Xchg_transfo operator-=(Xchg_transfo& t1,const Xchg_transfo& t2);  // deprecated

	//void Decompose_TS(Xchg_transfo& T,Xchg_transfo& S);  // deprecated
	//void Decompose_ST(Xchg_transfo& S,Xchg_transfo& T);  // deprecated

	//! \brief Translate the Xchg_transfo
	//! \brief Mathematics : just sum the given vector to the O center point.
	//! \param[in] V vector
	//!
	void addTranslate( const Xchg_dir& V );
	//! \brief Add a scale
	//! \brief Mathematics : just multiply the current scale by the given scale
	//! \param[in] scale scale to add.
	//!
	void addScale( Double64 );

	//! \brief turn the basis represented by Xchg_transfo around a given vector with origin on (0.0, 0.0, 0.0) and scale equal to 1.0 
	//! \param[in] axis  axis of the ratation
	//! \param[in] angle angle of rotation (radian)
	//!
	void setAxisRotation( Xchg_dir axis, Double64 angle );

	//void addRotate_axeX(double angle);
	//void addRotate_axeY(double angle);
	//! \brief 2D operation, not yet generalized but soon
	//! \brief turn the basis represented by Xchg_transfo around Z vector
	//! \param[in] angle angle of rotation (radian)
	//!
	void addRotate_axeZ( Double64 angle );

	//! \brief Xchg_matrix conversion
	//! \return Xchg_matrix corresponding
	//!
	Xchg_matrix * GetDtkMatrix();

	void Normalize()
	{
		v[ 0 ].normalize();
		v[ 1 ].normalize();
		v[ 2 ].normalize();

	}



	//downcasting
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_Object::DtkDynamicType(inId);
	}
	inline static Xchg_transfo* DtkDynamicCast(Xchg_Object* s)
	{
		if (s && s->DtkDynamicType(_typeID))
			return static_cast<Xchg_transfo*>(s);
		return NULL;
	}

private:
	friend std::ostream& operator<<(std::ostream& o, const Xchg_transfo& d);
	friend Xchg_transfo Xchg_transfoPtr_to_Xchg_transfo(SmartPtr<Xchg_transfo>& t);
};


#endif //#ifndef __XCHG_TRANSFO_HPP__
