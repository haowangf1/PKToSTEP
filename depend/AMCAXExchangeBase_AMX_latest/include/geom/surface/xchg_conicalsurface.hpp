#ifndef _GEOM_CONICAL_SURFACE_HPP_
#define _GEOM_CONICAL_SURFACE_HPP_

#include "base/xchg_export.hpp"
#include "geom/surface/xchg_surface.hpp"

// Forward declarations
class Xchg_ConicalSurface;
typedef SmartPtr<Xchg_ConicalSurface> Xchg_ConicalSurfacePtr;

//! \ingroup geom
//! \class Xchg_ConicalSurface
//! \brief This is the Conical Surface Class.
//! U parametrisation : [0.0 - 2PI]  0.0 is given by inUDirection or inXdir or inOriginDirection
//! V parametrisation : [-infini - +infini] as revolved curve is a line
//! \image html conical.png
//! It is part of the Xchg_ConicalSurface entity
//! \warning Please use the Xchg_ConicalSurfacePtr class to handle it...
//!
class XCHG_API Xchg_ConicalSurface : public Xchg_Surface
{
protected:
    enum { _typeID = XCHG_TYPE_CONICAL_SURFACE };
    // constructors/destructors
    Xchg_ConicalSurface();
    Xchg_ConicalSurface(const Xchg_ConicalSurface& s);
    Xchg_ConicalSurface(const Xchg_pnt& inOrigin,
                        const Xchg_dir& inNormal,
                        const Xchg_Double64& inRadius,
                        const Xchg_Double64& inAngle);
    Xchg_ConicalSurface(const Xchg_pnt& inOrigin,
                        const Xchg_dir& inNormal,
                        const Xchg_dir& inUDirection,
                        const Xchg_dir& inVDirection,
                        const Xchg_Double64& inRadius,
                        const Xchg_Double64& inAngle);
    virtual ~Xchg_ConicalSurface();

    // clone
    virtual Xchg_Object* Clone() const override;

private:
    Xchg_pnt _Origin{};
    Xchg_dir _Normal{};
    Xchg_dir _UDirection{};
    Xchg_dir _VDirection{};
    Xchg_Double64 _Radius{};
    Xchg_Double64 _SemiAngle{};
    friend class SmartPtr<Xchg_ConicalSurface>;

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

    inline static Xchg_ConicalSurface* DtkDynamicCast(Xchg_Object* s)
    {
        if (s && s->DtkDynamicType(_typeID))
        {
            return static_cast<Xchg_ConicalSurface*>(s);
        }
        return nullptr;
    }

    // create methods
    //! \brief  Create an infinite Cone
    //! \param [in] inOrigin : Cone origin
    //! \param [in] inNormal : Cone Axis or Zdir
    //! \param [in] inUDirection : UDirection ( origin of Uparametrisation) or Xdir
    //! \param [in] inFirstRadius : Cone Radius at origin
    //! \param [in] inAngle : semi_angle with Axis
    //! \param [in] inVDirection  : Optional VDirection or Ydir
    //! \return Xchg_ConicalSurfacePtr created surface
    static Xchg_ConicalSurfacePtr Create(const Xchg_pnt& inOrigin,
                                         const Xchg_dir& inNormal,
                                         const Xchg_dir& inUDirection,
                                         const Xchg_Double64& inFirstRadius,
                                         const Xchg_Double64& inAngle,
                                         const Xchg_dir& inVDirection = Xchg_dir());
    static Xchg_ConicalSurfacePtr Create(const Xchg_pnt& inOrigin,
                                         const Xchg_dir& inNormal,
                                         const Xchg_Double64& inFirstRadius,
                                         const Xchg_Double64& inAngle);
    static Xchg_ConicalSurfacePtr Create(const Xchg_ConicalSurface& inSurfToCopy);

    // get/set methods
    virtual XCHG_TYPE_ENUM GetType() const override;
    Xchg_pnt GetOrigin() const;
    const Xchg_dir& GetXDirection() const;
    Xchg_dir GetYDirection() const;
    const Xchg_dir& GetZDirection() const;
    Xchg_Double64 GetRadius() const;
    Xchg_Double64 GetSemiAngle() const;
    virtual Xchg_ErrorStatus GetDomain(Xchg_Double64 outDomain[]) const override;
    virtual Xchg_bool IsUPeriodic() const override;
    virtual Xchg_bool IsVPeriodic() const override;
};
#endif