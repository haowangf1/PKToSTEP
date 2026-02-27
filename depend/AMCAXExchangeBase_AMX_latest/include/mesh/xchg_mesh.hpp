#ifndef _MESH_MESH_HPP_
#define _MESH_MESH_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_vector.hpp"
#include "base/xchg_entity.hpp"
#include "base/xchg_stream.hpp"
#include "base/xchg_matrix.hpp"
#include "base/xchg_pnt.hpp"
#include "base/xchg_dir.hpp"
#include "base/xchg_rgb.hpp"
#include "base/xchg_string.hpp"
#include "base/xchg_transfo.hpp"
#include "base/xchg_smartptr.hpp"
#include <cstdio>
#include "mesh/xchg_mesh_vertex.hpp"
#include "mesh/xchg_mesh_face.hpp"
#include "mesh/xchg_material.hpp"

// Forward declarations
class Xchg_facet; 

/* Filtres */

#define XCHG_MESH_XYZ                   1
#define XCHG_MESH_NORMAL                2
#define XCHG_MESH_RGB                   4
#define XCHG_MESH_UV_TEXTURES           8    // textures
#define XCHG_MESH_UV_BREP              16
#define XCHG_MESH_DOUBLEPRECISION	  32
// combinations
#define XCHG_MESH_XYZ_NORMAL            3
#define XCHG_MESH_XYZ_NORMAL_UVTEXTURES 11
#define XCHG_MESH_XYZ_NORMAL_RGB        7
#define XCHG_MESH_ALL                   31

/* Types de triangulation */

enum XCHG_FACET_TYPE { XCHG_UNKNOWN_FACET, XCHG_TRIANGLES, XCHG_TRIANGLE_STRIP, XCHG_TRIANGLE_FAN, XCHG_POLYGONS, XCHG_MOCKUP };

#define XCHG_VALUE_M        1000.0
#define XCHG_VALUE_CM       10.0
#define XCHG_VALUE_MM       1.0
#define XCHG_VALUE_INCH     25.4
#define XCHG_VALUE_FEET     304.8

class Xchg_mesh;
typedef SmartPtr<Xchg_mesh> Xchg_MeshPtr;


//! \class Xchg_mesh 
//! \brief This is a high level mesh class.
//!
//!    This class gathers all the datas related to a triangulated mesh.
class XCHG_API Xchg_mesh:public Xchg_Entity
{
	private:
		friend class SmartPtr<Xchg_mesh>;
	protected:
		enum {
			_typeID = XCHG_TYPE_MESH
		}; 		
		int                            id;
        Xchg_bool                       back_face_culling;
		Xchg_bool					   HasTextureValues; 
        int                            vertex_format;
#ifdef NOCOMPRESSEDVERTICES
		Xchg_vector<Xchg_mesh_vertex *>     vertices;      // hide, and hide subclasses.
#else
		Xchg_vector<Xchg_UChar8>	cvertices;
		Xchg_Size_t sizeonevertex,ofsVTX,ofsNRM,ofsRGB,ofsUV,ofsUVBrep;
		Xchg_Size_t nb_vertices = 0; // Actual number of vertices stored
		void C_GetVertex(Xchg_Size_t i,Xchg_pnt& out) const;
		void C_GetNormal(Xchg_Size_t i,Xchg_dir& out) const;
		void C_GetRGB(Xchg_Size_t i,Xchg_RGB& out) const;
		Xchg_Float32 C_GetU(Xchg_Size_t i) const;
		Xchg_Float32 C_GetV(Xchg_Size_t i) const;
		Xchg_Float32 C_GetUBrep(Xchg_Size_t i) const;
		Xchg_Float32 C_GetVBrep(Xchg_Size_t i) const;
		void C_SetVertex(Xchg_Size_t i,const Xchg_pnt& out);
		void C_SetNormal(Xchg_Size_t i,const Xchg_dir& out);
		void C_SetRGB(Xchg_Size_t i,const Xchg_RGB& out);
		void C_SetUV(Xchg_Size_t i,Xchg_Float32 u,Xchg_Float32 v);
		void C_SetUVBrep(Xchg_Size_t i,Xchg_Float32 u,Xchg_Float32 v);
		void C_ComputeOffsets();
#endif
		Xchg_StreamPtr _stream;
        Xchg_vector<Xchg_mesh_face *>       submeshes; 
        Xchg_MaterialPtr _material;     // Material of the mesh
        Xchg_RGB mesh_color;            // Color of the mesh
        friend class Xchg_mesh_face;
        Xchg_bool check_params(const Xchg_pnt * inVertex, const Xchg_dir * inNormal, const Xchg_RGB * inColor, Xchg_Float32 inU, Xchg_Float32 inV);
        Xchg_bool check_params(const Xchg_vector<Xchg_pnt> * inVertices, const Xchg_vector<Xchg_dir> * inNormals, const Xchg_vector<Xchg_RGB> * inColors, const Xchg_vector<Xchg_Float32> * inU, const Xchg_vector<Xchg_Float32> * inV);
		void clean_up();
        Xchg_Object* Clone() const override { return new Xchg_mesh(*this); }

public:
        Xchg_mesh(int inVertex_format = XCHG_MESH_XYZ, Xchg_bool inBack_face_culling = true);
        Xchg_mesh(const Xchg_mesh &inMesh);
		Xchg_mesh& operator=(const Xchg_mesh &inMesh);
        ~Xchg_mesh();
    
