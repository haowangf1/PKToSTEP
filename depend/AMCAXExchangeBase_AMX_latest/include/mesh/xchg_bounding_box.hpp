#ifndef _MESH_BOUNDING_BOX_HPP_
#define _MESH_BOUNDING_BOX_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_pnt.hpp"
#include "base/xchg_vector.hpp"

/// @cond OLD_XCHG_API
struct XCHG_API Xchg_bounding_box
{
    Xchg_pnt A;
    Xchg_pnt B;
    Xchg_pnt M;
};

typedef Xchg_bounding_box xchg_mesh_face_bb;
typedef Xchg_vector<xchg_mesh_face_bb> xchg_mesh_body_face_bb;

typedef Xchg_vector<xchg_mesh_body_face_bb> xchg_mesh_per_body_face_bb;
typedef Xchg_bounding_box xchg_mesh_body_bb;

typedef Xchg_vector<xchg_mesh_body_bb> xchg_mesh_bodies_bb;
typedef Xchg_bounding_box xchg_mesh_all_bodies_bb;

typedef Xchg_bounding_box xchg_mesh_crv_bb;
typedef Xchg_vector<xchg_mesh_crv_bb> xchg_mesh_per_crv_bb;
typedef Xchg_bounding_box xchg_mesh_all_crvs_bb;

#endif // _MESH_BOUNDING_BOX_HPP_

