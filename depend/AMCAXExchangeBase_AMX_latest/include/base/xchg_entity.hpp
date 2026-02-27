#ifndef _UTIL_ENT_XCHG_HPP_
#define _UTIL_ENT_XCHG_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_metadata.hpp"
#include "base/xchg_objectbase.hpp"
#include "base/xchg_transfo.hpp"
#include "base/xchg_rgb.hpp"
#include "base/xchg_val.hpp"
#include "base/xchg_vector.hpp"
#include "base/xchg_smartptr.hpp"
#include <iosfwd>


class Xchg_Info;
class Xchg_Entity;
class Xchg_material;
class Xchg_MetaData;
class Xchg_RenderInfos;
class Xchg_PhysicalMaterialInfos;
class Xchg_LineTypeDefinition;

typedef SmartPtr<Xchg_material>              Xchg_MaterialPtr;
typedef SmartPtr<Xchg_Info>                  Xchg_InfoPtr;
typedef SmartPtr<Xchg_RenderInfos>           Xchg_RenderInfosPtr;
typedef SmartPtr<Xchg_PhysicalMaterialInfos> Xchg_PhysicalMaterialInfosPtr;
typedef SmartPtr<Xchg_LineTypeDefinition>    Xchg_LineTypeDefinitionPtr;

enum Xchg_PointType {
    XCHG_PT_UNSPECIFIED = 0,
    XCHG_PT_CROSS,           // a cross which looks like a "X"
    XCHG_PT_PLUS,			// a cross which looks like a "+"
    XCHG_PT_CONCENTRIC,      // an unfilled circle
    XCHG_PT_COINCIDENT,      // two unfilled concentric circles
    XCHG_PT_FULLCIRCLE,      // a filled circle
    XCHG_PT_FULLSQUARE,      // a filled square
    XCHG_PT_STAR,            // a star which is the union of a 2D marker CROSS, a 2D marker PLUS and a 2D marker DOT
    XCHG_PT_DOT,             // a dot
    XCHG_PT_SMALLDOT,        // a small dot whose size is one pixel
    XCHG_PT_BIGFULLCIRCLE,      // a big filled circle
    XCHG_PT_BIGFULLSQUARE      // a big filled square
};

//! \enum Xchg_FontLineType
//! \brief This is a set of line styles
enum Xchg_FontLineType {
    XCHG_NO_PATTERN = 0,
    // ________________________________
    XCHG_SOLIDLINE,
    // _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
    XCHG_DASHED,
    // ________ _ _ _________ _ _ _____
    XCHG_PHANTOM,
    // ________ _ _________ _ _________
    XCHG_CENTERLINE,
    // ................................
    XCHG_DOTTED,
    // ___ ___ ___ ___ ___ ___ ___ ___
    XCHG_LONG_DASHED,
    // ___ . ___ . ___ . ___ . ___ . __
    XCHG_DOTTED_DASHED,
    // --'\,-----'\,-----'\,-----'\,---
    XCHG_BREAKLINE,
    XCHG_NUM_FONT_LINE_TYPES
};


class Xchg_Val;
class Xchg_Info;


class Xchg_pnt;
class Xchg_dir;
class Xchg_RGB;
class Xchg_string;


//! \ingroup base_types
//! \class Xchg_Info
//! \brief This is the generic Info class.
//!
//!    This class provides informations about Color/Layer/BlankedStatus/Name etc...
//!    This class also provides additionnal information specific to the owning entity
class XCHG_API Xchg_Info : public Xchg_Object
{
public:
    enum MandatoryFlag
	{
		NotMandatory = 0,
		XYPlane,
		YZPlane,
		ZXPlane,
		OriginPoint
	};

protected:
    enum { _typeID = XCHG_TYPE_INFO };
    struct Xchg_Handle;       // Not defined here
    Xchg_Handle *_Private = nullptr;   // Handle
    
    void _init();
    void _copy(const Xchg_Info& s);
    void _reset();
    Xchg_Info();
    Xchg_Info(const Xchg_Info& s);
    Xchg_Info( Xchg_Info&& s) XCHG_NOEXCEPT;
    friend class SmartPtr<Xchg_Info>;
    virtual Xchg_Object* Clone() const override { return new Xchg_Info(*this); }
public:

    //downcasting
    inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
    {
        if (inId == _typeID) return 1;
        return Xchg_Object::DtkDynamicType(inId);
    }
    
    inline static Xchg_Info* DtkDynamicCast(Xchg_Object* s)
    {
        if (s && s->DtkDynamicType(_typeID))
            return static_cast<Xchg_Info*>(s);
        return NULL;
    }

