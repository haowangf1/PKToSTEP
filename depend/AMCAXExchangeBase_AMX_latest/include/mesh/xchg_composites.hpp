#ifndef _MESH_COMPOSITES_HPP_
#define _MESH_COMPOSITES_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_string.hpp"
#include "base/xchg_vector.hpp"
#include "mesh/xchg_material_value.hpp"

class XCHG_API Xchg_composites
{
public:
    Xchg_string name;
    Xchg_vector<Xchg_material_value> value;

    void _Copy(const Xchg_composites* s);
    Xchg_composites();
    ~Xchg_composites();
};

#endif // _MESH_COMPOSITES_HPP_