	public:
		XCHG_TYPE_ENUM GetType() const { return XCHG_TYPE_MESH;};

		//! \DtkDynamicType
		inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
		{
			if (inId == _typeID) return 1;
			return Xchg_Entity::DtkDynamicType(inId);
		}

		//! \DtkDynamicCast
		inline static Xchg_mesh* DtkDynamicCast(Xchg_Object* s)
		{
			if (s && s->DtkDynamicType(_typeID))
				return static_cast<Xchg_mesh*>(s);
			return NULL;
		}

		//! \brief Constructor
		//! \param inVertex_format : the chosen vertex format you will work with
		//! \param inBack_face_culling : culling mode
		//!
		//! \b Vertex \b formats \b available: \n
		//!
		//!    -    \b XCHG_MESH_XYZ                    : each point has xyz coords\n
		//!    -    \b XCHG_MESH_NORMAL                 : each point has a normal\n
		//!    -    \b XCHG_MESH_RGB                    : each point has a color\n
		//!    -    \b XCHG_MESH_UVTEXTURES             : each point has UV texture coords\n
		//!    -    \b XCHG_MESH_UVBBREP                : each point has UVbrep match coords\n
		//!    -    \b XCHG_MESH_DOUBLEPRECISION        : each point has xyz coords as double\n
		//!\n
		//!\b For \b example : XCHG_MESH_XYZ | XCHG_MESH_NORMAL | XCHG_MESH_UVTEXTURES : to have coords,normals, and uv textures
		//!\n
		//void dump_vertices();
		static Xchg_MeshPtr Create(int inVertex_format = XCHG_MESH_XYZ, Xchg_bool inBack_face_culling = true);

		//! \brief Copy constructor
		static Xchg_MeshPtr Create(const Xchg_mesh &inToBeCopied);

        //! \brief Checks The current vertex format of the Xchg_mesh object
        //! \param inFormat : vertex format to test with
        //! \return true if the format includes inFormat parameters, else false
        Xchg_bool check_format(int inFormat);

        //! \brief Compute round normals from geometry
        //! \warning Can alter vertex_format
        //! \return return 0 if ok 
        int compute_round_normals();

        //! \brief Compute the mesh Bounding Box
        //! \param[out] min, max : computed extrem points of the boundig box
        //! \return return 0 if ok
        int compute_bb(Xchg_pnt * min, Xchg_pnt * max);

        //! \brief Compute the mesh Bounding Box
        //! \param[out] min, max : computed extrem points of the boundig box
        //! \return return 0 if ok
        int compute_bb_center(Xchg_pnt * i);

		//! \brief Decompose all complex geometry (triangle strips, fan, polygons) into triangles, and append it into the internal triangle array
		//! \brief Note that mockup decomposition is not yet supported.
		//! \return return 0 if ok
		int explode();

		//! \brief Decompose complex geometry (triangle strips, fan, polygons) according to specified parameters into triangles, and append it into the internal triangle array
		//! \brief Note that mockup decomposition is not yet supported.
		//! \return return 0 if ok
		int explode(Xchg_bool ExplodeTristrips, Xchg_bool ExplodeTrifans, Xchg_bool ExplodePolygons, Xchg_bool ExplodeMockups);

		//! \brief Assemble all simple triangle, try to make triange-strips
		//! \return return 0 if ok
		int makestrips();

		//! \brief Change Vertex format, check constructor comment to see options
		//! \param[in] inVertex_format : the chosen vertex format you will work with
		//! \return return 0 if ok
		// int change_vertex_format(int inVertex_format);

        //! \brief Apply a transformation matrix to the Mesh (Deprecated method)
		//! \param[in] inMatrix : Matrix to apply
		//! \return return 0 if ok
        int transform(Xchg_matrix * inMatrix);

