#ifndef _XCHG_LUMP_HPP_
#define _XCHG_LUMP_HPP_

#include "base/xchg_export.hpp"
#include "topology/xchg_topologicalentity.hpp"

// Forward declarations
class Xchg_Shell;
typedef SmartPtr<Xchg_Shell> Xchg_ShellPtr;

class Xchg_Lump;
typedef SmartPtr<Xchg_Lump> Xchg_LumpPtr;

//! \ingroup TOPO
//! \class Xchg_Lump 
//! \brief A Xchg_Lump is a manifold part of a non manifold object.
//! \brief It directly contains Shells (outer, inner, open, wire).
//! \brief This replaces the previous Volume+Lump hierarchy, aligning with ACIS LUMP and Parasolid Region.
class XCHG_API Xchg_Lump : public Xchg_TopologicalEntity
{
protected:
	struct Xchg_Handle;       // Not defined here
	Xchg_Handle* _Private = nullptr;   // Handle
	enum { _typeID = XCHG_TYPE_LUMP }; 

	void _Init();
	void _Reset();
	void _Copy(const Xchg_Lump& inLumpToCopy, Xchg_Body* inParentBody);
	
	Xchg_Lump();
	Xchg_Lump(const Xchg_Lump& inLumpToCopy, Xchg_Body* inParentBody);
	virtual ~Xchg_Lump();
	
	friend class SmartPtr<Xchg_Lump>;
	friend class Xchg_TopologicalEntity;
	void _Clone(Xchg_TopologicalEntityPtr& outTopo, Xchg_Body* _inParentBody) override;

public:
	Xchg_ID GetTopoID() const override;

	//! \brief Create a lump in a body
	//! \param [in] inParentBody : parent body
	//! \return static Xchg_LumpPtr 
	static Xchg_LumpPtr Create(const Xchg_BodyPtr& inParentBody);

	//! \DtkDynamicType
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_TopologicalEntity::DtkDynamicType(inId);
	}
	
	//! \DtkDynamicCast
	inline static Xchg_Lump* DtkDynamicCast(Xchg_Object* inObject)
	{
		if (inObject && inObject->DtkDynamicType(_typeID))
			return static_cast<Xchg_Lump*>(inObject);
		return NULL;
	}

	Xchg_ErrorStatus GetTopologyStatus() const;
	void SetTopologyStatus(Xchg_ErrorStatus inStatus);

	//! \brief Get Number of Shells in lump 
	//! \return Number of Shells in lump   
	Xchg_Size_t GetNumShells() const;
	
	//! \brief Get Shell
	//! \param [in] inIndex : index of shell 
	//! \param [out] outShell : pointer on shell
	//! \return Xchg_ErrorStatus: dtkNoError if OK     
	Xchg_ErrorStatus GetShell(const Xchg_Size_t& inIndex, Xchg_ShellPtr& outShell) const; 
	Xchg_ErrorStatus GetShell(const Xchg_Size_t& inIndex, Xchg_ID& outShellId) const; 
	
	//! \brief Add Open Shell (only one supported) 
	//! \param [in] inShell : pointer on outer shell
	//! \return Xchg_ErrorStatus: dtkNoError if OK error if body has already a closed (inner or outer) shell    
	Xchg_ErrorStatus AddOpenShell(const Xchg_ShellPtr& inShell);
	
	//! \brief Add Outer Shell (only one supported) 
	//! \param [in] inShell : pointer on outer shell
	//! \return Xchg_ErrorStatus: dtkNoError if OK error if body has already an open shell or an outer shell
	Xchg_ErrorStatus AddOuterShell(const Xchg_ShellPtr& inShell);
	
	//! \brief Add Inner Shell 
	//! \param [in] inShell : pointer on inner shell
	//! \return Xchg_ErrorStatus: dtkNoError if OK  
	Xchg_ErrorStatus AddInnerShell(const Xchg_ShellPtr& inShell);
	
	//! \brief Add Wire Shell 
	//! \param [in] inShell : pointer on inner shell
	//! \return Xchg_ErrorStatus: dtkNoError if OK  
	Xchg_ErrorStatus AddWireShell(const Xchg_ShellPtr& inShell);
	
	//! \brief Get Parent Body
	//! \param [out] outParentBody : parent body
	//! \return Xchg_ErrorStatus: dtkNoError if OK     
	Xchg_ErrorStatus GetParentBody(Xchg_BodyPtr& outParentBody) const;
	
	//! \brief Set Parent Body
	//! \param [in] inParentBody : parent body
	//! \return Xchg_ErrorStatus: dtkNoError if OK     
	Xchg_ErrorStatus SetParentBody(const Xchg_BodyPtr& inParentBody);
	Xchg_ErrorStatus SetParentBody(Xchg_Body* inParentBody);

	enum XCHG_TYPE_ENUM GetType() const override;
};

#endif // _XCHG_LUMP_HPP_

