#ifndef _GEOM_REVOLUTION_SURFACE_HPP_
#define _GEOM_REVOLUTION_SURFACE_HPP_

#include "base/xchg_export.hpp"
#include "geom/curve/xchg_curve.hpp"
#include "geom/surface/xchg_surface.hpp"

// Forward declarations
class Xchg_RevolutionSurface;
typedef SmartPtr<Xchg_RevolutionSurface> Xchg_RevolutionSurfacePtr;

class Xchg_Curve;
typedef SmartPtr<Xchg_Curve> Xchg_CurvePtr;

//! \ingroup geom
//! \class Xchg_RevolutionSurface
//! \brief This is the Revolution Surface Class.
//! U parametrisation : [0.0 - 2PI]  0.0 is given by inUDirection or inXdir or inOriginDirection
//! V parametrisation : revolved curve parametrisation
//! It is part of the Xchg_RevolutionSurface entity
//! \warning Please use the Xchg_RevolutionSurfacePtr class to handle it...
//!
class XCHG_API Xchg_RevolutionSurface : public Xchg_Surface
{
protected:
    enum { _typeID = XCHG_TYPE_REVOLUTION_SURFACE };
    // constructors/destructors
    Xchg_RevolutionSurface();

    Xchg_RevolutionSurface(const Xchg_RevolutionSurface& inSurfToCopy);

    Xchg_RevolutionSurface(const Xchg_CurvePtr& inRevolvedCurve,
                           const Xchg_pnt& inAxisposition,
                           const Xchg_dir& inRevolutionAxis);

    Xchg_RevolutionSurface(const Xchg_CurvePtr& inRevolvedCurve,
                           const Xchg_pnt& inAxisposition,
                           const Xchg_dir& inRevolutionAxis,
                           const Xchg_dir& inUDirection,
                           const Xchg_dir& inVDirection);
    virtual ~Xchg_RevolutionSurface();

    // clone
    virtual Xchg_Object* Clone() const override;

private:
    Xchg_CurvePtr _RevolvedCurve{nullptr};
    Xchg_pnt _AxisPosition{};
    Xchg_dir _RevolutionAxis{};
    Xchg_dir _UDirection{};
    Xchg_dir _VDirection{};
    friend class SmartPtr<Xchg_RevolutionSurface>;

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

    inline static Xchg_RevolutionSurface* DtkDynamicCast(Xchg_Object* s)
    {
        if (s && s->DtkDynamicType(_typeID))
        {
            return static_cast<Xchg_RevolutionSurface*>(s);
        }
        return nullptr;
    }

    // create methods
    //! \brief OBSOLETE Create Xchg_RevolutionSurface
    //! \deprecated Create Xchg_StandardRevolutionSurface
    //! \param [in] inRevolvedCurve : Curve to be revolved
    //! \param [in] inAxisposition : Revolution Axis Position
    //! \param [in] inRevolutionAxis : Revolution Axis or Zdir
    //! \param [in] inUDirection : UDirection (origin of Uparametrisation) or Xdir
    //! \param [in] inVDirection : Optional VDirection or Ydir
    //! \return Xchg_RevolutionSurfacePtr created surface
    static Xchg_RevolutionSurfacePtr Create(const Xchg_CurvePtr& inRevolvedCurve,
                                            const Xchg_pnt& inAxisposition,
                                            const Xchg_dir& inRevolutionAxis,
                                            const Xchg_dir& inUDirection,
                                            const Xchg_dir& inVDirection = Xchg_dir());

    static Xchg_RevolutionSurfacePtr Create(const Xchg_CurvePtr& inRevolvedCurve,
                                            const Xchg_pnt& inAxisposition,
                                            const Xchg_dir& inRevolutionAxis);

    static Xchg_RevolutionSurfacePtr Create(const Xchg_RevolutionSurface& inSurfToCopy);

    // get methods
    virtual XCHG_TYPE_ENUM GetType() const override;
    Xchg_CurvePtr GetRevolvedCurve() const;
    const Xchg_dir& GetRevolutionAxis() const;
    const Xchg_pnt& GetAxisPosition() const;
    const Xchg_dir& GetXDirection() const;
    Xchg_dir GetYDirection() const;
    const Xchg_dir& GetZDirection() const;
    virtual Xchg_ErrorStatus GetDomain(Xchg_Double64 outDomain[]) const override;
    virtual Xchg_bool IsUPeriodic() const override;
    virtual Xchg_bool IsVPeriodic() const override;
};

#endif