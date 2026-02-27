#ifndef _GEOM_CYLINDRICAL_SURFACE_HPP_
#define _GEOM_CYLINDRICAL_SURFACE_HPP_

#include "base/xchg_export.hpp"
#include "geom/surface/xchg_surface.hpp"

// Forward declarations
class Xchg_CylindricalSurface;
typedef SmartPtr<Xchg_CylindricalSurface> Xchg_CylindricalSurfacePtr;

//! \ingroup geom
//! \class Xchg_CylindricalSurface
//! \brief This is the Cylindrical Surface Class.
//! U parametrisation : [0.0 - 2PI]  0.0 is given by inUDirection or inXdir or inOriginDirection
//! V parametrisation : [-infini - +infini] as revolved curve is a line
//! \image html cylinder.png
//! It is part of the Xchg_CylindricalSurface entity
//! \warning Please use the Xchg_CylindricalSurfacePtr class to handle it...
//!
class XCHG_API Xchg_CylindricalSurface : public Xchg_Surface
{
protected:
    enum { _typeID = XCHG_TYPE_CYLINDRICAL_SURFACE };
    // constructors/destructors
    Xchg_CylindricalSurface();
    Xchg_CylindricalSurface(const Xchg_CylindricalSurface& s);
    Xchg_CylindricalSurface(const Xchg_pnt& inOrigin, const Xchg_dir& inNormal, const Xchg_Double64& inRadius);
    Xchg_CylindricalSurface(const Xchg_pnt& inOrigin,
                            const Xchg_dir& inNormal,
                            const Xchg_dir& inUDirection,
                            const Xchg_dir& inVDirection,
                            const Xchg_Double64& inRadius);
    virtual ~Xchg_CylindricalSurface();

    // clone
    virtual Xchg_Object* Clone() const override;

private:
    Xchg_pnt _Origin{};
    Xchg_dir _Normal{};
    Xchg_dir _UDirection{};
    Xchg_dir _VDirection{};
    Xchg_Double64 _Radius{};
    friend class SmartPtr<Xchg_CylindricalSurface>;

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

    inline static Xchg_CylindricalSurface* DtkDynamicCast(Xchg_Object* s)
    {
        if (s && s->DtkDynamicType(_typeID))
        {
            return static_cast<Xchg_CylindricalSurface*>(s);
        }
        return nullptr;
    }

    // create methods
    //! \brief Create an infinite cylinder
    //! \param [in] inOrigin : Cylinder origin
    //! \param [in] inNormal : Cylinder Axis or Zdir
    //! \param [in] inUDirection : UDirection ( origin of Uparametrisation) or Xdir
    //! \param [in] inRadius : Cylinder Radius
    //! \param [in] inVDirection  : Optional VDirection or Ydir
    //! \return Xchg_CylindricalSurfacePtr created surface
    static Xchg_CylindricalSurfacePtr Create(const Xchg_pnt& inOrigin,
                                             const Xchg_dir& inNormal,
                                             const Xchg_dir& inUDirection,
                                             const Xchg_Double64& inRadius,
                                             const Xchg_dir& inVDirection = Xchg_dir());
    static Xchg_CylindricalSurfacePtr Create(const Xchg_pnt& inOrigin,
                                             const Xchg_dir& inNormal,
                                             const Xchg_Double64& inRadius);
    static Xchg_CylindricalSurfacePtr Create(const Xchg_CylindricalSurface& inSurfToCopy);

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