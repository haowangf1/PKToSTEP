#ifndef _MESH_ANALYSIS_HPP_
#define _MESH_ANALYSIS_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_string.hpp"
#include "base/xchg_vector.hpp"
#include "mesh/xchg_material_value.hpp"

class XCHG_API Xchg_analysis
{
public:
    Xchg_string name;
    Xchg_vector<Xchg_material_value> value;

    void _Copy(const Xchg_analysis* s);
    Xchg_analysis();
    ~Xchg_analysis();
};

#endif // _MESH_ANALYSIS_HPP_

