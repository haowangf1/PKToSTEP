#ifndef COMPONENT_H
#define COMPONENT_H

#include "base/xchg_export.hpp"
#include <memory>
#include <string>
#include <vector>
#include "base/xchg_smartptr.hpp"
#include "base/xchg_vector.hpp"
#include "xchg_node.hpp"
#include "xchg_docelement.hpp"

// 前置声明 Xchg_ComponentInstance - 避免循环包含
class Xchg_ComponentInstance;
typedef SmartPtr<Xchg_ComponentInstance> Xchg_ComponentInstancePtr;

// 前置声明 Xchg_Node - 避免包含 Node.hpp
class Xchg_Node;
typedef SmartPtr<Xchg_Node> Xchg_NodePtr;

class Xchg_Component;
typedef SmartPtr<Xchg_Component> Xchg_ComponentPtr;
/////////////////////////////////////////////////////////////////////////////
//! \ingroup DtkAPIGroup
//! \class Xchg_Component 
//! \brief This is the Component Class.\n
//! The Xchg_ComponentPtr object is used as element into an assembly tree.\n
//! Is typically used as a Part into an Assembly Tree.\n
//! \SmartPtr{Xchg_Component}
class XCHG_API Xchg_Component : public Xchg_DocElement
{
public:
	enum ComponentAvailabilityEnum
	{
		ComponentMissing = 0,
		ComponentFound = 1,
		ComponentInternal = 2,
		ComponentPhantom = 4,
		ComponentAdditionnalReference = 8,
		ComponentAssemblyGeometry = 16,
		ComponentWithUnavailableReader = 32
	};


    static  Xchg_string ComponentAvailabilityEnumToString(const ComponentAvailabilityEnum& inEnum);

protected: 
    enum { _typeID = XCHG_TYPE_COMPONENT }; 
    
    // Direct data members
    ComponentAvailabilityEnum _ComponentAvailability;
    Xchg_Int32 _AdditionnalReferenceFlag;
    Xchg_string _ComponentFullPath;
    Xchg_string _ComponentOriginalName;
    
    // Node pool - all nodes belonging to this component
    Xchg_vector<Xchg_NodePtr> _NodesPool;
    
    // Component properties
    Xchg_Double64 _ConceptionUnitScale;
    Xchg_string _FileVersion;
    Xchg_Int32 _CurrentLayer;
    Xchg_vector<Xchg_Int32> _VisibleLayers;
    Xchg_vector<Xchg_Int32> _SelectableLayers;
    
    friend class Xchg_MainDoc;
    friend class Xchg_API;
    
    void _ReleaseNodes();
    void _ReleaseVolatileData();
    
private:
    void _Init();
    void _Reset();
    void _Copy(const Xchg_Component&s);

    //! \sa Xchg_MainDoc::CreatePhysicalXchg_Component()
    //! \sa Xchg_MainDoc::CreateVirtualXchg_Component()
    Xchg_Component( const Xchg_ComponentID& inID,
		const ComponentAvailabilityEnum& inComponentAvailability,
		const Xchg_Int32& inAdditionnalReferenceFlag,
        Xchg_string inComponentName,
        Xchg_string inComponentFullPath,
	    Xchg_string inComponentOriginalName);

    //! Copy Constructor
    Xchg_Component(const Xchg_Component& other);

    //! \BaseDestructor 
    ~Xchg_Component();
    friend class SmartPtr<Xchg_Component>;

public:
    //! \DtkDynamicType
    inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
    {
        if (inId == _typeID) return 1;
        return Xchg_DocElement::DtkDynamicType(inId);
    }

    //! \DtkDynamicCast
    inline static Xchg_Component* DtkDynamicCast(Xchg_Object* s)
    {
        if (s && s->DtkDynamicType(_typeID))
            return static_cast<Xchg_Component*>(s);
        return NULL;
    }

    //! \Clone
    inline virtual Xchg_Object* Clone() const override
    {
        return new Xchg_Component(*this);
    }

	//InternalUse
	Xchg_ErrorStatus SetComponentAvailability(const ComponentAvailabilityEnum &inComponentAvailability);


    //! \brief Adds a child Xchg_Component to main Component.
    //! \param inChild The Xchg_Component to be added as a child.
    inline void AddChild(const Xchg_ComponentInstancePtr& inChild) 
    { _AddChild( Xchg_DocElementPtr::DtkDynamicCast(inChild) ); }

    //! \brief Retrieves the ith Xchg_ComponentInstancePtr - read only -
    //! \param inPos Child component index.
    //! \return the inPos'th Xchg_ComponentInstancePtr
    inline const Xchg_ComponentInstancePtr GetChild(const Xchg_Size_t& inPos) const 
    { return Xchg_ComponentInstancePtr::DtkDynamicCast( _GetChild(inPos) ); }