    static SmartPtr<Xchg_Info> create(const Xchg_Info& s);
    static SmartPtr<Xchg_Info> create( Xchg_Info&& s );
    static SmartPtr<Xchg_Info> create();
    ~Xchg_Info();

    Xchg_ErrorStatus AddAttribute( Xchg_string name, Xchg_Val val); 

    Xchg_ErrorStatus FindAttribute(const Xchg_string& name, Xchg_Val& val) const;
    Xchg_Val const * FindAttribute(const Xchg_string& name) const; // If not found, returns nullptr.
    Xchg_ErrorStatus ListAttributes(const Xchg_string& substring, Xchg_vector<Xchg_string>& tab_keys) const;
    Xchg_ErrorStatus ListAllAttributes(Xchg_vector<Xchg_string>& tab_keys) const;
	Xchg_ErrorStatus ListAllAttributesWithVal(Xchg_vector<Xchg_pair<Xchg_string,Xchg_Val> >& tab) const;
	Xchg_ErrorStatus RemoveAttribute(const Xchg_string& name);

	Xchg_ErrorStatus SetAdditionnalGeometryFlag(const Xchg_Int32& inAdditionnalGeom);
	Xchg_ErrorStatus SetUse(const Xchg_Int32& inUse);
	Xchg_ErrorStatus SetActivationFlag(const Xchg_Int32& inActivation);
	Xchg_ErrorStatus SetFlexibleFlag(const Xchg_Int32& inIsFlexible);
	Xchg_ErrorStatus SetActiveChildren(const Xchg_vector<Xchg_Int32> &inIndices);
	Xchg_ErrorStatus SetDefaultChildIndex(const Xchg_Int32& inIndex);
	Xchg_ErrorStatus SetHierarchy(const Xchg_Int32& inHierarchy);
	Xchg_ErrorStatus SetHierarchyColorFlag(const Xchg_Int32& inHierarchy);
    Xchg_ErrorStatus SetFontId(const Xchg_Int32& inFontId);
    Xchg_ErrorStatus SetFormFlag(const Xchg_Int32& inFormFlag);
    Xchg_ErrorStatus SetMatId(const Xchg_Int32& inMatId);
    Xchg_ErrorStatus SetSubordFlag(const Xchg_Int32& inSubordFlag);
    Xchg_ErrorStatus SetId(const Xchg_Int32& inId);
    Xchg_ErrorStatus SetViewId(const Xchg_Int32& inViewId);
	Xchg_ErrorStatus SetColor( const int&R, const int&G, const int&B );
	Xchg_ErrorStatus SetColor( const Xchg_RGB & inColor );


    Xchg_ErrorStatus SetColorId(const Xchg_Int32& inColorId);
	Xchg_ErrorStatus SetName( Xchg_string inName);
	Xchg_ErrorStatus SetPersistentName( Xchg_string inName);
    Xchg_ErrorStatus SetLayer(const Xchg_Int32 & inLayer);
    Xchg_ErrorStatus SetBlankedStatus(const Xchg_Int32 &inBlankedStatus);
    Xchg_ErrorStatus SetCurveThickNess(const Xchg_Int32 &inCurveThickNess);
	//! \brief Add a Xchg_LineTypeDefinition to Xchg_API and set it to current object.
	Xchg_ErrorStatus AddCurveLineTypeDefinition( const Xchg_LineTypeDefinitionPtr & inLineTypeDefinition );
	//! \brief Set Xchg_LineTypeDefinition of current object via it's id in Xchg_API table.
	Xchg_ErrorStatus SetCurveLineTypeDefinitionId( const Xchg_ID & inLineTypeDefinitionId );
	//! \brief Set line type of current object thanks to Xchg_FontLineType enumeration.
	Xchg_ErrorStatus SetCurveLineType(const Xchg_Int32 &inCurveLineType);
    Xchg_ErrorStatus SetCurveThickNessInMM(const Xchg_Double64 inCurveThickNessInMM);
    Xchg_ErrorStatus SetPointSymbolType(const Xchg_PointType inPointSymbolType);
	Xchg_ErrorStatus SetInfiniteGeometryFlag(const Xchg_Int32 &inInfiniteGeometryFlag);
	Xchg_ErrorStatus SetRefEntityFlag( const Xchg_Int32 &inRefEntityFlag );
	Xchg_ErrorStatus SetMandatoryFlag(const enum MandatoryFlag &inMandatoryFlag);
	Xchg_ErrorStatus SetUuid(const Xchg_UUID &inUuid);
	Xchg_ErrorStatus SetAddedItemFlag(const Xchg_Int32 &inAddedItem);
	Xchg_ErrorStatus SetMetaData(Xchg_vector<Xchg_MetaDataPtr>& inArray) ;
	Xchg_ErrorStatus AddLayer(const Xchg_Int32 & inLayer);
	//Deprecated use AddRenderInfos and AddMaterialInfos methods instead
	Xchg_ErrorStatus  AddMaterial( const Xchg_MaterialPtr &inMaterial );
	Xchg_ErrorStatus AddPhysicalMaterialInfos( const Xchg_PhysicalMaterialInfosPtr &inMaterial );
	Xchg_ErrorStatus SetPhysicalMaterialInfosId( const Xchg_ID &inMaterialId );
	Xchg_ErrorStatus AddRenderInfos( const Xchg_RenderInfosPtr &inRender );
	Xchg_ErrorStatus SetRenderInfosId( const Xchg_ID &inRenderId );


