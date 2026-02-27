#ifndef __UTIL_XCHG_RENDER_HPP__
#define __UTIL_XCHG_RENDER_HPP__

#include "base/xchg_export.hpp"
#include "base/xchg_objectbase.hpp"
#include "base/xchg_smartptr.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_string.hpp"
#include "base/xchg_rgb.hpp"
#include "base/xchg_picture.hpp"
#include "base/xchg_transfo.hpp"
#include "base/xchg_metadata.hpp"
#include "base/xchg_vector.hpp"


class Xchg_TextureInfos;
class Xchg_LightMap;
class Xchg_RenderInfos;
class Xchg_PhysicalMaterialInfos;

typedef SmartPtr<Xchg_TextureInfos>          Xchg_TextureInfosPtr;
typedef SmartPtr<Xchg_LightMap>              Xchg_LightMapPtr;
typedef SmartPtr<Xchg_RenderInfos>           Xchg_RenderInfosPtr;
typedef SmartPtr<Xchg_PhysicalMaterialInfos> Xchg_PhysicalMaterialInfosPtr;


/////////////////////////////////////////////////////////////////////////////
//! \ingroup DtkAPIGroup
//! \class Xchg_RenderInfos 
//! \brief This is the Xchg_RenderInfos Class.\n
//! The Xchg_RenderInfosPtr object is used to store any informations about rendering
//! It contains lighting information and texture information.\n
//! \SmartPtr{Xchg_RenderInfosPtr}




class XCHG_API Xchg_TextureInfos : public Xchg_Object
{
public:
    enum TextureMappingTypeEnum
    {
        StoredUVMapping = 0,
        PlanarMapping = 1,
        CylinderMapping = 2,
        SphericalMapping = 3,
        CubicalMapping = 4,
		SkyPlanarMapping = 5,
		SkyCylinderMapping = 6,
		SkySphericalMapping = 7,
		SkyCubicalMapping = 8,
		UVBRepMapping		= 9,
		BuildingMapping = 10
    };

protected:
    enum { _typeID = XCHG_TYPE_TEXTURE }; 
    struct Xchg_Handle;
    Xchg_Handle *_Private = nullptr;

    //! \CopyConstructor{inToBeCopied}
    Xchg_TextureInfos(const Xchg_TextureInfos& inToBeCopied);

    //! \sa CreateMetaData
    //! \sa CreateParameterMetaData
    Xchg_TextureInfos(const TextureMappingTypeEnum& inType);

    //! \BaseDestructor 
    virtual ~Xchg_TextureInfos();
    friend class SmartPtr<Xchg_TextureInfos>;

    void _Init();
    void _Copy(const Xchg_TextureInfos& inToBeCopied);
    void _Reset();
	Xchg_Object* Clone() const override { return new Xchg_TextureInfos(*this); }




public:
    //! \brief Create a Xchg_TextureInfosPtr.
    //! \param  inType : Mapping type:
    //! Mapping type can be : 
    //! - StoredUVMapping : Using Xchg_Mesh UV texture coordinates, 
    //! - PlanarMapping / CylinderMapping / SphericalMapping / CubicalMapping : Applying standard mapping
    //! Coordinate (0, 0) represents top left corner of image.
    //! \return the constructed Xchg_TextureInfosPtr.
    static Xchg_TextureInfosPtr Create(const TextureMappingTypeEnum& inType);

    //! \DtkDynamicType
    inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
    { 
        if (inId == _typeID) return 1; 
        return Xchg_Object::DtkDynamicType(inId); 
    }

    //! \DtkDynamicCast
    inline static Xchg_TextureInfos* DtkDynamicCast(Xchg_Object* s)
    { 
        if (s && s->DtkDynamicType(_typeID)) 
            return static_cast<Xchg_TextureInfos*>(s); 
        return NULL; 
    } 

    //! \brief Returns U scale factor
    Xchg_Double64 GetUScale();
    
    //! \brief Returns V scale factor
    Xchg_Double64 GetVScale();

