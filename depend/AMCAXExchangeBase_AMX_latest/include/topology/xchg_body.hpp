#ifndef _XCHG_BODY_HPP_
#define _XCHG_BODY_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_entity.hpp"
#include "topology/xchg_shell.hpp"
#include "topology/xchg_lump.hpp"
#include "base/xchg_stream.hpp"
#include "base/xchg_vector.hpp"
#include <cstdio>

class Xchg_Body;
typedef SmartPtr<Xchg_Body> Xchg_BodyPtr;

//! \brief Body type enumeration, compatible with Parasolid body types
//! \remark Matches PK_BODY_type_t in Parasolid
enum Xchg_BodyType
{
    Xchg_BodyType_Empty = 0,      // Empty body (0D. An empty body has an unbounded void regicon that does not contain any components)
    Xchg_BodyType_Acorn,          // Acorn body (0D. One or more acorn components. An acorn body has one infinite void region, containing any number of shells, each of which contains only a single vertex. A minimum body is a special case of an acorn body containing exactly one isolated vertex)
    Xchg_BodyType_Wire,           // Wire body (1D. One or more wire components. A wire body has one infinite void region containing any number of shells, each of which contains a single set of connected edges)
    Xchg_BodyType_Sheet,          // Sheet body (2D. One or more sheet components. A sheet body that contains only open components has one infinite void region containing any number of shells, each of which contains a single set of connected faces. For each closed component, there is an extra bounded void region representing the interior of the closed component)
    Xchg_BodyType_Solid,          // Solid body (3D. One or more solid components. Each solid component has a coontinuous bound volume. A solid body has one infinite void regic, one solid region for each continuous solid volume, ume, and one void region for each bounded continuous surface void volume)
    Xchg_BodyType_General         // General body (mixed or complex topology)
};

//! \ingroup TOPO
//! \class Xchg_Body 
//! \brief A Xchg_Body Xchg_Body is the highest level topological entity. 
//! It allow you to handle manifold and non-manifold object, open skin or surfacic model and wire entities.
class XCHG_API Xchg_Body : public Xchg_Entity
{
protected:
	struct Xchg_Handle;       // Not defined here
	Xchg_Handle* _Private = nullptr;   // Handle
	enum { _typeID = XCHG_TYPE_BODY }; 
	
	void _Init();
	void _Reset();
	void _Copy(const Xchg_Body& inBodyToCopy);
	
	Xchg_Body(const Xchg_Body& inBodyToCopy);
	virtual ~Xchg_Body();
	
	friend class SmartPtr<Xchg_Body>;
	inline virtual Xchg_Object* Clone() const override { return new Xchg_Body(*this); }
	
public:
	Xchg_Body();

	//! \brief Create a body
	//! \return static Xchg_BodyPtr 
	static Xchg_BodyPtr Create();

	XCHG_TYPE_ENUM GetType() const { return XCHG_TYPE_BODY;};

