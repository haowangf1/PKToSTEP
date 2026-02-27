#ifndef __XCHG_METADATA_HPP__
#define __XCHG_METADATA_HPP__

#include "base/xchg_export.hpp"
#include "base/xchg_string.hpp"
#include "base/xchg_objectbase.hpp"
#include "base/xchg_smartptr.hpp"
#include "base/xchg_type.hpp" 


class Xchg_Properties;
typedef SmartPtr<Xchg_Properties>            Xchg_PropertiesPtr;
class Xchg_MetaData;
typedef SmartPtr<Xchg_MetaData>				Xchg_MetaDataPtr;


/////////////////////////////////////////////////////////////////////////////
//! \ingroup DtkAPIGroup
//! \class Xchg_MetaData 
//! \brief This is the Xchg_MetaData Class.\n
//! The Xchg_MetaDataPtr object is used to store any additional informations about
//! Xchg_ComponentPtr or Xchg_NodePtr.\n
//! It can be either properties or parameters for example.\n
//! \SmartPtr{Xchg_MetaData}
class XCHG_API Xchg_MetaData : public Xchg_Object
{
public:
    enum MetaDataTypeEnum
    {
        TypeUnknown         = 0,
		TypeProperty,
		TypeFileProperty,
		TypeConfigurationProperty,
		TypeSheetMetal,
		TypeParameter,
		TypeMassProperty,
        TypeElectricalProperty,
        TypeAttributeSet,
		TypeUserAttribute,
		TypeUserExpression
	};
    static inline Xchg_string MetaDataTypeEnumToString(const MetaDataTypeEnum& inEnum)
    {
        switch( inEnum )
        {
		case TypeProperty:                 return L"TypeProperty";
		case TypeFileProperty:             return L"TypeFileProperty";
		case TypeConfigurationProperty:    return L"TypeConfigurationProperty";
		case TypeSheetMetal:               return L"TypeSheetMetal";
		case TypeParameter:                return L"TypeParameter";
		case TypeMassProperty:             return L"TypeMassProperty";
		case TypeElectricalProperty:       return L"TypeElectricalProperty";
        case TypeAttributeSet:             return L"TypeAttributeSet";
		case TypeUserAttribute:            return L"TypeUserAttribute";
		case TypeUserExpression:           return L"TypeUserExpression";
        default:                            
        case TypeUnknown:                  return L"Unknown";
        }
    }
protected:
    enum { _typeID = XCHG_TYPE_METADATA }; 
    
    // Direct data members (no Pimpl)
    MetaDataTypeEnum _MetaDataType;
    Xchg_string _Type;
    Xchg_string _Title;
    Xchg_string _Value;
    Xchg_string _Category;
    Xchg_PropertiesPtr _PropertiesData;  // For property conversion

    //! \CopyConstructor{inToBeCopied}
    Xchg_MetaData(const Xchg_MetaData& inToBeCopied);
    Xchg_MetaData( Xchg_MetaData&& inToBeMoved) XCHG_NOEXCEPT;
    
    //! \sa CreateMetaData
    //! \sa CreateParameterMetaData
    Xchg_MetaData(const MetaDataTypeEnum& inType);
   
    //! \BaseDestructor 
    virtual ~Xchg_MetaData();
    friend class SmartPtr<Xchg_MetaData>;

    void _Init();
    void _Copy(const Xchg_MetaData& inToBeCopied);
    void _Reset();
    
public:
    // Clone method for Xchg_Object
    virtual Xchg_Object* Clone() const override { return new Xchg_MetaData(*this); }


public:
    //! \brief Create a property Xchg_MetaDataPtr from a Xchg_PropertiesPtr.
    //! \param inObject Property data.
    //! \return the constructed Xchg_MetaDataPtr.
    static Xchg_MetaDataPtr CreatePropertiesMetaData(const Xchg_PropertiesPtr& inObject);
// 	//! \brief Create a parameter Xchg_MetaDataPtr from a Xchg_PropertiesPtr.
// 	//! \param inObject Parameter data.
// 	//! \return the constructed Xchg_MetaDataPtr.
// 	static Xchg_MetaDataPtr CreateParameterMetaData(const Xchg_PropertiesPtr& inObject);