    //! \brief Returns U offset
    Xchg_Double64 GetUOffset();
    
    //! \brief Returns V offset
    Xchg_Double64 GetVOffset();

    //! \brief Returns image size
    Xchg_Double64 GetImageSize();

    //! \brief Returns rotate angle
    Xchg_Double64 GetRotateAngle();

    //! \brief Returns if U flipped
    Xchg_bool IsUFlipped();
    
    //! \brief Returns if V flipped
    Xchg_bool IsVFlipped();

    //! \brief Returns if texture is U repeat
    Xchg_bool IsURepeat();

    //! \brief Returns if texture is V repeat
    Xchg_bool IsVRepeat();

    //! \brief Return image path
    Xchg_string GetImagePath();

    //! \brief Returns Xchg_picture
    Xchg_picture *GetTextureImage();

    //! \brief Returns mapping type
    TextureMappingTypeEnum GetMappingType();

    //! \brief Returns texture transfo
    Xchg_transfoPtr GetTransfo();

    //! \brief Sets image U and V scale
    void SetScale(Xchg_Double64 inUScale,Xchg_Double64 inVScale);
    
    //! \brief Sets image U and V offset
    void SetOffset(Xchg_Double64 inUOffset,Xchg_Double64 inVOffset);
    
    //! \brief Sets image U and V flip
    void SetFlipFlag(Xchg_bool inUFlip,Xchg_bool inVFlip);
    
    //! \brief Sets image U and V flip
    void SetRepeat(Xchg_bool inURepeat, Xchg_bool inVRepeat);

    //! \brief Sets image size
    void SetImageSize(Xchg_Double64 inSize);

    //! \brief Sets image rotate angle
    void SetRotateAngle(Xchg_Double64 inAlpha);
    
    //! \brief Sets image file path
    void SetImagePath(Xchg_string inImagePath);
    
    //! \brief Sets Xchg_picture
    void SetTextureImage(Xchg_picture *inImage);

    //! \brief Sets image transfo
    void SetTransfo(Xchg_transfoPtr inTransfo);
};
/////////////////////////////////////////////////////////////////////////////


class XCHG_API Xchg_LightMap : public Xchg_Object
{
protected:
    enum { _typeID = XCHG_TYPE_LIGHT }; 
    struct Xchg_Handle;
    Xchg_Handle *_Private = nullptr;

    //! \CopyConstructor{inToBeCopied}
    Xchg_LightMap(const Xchg_LightMap& inToBeCopied);

    //! \sa Create Xchg_LightMap
    Xchg_LightMap();
    
    Xchg_LightMap(Xchg_RGB inAmbient,Xchg_RGB inDiffuse, Xchg_RGB inSpecular);
    
    //! \BaseDestructor 
    virtual ~Xchg_LightMap();
    friend class SmartPtr<Xchg_LightMap>;

    void _Init();
    void _Copy(const Xchg_LightMap& inToBeCopied);
    void _Reset();
	Xchg_Object* Clone() const override { return new Xchg_LightMap(*this); }


public:
    //! \brief Create a Xchg_LightMapPtr.
    //! \param Ambient RGB color
    //! \param Diffuse RGB color
    //! \param Specular RGB color
    //! \return the constructed Xchg_LightMapPtr.
    static Xchg_LightMapPtr Create(Xchg_RGB inAmbiant,Xchg_RGB inDiffuse, Xchg_RGB inSpecular);


    //! \DtkDynamicType
    inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
    { 
        if (inId == _typeID) return 1; 
        return Xchg_Object::DtkDynamicType(inId); 
    }

    //! \DtkDynamicCast
    inline static Xchg_LightMap* DtkDynamicCast(Xchg_Object* s)
    { 
        if (s && s->DtkDynamicType(_typeID)) 
            return static_cast<Xchg_LightMap*>(s); 
        return NULL; 
    } 

    //! \brief Returns the ambient light emitted from the surface
    Xchg_RGB GetAmbientColor()const;  
    
