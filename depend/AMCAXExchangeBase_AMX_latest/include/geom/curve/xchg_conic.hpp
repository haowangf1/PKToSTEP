// #ifndef _GEOM_CONIC_HPP_
// #define _GEOM_CONIC_HPP_

// #include "base/xchg_dir.hpp"
// #include "base/xchg_export.hpp"
// #include "base/xchg_pnt.hpp"
// #include "base/xchg_smartptr.hpp"
// #include "base/xchg_type.hpp"
// #include "base/xchg_vector.hpp"
// #include "geom/curve/xchg_curve.hpp"

// // Forward declarations
// class Xchg_Conic;
// typedef SmartPtr<Xchg_Conic> Xchg_ConicPtr;

// //! \ingroup geom
// //! \class  Xchg_Conic
// //! \brief This is the Conic Class.
// //! It is part of the Xchg_Curve entity
// //! \warning Please use the Xchg_ConicPtr class to handle it...
// //!
// class XCHG_API Xchg_Conic : public Xchg_Curve
// {
// protected:
//     enum { _typeID = XCHG_TYPE_CONIC };
//     // Constructors and destructor
//     Xchg_Conic();
//     Xchg_Conic(const Xchg_Conic& inConicToCopy);
//     Xchg_Conic(const Xchg_pnt& InCenter,
//                const Xchg_dir& InNormal,
//                const Xchg_dir& InOriginDir,
//                const Xchg_Double64& InStartAngle);
//     virtual ~Xchg_Conic();

//     friend class SmartPtr<Xchg_Conic>;

// public:
//     //! \brief Return Curve type
//     XCHG_TYPE_ENUM GetType() const
//     {
//         return static_cast<XCHG_TYPE_ENUM>(_typeID);
//     }

//     //! \DtkDynamicType
//     inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
//     {
//         if (inId == _typeID)
//         {
//             return 1;
//         }
//         return Xchg_Curve::DtkDynamicType(inId);
//     }

//     //! \DtkDynamicCast
//     inline static Xchg_Conic* DtkDynamicCast(Xchg_Object* inObject)
//     {
//         if (inObject && inObject->DtkDynamicType(_typeID))
//         {
//             return static_cast<Xchg_Conic*>(inObject);
//         }
//         return nullptr;
//     }

//     static Xchg_ConicPtr Create(const Xchg_pnt& InCenter,
//                                 const Xchg_dir& InNormal,
//                                 const Xchg_dir& InOriginDir,
//                                 const Xchg_Double64& InStartAngle);

//     Xchg_pnt GetCenterPoint() const;
//     Xchg_dir GetNormal() const;
//     Xchg_dir GetOriginDirection() const;
//     Xchg_Double64 GetStartAngle() const;
// };
// #endif