	//! \brief Create a Xchg_MetaDataPtr .
	//! \param [in] inEnumType : the type of the metadata 
    //! \param [in] inTitle : the title of the metadata 
    //! \param [in] inValue : the value of the metadata 
    //! \param [in] inType : the type of value (default is "STRING") 
	//! \return the constructed Xchg_MetaDataPtr.
    static Xchg_MetaDataPtr CreateMetaData(const MetaDataTypeEnum &inEnumType, Xchg_string inTitle, Xchg_string inValue, Xchg_string inValueType = Xchg_string(L"STRING"));
    //! \brief Create a Xchg_MetaDataPtr .
    //! \param [in] inEnumType : the type of the metadata 
    //! \param [in] inTitle : the title of the metadata 
    //! \param [in] inValue : the value of the metadata 
    //! \param [in] inUnits : the unit of the metadata 
    //! \param [in] inType : the type of value (default is "STRING")
	//! \return the constructed Xchg_MetaDataPtr.
    static Xchg_MetaDataPtr CreateMetaDataWithUnits(const MetaDataTypeEnum &inEnumType, Xchg_string inTitle, Xchg_string inValue, Xchg_string inUnits, Xchg_string inValueType = Xchg_string(L"STRING"));

    //! \DtkDynamicType
    inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
    {
        if (inId == _typeID) return 1;
        return Xchg_Object::DtkDynamicType(inId);
    }
    
    //! \DtkDynamicCast
    inline static Xchg_MetaData* DtkDynamicCast(Xchg_Object* s)
    { 
        if (s && s->DtkDynamicType(_typeID))
            return static_cast<Xchg_MetaData*>(s); 
        return NULL; 
    } 

    //! \brief Retrieves the Xchg_MetaDataPtr as a property - if exists -
    //! \return The Xchg_MetaDataPtr as a Xchg_PropertiesPtr if exists. Null Xchg_PropertiesPtr else.
    //! \remark This method is used to retrieve the property  
    //! informations - if MetaDataType() method returns TypeProperty -.
    Xchg_PropertiesPtr ToProperty();

    //! \brief Retrieves the Xchg_MetaDataPtr as a parameter - if exists -
    //! \return The Xchg_MetaDataPtr as a Xchg_PropertiesPtr if exists. Null Xchg_PropertiesPtr else.
    //! \remark This method is used to retrieve the parameter  
    //! informations - if MetaDataType() method returns TypeParameter -.
    Xchg_PropertiesPtr ToParameter();

    //! \brief Retrieves the Xchg_MetaDataPtr as a parameter - if exists -
    //! \return The Xchg_MetaDataPtr as a Xchg_PropertiesPtr if exists. Null Xchg_PropertiesPtr else.
    //! \remark This method is used to retrieve the parameter  
    //! informations - if MetaDataType() method returns ToMassProperty -.
    Xchg_PropertiesPtr ToMassProperty();

    //! \brief Retrieves the Xchg_MetaDataPtr as a parameter - if exists -
    //! \return The Xchg_MetaDataPtr as a Xchg_PropertiesPtr if exists. Null Xchg_PropertiesPtr else.
    //! \remark This method is used to retrieve the parameter  
    //! informations - if MetaDataType() method returns TypeElectricalProperty -.
    Xchg_PropertiesPtr ToElectricalProperty();

    //! \brief Retrieves the Xchg_MetaDataPtr as a parameter - if exists -
    //! \return The Xchg_MetaDataPtr as a Xchg_PropertiesPtr if exists. Null Xchg_PropertiesPtr else.
    //! \remark This method is used to retrieve the parameter  
    //! informations - if MetaDataType() method returns TypeUserAttribute -.
    Xchg_PropertiesPtr ToUserAttribute();

