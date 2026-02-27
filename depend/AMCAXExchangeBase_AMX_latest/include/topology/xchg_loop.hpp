#ifndef _XCHG_LOOP_HPP_
#define _XCHG_LOOP_HPP_

#include "base/xchg_export.hpp"
#include "topology/xchg_topologicalentity.hpp"
#include "topology/xchg_coedge.hpp"
#include "topology/xchg_vertex.hpp"
#include "base/xchg_vector.hpp"

// Forward declarations
class Xchg_Face;
typedef SmartPtr<Xchg_Face> Xchg_FacePtr;

class Xchg_Loop;
typedef SmartPtr<Xchg_Loop> Xchg_LoopPtr;

//! \ingroup TOPO
//! \class Xchg_Loop 
//! \brief Xchg_Loop is bound of a face it can be outer or inner
class XCHG_API Xchg_Loop : public Xchg_TopologicalEntity
{
protected:
	struct Xchg_Handle;       // Not defined here
	Xchg_Handle* _Private = nullptr;   // Handle
	enum { _typeID = XCHG_TYPE_LOOP }; 

	void _Init();
	void _Reset();
	Xchg_ErrorStatus SetCoedge(const Xchg_Size_t& inIndex, const Xchg_CoedgePtr& inCoedge, const Xchg_bool& inOrientation);

	void _Copy(const Xchg_Loop& inTopo, Xchg_Body* inParentBody);
	void _Clone(Xchg_TopologicalEntityPtr& outTopo, Xchg_Body* _inParentBody) override;

	Xchg_Loop();
	Xchg_Loop(const Xchg_Loop& inTopo, Xchg_Body* inParentBody);
	virtual ~Xchg_Loop();
	Xchg_ErrorStatus GetParentBody(Xchg_Body*& outParentBody) const;
	
	friend class SmartPtr<Xchg_Loop>;
	friend class Xchg_TopologicalEntity;

public:
	Xchg_ID GetTopoID() const override;
	Xchg_ErrorStatus SetParentBody(const Xchg_BodyPtr& inParentBody) override;
	Xchg_ErrorStatus RemoveCoedgeFromLoop(const Xchg_CoedgePtr& inCoedge);

	//! \DtkDynamicType
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_TopologicalEntity::DtkDynamicType(inId);
	}
	
	//! \DtkDynamicCast
	inline static Xchg_Loop* DtkDynamicCast(Xchg_Object* s)
	{
		if (s && s->DtkDynamicType(_typeID))
			return static_cast<Xchg_Loop*>(s);
		return NULL;
	}

	Xchg_ErrorStatus GetTopologyStatus() const;
	void SetTopologyStatus(Xchg_ErrorStatus inStatus);
	Xchg_ErrorStatus SetOuterInfo(const Xchg_bool& inIsOuter);
	enum XCHG_TYPE_ENUM GetType() const override;

	//! \brief Create a loop in a body
	//! \param [in] inParentBody : parent body
	//! \return static Xchg_LoopPtr 
	static Xchg_LoopPtr Create(const Xchg_BodyPtr& inParentBody);

	//! \brief Create a loop in a body
	//! \param [in] inParentBody : parent body
	//! \param [in] inCoedgeReserve : Coedge size to reserve
	//! \return static Xchg_LoopPtr 
	static Xchg_LoopPtr Create(const Xchg_BodyPtr& inParentBody, Xchg_UInt32 inCoedgeReserve);

	//! \brief Return true if Loop is outer
	Xchg_bool IsOuter() const;
	
	//! \brief Return true if Loop is a vertex loop
	Xchg_bool IsVertexLoop() const;

	//! \brief Return orientation for Loop relative to face
	//! \return True if material is on the left
	Xchg_bool GetOrientation() const;
	
	//! \brief Set orientation for Loop relative to face
	//! \param [in] :inOrientation True if material is on the left
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus SetOrientation(const Xchg_bool& inOrientation);

	//! \brief Return number of coedge in the loop
	Xchg_Size_t GetNumCoedges() const;
	
	//! \brief Get a coedge in the loop
	//! \param [in] : inIndex index of coedge to get
	//! \param [out] : outCoedge : pointer on coedge you ask
	//! \param [out] : outCoedgeOrientation : orientation of coedge in the loop
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus GetCoedge(const Xchg_Size_t& inIndex, Xchg_CoedgePtr& outCoedge, Xchg_bool& outCoedgeOrientation) const;
	Xchg_ErrorStatus GetCoedge(const Xchg_Size_t& inIndex, Xchg_ID& outCoedgeId, Xchg_bool& outCoedgeOrientation) const;

	//! \brief Get an index of a coedge in the loop
	//! \param [in] : inCoedge : pointer on coedge you want the index
	//! \param [out] : outIndex index of the coedge
	//! \return Xchg_ErrorStatus: dtkNoError if found
	Xchg_ErrorStatus GetCoedgeIndex(Xchg_CoedgePtr& inCoedge, Xchg_Size_t& outIndex) const;

	//! \brief Add coedge to the loop
	//! \param [in] : inCoedge : pointer on coedge you add
	//! \param [in] : inCoedgeOrientation : orientation of coedge in the loop (true is loop and coedge go in same sens)
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus AddCoedge(const Xchg_CoedgePtr& inCoedge, const Xchg_bool& inCoedgeOrientation);
	Xchg_ErrorStatus SetCoedges(const Xchg_vector<Xchg_CoedgePtr>& inCoedgeArray, const Xchg_vector<Xchg_bool>& inCoedgeOrientationArray);

	//! \brief Return vertex loop
	//! \param [out] : outVertexLoop : pointer on vertex loop
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus GetVertexLoop(Xchg_VertexPtr& outVertexLoop) const;
	
	//! \brief Set vertex loop
	//! \param [in] : inVertexLoop : pointer on vertex loop
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus SetVertexLoop(const Xchg_VertexPtr inVertexLoop);

	//! \brief Return pointer on parent face 
	//! \param [out]: outParentFace : parent face
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus GetParentFace(Xchg_FacePtr& outParentFace) const;

	//! \brief Set pointer on parent face 
	//! \param [in]: inParentFace : parent face
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus SetParentFace(const Xchg_FacePtr& inParentFace);
	Xchg_ErrorStatus SetParentFace(const Xchg_ID& inParentFaceId);

	//! \brief Inverse sens of all coedges in the loop
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus InverseCoedgeOrientation() const;


};

#endif // _XCHG_LOOP_HPP_

