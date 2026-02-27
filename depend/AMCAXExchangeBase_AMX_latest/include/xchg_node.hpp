#ifndef __UTIL_XCHG_NODE_HPP__
#define __UTIL_XCHG_NODE_HPP__

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_metadata.hpp"
#include "base/xchg_objectbase.hpp"
#include "base/xchg_smartptr.hpp"
#include "xchg_docelement.hpp"
#include "mesh/mesh.hpp"
#include "topology/xchg_body.hpp"
#include "base/xchg_entity.hpp"
#include "util/xchg_kernel_utils.hpp"

class Xchg_Node;
typedef SmartPtr<Xchg_Node> Xchg_NodePtr;
/////////////////////////////////////////////////////////////////////////////
//! \ingroup DtkAPIGroup
//! \class Xchg_Node 
//! \brief This is the Node Class
//! The Xchg_Node object is used to retrieve informations in a Xchg_ComponentPtr object
//! Is typically used as geometry into a Part.
//! \SmartPtr{Xchg_Node}
class XCHG_API Xchg_Node : public Xchg_DocElement
{
public:

    enum NodeDataTypeEnum
    {
        NullType = 0,
		BodyType,                           // for 3D
		MeshType,                           // for 3D Tesselated
        AnnotationSetType,                  // for FdtAnnotationSet -- Obsolete 
        FdtType,                            // for Fdt
        DrawingType,                        // for 2D
		KinematicsType,						// Kinematics
		AxisSystemType,					    // AxisPlacement
		LayerInfosSetType,					// LayerInfosSet
		ColorInfosSetType,					// ColorInfosSet
		MetaDataType,                       // MetaData 
        ModelDisplayType,					// Model Display 
        CameraType,					        // Camera
		VirtualType,                        // just for containing children
		ConstraintType,                     // ConstraintType
        MaxNodeDataType
    };

	

    static inline Xchg_string NodeDataTypeEnumToString(const NodeDataTypeEnum& inEnum)
    {
        switch( inEnum )
        {
        case NullType: return L"NullType";
		case BodyType: return L"BodyType";
		case MeshType: return L"MeshType";
        case AnnotationSetType: return L"AnnotationSetType";
        case FdtType: return L"FdtType";
        case DrawingType: return L"DrawingType";
		case KinematicsType: return L"KinematicsType";
		case AxisSystemType: return L"AxisSystemType";
		case LayerInfosSetType: return L"LayerInfosSetType";
		case ColorInfosSetType: return L"ColorInfosSetType";
		case MetaDataType: return L"MetaDataType";
        case VirtualType: return L"VirtualType";
        case ModelDisplayType : return L"ModelDisplayType";
        case CameraType : return L"CameraType";
        case ConstraintType: return L"Constraint";
        default: return L"Unknown";
        }
    }

	static inline Xchg_Int32 NodeDataStringToEnumType( const Xchg_string &type )
	{
		if( type == L"NullType" )
			return NullType;
		else if( type == L"BodyType" )
			return BodyType;
		else if( type == L"MeshType" )
			return MeshType;
		else if( type == L"AnnotationSetType" )
			return AnnotationSetType;
		else if( type == L"FdtType" )
			return FdtType;
		else if( type == L"DrawingType" )
			return DrawingType;
		else if( type == L"KinematicsType" )
			return KinematicsType;
		else if( type == L"AxisSystemType" )
			return AxisSystemType;
		else if (type == L"LayerInfosSetType")
			return LayerInfosSetType;
		else if (type == L"ColorInfosSetType")
			return ColorInfosSetType;
		else if( type == L"MetaDataType" )
			return MetaDataType;
		else if( type == L"VirtualType" )
			return VirtualType;
        else if (type == L"ModelDisplayType")
            return ModelDisplayType;
        else if (type == L"CameraType")
            return CameraType;
        else if (type == L"ConstraintType")
            return ConstraintType;
		else return -1;		
	}

protected: 
    enum { _typeID = XCHG_TYPE_NODE }; 
    
    Xchg_ComponentID _OwningComponentId;
    NodeDataTypeEnum _NodeDataType;
    Xchg_ID _EntityID;           // ID of the entity (Body, Mesh, etc.)
    Xchg_Int32 _BodyFlag;        // Body flags (Wire, Solid, etc.)
    
    // Cached pointers to different data types
    Xchg_BodyPtr _BodyPtr;
    Xchg_MeshPtr _MeshPtr;
    
    // Parasolid BREP representation
    Xchg_Int32 _PKBody; 

    friend class Xchg_Component;
private:
    void _Init();
    void _Reset();
    void _Copy(const Xchg_Node&s);