	//! \brief Retrieves the Xchg_MetaDataPtr as a parameter - if exists -
	//! \return The Xchg_MetaDataPtr as a Xchg_PropertiesPtr if exists. Null Xchg_PropertiesPtr else.
	//! \remark This method is used to retrieve the parameter  
	//! informations - if MetaDataType() method returns TypeUserExpression -.
	Xchg_PropertiesPtr ToUserExpression();


	Xchg_PropertiesPtr ToFileProperty();
	Xchg_PropertiesPtr ToConfigurationProperty();
    //! \brief Retrieves the Xchg_MetaDataPtr type - read only -.
    //! \return The Xchg_MetaDataPtr type.
    const MetaDataTypeEnum& MetaDataType() const;
	void SetMetaDataType( const MetaDataTypeEnum& inType );


	//! \brief Get the Units of the MetaData.
	//! \param[out] outUnits :  the units string if present, empty string otherwise
	//! \return dtkNoError if Units are present / dtkErrorAttributeNotFound if no Units  .
	Xchg_ErrorStatus GetUnits( Xchg_string& outUnits ) const;

	//! \brief Get the Type of the MetaData .
	//! \return the Type of this MetaData  .
	//!
	const Xchg_string& GetType() const;

	//! \brief Set the Type of the MetaData .
	//!
	void SetType( Xchg_string in) ;

	//! \brief Get the Title of the MetaData .
	//! \return the Title of this MetaData  .
	//!
	const Xchg_string& GetTitle() const;
	//! \brief Set the Title of the MetaData .
	//!
	void SetTitle( Xchg_string in) ;

	//! \brief Get the Value of the MetaData .
	//! \return the Value of this MetaData  .
	//!
	const Xchg_string& GetValue() const;
	//! \brief Set the Value of the MetaData .
	//!
	void SetValue( Xchg_string in) ;


	//! \brief Get the Category of the MetaData .
	//! \return the Category of this MetaData  .
	//!
	const Xchg_string& GetCategory() const;
	//! \brief Set the Category of the MetaData .
	//!
	void SetCategory( Xchg_string in) ;




	//! \brief Get the Formula Type of result and its Estimated value of the MetaData .
	//! \param[out] outEstimatedValue : Estimated value of formula
	//! \param[out] outType : Type of Estimated value 
	//! \return DtkNoError if it is a Formula , dtkErrorCorruptedEntity otherwise.
	//!
	Xchg_ErrorStatus GetFormula(Xchg_string& outFormula,Xchg_string& outEstimatedValue,Xchg_string& outTitle,Xchg_string& outResultType) ;

	//! \brief Get the Formula and its Estimated value of the MetaData .
	//! \param[out] outEstimatedValue : Estimated value of formula
	//! \return the Formula of this MetaData  .
	//!
	Xchg_string GetFormula(Xchg_string& outEstimatedValue) ;

	//! \brief Set the MetaData as Formula .
	//! \param[in] inFormula : Formula
	//! \param[in] inEstimatedValue : Estimated value of formula
	//! \param[in] inResultType : Type of the EstimateValue
	//! \return the Formula of this MetaData  .
	//!
	void SetFormula( Xchg_string inFormula, Xchg_string inEstimatedValue, Xchg_string inTitle, Xchg_string inResultType);


	Xchg_bool IsReal() const;
	Xchg_bool IsInteger() const;
	Xchg_bool IsBoolean() const;
	Xchg_bool IsFormula() const;
	Xchg_bool IsReference() const;
	Xchg_bool IsDate() const;
	Xchg_bool IsString() const;
	Xchg_bool IsNull() const;
};

class Xchg_MetaDataWithUnit;

typedef SmartPtr<Xchg_MetaDataWithUnit> Xchg_MetaDataWithUnitPtr;