		//! \brief Apply a transformation matrix to the Mesh
		//! \param[in] inMatrix : Matrix to apply
		//! \return return 0 if ok
		int transform(Xchg_transfo * inMatrix);

		//! \brief Make a copy of the current mesh, applying a transformation on it.
		//! \param[in] inMatrix : Matrix to apply
		//! \return The copy mesh, NULL if error.
        Xchg_mesh * transform2Cpy(Xchg_transfo * inMatrix);

        //! \brief Return the true of false if culled or not
		//! \return True or false.
        Xchg_bool is_backface_culled();
        int to_stl(Xchg_string inFilename,Xchg_bool inIsAciiMode=XCHG_FALSE);

        //! \brief Return true if the current vertex format includes 3D coords (XYZ)
        Xchg_bool has_xyz();
        //! \brief Return true if the current vertex format includes normal
        Xchg_bool has_normals();
        //! \brief Return true if the current vertex format includes color (RGB)
        Xchg_bool has_colors();
        //! \brief Return true if the current vertex format includes texture coords (UVTEXTURES)
        Xchg_bool has_texcoords();

        //! \brief Add a vertex to the mesh vertices list
        //! \param inVertex : vertex to add
        //! \param inNormal : normal to add (if vertex format includes normal)
        //! \param inColor : color to add (if vertex format includes color)
        //! \param inU, inV : texture coords (if vertex format includes tex coords)
        //! \return return 0 if ok, 1 if error, 2 if wrong vertex format
        int add_vertex(const Xchg_pnt * inVertex,const  Xchg_dir * inNormal = NULL,const  Xchg_RGB * inColor = NULL, Xchg_Float32 inU = 0., Xchg_Float32 inV = 0., Xchg_Float32 inUBrep = 0., Xchg_Float32 inVBrep = 0.);    
        
		int reserve_vertex(Xchg_Size_t nb);
        //! \brief Add a face (Xchg_mesh_face) to the mesh
        //! \param in_mesh_face : face to add
        //! \return return 0 if ok, 1 if error
        int add_mesh_face(Xchg_mesh_face * inMesh_face);

        //! \brief Set a vertex 
        //! \param inPos : indice in the vertices array
        //! \param inVertex : vertex to set
        //! \param inNormal : normal to set (if vertex format includes normal)
        //! \param inColor : color to set (if vertex format includes color)
        //! \param inU, inV : texture coords (if vertex format includes tex coord)
        //! \return return 0 if ok, 1 if error, 2 if wrong vertex format
        int set_vertex(Xchg_Size_t inPos, const Xchg_pnt * inVertex, const Xchg_dir * inNormal = NULL, const Xchg_RGB * inColor = NULL, Xchg_Float32 inU = 0., Xchg_Float32 inV = 0., Xchg_Float32 inUBrep = 0., Xchg_Float32 inVBrep = 0.);

        //! \brief Set a normal to a point (if vertex format includes normal)
        //! \param inPos : indice in the vertices array
        //! \param inNormal : normal to set
        //! \return return 0 if ok, 1 if error, 2 if wrong vertex format
        int set_normal(Xchg_Size_t inPos, const Xchg_dir * inNormal);

        //! \brief Set a color to a point (if vertex format includes color)
        //! \param inPos : indice in the vertices array
        //! \param inColor : color to set
        //! \return return 0 if ok, 1 if error, 2 if wrong vertex format
        int set_color(Xchg_Size_t inPos, const Xchg_RGB * inColor);

        //! \brief Set texture coords to a point (if vertex format includes tex coords)
        //! \param inPos : indice in the vertices array
        //! \param inU, inV : U,V to set
        //! \return return 0 if ok, 1 if error, 2 if wrong vertex format
        int set_texcoords(Xchg_Size_t inPos, Xchg_Float32 inU, Xchg_Float32 inV);

		//! \brief Return texture coords (if vertex format includes tex coords)
		//! \param inPos : indice in the vertices array
		//! \param[out] outU, outV : UVtextures returned
		//! \return return 0 if ok, 1 if error, 2 if wrong vertex format
		int get_texcoords(Xchg_Size_t inPos, Xchg_Float32 * outU, Xchg_Float32 * outV);

        //! \brief Return the vertices array size
        Xchg_Size_t get_nb_vertices() const;

        //! \brief Return a vertex
        //! \param inPos : indice in the vertices array
        //! \param[out] outVertex : vertex returned
        //! \return return 0 if ok, 1 if error, 2 if wrong vertex format
        int get_vertex(Xchg_Size_t inPos, Xchg_pnt * outVertex);
        
