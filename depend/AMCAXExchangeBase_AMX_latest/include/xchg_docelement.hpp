#ifndef __UTIL_XCHG_DOC_ELEMENT_HPP__
#define __UTIL_XCHG_DOC_ELEMENT_HPP__
#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_metadata.hpp"
#include "base/xchg_objectbase.hpp"
#include "base/xchg_smartptr.hpp"
#include "base/xchg_string.hpp"
#include "base/xchg_vector.hpp"
#include "base/xchg_entity.hpp"
#include "mesh/mesh.hpp"

// Forward declarations
class Xchg_DocElement;
typedef SmartPtr<Xchg_DocElement> Xchg_DocElementPtr;

/////////////////////////////////////////////////////////////////////////////
//! \ingroup DtkAPIGroup
//! \class Xchg_DocElement 
//! \brief This is the Main Doc Element Class
//! The Xchg_DocElement object is common parent class of the Xchg_Node and the Xchg_Component classes
//! It typically used as geometry into a Part.
//! \SmartPtr{Xchg_DocElement}
class XCHG_API Xchg_DocElement : public Xchg_Object
{
protected: 
    enum { _typeID = XCHG_TYPE_DOC_ELEMENT }; 
    
    // Direct data members
    Xchg_ID _ID;                                     // Element ID
    Xchg_string _Name;                               // Element name
    Xchg_vector<Xchg_DocElementPtr> _Children;          // Child elements
    Xchg_vector<Xchg_ID> _FatherIDs;                    // Father IDs
    Xchg_InfoPtr _Infos;                             // Info data
    Xchg_MaterialPtr _Material;                      // Material
    Xchg_vector<Xchg_MetaDataPtr> _MetaDataArray;       // MetaData array
    Xchg_vector<Xchg_MetaDataGroupPtr> _MetaDataGroups; // MetaData groups
    Xchg_vector<Xchg_PropertiesPtr> _PropertiesArray;   // Properties array
    
protected:
    void _Init();
    void _Reset();
    void _Copy(const Xchg_DocElement& s);

    //! \internal
    Xchg_DocElement(const Xchg_ID& inID, Xchg_string inName);

    //! \BaseDestructor 
    virtual ~Xchg_DocElement();


    //! \brief Adds a child Xchg_DocElement to main Element.
    //! \param inChild The Xchg_DocElement to be added as a child.
    void _AddChild(const Xchg_DocElementPtr& inChild);

    //! \brief Retrieves the ith Xchg_DocElement - read only -
    //! \param inPos Child element index.
    //! \return the inPos'th Xchg_DocElement
    const Xchg_DocElementPtr _GetChild(const Xchg_Size_t& inPos) const;

    //! \brief Replaces a child by another one. Also update father lists.
    //! \param inPos Old Child element index.
    //! \param inNewChild The Xchg_DocElement replacing the old one
    //! \DtkInternal
    //! \return the DtkNoerrror if success.
    //! \return the dtkErrorOutOfRange if bad inPos parameter.
    //! \return the dtkErrorNullPointer if inNewChild is NULL.
    Xchg_ErrorStatus _ReplaceChild(const Xchg_Size_t& inPos, const Xchg_DocElementPtr& inNewChild);
 
    friend class SmartPtr<Xchg_DocElement>;
    friend class Xchg_MainDoc;

public:
    //! \DtkDynamicType
    inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
    {
        if (inId == _typeID) return 1;
        return Xchg_Object::DtkDynamicType(inId);
    }

    //! \DtkDynamicCast
    inline static Xchg_DocElement* DtkDynamicCast(Xchg_Object* s)
    {
        if (s && s->DtkDynamicType(_typeID))
            return static_cast<Xchg_DocElement*>(s);
        return NULL;
    }

    //! \brief Retrieves the number of the Xchg_DocElement children - read only -
    //! \return The Children number.
    Xchg_Size_t GetNumChildren() const;

    //! \brief Retrieves the Xchg_DocElement Name - read only -
    //! \return The Element Name.
    const Xchg_string& Name() const;
    Xchg_string& Name();

    //! \brief Retrieves the Xchg_DocElement ID - read only -
    //! \return The element ID.
    Xchg_ID GetID() const;

    //! \brief Retrieves Xchg_DocElement ith FatherID - read only -
    //! \return The ith FatherID.
    //! TODO NEED MORE THINKING!!!
    Xchg_ID GetFatherID(Xchg_Size_t inIndex)const;

	//! \brief GetNumFather
	//! \return Number of fathers
    //! TODO NEED MORE THINKING!!!
	Xchg_Size_t GetNumFathers()const;


    //! \brief Retrieves the Xchg_DocElement Xchg_InfoPtr - read only -
    //! \return The element Xchg_InfoPtr.
    Xchg_InfoPtr GetInfos() const;
    void SetInfos(const Xchg_InfoPtr& inInfos);

	//! \brief Retrieves the Xchg_DocElement Xchg_MaterialPtr - read only -
	//! \return The element Xchg_MaterialPtrif if exists .
	Xchg_MaterialPtr GetMaterial() const;
	void SetMaterial(Xchg_MaterialPtr inMaterial);

    //! \brief Adds a Xchg_MetaDataPtr to the Xchg_DocElement.
    //! \param inMetaData The Xchg_MetaDataPtr to be added.
    void AddMetaData(const Xchg_MetaDataPtr& inMetaData);

	//! \brief Adds a Xchg_MetaDataGroupPtr to the Xchg_DocElement.
	//! \param inMetaDataGroup The Xchg_MetaDataGroupPtr to be added.
	//! \param inAddMetadata If we also add the metadata of the group to the Xchg_DocElement
	void AddMetaDataGroup(const Xchg_MetaDataGroupPtr& inMetaDataGroup, Xchg_bool inAddMetadata = XCHG_FALSE);

    //! \brief Adds a Xchg_PropertiesPtr to the Xchg_DocElement.
    //! \param inProperty The Xchg_PropertiesPtr to be added.
    void AddProperty(const Xchg_PropertiesPtr & inProperty);

    //! \brief Retrieves the ith Xchg_MetaDataPtr - read only -
    //! \param inPos Metadata index.
    //! \return the inPos'th Xchg_MetaDataPtr
    Xchg_MetaDataPtr GetMetaData(const Xchg_Size_t& inPos) const;

	//! \brief Retrieves the ith Xchg_MetaDataGrouplPtr - read only -
	//! \param inPos Group index.
	//! \return the inPos'th Xchg_MetaDataGrouplPtr
	Xchg_MetaDataGroupPtr GetMetaDataGroup(const Xchg_Size_t& inPos) const;

    //! \brief Retrieves the number of the Xchg_MetaDataPtr - read only -
    //! \return The Metadata number.
    Xchg_Size_t GetNumMetaData() const;

	//! \brief Retrieves the number of the Xchg_MetaDataGroupPtr - read only -
	//! \return The Group number.
	Xchg_Size_t GetNumMetaDataGroup() const;
};

#endif