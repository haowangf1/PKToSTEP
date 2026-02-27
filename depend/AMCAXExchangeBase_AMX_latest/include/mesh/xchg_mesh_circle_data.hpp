#ifndef _MESH_MESH_CIRCLE_DATA_HPP_
#define _MESH_MESH_CIRCLE_DATA_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_pnt.hpp"
#include "base/xchg_dir.hpp"

//! \class Xchg_mesh_circle_data
//! \brief This class gathers circle datas.
class XCHG_API Xchg_mesh_circle_data
{
    Xchg_dir axis;
    Xchg_pnt center;
    double    radius;

    public:
        //! \brief Default constructor
        Xchg_mesh_circle_data();
        //! \brief Default destructor
        ~Xchg_mesh_circle_data();

        //! \brief Used to set the circle/cylinder axis
        //! \param inAxis : axis 
        void        set_axis(Xchg_dir inAxis);
        //! \brief Return the circle/cylinder axis
        Xchg_dir        get_axis() const;

        //! \brief Used to set the circle/cylinder center
        //! \param inCenter : center
        void        set_center(Xchg_pnt inCenter);
        //! \brief Return the circle/cylinder center
        Xchg_pnt        get_center() const;

        //! \brief Used to set the circle/cylinder radius
        //! \param inRadius : radius
        void        set_radius(double inRadius);
        //! \brief Return the circle/cylinder radius
        double        get_radius() const;
};

#endif // _MESH_MESH_CIRCLE_DATA_HPP_