    //! \brief Returns the ambient ratio
	//! \remark : Default ratio is 1.0
    Xchg_Double64 GetAmbientRatio()const;
    
    //! \brief Returns the ambient light diffusely reflected from the surface of this object
    Xchg_RGB GetDiffuseColor()const;
    
    //! \brief Returns the diffuse ratio
	//! \remark : Default ratio is 1.0 
    Xchg_Double64 GetDiffuseRatio()const;

    //! \brief Returns the ambient light specularly reflected from the surface of this object
    Xchg_RGB GetSpecularColor()const;
    
    //! \brief Returns the specular ratio 
	//! \remark : Default ratio is 1.0
    Xchg_Double64 GetSpecularRatio()const;


    //! \brief Returns the color and amount of a perfect mirror reflection
    Xchg_RGB GetReflectiveColor()const;
    
    //! \brief Returns the reflective ratio 
	//! \remark : Default ratio is 1.0
    Xchg_Double64 GetReflectiveRatio()const;
    
    //! \brief Returns the color and amount of perfectly refracted light
    Xchg_RGB GetTransparentColor()const;
    
    //! \brief Returns the transparent ratio 
	//! \remark : Default ratio is 0.0
    Xchg_Double64 GetTransparentRatio()const;
    
    //! \brief Returns the color and amount of light emitted from the surface of this object
    Xchg_RGB GetEmissiveColor()const;
    
	//! \brief Returns the emissive ratio 
	//! \remark : Default ratio is 1.0
	Xchg_Double64 GetEmissiveRatio()const;

	//! \brief Returns the Shininess ratio 
	Xchg_Double64 GetShininessRatio()const;

	//! \brief Set the Shininess ratio (0.0 - 1.0)
	void SetShininessRatio(Xchg_Double64 inShininess);

    //! \brief Adds the color and amount of a perfect mirror reflection
    void AddReflectiveColor(const Xchg_RGB &inColor);
    
    //! \brief Adds the color and amount of perfectly refracted light
    void AddTransparentColor(const Xchg_RGB &inColor);

	//! \brief Sets the ratio of transparency (0.0 : opaque, 1.0 : transparent)
	void SetTransparentRatio( Xchg_Double64 inRatio );
    
    //! \brief Adds the light emitted from the surface of this object
    void AddEmissiveColor(const Xchg_RGB &inColor);

};
/////////////////////////////////////////////////////////////////////////////



class XCHG_API Xchg_RenderInfos : public Xchg_Object
{
public:
	enum RenderMappingShaderTypeEnum
	{
		Unknown = 0,
		Default = 1,
		ArchitecturalAndDesign = 2,
		PolishedMetal = 3,
		SatinatedMetal = 4,
		BrushedMetal = 5,
		ClearGlass = 6,
		ColoredGlass = 7,
		FrostedGlass = 8,
		PolishedPlastic = 9,
		SoftPlastic = 10,
		Translucent = 11,
		MetallicPaint = 12,
		CarPaint = 13,
		SubsurfaceScattering = 14,
		Diffuse = 15
	};
protected:
    enum { _typeID = XCHG_TYPE_RENDER }; 
    struct Xchg_Handle;
    Xchg_Handle *_Private = nullptr;

    //! \CopyConstructor{inToBeCopied}
    Xchg_RenderInfos(const Xchg_RenderInfos& inToBeCopied);

    //! \sa CreateXchg_LightMap
    Xchg_RenderInfos(const Xchg_string &inName,const Xchg_LightMapPtr &inLigths,const Xchg_TextureInfosPtr &inTexture);

    //! \BaseDestructor 
    virtual ~Xchg_RenderInfos();
    friend class SmartPtr<Xchg_RenderInfos>;

    void _Init();
    void _Copy(const Xchg_RenderInfos& inToBeCopied);
    void _Reset();
	Xchg_Object* Clone() const override { return new Xchg_RenderInfos(*this); }


public:
    //! \brief Create a Xchg_RenderInfosPtr
    //! \param inName name.
    //! \return the constructed Xchg_RenderInfosPtr.
    static Xchg_RenderInfosPtr Create(const Xchg_string &inName);
    
