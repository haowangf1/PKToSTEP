#ifndef _XCHG_TOPOLOGICAL_ENTITY_HPP_
#define _XCHG_TOPOLOGICAL_ENTITY_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_entity.hpp"
#include "base/xchg_transfo.hpp"

// Forward declarations
class Xchg_Body;
typedef SmartPtr<Xchg_Body> Xchg_BodyPtr;

class Xchg_TopologicalEntity;
typedef SmartPtr<Xchg_TopologicalEntity> Xchg_TopologicalEntityPtr;

//! \class Xchg_TopologicalEntity
//! \brief Base class for all topological entities
class XCHG_API Xchg_TopologicalEntity : public Xchg_Entity
{
protected:
	Xchg_ID _TopoID;                         // Topological ID - moved from derived classes
	static Xchg_ID _NextTopoID;              // Static counter for generating unique IDs
	enum { _typeID = XCHG_TYPE_TOPOLOGICAL_REPRESENTATION_ITEM };

	Xchg_TopologicalEntity();
	Xchg_TopologicalEntity(const Xchg_TopologicalEntity& inToBeCopied) : Xchg_Entity(inToBeCopied) {}
	virtual ~Xchg_TopologicalEntity();
	
	friend class SmartPtr<Xchg_TopologicalEntity>;
	friend class Xchg_Body;
	
	virtual void _Clone(Xchg_TopologicalEntityPtr& outTopo, Xchg_Body* _inParentBody) = 0;
	virtual Xchg_Object* Clone() const override { return NULL; }
	
public:
	virtual Xchg_ID GetTopoID() const;
	virtual void SetTopoID(const Xchg_ID& inTopoID) { _TopoID = inTopoID; }
	virtual Xchg_ErrorStatus SetParentBody(const Xchg_BodyPtr& inParentBody);
	
	//! \DtkDynamicType
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_Entity::DtkDynamicType(inId);
	}
	
	//! \DtkDynamicCast
	inline static Xchg_TopologicalEntity* DtkDynamicCast(Xchg_Object* inObject)
	{
		if (inObject && inObject->DtkDynamicType(_typeID))
			return static_cast<Xchg_TopologicalEntity*>(inObject);
		return NULL;
	}
	
	//! \brief Return Type of entity
	//! \return enum type_detk 
	virtual enum XCHG_TYPE_ENUM GetType() const; 
	
	//! \brief Transform object 
	//! \param [in] inTransfo : matrix to apply 
	//! \return Xchg_ErrorStatus 
	virtual Xchg_ErrorStatus Transform(const Xchg_transfo& inTransfo);
};

#endif // _XCHG_TOPOLOGICAL_ENTITY_HPP_

