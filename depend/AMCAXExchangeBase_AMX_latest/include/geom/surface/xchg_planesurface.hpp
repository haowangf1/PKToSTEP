#ifndef _GEOM_PLANE_SURFACE_HPP_
#define _GEOM_PLANE_SURFACE_HPP_

#include "base/xchg_export.hpp"
#include "geom/surface/xchg_surface.hpp"

// Forward declarations
class Xchg_PlaneSurface;
typedef SmartPtr<Xchg_PlaneSurface> Xchg_PlaneSurfacePtr;

//! \ingroup geom
//! \class Xchg_PlaneSurface
//! \brief This is the Plane Surface Class.
//! U parametrisation : [-infini - +infini] : 0 is inOrigin and unit vector is inUDirection
//! V parametrisation : [-infini - +infini] : 0 is inOrigin and unit vector is inVDirection
//! \image html plane.png
//! It is part of the Xchg_PlaneSurface entity
//! \warning Please use the Xchg_PlaneSurfacePtr class to handle it...
//!
class XCHG_API Xchg_PlaneSurface : public Xchg_Surface
{
protected:
    enum { _typeID = XCHG_TYPE_PLANE };
    // constructors/destructors
    Xchg_PlaneSurface();
    Xchg_PlaneSurface(const Xchg_PlaneSurface& s);
    Xchg_PlaneSurface(const Xchg_pnt& inOrigin, const Xchg_dir& inNormal);
    Xchg_PlaneSurface(const Xchg_pnt& inOrigin,
                      const Xchg_dir& inNormal,
                      const Xchg_dir& inUDirection,
                      const Xchg_dir& inVDirection);
    virtual ~Xchg_PlaneSurface();

    // clone
    virtual Xchg_Object* Clone() const override;

private:
    Xchg_pnt _Origin{};
    Xchg_dir _Normal{};
    Xchg_dir _UDirection{};
    Xchg_dir _VDirection{};
    friend class SmartPtr<Xchg_PlaneSurface>;

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

    inline static Xchg_PlaneSurface* DtkDynamicCast(Xchg_Object* s)
    {
        if (s && s->DtkDynamicType(_typeID))
        {
            return static_cast<Xchg_PlaneSurface*>(s);
        }
        return nullptr;
    }

    // create methods
    //! \brief Create an infinite plane surface
    //! \param [in] inOrigin : Origin of plane ( origin of Uparametrisation and Vparametrisation )
    //! \param [in] inNormal : Plane Normal or Zdir
    //! \param [in] inUDirection : UDirection (origin of Uparametrisation) or Xdir
    //! \param [in] inVDirection : optionnal VDirection (origin of Vparametrisation) or Ydir
    //! \return Xchg_PlaneSurfacePtr created surface
    static Xchg_PlaneSurfacePtr Create(const Xchg_pnt& inOrigin,
                                       const Xchg_dir& inNormal,
                                       const Xchg_dir& inUDirection,
                                       const Xchg_dir& inVDirection = Xchg_dir());
    static Xchg_PlaneSurfacePtr Create(const Xchg_pnt& inOrigin, const Xchg_dir& inNormal);
    //! \brief Create an infinite plane surface by copy
    //! \param [in] inPlaneToCopy : Plane surface to copy
    //! \return Xchg_PlaneSurfacePtr created surface
    static Xchg_PlaneSurfacePtr Create(const Xchg_PlaneSurface& inPlaneToCopy);

    // get/set methods
    virtual XCHG_TYPE_ENUM GetType() const override;
    Xchg_pnt GetOrigin() const;
    Xchg_dir GetNormal() const;
    Xchg_dir GetUDirection() const;
    Xchg_dir GetVDirection() const;
    Xchg_dir GetXDirection() const;
    Xchg_dir GetYDirection() const;
    Xchg_dir GetZDirection() const;
    virtual Xchg_ErrorStatus GetDomain(Xchg_Double64 outDomain[]) const override;
    virtual Xchg_bool IsUPeriodic() const override;
    virtual Xchg_bool IsVPeriodic() const override;
};

#endif