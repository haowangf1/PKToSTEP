#ifndef _GEOM_SPHERICAL_SURFACE_HPP_
#define _GEOM_SPHERICAL_SURFACE_HPP_

#include "base/xchg_export.hpp"
#include "geom/surface/xchg_surface.hpp"

// Forward declarations
class Xchg_SphericalSurface;
typedef SmartPtr<Xchg_SphericalSurface> Xchg_SphericalSurfacePtr;

//! \ingroup geom
//! \class Xchg_SphericalSurface
//! \brief This is the Spherical Surface Class.
//! U parametrisation : [0.0 - 2PI]  0.0 is given by inUDirection or inXdir or inOriginDirection
//! V parametrisation : [-Pi/2 - +Pi/2] as revolved curve is a trimmed circle
//! \image html sphere.png
//! It is part of the Xchg_SphericalSurface entity
//! \warning Please use the Xchg_SphericalSurfacePtr class to handle it...
//!
class XCHG_API Xchg_SphericalSurface : public Xchg_Surface
{
protected:
    enum { _typeID = XCHG_TYPE_SPHERICAL_SURFACE };
    // constructors/destructors
    Xchg_SphericalSurface();
    Xchg_SphericalSurface(const Xchg_SphericalSurface& s);
    Xchg_SphericalSurface(const Xchg_pnt& inOrigin, const Xchg_dir& inNormal, const Xchg_Double64& inRadius);
    Xchg_SphericalSurface(const Xchg_pnt& inOrigin,
                          const Xchg_dir& inNormal,
                          const Xchg_dir& inUDirection,
                          const Xchg_dir& inVDirection,
                          const Xchg_Double64& inRadius);
    virtual ~Xchg_SphericalSurface();

    // clone
    virtual Xchg_Object* Clone() const override;

private:
    Xchg_pnt _Origin{};
    Xchg_dir _Normal{};
    Xchg_dir _UDirection{};
    Xchg_dir _VDirection{};
    Xchg_Double64 _Radius{};
    friend class SmartPtr<Xchg_SphericalSurface>;

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

    inline static Xchg_SphericalSurface* DtkDynamicCast(Xchg_Object* s)
    {
        if (s && s->DtkDynamicType(_typeID))
        {
            return static_cast<Xchg_SphericalSurface*>(s);
        }
        return nullptr;
    }

    // create methods
    //! \brief Create Xchg_SphericalSurface
    //! \param [in] inOrigin : Spherical origin
    //! \param [in] inNormal : Spherical Axis or Zdir
    //! \param [in] inUDirection : UDirection ( origin of Uparametrisation) or Xdir
    //! \param [in] inRadius : Spherical Radius
    //! \param [in] inYDir  : Optional Ydir
    //! \return Xchg_SphericalSurfacePtr created surface
    static Xchg_SphericalSurfacePtr Create(const Xchg_pnt& inOrigin,
                                           const Xchg_dir& inNormal,
                                           const Xchg_dir& inUDirection,
                                           const Xchg_Double64& inRadius,
                                           const Xchg_dir& inVDirection = Xchg_dir());
    static Xchg_SphericalSurfacePtr Create(const Xchg_pnt& inOrigin,
                                           const Xchg_dir& inNormal,
                                           const Xchg_Double64& inRadius);
    static Xchg_SphericalSurfacePtr Create(const Xchg_SphericalSurface& inSurfToCopy);

    // get/set methods
    virtual XCHG_TYPE_ENUM GetType() const override;
    Xchg_pnt GetOrigin() const;
    const Xchg_dir& GetXDirection() const;
    Xchg_dir GetYDirection() const;
    const Xchg_dir& GetZDirection() const;
    Xchg_Double64 GetRadius() const;
    virtual Xchg_ErrorStatus GetDomain(Xchg_Double64 outDomain[]) const override;
    virtual Xchg_bool IsUPeriodic() const override;
    virtual Xchg_bool IsVPeriodic() const override;
};

#endif