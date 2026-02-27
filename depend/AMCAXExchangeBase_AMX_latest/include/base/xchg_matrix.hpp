#ifndef __XCHG_MATRIX_HPP__
#define __XCHG_MATRIX_HPP__
#include "base/xchg_export.hpp"
#include "base/xchg_objectbase.hpp"
#include "base/xchg_type.hpp"


class XCHG_API Xchg_matrix: public Xchg_Object

{
protected:
	void _init();
	void _copy( const Xchg_matrix& s );
	void _reset();
	Xchg_matrix * post_matrix( Size_t ligne_x );
	enum { _typeID = XCHG_TYPE_MATRIX };
	Double64 **_mat;
	Size_t _num_lines;
	Size_t _num_columns;

private:
	bool inverse3_3();
	void transposeCarre();
public:
	Xchg_matrix( Size_t, Size_t, bool allZeroes = false );
	Xchg_matrix( Size_t a );
	Xchg_matrix();
	Xchg_matrix( const Xchg_matrix& m ) :Xchg_Object( m ) { _copy( m ); };
	~Xchg_matrix();
	void set_nulle();
	bool inverse( const bool &inIsDiag = false );
	void transpose();
	int is_identity();
	double det();
	Xchg_matrix * get_sous_mat( Size_t xmin, Size_t xmax, Size_t ymin, Size_t ymax );
	int set_sous_matrix( Xchg_matrix * sub, Size_t x, Size_t y );
	int operator==( Xchg_matrix& );
	Xchg_matrix& operator=( const Xchg_matrix & );

	//downcasting
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_Object::DtkDynamicType(inId);
	}
	inline static Xchg_matrix* DtkDynamicCast(Xchg_Object* s)
	{
		if (s && s->DtkDynamicType(_typeID))
			return static_cast<Xchg_matrix*>(s);
		return NULL;
	}
	
	// Clone method for deep copying
	Xchg_Object* Clone() const override;

	// accesseurs	
	double& set_val( const Size_t &row, const Size_t &col );
	int get_dim( Size_t *x, Size_t * y ) const;
	Size_t X() const;
	Size_t Y() const;
	Double64* operator[]( Size_t col );
	const Double64* operator[]( Size_t col ) const;

	// mutliplication
	Xchg_matrix *multiplie( Xchg_matrix* );
	Xchg_matrix multiplie( Xchg_matrix& );
	Xchg_matrix multiplie( double );
	Xchg_matrix addition( Xchg_matrix& );
	Xchg_matrix soustraction( Xchg_matrix& );
	void opposee();
	friend XCHG_API Xchg_matrix operator*( Xchg_matrix&, Xchg_matrix& );

private:
	friend class SmartPtr<Xchg_matrix>;
};

#endif //#ifndef __XCHG_MATRIX_HPP__
