#ifndef _MESH_DRAFTING_HPP_
#define _MESH_DRAFTING_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_string.hpp"
#include "base/xchg_vector.hpp"
#include "mesh/xchg_material_value.hpp"
#include "mesh/xchg_drw.hpp"

class XCHG_API Xchg_drafting
{
public:
    Xchg_string name;
    Xchg_vector<Xchg_drw*> drw;
    Xchg_vector<Xchg_material_value> value;

    void _Copy(const Xchg_drafting* s);
    Xchg_drafting();
    ~Xchg_drafting();
};

#endif // _MESH_DRAFTING_HPP_

