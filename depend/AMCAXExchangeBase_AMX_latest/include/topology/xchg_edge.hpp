#ifndef _XCHG_EDGE_HPP_
#define _XCHG_EDGE_HPP_

#include "base/xchg_export.hpp"
#include "topology/xchg_topologicalentity.hpp"
#include "topology/xchg_vertex.hpp"
#include "geom/geom.hpp"

// Forward declarations
class Xchg_Coedge;
typedef SmartPtr<Xchg_Coedge> Xchg_CoedgePtr;

class Xchg_Edge;
typedef SmartPtr<Xchg_Edge> Xchg_EdgePtr;

//! \ingroup TOPO
//! \class Xchg_Edge 
//! \brief Edge has two vertex for bounds. It has a 3D curve as geometry value
//! \brief Geometrical orientation relative to 3D Curve is always True
class XCHG_API Xchg_Edge : public Xchg_TopologicalEntity
{
protected:
	struct Xchg_Handle;       // Not defined here
	Xchg_Handle* _Private = nullptr;   // Handle
	enum { _typeID = XCHG_TYPE_EDGE }; 

	void _Init();
	void _Reset();
	void _Copy(const Xchg_Edge& inTopo, Xchg_Body* inParentBody);
	void _Clone(Xchg_TopologicalEntityPtr& outTopo, Xchg_Body* _inParentBody) override;

	Xchg_Edge();
	Xchg_Edge(const Xchg_Edge& inEdgeTpCopy, Xchg_Body* inParentBody);
	virtual ~Xchg_Edge();
	
	friend class SmartPtr<Xchg_Edge>;
	friend class Xchg_TopologicalEntity;

private:
	Xchg_bool _Degenerated{};

public:
	Xchg_ID GetTopoID() const override;
	Xchg_ErrorStatus SetParentBody(const Xchg_BodyPtr& inParentBody) override;

	//! \brief Create an edge in a body
	//! \param [in] inParentBody : parent body
	//! \return static Xchg_EdgePtr 
	static Xchg_EdgePtr Create(const Xchg_BodyPtr& inParentBody);

	//! \DtkDynamicType
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_TopologicalEntity::DtkDynamicType(inId);
	}
	
	//! \DtkDynamicCast
	inline static Xchg_Edge* DtkDynamicCast(Xchg_Object* inObject)
	{
		if (inObject && inObject->DtkDynamicType(_typeID))
			return static_cast<Xchg_Edge*>(inObject);
		return NULL;
	}

	//! \brief Set sameSense for edge relative to geometry 
	//! \param [in] :insameSense true if Edge has SameSense relative to geometry
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus SetSameSense(const Xchg_bool& insameSense);

	//! \brief Return SameSense relative to geometry 
	//! \return True if Edge has SameSense relative to geometry
	Xchg_bool GetSameSense() const;

	//! \brief Get Start Vertex for edge
	//! \param [out] :outVertex start vertex of edge
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus GetStartVertex(Xchg_VertexPtr& outVertex) const;
	Xchg_ErrorStatus GetStartVertex(Xchg_ID& outVertexId) const;
	
	//! \brief Set Start Vertex for edge
	//! \param [in] :inVertex start vertex to set
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus SetStartVertex(const Xchg_VertexPtr& inVertex);
	Xchg_ErrorStatus SetStartVertex(const Xchg_ID& inVertex);

	//! \brief Get End Vertex for edge
	//! \param [out] :outVertex end vertex of edge
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus GetEndVertex(Xchg_VertexPtr& outVertex) const;
	Xchg_ErrorStatus GetEndVertex(Xchg_ID& outVertexId) const;

	//! \brief Set End Vertex for edge
	//! \param [in] :inVertex end vertex to set
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus SetEndVertex(const Xchg_VertexPtr& inVertex);
	Xchg_ErrorStatus SetEndVertex(const Xchg_ID& inVertex);

	//! \brief Get Number of coedges
	//! \return Number of coedges
	Xchg_Size_t GetNumCoedges() const;

	//! \brief Get Corresponding coedges from index
	//! \param [in] :inIndex coedge index
	//! \param [out] :outSecondCoedge coedge pointer
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus GetParentCoedge(const Xchg_Size_t& inIndex, Xchg_CoedgePtr& outCoedge) const;
	Xchg_ErrorStatus GetParentCoedge(const Xchg_Size_t& inIndex, Xchg_ID& outCoedgeId) const;

	//! \brief Set Corresponding coedges
	//! \param [in] :inFirstCoedge 
	//! \param [in] :inSecondCoedge 
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus SetCoedges(const Xchg_CoedgePtr& inFirstCoedge, const Xchg_CoedgePtr& inSecondCoedge);

	//! \brief Add a Coedge to Edge
	//! \param [in] :inCoedge 
	//! \return Xchg_ErrorStatus: dtkNoError if OK
	Xchg_ErrorStatus AddCoedge(const Xchg_CoedgePtr& inCoedge);
	Xchg_ErrorStatus AddCoedge(const Xchg_ID& inCoedgeId);

	//! \brief Remove a Coedge to Edge's list of coedges
	//! \param [in] :inCoedge 
	void RemoveCoedge(const Xchg_ID& inCoedgeId);

	enum XCHG_TYPE_ENUM GetType() const override;
	
	//! \brief Get edge geometry
	//! \param [in] :inWithVerticesTrim if XCHG_TRUE Curve is trimmed with vertices of edge
	//! \return Xchg_CurvePtr 
	Xchg_CurvePtr GetGeom(Xchg_bool inWithVerticesTrim = XCHG_TRUE) const;
	
	//! \brief Set Geometry
	//! \param [in] inCurve3D : 3D curve supporting the edge 
	//! \return void 
	void SetGeom(const Xchg_CurvePtr& inCurve3D);

	void SetDegenerated(Xchg_bool inDegen = XCHG_TRUE);
	Xchg_bool IsDegenerated() const;

	//! \brief Get Tolerance 
	//! \param [out] outTolerance : edge tolerance
	//! \return dtkNoError if value set or dtkWarningDefaultValue if default
	Xchg_ErrorStatus GetTolerance(Xchg_Double64& outTolerance);

	//! \brief Set Tolerance 
	//! \param [in] inTol : Tolerance
	void SetTolerance(Xchg_Double64 inTol);

};

#endif // _XCHG_EDGE_HPP_

