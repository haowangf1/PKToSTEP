#ifndef _MESH_MESH_CURVE_HPP_
#define _MESH_MESH_CURVE_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_pnt.hpp"
#include "base/xchg_rgb.hpp"
#include "base/xchg_transfo.hpp"
#include "base/xchg_vector.hpp"
#include "mesh/xchg_mesh_circle_data.hpp"

enum XCHG_MESH_CRV_TYPE  { XCHG_MESH_UNKNOWN_CRV_TYPE, XCHG_MESH_CIRCLE, XCHG_MESH_LINE, XCHG_MESH_OTHERCRV };

//! \class Xchg_mesh_curve
//! \brief This is a high level curve class.
//!
//!    This class gathers all the datas related to a curve.
//! In this class, vertices are indexed or not.
class XCHG_API Xchg_mesh_curve
{
        int                    type;        // kind or crv
        XCHG_MESH_CRV_TYPE      crv_type;    // crv type
        Xchg_vector<int>           indices;     // si type != 120
        Xchg_vector<Xchg_pnt>       vertices;    // si type == 120
        Xchg_RGB                color;
        unsigned long          id;
        int                    line_type;
        float                  thickness;
        Xchg_mesh_circle_data * circle_data;

    public:

        //! \brief Default constructor
        Xchg_mesh_curve();
        //! \brief Default destructor
        ~Xchg_mesh_curve();

        //! \brief Add an indexed vertex to the curve (indice to an external vertices array, like the one inside Xchg_mesh)
        //! \param inIndice : indice
        void    add_indice(int inIndice);
        //! \brief Get an indice from the indices array. Indices array size must be greater than inNum
        //! \param inNum : indice to return
        int        get_indice(Xchg_Size_t inNum);
        //! \brief Pop the last indice from the indices array
        int        pop_indice();

        //! \brief Add a vertex to the vertices array
        //! \param inVertex : vertex to add
        void    add_vertex(Xchg_pnt inVertex);
        //! \brief Get a vertex from the verxtex array
        //! \param inIndice : number of the vertex to get
        //! \param[out] outVertex : vertex returned
        //! \return return 0 if ok
        int        get_vertex(Xchg_Size_t inIndice, Xchg_pnt * outVertex);
        //! \brief Pop the last vertex from the vertices array
        int        pop_vertex(Xchg_pnt*);

        //! \brief Return the vertices array size
        Xchg_Size_t     get_nb_vertices();
        //! \brief Return the indices array size
        Xchg_Size_t     get_nb_indices();

        //! \brief Set an ID to the curve
        //! \param inID : ID
        void            set_id(unsigned long inID);
        //! \brief Return the ID of the curve
        unsigned long    get_id();

        //! \brief Set the curve type 
        //! \param inType : type of the curve, can be : 
        //!
        //!
        //!    -    \b XCHG_CGR_CRV_WIREFRAME_INDICE        : wireframe curve with indexed vertices\n
        //!    -    \b XCHG_CGR_CRV_WIREFRAME_PNTS        : wireframe curve non indexed\n
        //!    -    \b XCHG_CGR_CRV_BOUNDARY_INDICE        : boundary curve with indexed vertices\n
        //!    -    \b XCHG_CGR_CRV_BOUNDARY_PNTS        : boundary curve non indexed\n
        //!    -    \b XCHG_CGR_CRV_NO_BOUNDARY_INDICE    : internal\n
        //!    -    \b XCHG_CGR_CRV_NO_BOUNDARY_PNTS        : internal\n
        //!\n
        void set_type(int inType);
        //! \brief Return the curve type
        int get_type();
        //! \brief Set the curve nature
        //! \param type : can be XCHG_MESH_CIRCLE, XCHG_MESH_LINE or XCHG_MESH_OTHERCRV
        //! if XCHG_MESH_CIRCLE, the Xchg_mesh_curve object must have a mesh_circle_data object set (see set_mesh_circle_data(..) help)
        void set_crv_type(XCHG_MESH_CRV_TYPE type);
        //! \brief Return the curve nature
        //! \return XCHG_MESH_CRV_TYPE value
        XCHG_MESH_CRV_TYPE get_crv_type();

        //! \brief Set a RGB color to the curve
        //! \param R, G, B : RGB values
        void    set_color(int R, int G, int B);
        //! \brief Return the curve color
        //! \return A Xchg_RGB object is returned
        Xchg_RGB    get_color();

        //! \brief Set thickness in Xchg_mesh_curve
        void set_thickness(float inThickness);

        //! \brief Get thickness of Xchg_mesh_curve
        float get_thickness();

        //! \brief Set line type in Xchg_mesh_curve
        void set_linetype(int inLineType);

        //! \brief Get line type of Xchg_mesh_curve
        int get_linetype();

        //! \brief Set a mesh_circle_data object to the face, in order to add circular datas (for circles)
        //! \param inCircle A Xchg_mesh_circle_data object, filled.
        void                    set_mesh_circle_data(Xchg_mesh_circle_data * inCircle);
        Xchg_mesh_circle_data *    get_mesh_circle_data();
        //! \brief
        Xchg_mesh_curve * transform2Cpy(Xchg_transfo * inMatrix);
};

#endif // _MESH_MESH_CURVE_HPP_

