#ifndef _MESH_DRW_HPP_
#define _MESH_DRW_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_string.hpp"
#include "base/xchg_picture.hpp"
#include "base/xchg_vector.hpp"
#include "mesh/xchg_material_value.hpp"

class XCHG_API Xchg_drw
{
public:
    Xchg_string name;
    //! \brief picture
    Xchg_picture* picture;
    //! \brief picture
    Xchg_string picturename;
    Xchg_vector<Xchg_material_value> value;

    void _Copy(const Xchg_drw* s);
    Xchg_drw();
    ~Xchg_drw();
};

#endif // _MESH_DRW_HPP_