	//! \DtkDynamicType
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_Entity::DtkDynamicType(inId);
	}

	//! \DtkDynamicCast
	inline static Xchg_Body* DtkDynamicCast(Xchg_Object* s)
	{
		if (s && s->DtkDynamicType(_typeID))
			return static_cast<Xchg_Body*>(s);
		return NULL;
	}


	Xchg_ErrorStatus GetTopologyStatus() const;
	void SetTopologyStatus(Xchg_ErrorStatus inStatus);
	Xchg_ErrorStatus GetBodyStatus() const;
	Xchg_ErrorStatus SetBodyStatus(Xchg_ErrorStatus inBodyStatus);
	Xchg_Size_t GetNumLumps() const;
	Xchg_ErrorStatus GetLump(const Xchg_Size_t& inIndex, Xchg_LumpPtr& outLump) const; 
	Xchg_ErrorStatus GetLump(const Xchg_Size_t& inIndex, Xchg_ID& outLumpId) const; 
	Xchg_ErrorStatus AddLump(const Xchg_LumpPtr& inLump);
	Xchg_TopologicalEntityPtr GetPtr(const Xchg_ID& inId) const;
	Xchg_ID AddPtr(const Xchg_TopologicalEntityPtr& outEntityPtr);

	//! \brief Add Shell in Body in first lump (created if none)
	//! \param [in] inShell : pointer on inner shell
	//! \param [in] inOuterInfo : XCHG_TRUE if outer - XCHG_FALSE if inner not yet supported -
	//! \param [in] inClosedInfo : XCHG_TRUE if closed XCHG_FALSE if open
	//! \return Xchg_ErrorStatus: dtkNoError if OK error if body has 2 open shell or 2 outer shell or dtkWarningTopologyIncomplete if inOuterInfo == XCHG_FALSE
	//! \deprecated Use Xchg_Body::AddOpenShell, Xchg_Lump::AddOuterShell or Xchg_Lump::AddInnerShell to get valid topology
	//SetAsDeprecated("2025.1", "Use Xchg_Body::AddOpenShell, Xchg_Lump::AddOuterShell or Xchg_Lump::AddInnerShell to get valid topology")
	Xchg_ErrorStatus AddShell(const Xchg_ShellPtr& inShell, const Xchg_bool& inOuterInfo, const Xchg_bool& inClosedInfo);
	
	//! \brief Add Open Shell in first lump (created if none)
	//! \param [in] inShell : pointer on outer open shell
	//! \return Xchg_ErrorStatus: dtkNoError if OK error if body has already a closed (inner or outer )shell    
	Xchg_ErrorStatus AddOpenShell(const Xchg_ShellPtr& inShell);
	Xchg_ErrorStatus AddOpenShell2(const Xchg_ShellPtr& inShell, Xchg_LumpPtr& lump);

	//! \brief Return Number of Open Shell
	//! \return Xchg_Size_t 
	Xchg_Size_t GetNumOpenShells() const;
	
	//! \brief Return Open Shell 
	//! \param [in] inIndex : index of Open Shell 
	//! \param [out] outShell : pointer on open shell
	//! \return Xchg_ErrorStatus: dtkNoError if OK     
	Xchg_ErrorStatus GetOpenShell(Xchg_Size_t inIndex, Xchg_ShellPtr& outShell) const;

	//! \brief Add Open Shell with a one Unbounded Surface
	//! \param [in] inSurface : pointer on surface
	//! \return Xchg_ErrorStatus: dtkNoError if OK error if body has already a shell     
	Xchg_ErrorStatus AddUnboundedFace(Xchg_SurfacePtr inSurface);

	//! \brief Add Wire Shell 
	//! \param [in] inShell : pointer on wire shell
	//! \return Xchg_ErrorStatus: dtkNoError if OK     
	Xchg_ErrorStatus AddWireShell(const Xchg_ShellPtr& inShell);

	int GetWireCurves(Xchg_vector<Xchg_CurvePtr>& curves);
	int GetWireEntities(Xchg_vector<Xchg_CurvePtr>& curves, Xchg_vector<Xchg_PointPtr>& points);



	//! \brief Get Tolerance 
	//! \param [out] outTolerance : body tolerance
	//! \return dtkNoError if value set or dtkWarningDefaultValue if default
	Xchg_ErrorStatus GetTolerance(Xchg_Double64& outTolerance);

	//! \brief Set Tolerance 
	//! \param [in] inTol : Tolerance
	void SetTolerance(Xchg_Double64 inTol);

	//! \brief Get Num entities 
	//! \param [in] inType : Type of entities to count
	Xchg_Size_t GetNumTopologicalEntities(enum XCHG_TYPE_ENUM inType) const;

	//! \brief Get entity with Info->GetId() == inInfoId
	//! \param [in] inInfoId : Info->GetId() of entity to be found
	//! \deprecated
	//! \sa Xchg_Body::GetEntities(const Xchg_ID& inInfoId, Xchg_vector<Xchg_EntityPtr> &outTab)
	Xchg_EntityPtr GetEntity(const Xchg_ID& inInfoId) const;
	
	//! \brief Get entities with Info->GetId() == inInfoId
	//! \param [in] inInfoId : Info->GetId() of entities to be found
	//! \remark : several entities can have same identifiers (e.g. splitted entities....)
	Xchg_ErrorStatus GetEntities(const Xchg_ID& inInfoId, Xchg_vector<Xchg_EntityPtr>& outTab) const;

	//! \brief Get Body Type (compatible with Parasolid: Acorn, Wire, Sheet, Solid)
	//! \return Xchg_BodyType
	Xchg_BodyType GetBodyType() const;

	//! \brief Set Body Type
	//! \param [in] inType : Body type
	void SetBodyType(Xchg_BodyType inType);

	//! \brief Auto-detect and set Body Type based on its contents
	//! This method analyzes the shells in the body to determine its type
	//! \return Xchg_BodyType
	Xchg_BodyType DetectBodyType() const;

};

#endif // _XCHG_BODY_HPP_

