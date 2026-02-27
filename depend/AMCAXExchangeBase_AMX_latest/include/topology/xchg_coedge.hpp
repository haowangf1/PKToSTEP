#ifndef _XCHG_COEDGE_HPP_
#define _XCHG_COEDGE_HPP_

#include "base/xchg_export.hpp"
#include "topology/xchg_topologicalentity.hpp"
#include "topology/xchg_edge.hpp"
#include "geom/geom.hpp"

// Forward declarations
class Xchg_Loop;
typedef SmartPtr<Xchg_Loop> Xchg_LoopPtr;

class Xchg_Face;
typedef SmartPtr<Xchg_Face> Xchg_FacePtr;

class Xchg_Coedge;
typedef SmartPtr<Xchg_Coedge> Xchg_CoedgePtr;

//! \ingroup TOPO
//! \class Xchg_Coedge 
//! \brief Xchg_Coedge is a part of a loop. It's geometry is a UV curve on a surface
//! \brief Orientation is topological orientation relative to Edge
//! \brief Geometrical orientation relative to UV Curve is always True
class XCHG_API Xchg_Coedge : public Xchg_TopologicalEntity
{
protected:	
	struct Xchg_Handle;       // Not defined here
	Xchg_Handle* _Private = nullptr;   // Handle
	enum { _typeID = XCHG_TYPE_COEDGE }; 

	void _Init();
	void _Reset();
	void _Copy(const Xchg_Coedge& inTopo, Xchg_Body* inParentBody);
	void _Clone(Xchg_TopologicalEntityPtr& outTopo, Xchg_Body* _inParentBody) override;

	Xchg_Coedge();
	Xchg_Coedge(const Xchg_Coedge& inCoedgeToCopy, Xchg_Body* inParentBody);
	virtual ~Xchg_Coedge();
	
	friend class SmartPtr<Xchg_Coedge>;
	friend class Xchg_TopologicalEntity;

public:
	Xchg_ID GetTopoID() const override;
	Xchg_ErrorStatus SetParentBody(const Xchg_BodyPtr& inParentBody) override;

	//! \brief Create a coedge in a body
	//! \param [in] inParentBody : parent body
	//! \return static Xchg_CoedgePtr 
	static Xchg_CoedgePtr Create(const Xchg_BodyPtr& inParentBody);

	//! \DtkDynamicType
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_TopologicalEntity::DtkDynamicType(inId);
	}
	
	//! \DtkDynamicCast
	inline static Xchg_Coedge* DtkDynamicCast(Xchg_Object* inObject)
	{
		if (inObject && inObject->DtkDynamicType(_typeID))
			return static_cast<Xchg_Coedge*>(inObject);
		return NULL;
	}

	//! \brief Return orientation for coedge relative to Edge
	//! \return True if coedge and associated Edge has same sens
	Xchg_bool GetOrientation() const;
	
	//! \brief Set orientation for coedge relative to Edge
	//! \param [in] :inOrientation true if coedge and associated Edge has same sens
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus SetOrientation(const Xchg_bool& inOrientation);

	//! \brief Indicates if Coedge is degenerated or not 
	//! \return XCHG_TRUE, XCHG_FALSE, or XCHG_BOOL_UNKNOWN 
	Xchg_bool IsDegenerated() const;
	
	//! \brief Active Degenerated flag for coedge 
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus SetDegeneratedFlag();

	//! \brief Get Edge for coedge 
	//! \param [out] :outEdge Corresponding Edge
	//! \return Xchg_ErrorStatus: dtkNoError if OK 
	Xchg_ErrorStatus GetEdge(Xchg_EdgePtr& outEdge) const;

	//! \brief Set Edge for coedge 
	//! \param [in] :inEdge Corresponding Edge
	//! \return Xchg_ErrorStatus: dtkNoError if OK 
	Xchg_ErrorStatus SetEdge(const Xchg_EdgePtr& inEdge);
	Xchg_ErrorStatus SetEdge(const Xchg_ID& inEdgeId);

	//! \brief Get Number of adjacent coedges
	//! \return Number of adjacent coedges
	Xchg_Size_t GetNumAdjacentCoedges() const;

	//! \brief Get adjacent coedges from index
	//! \param [in] :inIndex coedge index
	//! \param [out] :outAdjacentCoedge coedge pointer
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus GetAdjacentCoedge(const Xchg_Size_t& inIndex, Xchg_CoedgePtr& outAdjacentCoedge) const;

	//! \brief Get parent loop 
	//! \param [out] :outParentLoop 
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus GetParentLoop(Xchg_LoopPtr& outParentLoop) const;
	
	//! \brief Get parent face 
	//! \param [out] :outParentFace 
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus GetParentFace(Xchg_FacePtr& outParentFace) const;

	//! \brief Get basis surface 
	//! \param [out] :outBasisSurface 
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus GetBasisSurface(Xchg_SurfacePtr& outBasisSurface) const;

	//! \brief Set parent loop 
	//! \param [in] :inParentLoop 
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus SetParentLoop(const Xchg_LoopPtr& inParentLoop);
	Xchg_ErrorStatus SetParentLoop(const Xchg_ID& inParentLoopId);

	enum XCHG_TYPE_ENUM GetType() const override;
	
	//! \brief Get Geometry
	//! \return Xchg_CurvePtr
	Xchg_CurvePtr GetGeom() const;
	
	//! \brief Set Geometry
	//! \param [in] inCurveUV : Curve UV 
	void SetGeom(const Xchg_CurvePtr& inCurveUV);

	Xchg_bool HasDegeneratedFlag();
}; 

#endif // _XCHG_COEDGE_HPP_

