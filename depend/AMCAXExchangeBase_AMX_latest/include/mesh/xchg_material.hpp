#ifndef _MESH_MATERIAL_HPP_
#define _MESH_MATERIAL_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_string.hpp"
#include "base/xchg_picture.hpp"
#include "base/xchg_objectbase.hpp"
#include "base/xchg_smartptr.hpp"
#include "mesh/xchg_rendering.hpp"
#include "mesh/xchg_drafting.hpp"
#include "mesh/xchg_analysis.hpp"
#include "mesh/xchg_composites.hpp"
#include "mesh/xchg_positioned.hpp"

//! \brief Gathers material related datas
class XCHG_API Xchg_material:public Xchg_Object
{
protected:
    enum 
    {
        _typeID = XCHG_TYPE_MATERIAL
    };
private:
    friend class SmartPtr<Xchg_material>;
public:
    //! \brief type of material
    Xchg_string label;

    double ambient;             
    double diffuse;
    double specular;
    double refraction;
    double transparency;
    double reflectivity;
    double roughness;
    double  _Density;

	Xchg_Int32 _renderID;
	Xchg_Int32 _physicalID;
	//! \brief identifiant dans la library de material
	Xchg_Int32 _identifier_library;
    //! \brief Texture clamping setting (u)
    bool repeat_u;
    //! \brief Texture clamping setting (v)
    bool repeat_v;
    //! \brief picture
    Xchg_picture * picture;
    //! \brief Texture filename
    Xchg_string texture;

    //! \brief other properties
    Xchg_rendering  * rendering;
    Xchg_drafting   * drafting;
    Xchg_analysis   * analysis;
    Xchg_composites * composites;
    Xchg_positioned * positioned;

    //! \DtkDynamicType
    inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
    { 
        if (inId == _typeID)
            return 1;
        return Xchg_Object::DtkDynamicType(inId);
    }
    //! \DtkDynamicCast
    inline static Xchg_material* DtkDynamicCast(Xchg_Object* s)
    { 
        if (s && s->DtkDynamicType(_typeID)) 
            return static_cast<Xchg_material*>(s); 
        return NULL; 
    } 

    void _Copy(const Xchg_MaterialPtr& s);
    Xchg_material();
    ~Xchg_material();

    Xchg_Object* Clone() const override;

	inline Xchg_Double64 GetDensity() const {return _Density;}
	inline void SetDensity(const Xchg_Double64 inDensity) {_Density = inDensity;}
	inline Xchg_Int32 GetRenderID() const {return _renderID;}
	inline void SetRenderID(const Xchg_Int32 inRenderID) {_renderID = inRenderID;}
	inline Xchg_Int32 GetPhysicalMaterialID() const { return _physicalID; }
	inline void SetPhysicalMaterialID(const Xchg_Int32 inPhysicalID) { _physicalID = inPhysicalID; }

};

typedef SmartPtr<Xchg_material> Xchg_MaterialPtr;

#endif // _MESH_MATERIAL_HPP_

