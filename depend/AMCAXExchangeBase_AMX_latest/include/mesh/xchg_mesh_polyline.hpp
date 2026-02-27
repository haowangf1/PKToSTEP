#ifndef _MESH_MESH_POLYLINE_HPP_
#define _MESH_MESH_POLYLINE_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_objectbase.hpp"
#include "base/xchg_smartptr.hpp"
#include "base/xchg_vector.hpp"

// new class Xchg_MeshPolyline
//! \class  Xchg_MeshPolyline
//! \brief This is the Polyline in Mesh mode.
//!

class XCHG_API Xchg_MeshPolyline :public Xchg_Object
{
protected:
	enum { _typeID = XCHG_TYPE_MESH_POLYLINE };
private:
	friend class SmartPtr<Xchg_MeshPolyline>;
	Xchg_vector<Xchg_UInt32> points;   // array of indice in vertex array from Xchg_MEsh of Polyline
	int  TopoID;

public:
	Xchg_MeshPolyline();
	virtual ~Xchg_MeshPolyline();
	void AddVertex(const Xchg_UInt32 ind);
	Xchg_UInt32 GetVertexInd(const Xchg_UInt32 ival);
	int GetTopoID() { return (TopoID); };
	void SetTopoID(const int ival);
	Xchg_Size_t GetNbVertex() { return (points.size()); };

	Xchg_Object* Clone() const override { return new Xchg_MeshPolyline(*this); }
	//downcasting
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_Object::DtkDynamicType(inId);
	}
	inline static Xchg_MeshPolyline* DtkDynamicCast(Xchg_Object* s)
	{
		if (s && s->DtkDynamicType(_typeID))
			return static_cast<Xchg_MeshPolyline*>(s);
		return NULL;
	}


};

typedef SmartPtr<Xchg_MeshPolyline>				Xchg_MeshPolylinePtr;

#endif // _MESH_MESH_POLYLINE_HPP_

