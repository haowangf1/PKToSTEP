#ifndef _GEOM_CURVE_HPP_
#define _GEOM_CURVE_HPP_

#include "base/xchg_entity.hpp"
#include "base/xchg_export.hpp"
#include "base/xchg_smartptr.hpp"
#include "base/xchg_type.hpp"

// Forward declarations
class Xchg_Curve;
typedef SmartPtr<Xchg_Curve> Xchg_CurvePtr;

//! \ingroup geom
//! \class Xchg_Curve
//! \brief Xchg_Curve is a geometrical entity representing a curve. It is an abstract class that cannot be instancied.
//! \n \warning Please use the Xchg_CurvePtr class to handle it... It has several utility methods :\n
//!
//!- GetDomain  allow you to get the whole definition domain for the curve.
//!\code
//! Xchg_ErrorStatus GetDomain(Xchg_Double64& outUmin,Xchg_Double64& outUmax) const;
//!\endcode
//!- SetTrimmingParameters/GetTrimUMin/GetTrimUMax  allow you to specify/get the used domain for the curve.
//!\code
//! Xchg_ErrorStatus SetTrimmingParameters(const Xchg_Double64& inUmin,const Xchg_Double64& inUmax);
//! Xchg_Double64 GetTrimUMin() const;
//! Xchg_Double64 GetTrimUMax() const;
//!\endcode
//!- ToNurbs convert the curve into nurbs curve.
//!- ToPolyline convert the curve into polyline .
//!

class XCHG_API Xchg_Curve : public Xchg_Entity
{
private:
    Xchg_Size_t _Dimension{3};

protected:
    enum { _typeID = XCHG_TYPE_CURVE };
    Xchg_transfoPtr _Transfo{nullptr};
    Xchg_Double64 _TrimmedUmin{};
    Xchg_Double64 _TrimmedUmax{};
    Xchg_bool _IsTrimmed{XCHG_FALSE};
    // Constructors and Destructor
    Xchg_Curve() = default;
    Xchg_Curve(const Xchg_Curve& s) = default;
    virtual ~Xchg_Curve() = default;

private:
    friend class SmartPtr<Xchg_Curve>;

public:
    // Constructors and Destructor

    //! \DtkDynamicType
    inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
    {
        if (inId == _typeID)
        {
            return 1;
        }
        return Xchg_Entity::DtkDynamicType(inId);
    }

    //! \DtkDynamicCast
    inline static Xchg_Curve* DtkDynamicCast(Xchg_Object* inObject)
    {
        if (inObject && inObject->DtkDynamicType(_typeID))
        {
            return static_cast<Xchg_Curve*>(inObject);
        }
        return nullptr;
    }
    //! \brief Return Curve type
    virtual XCHG_TYPE_ENUM GetType() const = 0;

    // Dimension
    Xchg_Size_t GetDimension() const
    {
        return _Dimension;
    }

    void SetDimension(const Xchg_Size_t& inDimension)
    {
        _Dimension = inDimension;
    }

    virtual Xchg_ErrorStatus GetDomain(Xchg_Double64& outUmin, Xchg_Double64& outUmax) const = 0;
    virtual Xchg_ErrorStatus SetTrimmingParameters(const Xchg_Double64& inUmin, const Xchg_Double64& inUmax);
    Xchg_Double64 GetTrimUMax() const;
    Xchg_Double64 GetTrimUMin() const;
    Xchg_bool IsTrimmed() const;
    virtual Xchg_bool IsPeriodic() const;

    void SetTransfo(const Xchg_transfoPtr& inTransfo);
    const Xchg_transfoPtr& GetTransfo() const;
};
#endif
