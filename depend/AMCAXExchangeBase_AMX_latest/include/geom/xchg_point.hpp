#ifndef _GEOM_POINT_HPP_
#define _GEOM_POINT_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_entity.hpp"
#include "base/xchg_smartptr.hpp"

/***************************************   POINTS **********************************/

//! \ingroup geom
//! \class Xchg_Point 
//! \brief This is the POINT Class.
//! It is part of the Xchg_Point entity
//! \warning Please use the Xchg_PointPtr class to handle it...
//!


// Typedef after class definition
class Xchg_Point;
typedef SmartPtr<Xchg_Point> Xchg_PointPtr;


class XCHG_API Xchg_Point : public Xchg_Entity
{
protected:
    Xchg_Double64 _Coords[3];
	Xchg_transfoPtr _Transfo{nullptr};
    enum { _typeID = XCHG_TYPE_POINT }; 
    Xchg_Point();
    Xchg_Point(const Xchg_Point& s);
    virtual ~Xchg_Point();

    friend class SmartPtr<Xchg_Point>;
public:
	//! \brief constructors 
    Xchg_Point(const Xchg_pnt& pt);

	//! \DtkDynamicType
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_Entity::DtkDynamicType(inId);
	}
		
	//! \DtkDynamicCast
	inline static Xchg_Point* DtkDynamicCast(Xchg_Object* inObject)
	{
		if (inObject && inObject->DtkDynamicType(_typeID))
			return static_cast<Xchg_Point*>(inObject);
		return NULL;
	}

	inline Xchg_Double64 x () const { return _Coords[0]; }
	inline Xchg_Double64 y () const { return _Coords[1]; }
	inline Xchg_Double64 z () const { return _Coords[2]; }
	inline void set_x (const Xchg_Double64 & inX) { _Coords[0] = inX; }
	inline void set_y (const Xchg_Double64 & inY) { _Coords[1] = inY; }
	inline void set_z (const Xchg_Double64 & inZ) { _Coords[2] = inZ; }

	//! \brief constructors returning Smart pointers
    static Xchg_PointPtr Create(const Xchg_Point& inToCopy);
    static Xchg_PointPtr Create(const Xchg_pnt& inToCopy);
    static Xchg_PointPtr Create(const Xchg_Double64 & inX, const Xchg_Double64 & inY, const Xchg_Double64 & inZ);

    //methods
	//! \brief Get Coordinates For Point
    Xchg_ErrorStatus GetCoordinates(Xchg_Double64 &x, Xchg_Double64 &y, Xchg_Double64 &z) const;
    Xchg_ErrorStatus GetCoordinates(Xchg_pnt &pnt) const;

	//! \brief Return type
    enum XCHG_TYPE_ENUM GetType() const { return XCHG_TYPE_POINT; }

	void SetTransfo(const Xchg_transfoPtr& inTransfo);
	const Xchg_transfoPtr& GetTransfo() const;
};

#endif