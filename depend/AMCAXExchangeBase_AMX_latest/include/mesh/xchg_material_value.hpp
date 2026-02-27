#ifndef _MESH_MATERIAL_VALUE_HPP_
#define _MESH_MATERIAL_VALUE_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_string.hpp"
#include "base/xchg_val.hpp"
#include "base/xchg_vector.hpp"

class XCHG_API Xchg_material_value
{
public:
    Xchg_string NameProperties;
    Xchg_vector<Xchg_Val > PropertiesValue;
    
    void _Copy(const Xchg_material_value& s);
    Xchg_material_value();
    ~Xchg_material_value();
};

#endif // _MESH_MATERIAL_VALUE_HPP_