	int         GetActivationFlag() const;
	int			GetFlexibleFlag() const;
	Xchg_vector<Xchg_Int32> GetActiveChildren() const;
	int         GetDefaultChildIndex() const;
	int         GetAdditionnalGeometryFlag() const;
	int         GetUse() const;
	int         GetAddedItemFlag() const;
	int         GetHierarchy() const;
	int         GetHierarchyColorFlag() const;
    int         GetFontId() const;
    int         GetFormFlag() const;
    int         GetMatId() const;
    int         GetSubordFlag() const;
    int         GetId() const;
    int         GetViewId() const;
	Xchg_vector<Xchg_Int32> GetLayerList() const;

    //! \brief Retrieves the entity color as Xchg_RGBA values.
    //! \return the Xchg_RGBA value is succeeds. Xchg_RGB(-1,-1,-1) otherwise - no color found for example -.
    //!
    //! \b Sample:
    //! \code
    //! //Get Xchg_InfoPtr class
    //! Xchg_InfoPtr infos = inNode->GetInfos(); //inNode is a Xchg_Node in this sample
    //! Xchg_RGB MyColor = infos->GetColor(); //returned RGB is (-1,-1,-1,-1)
    //! infos->SetColor( Xchg_RGB( 128, 50, 255, 255 ) ); //Red=128 / Green=50 / Blue=255 / Alpha = 255
    //! MyColor = infos->GetColor(); // no RGB is (128,50,255,255)
    //! \endcode
    //! \sa SetColor(const Xchg_RGB & inColor)
    //! \sa SetColor(const int&R,const int&G,const int&B)
    Xchg_RGB     GetColor() const;
    int         GetColorId() const;

	//! \brief Retrieves the entity name.
	//! \return the name if succeeds, empty string otherwise.
	//! \sa SetName( const Xchg_string &inName )
	Xchg_string  GetName() const;
	Xchg_string  GetPersistentName() const;

	//! \brief Retrieves the entity layer.
	//! \return The layer number if succeed or -1 otherwise - If Undefined or no layer set -.
    //! \sa SetLayer(const int & inLayer);
    int         GetLayer() const;

	//! \brief Retrieves the entity Blanked Status.
	//! \return -1 If Undefined - or no blanked status set - .
	//! \return  0 If Visible.
	//! \return  1 If Invisible
	//! \return  2 If Construction Geometry
	//! \sa SetBlankedStatus(const int &inBlankedStatus)
    int         GetBlankedStatus() const;

	//! \deprecated Use GetCurveThickNessInMM() const method instead.
	int         GetCurveThickNess() const;

	//! \brief Retrieves the curve thickness - if exists and relevant -.
	//! \return The curve thickness in millimeters if succeeds, -1.0 otherwise.
	//! \sa SetCurveThickNessInMM( const Xchg_Double64 	inCurveThickNessInMM )
	Xchg_Double64  GetCurveThickNessInMM() const;

	//! \brief Retrieves the curve LineType - for curve entities -
    //! \return The curve LineType - Xchg_FontLineType::XCHG_SOLIDLINE if not type set -.
	Xchg_FontLineType      GetCurveLineType() const;

	//! \brief Retrieves the definition of the curve LineType - for curve entities -
	//! \brief Returns null pointer if not set.
	Xchg_LineTypeDefinitionPtr GetCurveLineTypeDefinition() const;

	//! \brief Retrieves the Id of Xchg_LineTypeDefinition of the entity in The Xchg_API table.
	//! \return The id or 0 if None.
	Xchg_ID GetCurveLineTypeDefinitionId() const;

	//! \brief Retrieves the Symbol Type - for point only -.
	//! \return The Symbol Type - Xchg_PointType::XCHG_PT_UNSPECIFIED if no type set -.
	Xchg_PointType         GetPointSymbolType() const;

	//! \brief Retrieves the Mandatory Flag 
	//! \return Mandatory flag
	enum MandatoryFlag GetMandatoryFlag() const;


