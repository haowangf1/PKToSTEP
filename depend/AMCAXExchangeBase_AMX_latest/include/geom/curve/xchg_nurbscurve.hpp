#ifndef _GEOM_NURBS_CURVE_HPP_
#define _GEOM_NURBS_CURVE_HPP_
#include "base/xchg_dir.hpp"
#include "base/xchg_export.hpp"
#include "base/xchg_pnt.hpp"
#include "base/xchg_smartptr.hpp"
#include "base/xchg_vector.hpp"
#include "geom/curve/xchg_curve.hpp"

// Forward declarations
class Xchg_NurbsCurve;
typedef SmartPtr<Xchg_NurbsCurve> Xchg_NurbsCurvePtr;

//! \ingroup geom
//! \class  Xchg_NurbsCurve
//! \brief This is the Nurbs Class.
//! It is part of the Xchg_Curve entity
//! \warning Please use the Xchg_NurbsCurvePtr class to handle it...
//!
class XCHG_API Xchg_NurbsCurve : public Xchg_Curve
{
private:
    Xchg_UInt32 _Degree{};
    Xchg_vector<Xchg_Double64> _Knots{};
    Xchg_vector<Xchg_UChar8> _KnotsMult{};
    Xchg_vector<Xchg_pnt> _ControlPoints{};
    Xchg_vector<Xchg_Double64> _Weights{};
    Xchg_vector<Xchg_Double64> _FlatKnot{};
    Xchg_bool _Closed{};
    Xchg_bool _Periodic{};

    /// NOT USED
    // enum class KnotDistri { NonUniform, Uniform, QuasiUniform, PiecewiseBezier, };
    // enum class MultDistri { NonConstant, Constant, QuasiConstant, };

    void CalKnotsMult(const Xchg_vector<Xchg_Double64>& inKnots);
    void CalFlatKnot();
    int CalFirstKnotIndex() const;
    int CalLastKnotIndex() const;
    int CalNPoles() const;

    /// NOT USED
    // KnotDistri CalKnotForm() const;
    // MultDistri CalMultForm() const;
    // int CalKnotSeqLenPeriodic() const;
    // void CalFlatKnotPeriodic();
    // void AnalysisKnots(KnotDistri& form, int& maxMult) const;
    // void UpdateKnots();

    Xchg_bool CheckCurveDataPeriodic() const;
    void CompletePeriodic();
    void CalPeriodic();

protected:
    enum { _typeID = XCHG_TYPE_NURBS_CURVE };

    // Constructors and Destructor
    Xchg_NurbsCurve();
    Xchg_NurbsCurve(const Xchg_NurbsCurve& s);

    // Construct as Bezier
    Xchg_NurbsCurve(const Xchg_UInt32& inDegree, const Xchg_vector<Xchg_pnt>& inControlPoints);

    // Construct as Rational Bezier
    Xchg_NurbsCurve(const Xchg_UInt32& inDegree,
                    const Xchg_vector<Xchg_pnt>& inControlPoints,
                    const Xchg_vector<Xchg_Double64>& inWeights);

    Xchg_NurbsCurve(const Xchg_UInt32& inDegree,
                    const Xchg_vector<Xchg_Double64>& inKnots,
                    const Xchg_vector<Xchg_pnt>& inControlPoints,
                    const Xchg_vector<Xchg_Double64>& inWeights = Xchg_vector<Xchg_Double64>());

    Xchg_NurbsCurve(const Xchg_UInt32& inDegree,
                    const Xchg_vector<Xchg_Double64>& inKnotsValues,
                    const Xchg_vector<Xchg_UChar8>& inKnotsMultiplicities,
                    const Xchg_vector<Xchg_pnt>& inControlPoints,
                    const Xchg_vector<Xchg_Double64>& inWeights = Xchg_vector<Xchg_Double64>());
    virtual ~Xchg_NurbsCurve();

    friend class SmartPtr<Xchg_NurbsCurve>;

public:
    //------------------------- Factory Methods -------------------------
    static Xchg_NurbsCurvePtr Create(const Xchg_NurbsCurve& s);

    static Xchg_NurbsCurvePtr Create(const Xchg_UInt32& inDegree, const Xchg_vector<Xchg_pnt>& inControlPoints);

    static Xchg_NurbsCurvePtr Create(const Xchg_UInt32& inDegree,
                                     const Xchg_vector<Xchg_pnt>& inControlPoints,
                                     const Xchg_vector<Xchg_Double64>& inWeights);

    static Xchg_NurbsCurvePtr Create(const Xchg_UInt32& inDegree,
                                     const Xchg_vector<Xchg_Double64>& inKnots,
                                     const Xchg_vector<Xchg_pnt>& inControlPoints,
                                     const Xchg_vector<Xchg_Double64>& inWeights = Xchg_vector<Xchg_Double64>());

    static Xchg_NurbsCurvePtr Create(const Xchg_UInt32& inDegree,
                                     const Xchg_vector<Xchg_Double64>& inKnots,
                                     const Xchg_vector<Xchg_UChar8>& inMultiplicities,
                                     const Xchg_vector<Xchg_pnt>& inControlPoints,
                                     const Xchg_vector<Xchg_Double64>& inWeights = Xchg_vector<Xchg_Double64>());
    //------------------------- Methods -------------------------
    Xchg_UInt32 GetDegree() const;
    Xchg_Size_t GetNumKnots() const;
    Xchg_Double64 GetKnotValue(const Xchg_Size_t& inIndex) const;
    Xchg_UInt32 GetKnotMultiplicity(const Xchg_Size_t& inIndex) const;
    Xchg_Size_t GetNumPoints() const;
    Xchg_pnt& Point(const Xchg_Size_t& inIndex);
    const Xchg_pnt& Point(const Xchg_Size_t& inIndex) const;
    Xchg_Double64 GetWeight(const Xchg_Size_t& inIndex) const;

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
    inline static Xchg_NurbsCurve* DtkDynamicCast(Xchg_Object* inObject)
    {
        if (inObject && inObject->DtkDynamicType(_typeID))
        {
            return static_cast<Xchg_NurbsCurve*>(inObject);
        }
        return nullptr;
    }

    //! \brief Return Curve type
    XCHG_TYPE_ENUM GetType() const
    {
        return static_cast<XCHG_TYPE_ENUM>(_typeID);
    }

    virtual Xchg_ErrorStatus GetDomain(Xchg_Double64& outUmin, Xchg_Double64& outUmax) const override;
    virtual Xchg_bool IsPeriodic() const override;
    // check and adjust knots / mults, so that a periodic curve can be created.
    Xchg_bool SetPeriodic();
    void SetClosed(Xchg_bool closed);
    Xchg_bool IsClosed() const;
    Xchg_bool IsRational() const;
};
#endif