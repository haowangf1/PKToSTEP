#ifndef _MESH_POSITIONED_HPP_
#define _MESH_POSITIONED_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_string.hpp"
#include "base/xchg_transfo.hpp"
#include "base/xchg_vector.hpp"
#include "mesh/xchg_material_value.hpp"

class XCHG_API Xchg_positioned
{
public:
    Xchg_string name;
    Xchg_vector<Xchg_material_value> value;
    Xchg_transfo* matrix;

    void _Copy(const Xchg_positioned* s);
    Xchg_positioned();
    ~Xchg_positioned();
};

#endif // _MESH_POSITIONED_HPP_

