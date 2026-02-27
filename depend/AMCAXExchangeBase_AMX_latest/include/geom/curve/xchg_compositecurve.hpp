#ifndef _GEOM_COMPOSITE_CURVE_HPP_
#define _GEOM_COMPOSITE_CURVE_HPP_

#include "base/xchg_export.hpp"
#include "geom/curve/xchg_curve.hpp"

// Forward declarations
class Xchg_CompositeCurve;
typedef SmartPtr<Xchg_CompositeCurve> Xchg_CompositeCurvePtr;

//! \class Xchg_CompositeCurve
//! \brief This is the Composite Curve Class.
//! It is part of the Xchg_CompositeCurve entity
//! \warning Please use the Xchg_CompositeCurvePtr class to handle it...
//!
class XCHG_API Xchg_CompositeCurve : public Xchg_Curve
{
protected:
    enum { _typeID = XCHG_TYPE_COMPOSITE_CURVE };
    // constructors/destructors
    Xchg_CompositeCurve();
    Xchg_CompositeCurve(const Xchg_CompositeCurve& s);
    Xchg_CompositeCurve(const Xchg_vector<Xchg_CurvePtr>& inCurves);
    virtual ~Xchg_CompositeCurve();

    // clone
    virtual Xchg_Object* Clone() const override;

private:
    Xchg_vector<Xchg_CurvePtr> _Curves;
    friend class SmartPtr<Xchg_CompositeCurve>;

public:
    // downcasting
    inline virtual Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId) override
    {
        if (inId == _typeID)
        {
            return 1;
        }

        return Xchg_Curve::DtkDynamicType(inId);
    }

    inline static Xchg_CompositeCurve* DtkDynamicCast(Xchg_Object* s)
    {
        if (s && s->DtkDynamicType(_typeID))
        {
            return static_cast<Xchg_CompositeCurve*>(s);
        }
        return nullptr;
    }

    static Xchg_CompositeCurvePtr Create(const Xchg_vector<Xchg_CurvePtr>& inCurves);

    // get/set methods
    virtual XCHG_TYPE_ENUM GetType() const override;
    virtual Xchg_ErrorStatus GetDomain(Xchg_Double64& outUmin, Xchg_Double64& outUmax) const override;
    virtual Xchg_bool IsPeriodic() const;
    const Xchg_vector<Xchg_CurvePtr>& GetCurves() const;
};

#endif