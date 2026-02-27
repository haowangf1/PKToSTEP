#ifndef _GEOM_SURFACE_HPP_
#define _GEOM_SURFACE_HPP_

#include "base/xchg_entity.hpp"
#include "base/xchg_export.hpp"

// Forward declarations
class Xchg_Surface;
typedef SmartPtr<Xchg_Surface> Xchg_SurfacePtr;

/***************************************  SURFACES **********************************/
//! \ingroup geom
//! \class Xchg_Surface
//! \brief Xchg_Surface is a geometrical entity representing a surface.
//!	Some type of surface are unlimited such as plane, conical, cylindrical, extrusion surface.
//! It is an abstract class that cannot be instancied.  \n
//! It has several utility methods :  \n
//! - GetDomain  allow you to get the whole definition domain for the surface.
//! - SetTrimDomain/GetTrimDomain  allow you to specify/get the used domain for the surface.
//! - ToNurbs convert the surface into nurbs surface.

class XCHG_API Xchg_Surface : public Xchg_Entity
{
protected:

    enum { _typeID = XCHG_TYPE_SURFACE };
    Xchg_transfoPtr _Transfo{nullptr};
    Xchg_Double64 _TrimUVBox[4]{};
    Xchg_bool _IsTrimmed{XCHG_FALSE};
    // constructors/destructors
    Xchg_Surface() = default;
    Xchg_Surface(const Xchg_Surface& s) = default;
    virtual ~Xchg_Surface() = default;

    // clone
    virtual Xchg_Object* Clone() const = 0;

private:
    friend class SmartPtr<Xchg_Surface>;

public:
    // downcasting
    inline virtual Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId) override
    {
        if (inId == _typeID)
        {
            return 1;
        }
        return Xchg_Entity::DtkDynamicType(inId);
    }

    inline static Xchg_Surface* DtkDynamicCast(Xchg_Object* s)
    {
        if (s && s->DtkDynamicType(_typeID))
        {
            return static_cast<Xchg_Surface*>(s);
        }
        return nullptr;
    }

    // get/set methods
    virtual XCHG_TYPE_ENUM GetType() const = 0;
    virtual Xchg_ErrorStatus GetDomain(Xchg_Double64 outDomain[]) const = 0;
    Xchg_ErrorStatus GetTrimUVBox(Xchg_Double64 outTrimValues[]) const;
    Xchg_bool IsTrimmed() const;
    virtual Xchg_ErrorStatus SetTrimUVBox(Xchg_Double64 inTrimValues[]);
    virtual Xchg_bool IsUPeriodic() const;
    virtual Xchg_bool IsVPeriodic() const;

    void SetTransfo(const Xchg_transfoPtr& inTransfo);
    const Xchg_transfoPtr& GetTransfo() const;
};

#endif