    //! \sa Xchg_Component::CreateVirtualNode( const Xchg_string& )
	//! \sa Xchg_Component::CreateMeshNode(const Xchg_string&, const Xchg_ID&);
	//! \sa Xchg_Component::CreateBodyNode(const Xchg_string&, const Xchg_ID&);
    //! \sa Xchg_Component::CreateFdtAnnotationSetNode(const Xchg_string&, const Xchg_FdtAnnotationSetPtr&);
	//! \sa Xchg_Component::CreateKinematicsNode(const Xchg_string&, const Xchg_KinematicsPtr&);
    //! \sa Xchg_Component::CreateDrawingNode(const Xchg_string&, const Xchg_DrawingPtr&);
    Xchg_Node(const Xchg_NodeID& inID, 
        const Xchg_ComponentID inOwningComponentId,
        Xchg_string inName, const NodeDataTypeEnum& inDataNodeType, Xchg_EntityPtr inEntity, const Xchg_Int32 inBodyFlag = 0);

    //! \BaseDestructor 
    ~Xchg_Node();
    
    Xchg_Node(const Xchg_Node& other);

    friend class SmartPtr<Xchg_Node>;

public:
    //! \DtkDynamicType
    inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
    {
        if (inId == _typeID) return 1;
        return Xchg_DocElement::DtkDynamicType(inId);
    }

    //! \DtkDynamicCast
    inline static Xchg_Node* DtkDynamicCast(Xchg_Object* s)
    {
        if (s && s->DtkDynamicType(_typeID))
            return static_cast<Xchg_Node*>(s);
        return NULL;
    }

    //! \Clone
    inline virtual Xchg_Object* Clone() const override
    {
        return new Xchg_Node(*this);
    }

    //! \brief Adds a child Xchg_Node to main Node.
    //! \param inChild The Xchg_Node to be added as a child.
    inline void AddChild(const Xchg_NodePtr& inChild) 
    { _AddChild( Xchg_DocElementPtr::DtkDynamicCast(inChild) ); }

    //! \brief Retrieves the ith Xchg_Node - read only -
    //! \param inPos Child node index.
    //! \return the inPos'th Xchg_Node
    inline const Xchg_NodePtr GetChild(const Xchg_Size_t& inPos) const 
    { return Xchg_NodePtr::DtkDynamicCast( _GetChild(inPos) ); }

    //! \brief Replaces a child Xchg_NodePtr by another one. Also updates father lists.
    //! \param inPos Old Child Xchg_NodePtr element index.
    //! \param inNewChild The Xchg_NodePtr replacing the old one
    //! \DtkInternal
    //! \return the DtkNoerrror if success.
    //! \return the dtkErrorOutOfRange if bad inPos parameter.
    //! \return the dtkErrorNullPointer if inNewChild is NULL.
    inline Xchg_ErrorStatus ReplaceChildNode(const Xchg_Size_t& inPos, const Xchg_NodePtr& inNewChild)
    { return _ReplaceChild( inPos, Xchg_DocElementPtr::DtkDynamicCast(inNewChild) ); }

    //! \brief Retrieves the Xchg_Node Node Type - read only -
    //! \return The Node Type.
    NodeDataTypeEnum GetNodeType() const;


    //! \brief Retrieves the Xchg_Node as a Xchg_BodyPtr - if exists -
    //! \return The Xchg_Node as a Xchg_BodyPtr if exists. Null Xchg_BodyPtr else.
    //! \remark This method is used to retrieve the body  
    //! informations - if GetNodeType() method returns BodyType -.
    Xchg_BodyPtr GetBodyPtr();

	//! \brief Retrieves the Xchg_Node as a Xchg_meshPtr - if exists -
	//! \return The Xchg_Node as a Xchg_meshPtr if exists. Null Xchg_meshPtr else.
	//! \remark This method is used to retrieve the mesh  
	//! informations - if GetNodeType() method returns MeshType -.
	Xchg_MeshPtr GetMeshPtr();

    //! \brief Retrieves the Parasolid BREP representation of the body
    //! \return The Parasolid BREP (PK_BODY_t) if previously converted, PK_ENTITY_null otherwise
    //! \remark Call ConvertToParasolidBREP() first to perform the conversion
    Xchg_Int32 GetParasolidBody();

    //! \brief Sets the Parasolid BREP representation
    //! \param body The Parasolid BREP to set
    void SetParasolidBody(Xchg_Int32 body);

    //! \brief Converts the Xchg_BodyPtr to Parasolid BREP format
    //! \return PK_ERROR_no_errors on success, error code otherwise
    //! \remark This method performs the conversion from internal body representation to Parasolid BREP
    Xchg_Int32 ConvertToPKBody();

