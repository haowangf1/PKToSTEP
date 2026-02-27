#ifndef _GEOM_LINE_HPP_
#define _GEOM_LINE_HPP_

#include "base/xchg_dir.hpp"
#include "base/xchg_export.hpp"
#include "base/xchg_pnt.hpp"
#include "base/xchg_smartptr.hpp"
#include "geom/curve/xchg_curve.hpp"

// Forward declarations
class Xchg_Line;
typedef SmartPtr<Xchg_Line> Xchg_LinePtr;

//! \ingroup geom
//! \class  Xchg_Line
//! \brief This is the Infinite Line Class.
//! It is part of the Xchg_Curve entity
//! 0.0 parametrisation is first point or origin
//! 1.0 parametrisation is second point or vector norm
//! \warning Please use the Xchg_LinePtr class to handle it...
//!
class XCHG_API Xchg_Line : public Xchg_Curve
{
private:
    Xchg_pnt _Origin{};     // 直线原点
    Xchg_dir _Direction{};  // 直线方向

protected:
    //! \brief Entity Type
    enum { _typeID = XCHG_TYPE_LINE };

    // Constructors and Destructor
    Xchg_Line();
    Xchg_Line(const Xchg_Line& inLineToCopy);
    Xchg_Line(const Xchg_pnt& inOrigin, const Xchg_dir& inDir);
    Xchg_Line(const Xchg_pnt& inPnt1, const Xchg_pnt& inPnt2);
    virtual ~Xchg_Line();

    //! \brief  smart pointer
    friend class SmartPtr<Xchg_Line>;

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
    inline static Xchg_Line* DtkDynamicCast(Xchg_Object* inObject)
    {
        if (inObject && inObject->DtkDynamicType(_typeID))
        {
            return static_cast<Xchg_Line*>(inObject);
        }
        return nullptr;
    }
    XCHG_TYPE_ENUM GetType() const
    {
        return static_cast<XCHG_TYPE_ENUM>(_typeID);
    }

public:
    //------------------------- Factory Methods -------------------------
    static Xchg_LinePtr Create(const Xchg_Line& inLineToCopy);
    static Xchg_LinePtr Create(const Xchg_pnt& inOrigin, const Xchg_dir& inDir);
    static Xchg_LinePtr Create(const Xchg_pnt& inPnt1, const Xchg_pnt& inPnt2);

    //------------------------- Methods -------------------------
    Xchg_pnt GetOrigin() const;
    Xchg_dir GetDirection() const;
    virtual Xchg_ErrorStatus GetDomain(Xchg_Double64& outUmin, Xchg_Double64& outUmax) const override;
    virtual Xchg_bool IsPeriodic() const override;
};
#endif