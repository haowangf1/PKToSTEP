#ifndef _GEOM_HYPERBOLA_HPP_
#define _GEOM_HYPERBOLA_HPP_
#include "base/xchg_dir.hpp"
#include "base/xchg_export.hpp"
#include "base/xchg_pnt.hpp"
#include "base/xchg_smartptr.hpp"
#include "geom/curve/xchg_curve.hpp"

// Forward declarations
class Xchg_Hyperbola;
typedef SmartPtr<Xchg_Hyperbola> Xchg_HyperbolaPtr;

class XCHG_API Xchg_Hyperbola : public Xchg_Curve
{
private:
    Xchg_pnt _Center;              // 中心点
    Xchg_dir _Normal;              // 法线方向
    Xchg_dir _Xref;                // X参考轴方向
    Xchg_Double64 _SemiAxis;       // 实半轴长度
    Xchg_Double64 _SemiImageAxis;  // 虚半轴长度

protected:
    enum { _typeID = XCHG_TYPE_HYPERBOLA };

    // Constructors and destructor
    Xchg_Hyperbola();
    Xchg_Hyperbola(const Xchg_Hyperbola& inToCopy);
    Xchg_Hyperbola(const Xchg_pnt& inCenter,
                   const Xchg_dir& inNormal,
                   const Xchg_Double64& inSemiAxis,
                   const Xchg_Double64& inSemiImageAxis);
    Xchg_Hyperbola(const Xchg_pnt& inCenter,
                   const Xchg_dir& inNormal,
                   const Xchg_dir& inXref,
                   const Xchg_Double64& inSemiAxis,
                   const Xchg_Double64& inSemiImageAxis);
    ~Xchg_Hyperbola();

    friend class SmartPtr<Xchg_Hyperbola>;

public:
    //! \DtkDynamicType
    inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
    {
        if (inId == _typeID)
        {
            return 1;
        }
        return Xchg_Curve::DtkDynamicType(inId);
    }

    //! \DtkDynamicCast
    inline static Xchg_Hyperbola* DtkDynamicCast(Xchg_Object* inObject)
    {
        if (inObject && inObject->DtkDynamicType(_typeID))
        {
            return static_cast<Xchg_Hyperbola*>(inObject);
        }
        return nullptr;
    }
    //! \brief Return Curve type
    XCHG_TYPE_ENUM GetType() const
    {
        return static_cast<XCHG_TYPE_ENUM>(_typeID);
    }

public:
    //------------------------- Factory Methods -------------------------
    static Xchg_HyperbolaPtr Create(const Xchg_Hyperbola& inToCopy);
    static Xchg_HyperbolaPtr Create(const Xchg_pnt& inCenter,
                                    const Xchg_dir& inNormal,
                                    const Xchg_Double64& inSemiAxis,
                                    const Xchg_Double64& inSemiImageAxis);
    static Xchg_HyperbolaPtr Create(const Xchg_pnt& inCenter,
                                    const Xchg_dir& inNormal,
                                    const Xchg_dir& inXref,
                                    const Xchg_Double64& inSemiAxis,
                                    const Xchg_Double64& inSemiImageAxis);

    //------------------------- Methods -------------------------
    Xchg_pnt GetCenterPoint() const;
    Xchg_dir GetNormalDirection() const;
    Xchg_dir GetOriginDirection() const;
    Xchg_Double64 GetSemiAxis() const;
    Xchg_Double64 GetSemiImageAxis() const;
    const Xchg_dir& GetXDirection() const;
    Xchg_dir GetYDirection() const;
    const Xchg_dir& GetZDirection() const;
    Xchg_ErrorStatus GetDomain(Xchg_Double64& outUmin, Xchg_Double64& outUmax) const override;
    virtual Xchg_bool IsPeriodic() const override;
};
#endif