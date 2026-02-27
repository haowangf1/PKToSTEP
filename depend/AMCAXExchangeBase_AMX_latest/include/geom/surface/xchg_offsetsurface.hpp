#ifndef _GEOM_OFFSET_SURFACE_HPP_
#define _GEOM_OFFSET_SURFACE_HPP_

#include "base/xchg_export.hpp"
#include "geom/surface/xchg_surface.hpp"

// Forward declarations
class Xchg_OffsetSurface;
typedef SmartPtr<Xchg_OffsetSurface> Xchg_OffsetSurfacePtr;

//! \class Xchg_OffsetSurface
//! \brief This is the Offset Surface Class.
//! U parametrisation : like basis surface
//! V parametrisation : like basis surface
//! \image html offset.png
//! It is part of the Xchg_OffsetSurface entity
//! \warning Please use the Xchg_OffsetSurfacePtr class to handle it...
//!
class XCHG_API Xchg_OffsetSurface : public Xchg_Surface
{
protected:
    enum { _typeID = XCHG_TYPE_OFFSET_SURFACE };
    // constructors/destructors
    Xchg_OffsetSurface();
    Xchg_OffsetSurface(const Xchg_OffsetSurface& s);
    Xchg_OffsetSurface(const Xchg_SurfacePtr& inSurf, const Xchg_Double64& inOffset);
    virtual ~Xchg_OffsetSurface();

    // clone
    virtual Xchg_Object* Clone() const override;

private:
    Xchg_SurfacePtr _Surf{nullptr};
    Xchg_Double64 _Offset{};
    friend class SmartPtr<Xchg_OffsetSurface>;

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

    inline static Xchg_OffsetSurface* DtkDynamicCast(Xchg_Object* s)
    {
        if (s && s->DtkDynamicType(_typeID))
        {
            return static_cast<Xchg_OffsetSurface*>(s);
        }
        return nullptr;
    }

    // create methods
    //! \brief Create an offset surface following surface normal
    //! \param [in] inSurf : Surface to be offset
    //! \param [in] inOffset : Size of offset
    //! \return Xchg_OffsetSurfacePtr created surface
    static Xchg_OffsetSurfacePtr Create(const Xchg_SurfacePtr& inSurf, const Xchg_Double64& inOffset);
    static Xchg_OffsetSurfacePtr Create(const Xchg_OffsetSurface& inSurfToCopy);

    // get/set methods
    virtual XCHG_TYPE_ENUM GetType() const override;
    Xchg_Double64 GetOffset() const;
    Xchg_SurfacePtr GetSurface() const;
    virtual Xchg_ErrorStatus GetDomain(Xchg_Double64 outDomain[]) const override;
    virtual Xchg_bool IsUPeriodic() const override;
    virtual Xchg_bool IsVPeriodic() const override;
    virtual Xchg_ErrorStatus SetTrimUVBox(Xchg_Double64 inTrimValues[]) override;
};

#endif