class XCHG_API Xchg_MetaDataWithUnit : public Xchg_MetaData
{
protected :
	enum { _typeID = XCHG_TYPE_METADATAWITHUNIT }; 
	Xchg_string		_Units;  // Unit string (e.g., "mm", "kg", "m/s")

private:
	void _Init();
	void _Reset();
	friend class SmartPtr<Xchg_MetaDataWithUnit>;

public:
	Xchg_string GetUnits();
	Xchg_MetaDataWithUnit(const MetaDataTypeEnum &inEnumType, Xchg_string inType, Xchg_string inTitle, Xchg_string inValue, Xchg_string inUnit);
	virtual ~Xchg_MetaDataWithUnit();

	// Clone method
	virtual Xchg_Object* Clone() const override { return new Xchg_MetaDataWithUnit(*this); }

	//! \DtkDynamicType
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_MetaData::DtkDynamicType(inId);
	}

	//! \DtkDynamicCast
	inline static Xchg_MetaDataWithUnit* DtkDynamicCast(Xchg_Object* s)
	{ 
		if (s && s->DtkDynamicType(_typeID))
			return static_cast<Xchg_MetaDataWithUnit*>(s); 
		return NULL; 
	} 
};

//! \class Xchg_MetaData 
//! \brief This is the Xchg_MetaDataGroup Class.\n
//! The Xchg_MetaDataGroupPtr object is used to store a group of Xchg_MetaDataPtr in 
//! a Xchg_ComponentPtr or a or Xchg_NodePtr.\n
//! The children metadata may or may not be also stored as single metadata but if it is
//! the case, then their category should be defined with the name of their Xchg_MetaDataGroup
//! in order to be distinguished from metadata only accessible alone.\n
//! It is defined by a name and a tab of metadatas.\n
//! \SmartPtr{Xchg_MetaDataGroup}
class Xchg_MetaDataGroup;

typedef SmartPtr<Xchg_MetaDataGroup> Xchg_MetaDataGroupPtr;

class XCHG_API Xchg_MetaDataGroup : public Xchg_Object
{
protected :
	enum { _typeID = XCHG_TYPE_METADATA_GROUP };
private:
	//! \brief Name of the Group
	Xchg_string _GroupName;

	//! \brief List of Metadata Children of the group
	Xchg_vector< Xchg_MetaDataPtr > _MetadataChildren;

	void _Init();
	void _Reset();

	friend class SmartPtr<Xchg_MetaDataGroup>;

public:

	//! \Create MetaDataGroup
	Xchg_MetaDataGroup( Xchg_string inGroupName);

	//! \BaseDestructor 
	virtual ~Xchg_MetaDataGroup();

	// Clone method
	virtual Xchg_Object* Clone() const override { return new Xchg_MetaDataGroup(*this); }

	//! \DtkDynamicType
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_Object::DtkDynamicType(inId);
	}

	//! \DtkDynamicCast
	inline static Xchg_MetaDataGroup* DtkDynamicCast(Xchg_Object* s)
	{
		if (s && s->DtkDynamicType(_typeID))
			return static_cast<Xchg_MetaDataGroup*>(s);
		return NULL;
	}

	static Xchg_MetaDataGroupPtr CreateMetaDataGroup( Xchg_string inGroupName, Xchg_vector< Xchg_MetaDataPtr > inMetadataChildren = Xchg_vector<Xchg_MetaDataPtr>());

	//! \brief Get the list of metadata of the MetaDataGroup.
	//! \return the list of metadata of this MetaDataGroup.
	const Xchg_vector< Xchg_MetaDataPtr >& GetListOfMetadatas() const;
	
	//! \brief Add a metadata to this Group
	void AddMetadata(Xchg_MetaDataPtr inChild);

	//! \brief Set the group name
	void SetName( Xchg_string inGroupName);

	//! \brief Get the group name
	const Xchg_string GetGroupName();

	//! \brief Get the number of metadatas in the Group 
	const Xchg_Size_t GetNumMetaData() const;
	
	//! \brief Get the inPos'th metadata of the Group 
	Xchg_MetaDataPtr GetMetaData(const Xchg_Size_t& inPos);
};

