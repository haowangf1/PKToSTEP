#ifndef _GEOM_NURBS_SURFACE_HPP_
#define _GEOM_NURBS_SURFACE_HPP_

#include "base/xchg_export.hpp"
#include "geom/surface/xchg_surface.hpp"

// Forward declarations
class Xchg_NurbsSurface;
typedef SmartPtr<Xchg_NurbsSurface> Xchg_NurbsSurfacePtr;

//! \ingroup geom
//! \class Xchg_NurbsSurface
//! \brief This is the Nurbs Surface Class.
//! It is part of the Xchg_NurbsSurface entity
//! \warning Please use the Xchg_NurbsSurfacePtr class to handle it...
//!
class XCHG_API Xchg_NurbsSurface : public Xchg_Surface
{
protected:
    enum { _typeID = XCHG_TYPE_NURBS_SURFACE };
    // constructors/destructors
    Xchg_NurbsSurface();

    Xchg_NurbsSurface(const Xchg_NurbsSurface& inSurfaceToCopy);

    // construct as Bezier surface
    Xchg_NurbsSurface(const Xchg_UInt32& inUDegree,
                      const Xchg_UInt32& inVDegree,
                      const Xchg_vector<Xchg_pnt>& inControlPoints);

    // construct as Rational Bezier surface
    Xchg_NurbsSurface(const Xchg_UInt32& inUDegree,
                      const Xchg_UInt32& inVDegree,
                      const Xchg_vector<Xchg_pnt>& inControlPoints,
                      const Xchg_vector<Xchg_Double64>& inWeights);

    Xchg_NurbsSurface(const Xchg_UInt32& inUDegree,
                      const Xchg_UInt32& inVDegree,
                      const Xchg_vector<Xchg_Double64>& inUKnots,
                      const Xchg_vector<Xchg_Double64>& inVKnots,
                      const Xchg_vector<Xchg_pnt>& inControlPoints,
                      const Xchg_bool uclosed,
                      const Xchg_bool vclosed,
                      const Xchg_vector<Xchg_Double64>& inWeights = Xchg_vector<Xchg_Double64>());

    Xchg_NurbsSurface(const Xchg_UInt32& inUDegree,
                      const Xchg_UInt32& inVDegree,
                      const Xchg_vector<Xchg_Double64>& inUKnotsValues,
                      const Xchg_vector<Xchg_Double64>& inVKnotsValues,
                      const Xchg_vector<Xchg_UChar8>& inUKnotsMultiplicities,
                      const Xchg_vector<Xchg_UChar8>& inVKnotsMultiplicities,
                      const Xchg_vector<Xchg_pnt>& inControlPoints,
                      const Xchg_bool uclosed,
                      const Xchg_bool vclosed,
                      const Xchg_vector<Xchg_Double64>& inWeights = Xchg_vector<Xchg_Double64>());

    virtual ~Xchg_NurbsSurface();

    // clone
    virtual Xchg_Object* Clone() const override;

private:
    Xchg_UInt32 _UDegree{};
    Xchg_UInt32 _VDegree{};
    Xchg_vector<Xchg_Double64> _UKnots{};
    Xchg_vector<Xchg_Double64> _VKnots{};
    Xchg_vector<Xchg_UChar8> _UKnotsMult{};
    Xchg_vector<Xchg_UChar8> _VKnotsMult{};
    Xchg_vector<Xchg_pnt> _ControlPoints{};
    Xchg_vector<Xchg_Double64> _Weights{};
    Xchg_vector<Xchg_Double64> _UFlatKnots{};
    Xchg_vector<Xchg_Double64> _VFlatKnots{};
    Xchg_bool _UClosed{false};
    Xchg_bool _VClosed{false};

    Xchg_Size_t _UNumPoints{};
    Xchg_Size_t _VNumPoints{};
    Xchg_bool _UPeriodic{};
    Xchg_bool _VPeriodic{};

    friend class SmartPtr<Xchg_NurbsSurface>;
    void CalNumPointsU();
    void CalNumPointsV();
    void CalKnotsMult(const Xchg_vector<Xchg_Double64>& inKnots,
                      Xchg_vector<Xchg_Double64>& outKnots,
                      Xchg_vector<Xchg_UChar8>& outKnotsMult);
    void CalFlatKnot(const Xchg_vector<Xchg_Double64>& inKnots,
                     const Xchg_vector<Xchg_UChar8>& inKnotsMult,
                     Xchg_vector<Xchg_Double64>& FlatKnots);