    //! \brief Create a Xchg_RenderInfosPtr
    //! \param inName name.
    //! \param inLights light map.
    //! \param inTexture texture.
    //! \return the constructed Xchg_RenderInfosPtr.
    static Xchg_RenderInfosPtr Create(const Xchg_string &inName,const Xchg_LightMapPtr &inLights,const Xchg_TextureInfosPtr &inTexture);


    //! \DtkDynamicType
    inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
    { 
        if (inId == _typeID) return 1; 
        return Xchg_Object::DtkDynamicType(inId); 
    }

    //! \DtkDynamicCast
    inline static Xchg_RenderInfos* DtkDynamicCast(Xchg_Object* s)
    { 
        if (s && s->DtkDynamicType(_typeID)) 
            return static_cast<Xchg_RenderInfos*>(s); 
        return NULL; 
    } 

    //! \brief Returns the texture name
    Xchg_string GetName() const; 

	//! \brief Returns shader type 
	RenderMappingShaderTypeEnum GetShaderType()const;
    
    //! \brief Set the render name
    void SetName(const Xchg_string& inName);

	//! \brief Set Xchg_LightMap
	void SetLightMap(const Xchg_LightMapPtr &inLights);
    
    //! \brief Set Xchg_Texture
    void SetTexture(const Xchg_TextureInfosPtr &inTexture);
    
    //! \brief Returns Xchg_LightMap
    Xchg_LightMapPtr GetLightMap() const;

    //! \brief Returns Xchg_Texture
    Xchg_TextureInfosPtr GetTexture() const;

    //! \brief Sets render file path
	void SetRenderFilePath(const Xchg_string& inFilePath);
	
    //! \brief Returns render file path
    Xchg_string GetRenderFilePath() const;

	//! \brief Adds the shader type
	void AddShaderType( const RenderMappingShaderTypeEnum & inShaderType );

};


class XCHG_API Xchg_PhysicalMaterialInfos : public Xchg_Object
{
public:

	enum MaterialSymetryType
	{
		UnknownType = 0,
		Isotropic = 1,
		IsotropeTrans = 2,
		Orthotrope = 3
	};

protected:
	enum { _typeID = XCHG_TYPE_MATERIAL_INFOS };
	struct Xchg_Handle;
	Xchg_Handle *_Private = nullptr;

	//! \Copy Constructor{inToBeCopied}
	Xchg_PhysicalMaterialInfos(const Xchg_PhysicalMaterialInfos& inToBeCopied);

	//! \sa Create Xchg_PhysicalMaterialInfos
	Xchg_PhysicalMaterialInfos(const Xchg_string &inName, MaterialSymetryType inType);

	//! \BaseDestructor 
	virtual ~Xchg_PhysicalMaterialInfos();
	friend class SmartPtr<Xchg_PhysicalMaterialInfos>;

	void _Init();
	void _Copy(const Xchg_PhysicalMaterialInfos& inToBeCopied);
	void _Reset();
	Xchg_Object* Clone() const override { return new Xchg_PhysicalMaterialInfos(*this); }

public:

	//! \brief Create a Xchg_MaterialInfosPtr
	//! \param inName name.
	//! \return the constructed Xchg_MaterialInfosPtr.
	static Xchg_PhysicalMaterialInfosPtr Create(const Xchg_string &inName, MaterialSymetryType inType);


