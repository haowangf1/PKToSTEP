#ifndef __UTIL_XCHG_DOCUMENT_HPP__
#define __UTIL_XCHG_DOCUMENT_HPP__

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_metadata.hpp"
#include "base/xchg_objectbase.hpp"
#include "base/xchg_smartptr.hpp"
#include "xchg_maindoc_ptr.hpp"
#include "xchg_documentinformation.hpp"
#include "xchg_component.hpp"
#include <iosfwd>


/////////////////////////////////////////////////////////////////////////////
//! \ingroup DtkAPIGroup
//! \class Xchg_MainDoc 
//! \brief This is the Document Class.\n
//! The Xchg_MainDocPtr object is used as root element produced by a translation.\n
//! It contains the root Xchg_ComponentPtr.
//! \SmartPtr{Xchg_MainDoc}
class XCHG_API Xchg_MainDoc : public Xchg_Object
{
public:
    struct Xchg_Handle;       // Not defined here
protected: 
    enum { _typeID = XCHG_TYPE_DOCUMENT }; 
    Xchg_Handle *_Private = nullptr;   // Handle
    friend class Xchg_API;
	void _ReleaseComponents();
private:
    void _Init();
    void _Reset();
    void _Copy(const Xchg_MainDoc&s);
    
    //! \sa Create
    Xchg_MainDoc();
    //! \BaseDestructor 
    ~Xchg_MainDoc();

    void _AddComponentIntoPool(Xchg_ComponentPtr& outElt);

    friend class SmartPtr<Xchg_MainDoc>;
public:

    //! \BaseConstructor 
    static Xchg_MainDocPtr Create();

    //! \DtkDynamicType
    Xchg_Int32 DtkDynamicType(const int& inId);

    //! \DtkDynamicCast
    static Xchg_MainDoc* DtkDynamicCast(Xchg_Object *s);

    //! \brief Set the root Xchg_ComponentPtr.
    //! \param inRootComponent The root Ccomponent 
    void SetRootComponent(const Xchg_ComponentPtr& inRootComponent);

    //! \brief Retrieves the root Xchg_ComponentPtr of the Xchg_MainDoc.
    //! \return The root Xchg_ComponentPtr.
    const Xchg_ComponentPtr& RootComponent() const;
    
    //! \brief Creates a physical Xchg_ComponentPtr.
    //! \param inComponentType Component type - Part or Assembly -.
    //! \param inComponentFullPath Component full path.
    //! \param inComponentName Component name.
    //! \param inInstanceName Instance name.
    //! \param inTransformationMatrix Transformation matrix.
    //! \param inAssociatedModule Associated module - Unknown by default -.
    //! \return the constructed Xchg_ComponentPtr.

    //! \brief Creates an Instance Xchg_ComponentPtr.
    //! \param inPrototype The Prototype Xchg_ComponentPtr used to create instance.
    //! \param inInstanceName Instance name.
    //! \param inTransformationMatrix Transformation matrix.
    //! \return the constructed InstanceXchg_ComponentPtr.
    //! \remark The Prototype is automatically linked to the instance.
    Xchg_ComponentInstancePtr CreateComponentInstance(
        const Xchg_ComponentPtr& inPrototype,
        Xchg_string inInstanceName,
        const Xchg_transfo& inTransformationMatrix);

    //! \brief Creates a Prototype Xchg_ComponentPtr.
    //! This type handles Assemblies and part components.
    //! \param inComponentFullPath Component full path.
    //! \param inComponentName Component name.
    //! \param inAvailability : Is component found/missing or internal
	//! \param inAdditionnalReferenceFlag : Is component an additionnal reference ( not part of assembly) or not
    //! \return the constructed Prototype Xchg_ComponentPtr.
    Xchg_ComponentPtr CreateComponent(
        Xchg_string inComponentFullPath,
        Xchg_string inComponentName,
		const Xchg_Component::ComponentAvailabilityEnum& inAvailability,
		const int inAdditionnalReferenceFlag = 0,
		Xchg_string inOriginalName=Xchg_string());

	
	//! \brief Retrieves a Xchg_ComponentPtr giving its ID.
    //! \param inComponentID The component ID 
    //! \return The wanted Xchg_ComponentPtr.
    Xchg_ComponentPtr GetComponentByID( const Xchg_ComponentID& inComponentID ) const;

    //! \brief Retrieves a Xchg_ComponentInstancePtr giving its ID.
    //! \param inComponentInstanceID The component instance ID 
    //! \return The wanted Xchg_ComponentInstancePtr.
    Xchg_ComponentInstancePtr GetComponentInstanceByID( const Xchg_ComponentInstanceID& inComponentInstanceID ) const;

    //! \brief Retrieves the inIndexth father of the giving Xchg_ComponentPtr.
	//! \param inComponent The component from which we want the father.
    //! \param inIndex index of father wanted.
    //! \return The wanted Xchg_ComponentPtr.
    //! TODO NEED MORE THINKING!!!
    Xchg_ComponentPtr GetFatherComponent(const Xchg_ComponentPtr& inComponent,Xchg_Size_t inIndex) const;

	//! \brief Retrieves the inIndexth father of the giving Xchg_ComponentPtr.
	//! \param inComponent The component from which we want the number of fathers.
	//! \return The number of father.
    //! TODO NEED MORE THINKING!!!
	Xchg_Size_t GetNumFatherComponent(const Xchg_ComponentPtr& inComponent);

	//! \brief Give number of Components in the Xchg_MainDoc (useful if you want to handle progress bar). 
	//! \return number of Components. 
	Xchg_Size_t GetNumComponents() const;

	//! \brief Give number of InstanceComponents in the Xchg_MainDoc 
	//! \return number of Instance Components. 
	Xchg_Int32 GetNumInstances() const;

	//! \brief return list of missing files into MainDoc.
	//! \param outFileList : list of missing files
	//! \return DtkNoError if success.
	Xchg_ErrorStatus GetMissingComponentList(Xchg_vector<Xchg_string> &outFileList) const;

	//! \brief return list of additional reference files into MainDoc.
	//! \param outFileList : list of additionnal reference files
	//! \return DtkNoError if success.
	Xchg_ErrorStatus GetAdditionalReferenceList(Xchg_vector<Xchg_string> &outFileList) const;

    Xchg_ErrorStatus DevalidateComponent(const Xchg_ComponentID& inComponentID);

    // ============================================================================
    // Document Information Methods
    // ============================================================================

    //! \brief Get the document information object
    //! \return Reference to the document information
    Xchg_DocumentInformation& GetDocumentInformation();

    //! \brief Get the document information object (const version)
    //! \return Const reference to the document information
    const Xchg_DocumentInformation& GetDocumentInformation() const;

    //! \brief Set the document information object
    //! \param inDocInfo The document information to set
    void SetDocumentInformation(const Xchg_DocumentInformation& inDocInfo);
};

#endif