    Xchg_Size_t CalFirstUKnotIndex() const;
    Xchg_Size_t CalFirstVKnotIndex() const;
    Xchg_Size_t CalLastUKnotIndex() const;
    Xchg_Size_t CalLastVKnotIndex() const;

    Xchg_Size_t CalNUPoles() const;
    Xchg_Size_t CalNVPoles() const;

    void CompleteUPeriodic();
    void CompleteVPeriodic();

    void CalUPeriodic();
    void CalVPeriodic();

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

    inline static Xchg_NurbsSurface* DtkDynamicCast(Xchg_Object* s)
    {
        if (s && s->DtkDynamicType(_typeID))
        {
            return static_cast<Xchg_NurbsSurface*>(s);
        }
        return nullptr;
    }

    // create methods
    static Xchg_NurbsSurfacePtr Create(const Xchg_NurbsSurface& s);

    static Xchg_NurbsSurfacePtr Create(const Xchg_UInt32& inUDegree,
                                       const Xchg_UInt32& inVDegree,
                                       const Xchg_vector<Xchg_pnt>& inControlPoints);

    static Xchg_NurbsSurfacePtr Create(const Xchg_UInt32& inUDegree,
                                       const Xchg_UInt32& inVDegree,
                                       const Xchg_vector<Xchg_pnt>& inControlPoints,
                                       const Xchg_vector<Xchg_Double64>& inWeights);

    static Xchg_NurbsSurfacePtr Create(const Xchg_UInt32& inUDegree,
                                       const Xchg_UInt32& inVDegree,
                                       const Xchg_vector<Xchg_Double64>& inUKnots,
                                       const Xchg_vector<Xchg_Double64>& inVKnots,
                                       const Xchg_vector<Xchg_pnt>& inControlPoints,
                                       const Xchg_bool uclosed,
                                       const Xchg_bool vclosed,
                                       const Xchg_vector<Xchg_Double64>& inWeights = Xchg_vector<Xchg_Double64>());

    static Xchg_NurbsSurfacePtr Create(const Xchg_UInt32& inUDegree,
                                       const Xchg_UInt32& inVDegree,
                                       const Xchg_vector<Xchg_Double64>& inUKnotsValues,
                                       const Xchg_vector<Xchg_Double64>& inVKnotsValues,
                                       const Xchg_vector<Xchg_UChar8>& inUKnotsMultiplicities,
                                       const Xchg_vector<Xchg_UChar8>& inVKnotsMultiplicities,
                                       const Xchg_vector<Xchg_pnt>& inControlPoints,
                                       const Xchg_bool uclosed,
                                       const Xchg_bool vclosed,
                                       const Xchg_vector<Xchg_Double64>& inWeights = Xchg_vector<Xchg_Double64>());
    // get methods
    virtual enum XCHG_TYPE_ENUM GetType() const override;
    Xchg_Size_t GetNumKnotsU() const;
    Xchg_Size_t GetNumKnotsV() const;
    Xchg_Size_t GetNumPointsU() const;
    Xchg_Size_t GetNumPointsV() const;
    Xchg_UInt32 GetUDegree() const;
    Xchg_UInt32 GetVDegree() const;
    Xchg_UInt32 GetUKnotMultiplicity(const Xchg_Size_t& inIndex) const;
    Xchg_UInt32 GetVKnotMultiplicity(const Xchg_Size_t& inIndex) const;
    Xchg_Double64 GetUKnotValue(const Xchg_Size_t& inIndex) const;
    Xchg_Double64 GetVKnotValue(const Xchg_Size_t& inIndex) const;
    Xchg_Size_t GetNumPoints() const;
    Xchg_Double64 GetWeight(const Xchg_Size_t& inUIndex, const Xchg_Size_t& inVIndex) const;
    const Xchg_pnt& Point(const Xchg_Size_t& inUIndex, const Xchg_Size_t& inVIndex) const;
    Xchg_pnt& Point(const Xchg_Size_t& inUIndex, const Xchg_Size_t& inVIndex);
    virtual Xchg_ErrorStatus GetDomain(Xchg_Double64 outDomain[]) const override;
    virtual Xchg_bool IsUPeriodic() const override;
    virtual Xchg_bool IsVPeriodic() const override;
    Xchg_bool IsUClosed() const;
    Xchg_bool IsVClosed() const;
    void SetUPeriodic();
    void SetVPeriodic();
    Xchg_bool IsRational() const;
};
#endif