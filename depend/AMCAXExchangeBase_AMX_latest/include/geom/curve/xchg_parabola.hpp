#ifndef _GEOM_PARABOLA_HPP_
#define _GEOM_PARABOLA_HPP_

#include "base/xchg_dir.hpp"
#include "base/xchg_export.hpp"
#include "base/xchg_pnt.hpp"
#include "base/xchg_smartptr.hpp"
#include "geom/curve/xchg_curve.hpp"

// Forward declarations
class Xchg_Parabola;
typedef SmartPtr<Xchg_Parabola> Xchg_ParabolaPtr;

class XCHG_API Xchg_Parabola : public Xchg_Curve
{
private:
    Xchg_pnt _Center;            // 顶点（抛物线顶点）
    Xchg_dir _Normal;            // 法线方向（对称轴方向）
    Xchg_dir _Xref;              // X参考轴方向（开口方向）
    Xchg_Double64 _MajorRadius;  // 焦距（顶点到焦点的距离）

protected:
    enum { _typeID = XCHG_TYPE_PARABOLA };
    // Constructors and Destructor
    Xchg_Parabola();
    Xchg_Parabola(const Xchg_Parabola& inToCopy);
    Xchg_Parabola(const Xchg_pnt& inCenter, const Xchg_dir& inNormal, const Xchg_Double64& inMajorRadius);
    Xchg_Parabola(const Xchg_pnt& inCenter,
                  const Xchg_dir& inNormal,
                  const Xchg_dir& inXref,
                  const Xchg_Double64& inMajorRadius);
    ~Xchg_Parabola();

    friend class SmartPtr<Xchg_Parabola>;

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
    inline static Xchg_Parabola* DtkDynamicCast(Xchg_Object* inObject)
    {
        if (inObject && inObject->DtkDynamicType(_typeID))
        {
            return static_cast<Xchg_Parabola*>(inObject);
        }
        return nullptr;
    }

public:
    static Xchg_ParabolaPtr Create(const Xchg_Parabola& inToCopy);
    static Xchg_ParabolaPtr Create(const Xchg_pnt& inCenter,
                                   const Xchg_dir& inNormal,
                                   const Xchg_Double64& inMajorRadius);
    static Xchg_ParabolaPtr Create(const Xchg_pnt& inCenter,
                                   const Xchg_dir& inNormal,
                                   const Xchg_dir& inXref,
                                   const Xchg_Double64& inMajorRadius);
    //------------------------- Method -------------------------
    Xchg_pnt GetCenterPoint() const;
    Xchg_dir GetNormalDirection() const;
    Xchg_dir GetOriginDirection() const;
    Xchg_Double64 GetFocalDistance() const;
    const Xchg_dir& GetXDirection() const;
    Xchg_dir GetYDirection() const;
    const Xchg_dir& GetZDirection() const;

    //! \brief Return Curve type
    XCHG_TYPE_ENUM GetType() const
    {
        return static_cast<XCHG_TYPE_ENUM>(_typeID);
    }
    Xchg_ErrorStatus GetDomain(Xchg_Double64& outUmin, Xchg_Double64& outUmax) const override;
    virtual Xchg_bool IsPeriodic() const override;
};
#endif