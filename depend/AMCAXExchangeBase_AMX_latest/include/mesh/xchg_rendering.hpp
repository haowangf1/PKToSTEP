#ifndef _MESH_RENDERING_HPP_
#define _MESH_RENDERING_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_string.hpp"
#include "base/xchg_vector.hpp"
#include "mesh/xchg_material_value.hpp"

class XCHG_API Xchg_rendering
{
public:
    Xchg_string name;
    Xchg_vector<Xchg_material_value> value;
    
    void _Copy(const Xchg_rendering* s);
    Xchg_rendering();
    ~Xchg_rendering();
};

#endif // _MESH_RENDERING_HPP_

