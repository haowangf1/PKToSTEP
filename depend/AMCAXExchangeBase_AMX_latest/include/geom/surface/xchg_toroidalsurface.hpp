#ifndef _GEOM_TOROIDAL_SURFACE_HPP_
#define _GEOM_TOROIDAL_SURFACE_HPP_

#include "base/xchg_export.hpp"
#include "geom/surface/xchg_surface.hpp"

// Forward declarations
class Xchg_ToroidalSurface;
typedef SmartPtr<Xchg_ToroidalSurface> Xchg_ToroidalSurfacePtr;

//! \ingroup geom
//! \class Xchg_ToroidalSurface
//! \brief This is the Toroidal Surface Class.
//! U parametrisation : [0.0 - 2PI]  0.0 is given by inUDirection or inXdir or inOriginDirection
//! V parametrisation : [0.0 - 2PI] as revolved curve is a circle
//! \image html tore.png
//! It is part of the Xchg_ToroidalSurface entity
//! \warning Please use the Xchg_ToroidalSurfacePtr class to handle it...
//!
class XCHG_API Xchg_ToroidalSurface : public Xchg_Surface
{
protected:
    enum { _typeID = XCHG_TYPE_TOROIDAL_SURFACE };
    // constructors/destructors
    Xchg_ToroidalSurface();
    Xchg_ToroidalSurface(const Xchg_ToroidalSurface& s);
    Xchg_ToroidalSurface(const Xchg_pnt& inOrigin,
                         const Xchg_dir& inNormal,
                         const Xchg_Double64& inMajorRadius,
                         const Xchg_Double64& inMinorRadius);
    Xchg_ToroidalSurface(const Xchg_pnt& inOrigin,
                         const Xchg_dir& inNormal,
                         const Xchg_dir& inUDirection,
                         const Xchg_dir& inVDirection,
                         const Xchg_Double64& inMajorRadius,
                         const Xchg_Double64& inMinorRadius);
    virtual ~Xchg_ToroidalSurface();

    // clone
    virtual Xchg_Object* Clone() const override;

private:
    Xchg_pnt _Origin{};
    Xchg_dir _Normal{};
    Xchg_dir _UDirection{};
    Xchg_dir _VDirection{};
    Xchg_Double64 _MajorRadius{};
    Xchg_Double64 _MinorRadius{};
    friend class SmartPtr<Xchg_ToroidalSurface>;

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

    inline static Xchg_ToroidalSurface* DtkDynamicCast(Xchg_Object* s)
    {
        if (s && s->DtkDynamicType(_typeID))
        {
            return static_cast<Xchg_ToroidalSurface*>(s);
        }
        return nullptr;
    }

    // create methods
    //! \brief Create Xchg_ToroidalSurface
    //! \param [in] inOrigin : Toroidal origin
    //! \param [in] inNormal : Toroidal Axis or Zdir
    //! \param [in] inUDirection : UDirection ( origin of Uparametrisation) or Xdir
    //! \param [in] inMajorRadius : Toroidal Major Radius
    //! \param [in] inMinorRadius : Toroidal Minor Radius
    //! \param [in] inVDirection  : Optional VDirection or Ydir
    //! \return Xchg_ToroidalSurfacePtr created surface
    static Xchg_ToroidalSurfacePtr Create(const Xchg_pnt& inOrigin,
                                          const Xchg_dir& inNormal,
                                          const Xchg_Double64& inMajorRadius,
                                          const Xchg_Double64& inMinorRadius);
    static Xchg_ToroidalSurfacePtr Create(const Xchg_pnt& inOrigin,
                                          const Xchg_dir& inNormal,
                                          const Xchg_dir& inUDirection,
                                          const Xchg_Double64& inMajorRadius,
                                          const Xchg_Double64& inMinorRadius,
                                          const Xchg_dir& inVDirection = Xchg_dir());
    static Xchg_ToroidalSurfacePtr Create(const Xchg_ToroidalSurface& inSurfToCopy);

    // get/set methods
    virtual XCHG_TYPE_ENUM GetType() const override;
    Xchg_pnt GetOrigin() const;
    const Xchg_dir& GetXDirection() const;
    Xchg_dir GetYDirection() const;
    const Xchg_dir& GetZDirection() const;
    Xchg_Double64 GetMajorRadius() const;
    Xchg_Double64 GetMinorRadius() const;
    virtual Xchg_ErrorStatus GetDomain(Xchg_Double64 outDomain[]) const override;
    virtual Xchg_bool IsUPeriodic() const override;
    virtual Xchg_bool IsVPeriodic() const override;
};

#endif