//! \ingroup base_types
//! \class Xchg_Properties 
//! \brief This is the Xchg_Properties class.
//! \brief A Xchg_Properties contains the property which defines by its Type,Title and Value
//!
//!	This class lets you use Xchg_Properties.
class XCHG_API Xchg_Properties : public Xchg_Object
{
public:

	Xchg_Properties(const Xchg_Properties& p);
	Xchg_Properties& operator=(const Xchg_Properties& p);



	//! \brief Get the Type of the Property .
	//! \return the Type of this Property  .
	//!
	const Xchg_string& GetType() const;

	//! \brief Set the Type of the Property .
	//! \return the Type of this Property  .
	//!
	void SetType( Xchg_string in) ;

	//! \brief Get the Title of the Property .
	//! \return the Title of this Property  .
	//!
	const Xchg_string& GetTitle() const;

	//! \brief Set the Title of the Property .
	//! \return the Type of this Property  .
	//!
	void SetTitle( Xchg_string) ;

	//! \brief Get the Value of the Property .
	//! \return the Value of this Property  .
	//!
	const Xchg_string& GetValue() const;

	//! \brief Set the Value of the Property .
	//! \return the Type of this Property  .
	//!
	void SetValue( Xchg_string) ;


	//! \brief Get the Formula Type of result and its Estimated value of the MetaData .
	//! \param[out] outEstimatedValue : Estimated value of formula
	//! \param[out] outType : Type of Estimated value 
	//! \return the Formula of this MetaData  .
	//!
	Xchg_ErrorStatus GetFormula(Xchg_string& outFormula,Xchg_string& outEstimatedValue,Xchg_string& outTitle,Xchg_string& outResultType) ;

	//! \brief Get the Formula and its Estimated value of the Property .
	//! \param[out] outEstimatedValue : Estimated value of formula
	//! \return the Value of this Property  .
	//!
	Xchg_string GetFormula(Xchg_string& outEstimatedValue) ;
	//! \brief Set the MetaData as Formula .
	//! \param[in] inFormula : Formula
	//! \param[in] inEstimatedValue : Estimated value of formula
	//! \param[in] inResultType : Type of the EstimateValue
	//! \return the Formula of this MetaData  .
	//!
	void SetFormula( Xchg_string inFormula, Xchg_string inEstimatedValue, Xchg_string inTitle, Xchg_string inResultType);


	static Xchg_PropertiesPtr create();
	virtual void conv_ptr(Xchg_Properties ** s){*s = this;}
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_Object::DtkDynamicType(inId);
	}
	inline static Xchg_Properties* DtkDynamicCast(Xchg_Object* s)
	{
		if (s && s->DtkDynamicType(_typeID))
			return static_cast<Xchg_Properties*>(s);
		return NULL;
	}

	void SetAsReal();
	void SetAsInteger();
	void SetAsBoolean();

	Xchg_bool IsReal() const;
	Xchg_bool IsInteger() const;
	Xchg_bool IsBoolean() const;
	Xchg_bool IsFormula() const;
	Xchg_bool IsReference() const;
	Xchg_bool IsDate() const;
	Xchg_bool IsString() const;
	Xchg_bool IsNull() const;

protected:
	enum { _typeID = XCHG_TYPE_PROPERTIES };
	
	// Direct data members (no Pimpl)
	Xchg_string _Type;
	Xchg_string _Title;
	Xchg_string _Value;
	Xchg_bool _IsFormula;
	Xchg_string _Formula;
	Xchg_string _EstimatedValue;
	Xchg_string _ResultType;
	
	Xchg_Properties();
	virtual ~Xchg_Properties();

	void _Init();
	void _copy(const Xchg_Properties& s);
	void _reset();
	friend class SmartPtr<Xchg_Properties>;

public:
	// Clone method
	virtual Xchg_Object* Clone() const override { return new Xchg_Properties(*this); }
};


#endif