	int         GetInfiniteGeometryFlag() const;
	int         GetRefEntityFlag() const;
	Xchg_UUID    GetUuid() const;
	Xchg_ErrorStatus GetMetaData(Xchg_vector<Xchg_MetaDataPtr>& outArray) const;

 	//Deprecated use GetRenderInfos() and GetPhysicalMaterialInfos() methods instead.
	Xchg_MaterialPtr GetMaterial() const;

	//! \brief Retrieves the entity RenderInfos of the entity.
	//! \return the Xchg_RenderInfosPtr or NULL if None.
	//! \sa GetRenderInfos()
	Xchg_RenderInfosPtr GetRenderInfos() const;

	//! \brief Retrieves the Id of RenderInfos of the entity in The Xchg_API table.
	//! \return the id or 0 if None.
	//! \sa GetRenderInfosId()
	Xchg_ID GetRenderInfosId() const;

	//! \brief Retrieves the entity MaterialInfos.
	//! \return the Xchg_MaterialInfosPtr if succeeds or NULL otherwise.
	Xchg_PhysicalMaterialInfosPtr GetPhysicalMaterialInfos() const;

	//! \brief Retrieves the Id of MaterialInfos of the entity in The Xchg_API table.
	//! \return the id or 0 if None.
	Xchg_ID GetPhysicalMaterialInfosId() const;

	Xchg_ErrorStatus SetBuildingInfosIds(const Xchg_vector<Xchg_Int32> inBuildingInfos);
	Xchg_vector<Xchg_Int32> GetBuildingInfosIds() const;


	Xchg_ErrorStatus GetProperties(Xchg_vector<Xchg_PropertiesPtr>& outArray) const;
	Xchg_ErrorStatus GetReferenceSet(Xchg_vector<Xchg_string>& outArray, Xchg_Int32 inRemovePrefix=XCHG_FALSE ) const;
	Xchg_string      GetConfigurationName() const;
	Xchg_ErrorStatus SetConfigurationName( const Xchg_string &inName );

	//! \brief Retrieves up to date state of current object.
	//! \return -1 if unknown, XCHG_TRUE if object is up to date, XCHG_FALSE if not up to date. Default is unknown.
	Xchg_bool GetIsUpToDate() const;
	//! \brief Set if current object is up to date or not.
	//! \param isUpToDate XCHG_TRUE if object is up to date, XCHG_FALSE if not. Anything else will set state to unknown.
	void SetIsUpToDate( const Xchg_bool isUpToDate );


    friend std::ostream& operator<<(std::ostream& o,const Xchg_Info& d);
};


// Suppress C4251 warning: SmartPtr template members don't need DLL interface
// This is safe because SmartPtr is a header-only template
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4251)  // class 'X' needs to have dll-interface to be used by clients of class 'Y'
#endif

class XCHG_API Xchg_Entity : public Xchg_Object
{
protected:
    enum { _typeID = XCHG_TYPE_ENTITY };
    
    // Direct data members (no Pimpl)
    Xchg_ID _InternalID = 0;
    Xchg_InfoPtr _Info;
    
    void _init();
    void _copy(const Xchg_Entity& s);
    void _reset();

    friend class SmartPtr<Xchg_Entity>;
public:
    Xchg_Entity();
	Xchg_Entity(const Xchg_Entity& s);
	Xchg_Entity( Xchg_Entity&& s) XCHG_NOEXCEPT;
	Xchg_Entity& operator=( const Xchg_Entity& s );
	Xchg_Entity& operator=( Xchg_Entity&& s ) XCHG_NOEXCEPT;

	virtual ~Xchg_Entity();
	//! \brief Get Internal UniqueID
	//! \return InternalID
	const Xchg_ID& GetID() const;

    virtual enum XCHG_TYPE_ENUM GetType() const=0;

    //downcasting
    inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
    {
        if (inId == _typeID) return 1;
        return Xchg_Object::DtkDynamicType(inId);
    }
    
    inline static Xchg_Entity* DtkDynamicCast(Xchg_Object* s)
    {
        if (s && s->DtkDynamicType(_typeID))
            return static_cast<Xchg_Entity*>(s);
        return NULL;
    }

    Xchg_InfoPtr GetInfo() const;
    Xchg_InfoPtr& GetInfo();
	// virtual Xchg_ErrorStatus Transform(const Xchg_transfo& inTransfo) { return XCHG_ERROR_UNDEFINED; };
    friend std::ostream& operator<<(std::ostream& o,const Xchg_Entity& d);


};

#ifdef _MSC_VER
    #pragma warning(pop)
#endif



#endif
