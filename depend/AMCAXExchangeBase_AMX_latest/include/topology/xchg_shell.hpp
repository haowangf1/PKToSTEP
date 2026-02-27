#ifndef _XCHG_SHELL_HPP_
#define _XCHG_SHELL_HPP_

#include "base/xchg_export.hpp"
#include "topology/xchg_topologicalentity.hpp"
#include "topology/xchg_face.hpp"
#include "base/xchg_vector.hpp"

// Forward declarations
class Xchg_Lump;
typedef SmartPtr<Xchg_Lump> Xchg_LumpPtr;

// Define face type enum
enum DtkFaceType
{
	DtkFaceType_All,
	DtkFaceType_Bounded,
	DtkFaceType_Wire
};

class Xchg_Shell;
typedef SmartPtr<Xchg_Shell> Xchg_ShellPtr;

//! \ingroup TOPO
//! \class Xchg_Shell 
//! \brief Xchg_Shell is a set of faces bounding a volume. Xchg_Shell can be outer or inner.
//! A Xchg_Body contain at least one open_shell or one outer closed shell and 0 or several inner closed shell.
class XCHG_API Xchg_Shell : public Xchg_TopologicalEntity
{
protected:
	struct Xchg_Handle;       // Not defined here
	Xchg_Handle* _Private = nullptr;   // Handle
	enum { _typeID = XCHG_TYPE_SHELL };

	void _Init();
	void _Reset();
	void _Clone(Xchg_TopologicalEntityPtr& outTopo, Xchg_Body* _inParentBody) override;

	void _Copy(const Xchg_Shell& inTopo, Xchg_Body* inParentBody);

	Xchg_Shell();
	Xchg_Shell(const Xchg_Shell& inTopo, Xchg_Body* inParentBody);
	virtual ~Xchg_Shell();
	
	friend class SmartPtr<Xchg_Shell>;
	friend class Xchg_TopologicalEntity;
	friend class Xchg_Body; // For ExtractEdgeBody

	Xchg_ErrorStatus SetFace(const Xchg_Size_t& inIndex, const Xchg_FacePtr& inFace, const Xchg_bool& inFaceOrientation);

public:
	Xchg_ID GetTopoID() const override;
	Xchg_ErrorStatus RemoveFaceFromShell(const Xchg_FacePtr& inFace);
	Xchg_ErrorStatus InverseFaceInShell(const Xchg_FacePtr& inFace);

	Xchg_ErrorStatus SetParentBody(const Xchg_BodyPtr& inParentBody) override;
	Xchg_ErrorStatus SetParentBody(Xchg_Body* inParentBody);

	//! \brief Create a shell in a body
	//! \param [in] inParentBody : parent body
	//! \return static Xchg_ShellPtr 
	static Xchg_ShellPtr Create(const Xchg_BodyPtr& inParentBody);
	
	//! \brief Create a shell in a body
	//! \param [in] inParentBody : parent body
	//! \param [in] inFacesReserve : Faces size to reserve
	//! \param [in] inWireReserve : Wire size to reserve
	//! \return static Xchg_ShellPtr 
	static Xchg_ShellPtr Create(const Xchg_BodyPtr& inParentBody, Xchg_UInt32 inFacesReserve, Xchg_UInt32 inWireReserve = 0);

