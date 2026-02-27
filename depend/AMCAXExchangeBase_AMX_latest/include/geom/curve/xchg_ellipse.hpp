#ifndef _GEOM_ELLIPSE_HPP_
#define _GEOM_ELLIPSE_HPP_

#include "base/xchg_dir.hpp"
#include "base/xchg_export.hpp"
#include "base/xchg_pnt.hpp"
#include "base/xchg_smartptr.hpp"
#include "geom/curve/xchg_curve.hpp"

// Forward declarations
class Xchg_Ellipse;
typedef SmartPtr<Xchg_Ellipse> Xchg_EllipsePtr;

//! \ingroup geom
//! \class  Xchg_Ellipse
//! \brief This is the ellipse and circle Class.
//! parametrisation :[ 0.0 - 2PI ] 0.0  is given by inXref or inOriginDirection
//! \image html Ellipse.png
//! It is part of the Xchg_Curve entity
//! \warning Please use the Xchg_EllipsePtr class to handle it...
//!
class XCHG_API Xchg_Ellipse : public Xchg_Curve
{
protected:
    Xchg_pnt _Center;            // 椭圆中心
    Xchg_dir _Normal;            // 椭圆法线方向（垂直于椭圆平面）
    Xchg_dir _Xref;              // 椭圆X参考方向（指向长轴）
    Xchg_Double64 _MajorRadius;  // 长轴半径
    Xchg_Double64 _MinorRadius;  // 短轴半径

protected:
    enum { _typeIDEllipse = XCHG_TYPE_ELLIPSE, _typeIDCircle = XCHG_TYPE_CIRCLE };
    //------------------------- Constructors and Destructor -------------------------
    Xchg_Ellipse();
    Xchg_Ellipse(const Xchg_Ellipse& inEllipseToCopy);
    Xchg_Ellipse(const Xchg_pnt& inCenter,
                 const Xchg_dir& inNormal,
                 const Xchg_Double64& inMajorRadius,
                 const Xchg_Double64& inMinorRadius);
    Xchg_Ellipse(const Xchg_pnt& inCenter,
                 const Xchg_dir& inNormal,
                 const Xchg_dir& inXref,
                 const Xchg_Double64& inMajorRadius,
                 const Xchg_Double64& inMinorRadius);
    virtual ~Xchg_Ellipse();

    friend class SmartPtr<Xchg_Ellipse>;

public:
    //! \DtkDynamicType
    inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
    {
        if (!IsCircle() && inId == _typeIDEllipse)
        {
            return 1;
        }
        else if (IsCircle() && inId == _typeIDCircle)
        {
            return 1;
        }
        return Xchg_Curve::DtkDynamicType(inId);
    }

    //! \DtkDynamicCast
    inline static Xchg_Ellipse* DtkDynamicCast(Xchg_Object* inObject)
    {
        if (inObject && (inObject->DtkDynamicType(_typeIDEllipse) || inObject->DtkDynamicType(_typeIDCircle)))
        {
            return static_cast<Xchg_Ellipse*>(inObject);
        }
        return nullptr;
    }
    //! \brief Return Curve type
    XCHG_TYPE_ENUM GetType() const
    {
        return IsCircle() ? static_cast<XCHG_TYPE_ENUM>(_typeIDCircle) : static_cast<XCHG_TYPE_ENUM>(_typeIDEllipse);
    }
    //------------------------- Factory Methods -------------------------
    static Xchg_EllipsePtr Create(const Xchg_Ellipse& inEllipseToCopy);
    static Xchg_EllipsePtr Create(const Xchg_pnt& inCenter,
                                  const Xchg_dir& inNormal,
                                  const Xchg_Double64& inMajorRadius,
                                  const Xchg_Double64& inMinorRadius);
    static Xchg_EllipsePtr Create(const Xchg_pnt& inCenter, const Xchg_dir& inNormal, const Xchg_Double64& inRadius);
    static Xchg_EllipsePtr Create(const Xchg_pnt& inCenter,
                                  const Xchg_dir& inNormal,
                                  const Xchg_dir& inXref,
                                  const Xchg_Double64& inMajorRadius,
                                  const Xchg_Double64& inMinorRadius);
    static Xchg_EllipsePtr Create(const Xchg_pnt& inCenter,
                                  const Xchg_dir& inNormal,
                                  const Xchg_dir& inXref,
                                  const Xchg_Double64& inRadius);
    //------------------------- Methods -------------------------
    Xchg_pnt GetCenterPoint() const;
    Xchg_dir GetNormalDirection() const;
    Xchg_dir GetOriginDirection() const;
    Xchg_Double64 GetMajorRadius() const;
    Xchg_Double64 GetMinorRadius() const;
    const Xchg_dir& GetXDirection() const;
    Xchg_dir GetYDirection() const;
    const Xchg_dir& GetZDirection() const;
    Xchg_ErrorStatus GetDomain(Xchg_Double64& outUmin, Xchg_Double64& outUmax) const override;
    Xchg_bool IsCircle() const;
    virtual Xchg_bool IsPeriodic() const override;
};
#endif