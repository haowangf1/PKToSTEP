#ifndef _XCHG_FACE_HPP_
#define _XCHG_FACE_HPP_

#include "base/xchg_export.hpp"
#include "topology/xchg_topologicalentity.hpp"
#include "topology/xchg_loop.hpp"
#include "geom/geom.hpp"

// Forward declarations
class Xchg_Shell;
typedef SmartPtr<Xchg_Shell> Xchg_ShellPtr;

class Xchg_Face;
typedef SmartPtr<Xchg_Face> Xchg_FacePtr;

//! \ingroup TOPO
//! \class Xchg_Face 
//! \brief is a surface bounded by Xchg_Loop. 
//! \brief Geometrical orientation relative to basis surface is always True
class XCHG_API Xchg_Face : public Xchg_TopologicalEntity
{
protected:
	struct Xchg_Handle;       // Not defined here
	Xchg_Handle* _Private = nullptr;   // Handle
	enum { _typeID = XCHG_TYPE_FACE }; 

	void _Init();
	void _Reset();
	void _Copy(const Xchg_Face& inTopo, Xchg_Body* inParentBody);
	Xchg_ErrorStatus SetLoop(const Xchg_Size_t& inIndex, const Xchg_LoopPtr& inLoop);
	void _Clone(Xchg_TopologicalEntityPtr& outTopo, Xchg_Body* _inParentBody) override;

	friend class SmartPtr<Xchg_Face>;
	friend class Xchg_TopologicalEntity;

	//constructors
	Xchg_Face();
	Xchg_Face(const Xchg_Face& inTopo, Xchg_Body* inParentBody);

	virtual ~Xchg_Face();
	
public:
	Xchg_ID GetTopoID() const override;
	Xchg_ErrorStatus SetParentBody(const Xchg_BodyPtr& inParentBody) override;
	Xchg_ErrorStatus RemoveLoopFromFace(const Xchg_LoopPtr& inLoop);

	Xchg_BodyPtr GetParentBody() const;

	//! \brief Create a face in a body
	//! \param [in] inParentBody : parent body
	//! \return static Xchg_FacePtr 
	static Xchg_FacePtr Create(const Xchg_BodyPtr& inParentBody);

	//! \DtkDynamicType
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_TopologicalEntity::DtkDynamicType(inId);
	}
	
	//! \DtkDynamicCast
	inline static Xchg_Face* DtkDynamicCast(Xchg_Object* inObject)
	{
		if (inObject && inObject->DtkDynamicType(_typeID))
			return static_cast<Xchg_Face*>(inObject);
		return NULL;
	}

	Xchg_ErrorStatus GetTopologyStatus() const;
	void SetTopologyStatus(Xchg_ErrorStatus inStatus);

	//! \brief Return number of parent shell (1 if manifold body) 
	Xchg_Size_t GetNumParentShells() const;

	//! \brief Get parent shell of face
	//! \param [in] : inIndex : index of parent shell 
	//! \param [out] : outParentShell : parent shell 
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus GetParentShell(const Xchg_Size_t& inIndex, Xchg_ShellPtr& outParentShell) const;

	//! \brief Get parent shell of face
	//! \param [in] : inIndex : index of parent shell 
	//! \param [out] : outParentShell : parent shell 
	//! \param [out] : outOrient : face orientation in parent shell 
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus GetParentShell(const Xchg_Size_t& inIndex, Xchg_ShellPtr& outParentShell, Xchg_bool& outOrient) const;

	//! \brief Add parent shell on a face
	//! \param [in] : inParentShell shell to add
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus AddParentShell(const Xchg_ShellPtr& inParentShell);
	Xchg_ErrorStatus AddParentShell(const Xchg_ID& inParentShellId);

	//! \brief Return number of bounding loop for face
	Xchg_Size_t GetNumLoops();

	//! \brief Get loop of face
	//! \param [in] : inIndex : index of loop 
	//! \param [out] : outLoop : loop
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus GetLoop(const Xchg_Size_t& inIndex, Xchg_LoopPtr& outLoop);
	Xchg_ErrorStatus GetLoop(const Xchg_Size_t& inIndex, Xchg_ID& outLoopId); 
	
	//! \brief Add Degenerated UV curves in loops
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus AddDegeneratedEdges() const;

	//! \brief Add inner loop on a face
	//! \param [in] : inLoop : inner loop to add
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus AddInnerLoop(const Xchg_LoopPtr& inLoop);
	
	//! \brief Add outer loop on a face
	//! \param [in] : inLoop : outer loop to add
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus AddOuterLoop(const Xchg_LoopPtr& inLoop);
	
	//! \brief Add vertex loop on a face
	//! \param [in] : inLoop : vertex loop to add
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus AddVertexLoop(const Xchg_VertexPtr& inVertexLoop);
	
	//! \brief Add loop on a face
	//! \param [in] : inLoop : loop to add
	//! \param [in] : inOuterInfo : outer info for inLoop (True if outer)
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus AddLoop(const Xchg_LoopPtr& inLoop, const Xchg_bool& inOuterInfo);

	enum XCHG_TYPE_ENUM GetType() const override;

	//! \brief Return associated geometry 
	//! \param [in] : inWithSetTrim if XCHG_TRUE surface set trimuvbox like the face
	//! \return Xchg_SurfacePtr 
	Xchg_SurfacePtr GetGeom(Xchg_bool inWithSetTrim = XCHG_TRUE) const;
	
	//! \brief Set Geometry
	//! \param [in] inSurface : Basis Surface 
	//! \return void 
	void SetGeom(const Xchg_SurfacePtr& inSurface);

	//! \brief Get Surface Trim Box : Umin = outTrimValues[0] Umax = outTrimValues[1] Vmin = outTrimValues[2] Vmax = outTrimValues[3]
	Xchg_ErrorStatus GetTrimUVBox(Xchg_Double64 outTrimValues[]) const; 
	
	//! \brief Set Surface Trim Box : Umin = inTrimValues[0] Umax = inTrimValues[1] Vmin = inTrimValues[2] Vmax = inTrimValues[3]
	Xchg_ErrorStatus SetTrimUVBox(Xchg_Double64 inTrimValues[]);

	//! \brief Create a new face with reversed surface
	//! \return a new single face with reversed normal 
	Xchg_FacePtr ReverseNormal();


	//! \brief Remove a Shell to Shell's list of face
	//! \param [in] :inShellId 
	void RemoveParentShell(const Xchg_ID& inShellId);
};

#endif // _XCHG_FACE_HPP_