	//! \DtkDynamicType
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_Object::DtkDynamicType(inId);
	}

	//! \DtkDynamicCast
	inline static Xchg_PhysicalMaterialInfos* DtkDynamicCast(Xchg_Object* s)
	{
		if (s && s->DtkDynamicType(_typeID))
			return static_cast<Xchg_PhysicalMaterialInfos*>(s);
		return NULL;
	}

	//! \brief Returns the material name
	Xchg_string GetName() const;

	//! \brief Returns the material type as string
	Xchg_string GetTypeAsString() const;

	//! \brief Returns the material type
	MaterialSymetryType GetType() const;


	//! \brief Returns the material density
	Xchg_ErrorStatus GetDensity(Xchg_Double64 &outValue, Xchg_string &outUnits) const;
	//! \brief Set the material density
	Xchg_ErrorStatus SetDensity(Xchg_Double64 inValue, const Xchg_string &inUnits);

	//! \brief Returns the material Thermal Expansion
	Xchg_ErrorStatus GetThermalExpansion(Xchg_Double64 &outValue, Xchg_string &outUnits) const;
	//! \brief Set the material Thermal Expansion
	Xchg_ErrorStatus SetThermalExpansion(Xchg_Double64 inValue, const Xchg_string &inUnits);

	//! \brief Returns the material Thermal Conductivity
	Xchg_ErrorStatus GetThermalConductivity(Xchg_Double64 &outValue, Xchg_string &outUnits) const;
	//! \brief Set the material Thermal Conductivity
	Xchg_ErrorStatus SetThermalConductivity(Xchg_Double64 inValue, const Xchg_string &inUnits);


	//! \brief Returns the material Yield Strength
	Xchg_ErrorStatus GetYieldStrength(Xchg_Double64 &outValue, Xchg_string &outUnits) const;
    //! \brief Set the material Yield Strength
	Xchg_ErrorStatus SetYieldStrength(Xchg_Double64 inValue, const Xchg_string &inUnits);

	//! \brief Returns the material Specific Heat
	Xchg_ErrorStatus GetSpecificHeat(Xchg_Double64 &outValue, Xchg_string &outUnits) const;
	//! \brief Set the material Specific Heat 
	Xchg_ErrorStatus SetSpecificHeat(Xchg_Double64 inValue, const Xchg_string &inUnits);

	//! \brief Returns the material Tensile Strength
	Xchg_ErrorStatus GetTensileStrength(Xchg_Double64 &outValue, Xchg_string &outUnits) const;
	//! \brief Set the material Tensile Strength 
	Xchg_ErrorStatus SetTensileStrength(Xchg_Double64 inValue, const Xchg_string &inUnits);


	//! \brief Returns the material Young Modulus
	Xchg_ErrorStatus GetYoungModulus(Xchg_Double64 outYoungModulus[3], Xchg_string &outUnits ) const;
	//! \brief Set the material Young Modulus
	Xchg_ErrorStatus SetYoungModulus(Xchg_Double64 inYoungModulus[3], const Xchg_string &inUnits );

	//! \brief Returns the material Shear Modulus
	Xchg_ErrorStatus GetShearModulus(Xchg_Double64 outShearModulus[3], Xchg_string &outUnits) const;
	//! \brief Set the material Shear Modulus
	Xchg_ErrorStatus SetShearModulus(Xchg_Double64 inShearModulus[3], const Xchg_string &inUnits);


	//! \brief Returns the material Poisson Ratio
	Xchg_ErrorStatus GetPoissonRatio(Xchg_Double64 outPoissonRatio[3]) const;
	//! \brief Set the material Poisson Ratio
	Xchg_ErrorStatus SetPoissonRatio(Xchg_Double64 inPoissonRatio[3]);

	//! \brief Returns the material Hardening Ratio
	Xchg_ErrorStatus GetHardeningRatio(Xchg_Double64 ouHardeningRatio[3]) const;
	//! \brief Set the material Hardening Ratio
	Xchg_ErrorStatus SetHardeningRatio(Xchg_Double64 inHardeningRatio[3]);

	//! \brief Set the material Custom data
	Xchg_ErrorStatus SetCustomData( Xchg_vector<Xchg_MetaDataPtr> const & inCustomData );

	//! \brief Returns the material Custom data
	Xchg_vector<Xchg_MetaDataPtr> const & GetCustomData();

};


#endif