		//! \brief Return a vertex (2nd form of get_vertex)
		//! \param inPos : indice in the vertices array
		//! \return The vertex
		Xchg_pnt get_vertex(Xchg_Size_t inPos);

        //! \brief Return a normal (if vertex format includes normal)
		//! \brief Note, if vertex format does not support normal, the fonction will return a null vector
        //! \param inPos : indice in the vertices array
        //! \param[out]: outNormal : vertex returned
        //! \param[in]: inMeshFace : current mesh face
        //! \return return 0 if ok, 1 if error, 2 if wrong vertex format
        int get_normal(Xchg_Size_t inPos, Xchg_dir * outNormal,Xchg_mesh_face * inMeshFace = NULL);

		//! \brief Return a vertex (2nd form of get_normal)
		//! \brief Note, if vertex format does not support normal, the fonction will return a null vector
		//! \param inPos : indice in the vertices array
		//! \return The normal
		Xchg_dir get_normal(Xchg_Size_t inPos);


		Xchg_Float32 GetUTexture(Xchg_Size_t inPos) const;
		Xchg_Float32 GetVTexture(Xchg_Size_t inPos) const;

		Xchg_Float32 GetUBrep(Xchg_Size_t inPos) const;
		Xchg_Float32 GetVBrep(Xchg_Size_t inPos) const;

        //! \brief Return a color (if vertex format includes color)
        //! \param inPos : indice in the vertices array
        //! \param[out] outColor : color returned
        //! \return return 0 if ok, 1 if error, 2 if wrong vertex format
        int get_color(Xchg_Size_t inPos, Xchg_RGB * outColor);    
        
        //! \brief Return the faces array size
        Xchg_Size_t get_nb_mesh_face() const;

        //! \brief Return a face
        //! \param inPos : indice in the faces array
        Xchg_mesh_face* get_mesh_face(Xchg_Size_t inPos);

		//! \brief Return the array of mesh faces
		Xchg_vector<Xchg_mesh_face *> & get_mesh_faces();
		Xchg_vector<Xchg_mesh_face *> const & get_mesh_faces() const;

        //! \brief Return total number of facets
        Xchg_Size_t get_nb_facets();
        
		//! \brief Set an id to the mesh 
		//! \param inId : id to set
		void set_id(int inId);

		//! \brief Return the mesh id
        int get_id();

        //! \brief set color in Xchg_mesh
        void set_mesh_color(const Xchg_RGB & inColor);
        
        //! \brief get color in Xchg_mesh
        Xchg_RGB get_mesh_color();

		//! \brief Return DtkInfosPtr of this Xchg_Mesh may be NULL
        const Xchg_InfoPtr& get_Infos();

		//! \brief Set DtkInfosPtr of this Xchg_Mesh
        void set_Infos(const Xchg_InfoPtr& inDtkInfosPtr);

		//! \brief Set the material of the mesh
        void set_material(Xchg_MaterialPtr inMat);

		//! \brief Get the material of the mesh
        Xchg_MaterialPtr get_material();

		//! \brief Perform a scale of the model
		//! \brief Note : calling this function has the same effect than calling "transform" method with a scale matrix.
        int rescale_model(float ratio);

		//! \brief Reduce memory cost of the Xchg_mesh, to call after adding all geometry
		//! \brief The Xchg_mesh class hold internal containers that keep more memory not to reallocate each data enqueuing (like reserve of a std::vector)
		//! \brief This function remove all reserves to free some useless memory
		//! TODO
		// void reduce();

		//! \brief Merge another Xchg_mesh.
        //! \param inMesh : mesh to merge.
		//! TODO
		// void merge(const Xchg_mesh* inMesh);
		// void merge(const SmartPtr<Xchg_mesh>& inMesh);

		// void mergemove( Xchg_vector<Xchg_MeshPtr>& inArraymesh );
		// void mergemove( SmartPtr<Xchg_mesh>& inmesh );
		// void mergemove( Xchg_mesh* inmesh );
		// void reserve_submesh( Xchg_Size_t nb );


		//! \brief Merge subfaces that have the same properties
		//! TODO
		//void reducesubfaces();

		Xchg_ErrorStatus Transform(const Xchg_transfo&);

		Xchg_Size_t GetSize() const;

        //! \brief Apply texture mapping to the mesh. It will modify points texture UV coordinates.
        // Xchg_bool apply_render_infos(Xchg_RenderInfosPtr);

};

typedef SmartPtr<Xchg_mesh> Xchg_MeshPtr;

#endif // _MESH_MESH_HPP_

