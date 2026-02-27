#ifndef _GEOM_POLYLINE_HPP_
#define _GEOM_POLYLINE_HPP_
#include "base/xchg_export.hpp"
#include "base/xchg_pnt.hpp"
#include "base/xchg_smartptr.hpp"
#include "base/xchg_vector.hpp"
#include "geom/curve/xchg_curve.hpp"

// Forward declarations
class Xchg_Polyline;
typedef SmartPtr<Xchg_Polyline> Xchg_PolylinePtr;

//! \ingroup geom
//! \class  Xchg_Polyline
//! \brief This is the Polyline and Segment Line Class.
//! It is part of the Xchg_Curve entity
//! \warning Please use the Xchg_PolylinePtr class to handle it...
//!
class XCHG_API Xchg_Polyline : public Xchg_Curve
{
private:
    Xchg_vector<Xchg_pnt> _Points{};

protected:
    //! \brief Entity Type
    enum { _typeID = XCHG_TYPE_POLYLINE };

    // Constructors and Destructor
    Xchg_Polyline();
    Xchg_Polyline(const Xchg_pnt& inPnt1, const Xchg_pnt& inPnt2);
    Xchg_Polyline(const Xchg_Polyline& inPolylineToCopy);
    Xchg_Polyline(const Xchg_vector<Xchg_pnt>& inPoints);
    virtual ~Xchg_Polyline();

    //! \brief  smart pointer
    friend class SmartPtr<Xchg_Polyline>;

public:
    //! \DtkDynamicType
    inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
    {
        if (inId == _typeID)
        {
            return 1;
        }
        return Xchg_Curve::DtkDynamicType(inId);
    }

    //! \DtkDynamicCast
    inline static Xchg_Polyline* DtkDynamicCast(Xchg_Object* inObject)
    {
        if (inObject && inObject->DtkDynamicType(_typeID))
        {
            return static_cast<Xchg_Polyline*>(inObject);
        }
        return nullptr;
    }

public:
    //------------------------- Factory Methods -------------------------
    static Xchg_PolylinePtr Create(const Xchg_pnt& inPnt1, const Xchg_pnt& inPnt2);
    static Xchg_PolylinePtr Create(const Xchg_Polyline& inPolylineToCopy);
    static Xchg_PolylinePtr Create(const Xchg_vector<Xchg_pnt>& inPoints);

    //------------------------- Methods -------------------------
    Xchg_Size_t GetNumPoints() const;
    Xchg_pnt& Point(const Xchg_Size_t& inIndex);
    const Xchg_pnt& Point(const Xchg_Size_t& inIndex) const;

    //! \brief Return Curve type
    XCHG_TYPE_ENUM GetType() const
    {
        return static_cast<XCHG_TYPE_ENUM>(_typeID);
    }

    virtual Xchg_Object* Clone() const override;
    virtual Xchg_ErrorStatus GetDomain(Xchg_Double64& outUmin, Xchg_Double64& outUmax) const override;
    virtual Xchg_bool IsPeriodic() const override;
};
#endif