    //! \brief Retrieves the Xchg_Node as a Xchg_FdtPtr - if exists -
    //! \return The Xchg_Node as a Xchg_FdtPtr if exists. Null Xchg_FdtPtr else.
    //! \remark This method is used to retrieve the FDT  
    //! informations - if GetNodeType() method returns FdtType -.
    // Xchg_FdtPtr GetXchg_FdtPtr();


	//! \brief Retrieves the Xchg_Node as a Xchg_KinematicPtr - if exists -
	//! \return The Xchg_Node as a Xchg_KinematicsPtr if exists. Null Xchg_KinematicPtr else.
	//! \remark This method is used to retrieve the Kinematics  
	//! informations - if GetNodeType() method returns KinematicsType -.
	// Xchg_KinematicPtr GetXchg_KinematicPtr();

	//! \brief Retrieves the Xchg_Node as a Xchg_FdtAnnotationSetPtr - if exists -
	//! \return The Xchg_Node as a MetaDataPtr if exists. Null MetaDataPtr else.
	//! \remark This method is used to retrieve the MetaData  
	//! informations - if GetNodeType() method returns MetaDataType -.
	// Xchg_MetaDataPtr GetXchg_MetaDataPtr();

    //! \brief Retrieves the Xchg_Node as a Xchg_DrawingPtr - if exists -
    //! \return The Xchg_Node as a Xchg_DrawingPtr if exists. Null Xchg_DrawingPtr else.
    //! \remark This method is used to retrieve the drawing  
    //! informations - if GetNodeType() method returns DrawingType -.
    // Xchg_DrawingPtr GetXchg_DrawingPtr();

	//! \brief Retrieves the Xchg_Node as a Xchg_AxisSystemPtr - if exists -
	//! \return The Xchg_Node as a Xchg_AxisSystemPtr if exists. Null Xchg_AxisSystemPtr else.
	//! \remark This method is used to retrieve the axis placement 
	//! informations - if GetNodeType() method returns AxisSystemType -.
	// Xchg_AxisSystemPtr GetXchg_AxisSystemPtr();

    //! \brief Retrieves the Xchg_Node as a Xchg_LayerInfosSetPtr - if exists -
    //! \return The Xchg_Node as a Xchg_LayerInfosSetPtr if exists. Null Xchg_LayerInfosSetPtr else.
    //! \remark This method is used to retrieve the layer infos set
    //! informations - if GetNodeType() method returns LayerInfosSetType -.
    // Xchg_LayerInfosSetPtr GetXchg_LayerInfosSetPtr();

	//! \brief Retrieves the Xchg_Node as a Xchg_ColorInfosSetPtr - if exists -
	//! \return The Xchg_Node as a Xchg_ColorInfosSetPtr if exists. Null Xchg_ColorInfosSetPtr else.
	//! \remark This method is used to retrieve the color infos set
	//! informations - if GetNodeType() method returns ColorInfosSetType -.	
	// Xchg_ColorInfosSetPtr GetXchg_ColorInfosSetPtr();

    //! \brief Retrieves the Xchg_Node as a Xchg_ModelDisplayPtr - if exists -
    //! \return The Xchg_Node as a Xchg_ModelDisplayPtr if exists. Null Xchg_ModelDisplayPtr else.
    //! \remark This method is used to retrieve the display model entity (camera capture)
    //! informations - if GetNodeType() method returns ModelDisplayType -.
    // Xchg_ModelDisplayPtr GetXchg_ModelDisplayPtr();

    //! \brief Retrieves the Xchg_Node as a Xchg_CameraPtr - if exists -
    //! \return The Xchg_Node as a Xchg_CameraPtr if exists. Null Xchg_CameraPtr else.
    //! \remark This method is used to retrieve the camera entity
    //! informations - if GetNodeType() method returns CameraType -.
    // Xchg_CameraPtr GetXchg_CameraPtr();  
	
	//! \brief Retrieves the Xchg_Node as a Xchg_ConstraintPtr - if exists -
    //! \return The Xchg_Node as a Xchg_ConstraintPtr if exists. Null Xchg_ConstraintPtr else.
    //! \remark This method is used to retrieve the Constraint entity
    //! informations - if GetNodeType() method returns ConstraintType -.
    // Xchg_ConstraintPtr GetXchg_ConstraintPtr();

    //! \brief Retrieves the Xchg_Node as a Xchg_KinematicsPtr - if exists -
	//! \return The Xchg_Node as a Xchg_Kinematic if exists. Null Xchg_Kinematics else.
	//! \remark This method is used to retrieve the kinematic info 
	//! informations - if GetNodeType() method returns KinematicFilterType -.
	//Xchg_KinematicsPtr GetXchg_KinematicPtr();

	//! \brief Retrieves the Xchg_Node EntityID - read only -
	//! \return The Entity ID of Node.
	Xchg_NodeID GetEntityID() const;
};

#endif