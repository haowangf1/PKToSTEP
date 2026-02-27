#ifndef _GEOM_LINEAR_EXTRUSION_SURFACE_HPP_
#define _GEOM_LINEAR_EXTRUSION_SURFACE_HPP_

#include "base/xchg_export.hpp"
#include "geom/curve/xchg_curve.hpp"
#include "geom/surface/xchg_surface.hpp"

// Forward declarations
class Xchg_LinearExtrusionSurface;
typedef SmartPtr<Xchg_LinearExtrusionSurface> Xchg_LinearExtrusionSurfacePtr;

class Xchg_Curve;
typedef SmartPtr<Xchg_Curve> Xchg_CurvePtr;

//! \ingroup geom
//! \class Xchg_LinearExtrusionSurface
//! \brief This is the Linear Extrusion Surface Class.
//! U parametrisation : Curve to be extruded
//! V parametrisation : [-infini - +infini] (0 is given by Curve position / unit vector is extrusion
//! axis )
//! \image html extrusion.png
//! It is part of the Xchg_LinearExtrusionSurface entity
//! \warning Please use the Xchg_LinearExtrusionSurfacePtr class to handle it...
//!
class XCHG_API Xchg_LinearExtrusionSurface : public Xchg_Surface
{
protected:
    enum { _typeID = XCHG_TYPE_LINEAR_EXTRUSION_SURFACE };
    // constructors/destructors
    Xchg_LinearExtrusionSurface();
    Xchg_LinearExtrusionSurface(const Xchg_LinearExtrusionSurface& s);
    Xchg_LinearExtrusionSurface(const Xchg_CurvePtr& inExtrudedCurve, const Xchg_dir& inExtrusionAxis);
    virtual ~Xchg_LinearExtrusionSurface();

    // clone
    virtual Xchg_Object* Clone() const override;

private:
    Xchg_CurvePtr _ExtrudedCurve{nullptr};
    Xchg_dir _ExtrusionAxis{};
    friend class SmartPtr<Xchg_LinearExtrusionSurface>;

public:
    // downcasting
    inline virtual Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId) override
    {
        if (inId == _typeID)
        {
            return 1;
        }
        return Xchg_Surface::DtkDynamicType(inId);
    }

    inline static Xchg_LinearExtrusionSurface* DtkDynamicCast(Xchg_Object* s)
    {
        if (s && s->DtkDynamicType(_typeID))
        {
            return static_cast<Xchg_LinearExtrusionSurface*>(s);
        }
        return nullptr;
    }

    // create methods
    //! \brief Create an infinite Xchg_LinearExtrusionSurface from a curve and a extrusion vector two
    //! curves
    //! \param [in] inExtrudedCurve : Curve to be extruded
    //! \param [in] inExtrusionAxis : Extrusion Axis
    //! \return Xchg_LinearExtrusionSurfacePtr created surface
    static Xchg_LinearExtrusionSurfacePtr Create(const Xchg_CurvePtr& inExtrudedCurve, const Xchg_dir& inExtrusionAxis);
    static Xchg_LinearExtrusionSurfacePtr Create(const Xchg_LinearExtrusionSurface& inSurfToCopy);

    // get methods
    virtual XCHG_TYPE_ENUM GetType() const override;
    Xchg_CurvePtr GetExtrudedCurve() const;
    const Xchg_dir& GetExtrusionAxis() const;
    virtual Xchg_ErrorStatus GetDomain(Xchg_Double64 outDomain[]) const override;
    virtual Xchg_bool IsUPeriodic() const override;
    virtual Xchg_bool IsVPeriodic() const override;
};
#endif