    //! \brief Replaces a child Xchg_ComponentPtr by another one. Also updates father lists.
    //! \param inPos Old Child Xchg_ComponentPtr element index.
    //! \param inNewChild The Xchg_ComponentPtr replacing the old one
    //! \DtkInternal
    //! \return the DtkNoerrror if success.
    //! \return the dtkErrorOutOfRange if bad inPos parameter.
    //! \return the dtkErrorNullPointer if inNewChild is NULL.
    inline Xchg_ErrorStatus ReplaceChildComponent(const Xchg_Size_t& inPos, const Xchg_ComponentInstancePtr& inNewChild)
    { return _ReplaceChild( inPos, Xchg_DocElementPtr::DtkDynamicCast(inNewChild) ); }

	//! \brief Give Xchg_Component availability - read only -
	//! \return The component availability.
	const ComponentAvailabilityEnum ComponentAvailability() const;

    //! \brief Retrieves the Xchg_Component FullPathName - read only -
    //! \return The component FullPathName.
    const Xchg_string& FullPathName() const;

	//! \brief Retrieves the Xchg_Component OriginalPathName (read in file) - read only -
	//! \return The component OriginalPathName.
	const Xchg_string& OriginalPathName() const;


	//! \brief Return XCHG_TRUE if Xchg_Component is an assembly.
	Xchg_bool IsAssembly() const;

    //! \brief Return XCHG_TRUE if Xchg_Component is an part.
	Xchg_bool IsPart() const;


	//! \brief Create a Body Xchg_NodePtr from a body ID.
	//! \param inName Node name.
	//! \param inBody Body pointer.
	//! \param inBodyFlag Body Info : Wire,Solid, MeshSurface and/or Surface.
	//! \return the constructed Xchg_NodePtr.
	Xchg_NodePtr CreateBodyNode( Xchg_string inName, const Xchg_BodyPtr& inBody, const Xchg_Int32 inBodyFlag );

	//! \brief Create a Mesh Xchg_NodePtr from a Mesh ID.
	//! \param inName Node name.
	//! \param inMesh Mesh pointer.
	//! \return the constructed Xchg_NodePtr.
	Xchg_NodePtr CreateMeshNode(
		Xchg_string inName,
		const Xchg_MeshPtr& inMesh,
		const Xchg_Int32 inMeshFlag);
 

    //! \brief Retrieves a Xchg_NodePtr from the Xchg_Component NodesPool giving his Id.
    //! \param inNodeID Wanted Xchg_NodePtr Id.
    //! \return The wanted Xchg_NodePtr.
    Xchg_NodePtr GetNodeByID( const Xchg_NodeID& inNodeID ) const;

    //! \brief Retrieves a Xchg_NodePtr from the Xchg_Component NodesPool giving his Id.
    //! \param index Node Index.
    //! \return The wanted Xchg_NodePtr.
    Xchg_NodePtr GetNodeByIndex(const size_t& index) const;

    //! \brief Retrieves the father Xchg_NodePtr of the given Xchg_NodePtr.
    //! \param inNode The Xchg_NodePtr from which we want the father.
    //! \return The wanted Xchg_NodePtr.
    Xchg_NodePtr GetFatherNode(const Xchg_NodePtr& inNode) const;

	//! \brief Get the Original Units for documents - geometrical data are given in MM (Scale to MM) 
	////! \param [out] outUnitScale : Unit Scale (1.0 for MM)
	////! \return DtkNoError if unit is read  
	Xchg_ErrorStatus GetConceptionUnitScale(Xchg_Double64 &outUnitScale)const;

	void SetConceptionUnitScale(Xchg_Double64 inUnitScale);

	//! \brief Get File Version 
	const Xchg_string & GetFileVersion()const;
	void SetFileVersion(const Xchg_string & inFileVersion);

	//! \brief Current layer for Component 
	Xchg_Int32 GetCurrentLayer() const;
	void SetCurrentLayer(Xchg_Int32 inCurrentLayer);

	//! \brief Visible layer for Component 
	void GetVisibleLayers(Xchg_vector<Xchg_Int32> &outVisibleLayers)const;
	void SetVisibleLayers(const Xchg_vector<Xchg_Int32> & inVisibleLayers);

	//! \brief Selectable layer for Component 
	void GetSelectableLayers(Xchg_vector<Xchg_Int32> &outSelectableLayers)const;
	void SetSelectableLayers(const Xchg_vector<Xchg_Int32> & inSelectableLayers);

	//! \brief Give number of Nodes in the Component (useful if you want to handle progress bar). 
	//! \return number of Nodes. 
	Xchg_Size_t GetNumNodes() const;


};


#endif // COMPONENT_H