	//! \DtkDynamicType
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_TopologicalEntity::DtkDynamicType(inId);
	}
	
	//! \DtkDynamicCast
	inline static Xchg_Shell* DtkDynamicCast(Xchg_Object* inObject)
	{
		if (inObject && inObject->DtkDynamicType(_typeID))
			return static_cast<Xchg_Shell*>(inObject);
		return NULL;
	}

	//! \brief Get Parent Body
	//! \param [out] outParentBody : parent body
	//! \return Xchg_ErrorStatus: dtkNoError if OK     
	Xchg_ErrorStatus GetParentBody(Xchg_BodyPtr& outParentBody) const;
	
	//! \brief Return parent lump of shell
	//! \param [out] outParentLump : pointer of parent lump
	//! \return Xchg_ErrorStatus: dtkNoError if OK 
	Xchg_ErrorStatus GetParentLump(Xchg_LumpPtr& outParentLump) const;
	
	//! \brief Set parent lump of shell
	//! \param [in] inParentLump : pointer of parent lump
	//! \return Xchg_ErrorStatus: dtkNoError if OK 
	Xchg_ErrorStatus SetParentLump(const Xchg_LumpPtr& inParentLump);
	Xchg_ErrorStatus SetParentLump(const Xchg_ID& inParentLumpId);

	Xchg_ErrorStatus GetTopologyStatus() const;
	void SetTopologyStatus(Xchg_ErrorStatus inStatus);

	//! \brief Return XCHG_TRUE if shell is closed
	Xchg_bool IsClosed() const;
	
	//! \brief Set Closed Info XCHG_TRUE if shell is closed
	//! \return Xchg_ErrorStatus: dtkNoError if OK 
	Xchg_ErrorStatus SetClosedInfo(const Xchg_bool& inClosedInfo);
	
	//! \brief Return XCHG_TRUE if shell is outer
	Xchg_bool IsOuter() const;
	
	//! \brief Return XCHG_TRUE if shell has Wire entities
	Xchg_bool HasWire() const;
	
	//! \brief Set Outer Info XCHG_TRUE if shell is outer
	//! \return Xchg_ErrorStatus: dtkNoError if OK 
	Xchg_ErrorStatus SetOuterInfo(const Xchg_bool& inOuterInfo);
	
	//! \brief Return Number of Faces following Face type
	//! \param [in] inFaceType : Type of face you want to count 
	//! ( DtkFaceType_Bounded = topological face / DtkFaceType_All = All Faces / DtkFaceType_Wire = Face used to store wire entities)
	//! \return number of face following Face type
	Xchg_Size_t GetNumFaces(DtkFaceType inFaceType = DtkFaceType_Bounded) const;

	//! \brief Return Number of Wire set of entities
	//! equivalent to GetNumFaces with type DtkFaceType_Wire 
	//! \return number of Wire set of entities
	Xchg_Size_t GetNumWireSet() const;

	//! \brief Get Wire Set
	//! \param [in] inIndex : index of wire set
	//! \param [out] outWire : array of Xchg_Entity (must be Xchg_PointPtr or Xchg_CurvePtr )
	//! \return Xchg_ErrorStatus: dtkNoError if OK 
	Xchg_ErrorStatus GetWireSet(const Xchg_Size_t& inIndex, Xchg_vector<Xchg_EntityPtr>& outWire) const;

	//! \brief Get Wire Set
	//! \param [in] inIndex : index of wire set
	//! \param [out] outWire : array of Xchg_Entity (must be Xchg_VertexPtr or Xchg_EdgePtr )
	//! \param [out] outOrientation : array of bool for edge orientation 
	//! \return Xchg_ErrorStatus: dtkNoError if OK 
	Xchg_ErrorStatus GetWireSetTopo(const Xchg_Size_t& inIndex, Xchg_vector<Xchg_EntityPtr>& outWire, Xchg_vector<Xchg_bool>& outOrientation, Xchg_bool& outLoopOrientation) const;
	Xchg_ErrorStatus GetWireSetTopo(const Xchg_Size_t& inIndex, Xchg_vector<Xchg_EntityPtr>& outWire) const;

	//! \brief Get Face following Face type
	//! \param [in] inIndex : index of face
	//! \param [out] outFace : Face
	//! \param [out] outOrientation : Face orientation relative to shell
	//! \param [in] inFaceType : Type of face ( DtkFaceType_Bounded = topological face / DtkFaceType_All = All Faces / DtkFaceType_Wire = Face used to store wire entities)
	//! \return Xchg_ErrorStatus: dtkNoError if OK 
	Xchg_ErrorStatus GetFace(const Xchg_Size_t& inIndex, Xchg_FacePtr& outFace, Xchg_bool& outOrientation, DtkFaceType inFaceType = DtkFaceType_Bounded) const;
	Xchg_ErrorStatus GetFace(const Xchg_Size_t& inIndex, Xchg_ID& outFaceId, Xchg_bool& outOrientation, DtkFaceType inFaceType = DtkFaceType_Bounded) const;

	//! \brief Add Face to shell
	//! \param [in] inFace : Face
	//! \param [in] inOrientation : Face orientation relative to shell (XCHG_TRUE if face material and shell material are on same side)
	//! \param [in] inFaceType : Type of face ( DtkFaceType_Bounded = topological face or DtkFaceType_Wire = Face used to store wire entities)
	//! \return Xchg_ErrorStatus: dtkNoError if OK 
	Xchg_ErrorStatus AddFace(const Xchg_FacePtr& inFace, Xchg_bool inFaceOrientation, DtkFaceType inFaceType = DtkFaceType_Bounded);
	
	//! \brief Add Wire entities
	//! \param [in] inWireArray : Array of Xchg_Curves
	//! \return Xchg_ErrorStatus: dtkNoError if OK 
	Xchg_ErrorStatus AddWire(Xchg_vector<Xchg_CurvePtr> inCurveArray);
	Xchg_ErrorStatus AddWire(Xchg_vector<Xchg_EdgePtr> inCurveArray);

	//! \brief Add Wire entities
	//! \param [in] inPointArray : Array of 3D Points
	//! \return Xchg_ErrorStatus: dtkNoError if OK 
	Xchg_ErrorStatus AddWire(Xchg_vector<Xchg_PointPtr> inPointArray);
	Xchg_ErrorStatus AddWire(Xchg_vector<Xchg_VertexPtr> inPointArray);

	//! \brief Check if Shell is Closed
	//! \return XCHG_TRUE if closed
	Xchg_bool CheckIfClosed();

	enum XCHG_TYPE_ENUM GetType() const override;
};

#endif // _XCHG_SHELL_HPP_

