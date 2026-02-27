#ifndef _GEOM_OFFSET_CURVE_HPP_
#define _GEOM_OFFSET_CURVE_HPP_

#include "base/xchg_export.hpp"
#include "geom/curve/xchg_curve.hpp"

// Forward declarations
class Xchg_OffsetCurve;
typedef SmartPtr<Xchg_OffsetCurve> Xchg_OffsetCurvePtr;

//! \class Xchg_OffsetCurve
//! \brief This is the Offset Curve Class.
//! U parametrisation : like basis curve
//! V parametrisation : like basis curve
//! It is part of the Xchg_OffsetCurve entity
//! \warning Please use the Xchg_OffsetCurvePtr class to handle it...
//!
class XCHG_API Xchg_OffsetCurve : public Xchg_Curve
{
protected:
    enum { _typeID2D = XCHG_TYPE_OFFSET_CURVE_2D, _typeID3D = XCHG_TYPE_OFFSET_CURVE_3D };
    // constructors/destructors
    Xchg_OffsetCurve();
    Xchg_OffsetCurve(const Xchg_OffsetCurve& s);
    Xchg_OffsetCurve(const Xchg_CurvePtr& inCurve, const Xchg_Double64& inOffset);
    virtual ~Xchg_OffsetCurve();

    // clone
    virtual Xchg_Object* Clone() const override;

private:
    Xchg_CurvePtr _Curve{nullptr};
    Xchg_Double64 _Offset{};
    friend class SmartPtr<Xchg_OffsetCurve>;

public:
    // downcasting
    inline virtual Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId) override
    {
        if (GetDimension() == 2 && inId == _typeID2D)
        {
            return 1;
        }
        else if (GetDimension() == 3 && inId == _typeID3D)
        {
            return 1;
        }

        return Xchg_Curve::DtkDynamicType(inId);
    }

    inline static Xchg_OffsetCurve* DtkDynamicCast(Xchg_Object* s)
    {
        if (s && (s->DtkDynamicType(_typeID2D) || s->DtkDynamicType(_typeID3D)))
        {
            return static_cast<Xchg_OffsetCurve*>(s);
        }
        return nullptr;
    }

    // create methods
    //! \brief Create an offset curve
    //! \param [in] inCurve : Curve to be offset
    //! \param [in] inOffset : Size of offset
    //! \return Xchg_OffsetCurvePtr created curve
    static Xchg_OffsetCurvePtr Create(const Xchg_CurvePtr& inCurve, const Xchg_Double64& inOffset);
    static Xchg_OffsetCurvePtr Create(const Xchg_OffsetCurve& inCurveToCopy);

    // get/set methods
    virtual XCHG_TYPE_ENUM GetType() const override;
    Xchg_Double64 GetOffset() const;
    Xchg_CurvePtr GetCurve() const;
    virtual Xchg_ErrorStatus GetDomain(Xchg_Double64& outUmin, Xchg_Double64& outUmax) const override;
    virtual Xchg_bool IsPeriodic() const;
};

#endif