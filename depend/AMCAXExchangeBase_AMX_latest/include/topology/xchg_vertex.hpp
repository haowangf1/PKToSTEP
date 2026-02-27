#ifndef _XCHG_VERTEX_HPP_
#define _XCHG_VERTEX_HPP_

#include "base/xchg_export.hpp"
#include "topology/xchg_topologicalentity.hpp"
#include "geom/geom.hpp"

// Forward declarations
class Xchg_Edge;
typedef SmartPtr<Xchg_Edge> Xchg_EdgePtr;

class Xchg_Vertex;
typedef SmartPtr<Xchg_Vertex> Xchg_VertexPtr;

//! \ingroup TOPO
//! \class Xchg_Vertex 
//! \brief Vertex indicates bound of an edge. Vertex has Xchg_Point as geometry value
class XCHG_API Xchg_Vertex : public Xchg_TopologicalEntity
{
protected:
	struct Xchg_Handle;       // Not defined here
	Xchg_Handle* _Private = nullptr;   // Handle
	enum { _typeID = XCHG_TYPE_VERTEX }; 

	void _Init();
	void _Reset();
	void _Copy(const Xchg_Vertex& inTopo, Xchg_Body* inParentBody);
	
	Xchg_Vertex();
	Xchg_Vertex(const Xchg_Vertex& inTopo, Xchg_Body* inParentBody);
	virtual ~Xchg_Vertex();
	
	void _Clone(Xchg_TopologicalEntityPtr& outTopo, Xchg_Body* _inParentBody) override;
	
	friend class SmartPtr<Xchg_Vertex>;
	friend class Xchg_TopologicalEntity;

public:
	Xchg_ID GetTopoID() const override;
	Xchg_ErrorStatus SetParentBody(const Xchg_BodyPtr& inParentBody) override;
	
	//! \brief Create a vertex in a body
	//! \param [in] inParentBody : parent body
	//! \return static Xchg_VertexPtr 
	static Xchg_VertexPtr Create(const Xchg_BodyPtr& inParentBody);
	
	//! \DtkDynamicType
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_TopologicalEntity::DtkDynamicType(inId);
	}
	
	//! \DtkDynamicCast
	inline static Xchg_Vertex* DtkDynamicCast(Xchg_Object* inObject)
	{
		if (inObject && inObject->DtkDynamicType(_typeID))
			return static_cast<Xchg_Vertex*>(inObject);
		return NULL;
	}
	
	//! \brief Return Type of entity
	//! \return enum type_detk 
	enum XCHG_TYPE_ENUM GetType() const override;

	//! \brief Return Number of Parent Edge
	//! \return Xchg_Size_t 
	Xchg_Size_t GetNumParentEdges() const;
	
	//! \brief Return parent edge at inIndex
	//! \param [in] inIndex : index of parent edge 
	//! \param [out] outParentEdge : pointer of parent edge
	//! \return Xchg_ErrorStatus: dtkNoError if OK 
	Xchg_ErrorStatus GetParentEdge(const Xchg_Size_t& inIndex, Xchg_EdgePtr& outParentEdge) const;
	
	//! \brief Add parent Edge
	//! \param [in] inParentEdge : pointer of parent edge
	//! \return Xchg_ErrorStatus: dtkNoError if OK 
	Xchg_ErrorStatus AddParentEdge(const Xchg_EdgePtr& inParentEdge);
	
	//! \brief Add parent Edge
	//! \param [in] inParentEdgeId : UUID of parent edge
	//! \return Xchg_ErrorStatus: dtkNoError if OK 
	Xchg_ErrorStatus AddParentEdge(const Xchg_ID& inParentEdgeId);
	
	//! \brief Return associated geometry 
	//! \return Xchg_PointPtr 
	Xchg_PointPtr GetGeom() const;
	
	//! \brief Set Geometry
	//! \param [in] inPoint : Point 
	//! \return void 
	void SetGeom(const Xchg_PointPtr& inPoint);


	//! \brief Get Tolerance 
	//! \param [out] outTolerance : vertex tolerance
	//! \return dtkNoError if value set or dtkWarningDefaultValue if default
	Xchg_ErrorStatus GetTolerance(Xchg_Double64& outTolerance);

	//! \brief Set Tolerance 
	//! \param [in] inTol : Tolerance
	void SetTolerance(Xchg_Double64 inTol);

};

#endif // _XCHG_VERTEX_HPP_

