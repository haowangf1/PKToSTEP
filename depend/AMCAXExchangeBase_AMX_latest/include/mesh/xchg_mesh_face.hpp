#ifndef _MESH_MESH_FACE_HPP_
#define _MESH_MESH_FACE_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_string.hpp"
#include "base/xchg_pnt.hpp"
#include "base/xchg_dir.hpp"
#include "base/xchg_rgb.hpp"
#include "base/xchg_objectbase.hpp"
#include "base/xchg_smartptr.hpp"
#include "base/xchg_vector.hpp"
#include "mesh/xchg_mesh_circle_data.hpp"

// Forward declarations
class Xchg_mesh;

enum XCHG_MESH_TYPE_FACE { XCHG_MESH_UNKNOWN_TYPE_FACE, XCHG_MESH_CYLINDER, XCHG_MESH_PLANE, XCHG_MESH_OTHERSRF };
class Xchg_mesh_face;
typedef SmartPtr<Xchg_mesh_face> Xchg_mesh_facePtr;

//! \class Xchg_mesh_face
//! \brief This is a high level face class.
//!
//!    This class gathers all the datas related to a triangulated mesh face.
class XCHG_API Xchg_mesh_face:public Xchg_Object
{
private:
	friend class SmartPtr<Xchg_mesh_face>;
protected:
	enum {
		_typeID = XCHG_TYPE_MESH_FACE
	}; 		
	Xchg_mesh * mesh;
// all types
	Xchg_vector<Xchg_UInt32> triangles;
    Xchg_vector<Xchg_vector<Xchg_UInt32> > polygons;
	Xchg_vector<Xchg_vector<Xchg_UInt32> > tristrips;
	Xchg_vector<Xchg_vector<Xchg_UInt32> > trifans;
	Xchg_vector<Xchg_vector<Xchg_vector<Xchg_UInt32> > > mockup;
	Xchg_vector<Xchg_vector<Xchg_UInt32> > polylines;
	Xchg_vector<Xchg_UInt32> points;
	Xchg_vector<Xchg_InfoPtr> subinfos;
// additionnal
    Xchg_InfoPtr _infos;
	
	int id;                              // Face ID
	Xchg_RGB face_color;                  // Face color
	int face_icolor;                     // Face color index
	XCHG_MESH_TYPE_FACE face_type;        // Face type
	Xchg_dir face_normal;                 // Face normal
	Xchg_string face_name;                // Face name
	Xchg_mesh_circle_data circle_data;    // Circle data (for cylinders)
	Xchg_bool circle_data_set;            // Flag indicating if circle_data is set
	friend class Xchg_mesh;
private:
	int explode_tristrips(Xchg_vector<Xchg_UInt32>* ret);
	int explode_trifans(Xchg_vector<Xchg_UInt32>* ret);
	int explode_polygons(Xchg_vector<Xchg_UInt32>* ret);
	int explode_mockups(Xchg_vector<Xchg_UInt32>* ret);
	int add_shapevertices(const Xchg_vector<Xchg_pnt> * inVertices,const  Xchg_vector<Xchg_dir> * inNormals,const  Xchg_vector<Xchg_RGB> * inColors,const  Xchg_vector<Xchg_Float32> * inU, const Xchg_vector<Xchg_Float32> * inV);

	int _finddoublonstriangles();
// forbidden
	Xchg_mesh_face& operator=(const Xchg_mesh_face& inMesh);
public:
    //! \brief Constructor
    //! \param inMesh : the mesh which will contain the face
    Xchg_mesh_face(Xchg_mesh * inMesh);

	//! \brief Copy Constructor
	//! \brief Do not use param shift.
	Xchg_mesh_face(const Xchg_mesh_face& inMeshface,Xchg_mesh* inMesh,Xchg_UInt32 shift=0);
	
    //! \brief Destructor
    ~Xchg_mesh_face();

public:
    Xchg_Object* Clone() const override { return new Xchg_mesh_face(*this); }

	//! \DtkDynamicType
	inline Xchg_Int32 DtkDynamicType(const Xchg_Int32& inId)
	{
		if (inId == _typeID) return 1;
		return Xchg_Object::DtkDynamicType(inId);
	}

	//! \DtkDynamicCast
	inline static Xchg_mesh_face* DtkDynamicCast(Xchg_Object* s)
	{
		if (s && s->DtkDynamicType(_typeID))
			return static_cast<Xchg_mesh_face*>(s);
		return NULL;
	}

	//! \CopyConstructor{inToBeCopied}
	static Xchg_mesh_facePtr Create(const Xchg_mesh_face &inToBeCopied);

    //! \brief Compute round normals from geometry
    //! \warning Can alter vertex_format
    //! \return return 0 if ok, 1 if error
    int compute_round_normals();

    //! \brief Compute the mesh Bounding Box
    //! \param[out] min, max : computed extrem points of the boundig box
    //! \return return 0 if ok
    int compute_bb(Xchg_pnt * min, Xchg_pnt * max);

    //! \brief Compute the mesh Bounding Box
    //! \param[out] in : computed center point of the boundig box
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

	int makestrips();

    //! \brief Compute all facets (triangles) holded by the class, including trianglestrips, fans, polygons
	//! \return Number of facets
	Xchg_Size_t get_nb_facets();

	//! \brief Compute the total number of vertices used in the face
	//! \param[in] inOnce (default = true) : Imagine a case we have 2 adjascent triangles, with 2 common vertices. If inOnce = true, return 4, if false, return 6.
	//! \return return an array of indices.
	Xchg_Size_t get_nb_vertices(int inOnce=1);

	Xchg_mesh* get_mesh();


// triangles

	//! \brief Add one of multiple triangles
	//! \brief On this form, indices MUST match an existing vertex inside Xchg_Mesh class
	//! \param[in] inIndices : Array of indices. Must be a multiple of 3. This will add arraysize/3 triangles
	//! \return return 0 if ok, other value if fails.
	int add_triangles(Xchg_vector<Xchg_UInt32> * inIndices);
	int add_triangles(const Xchg_UInt32* inIndices,Xchg_Size_t nb);

	//! \brief Add one of multiple triangles and the corresponding vertices
	//! \param[in] inIndices : Array of indices. Must be a multiple of 3. This will add arraysize/3 triangles.
	//! \param[in] inVertices : Array of vertices. They will be added into the mesh. inIndices values are indices of this table
	//! \param[in] inNormals : Array of normals (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \param[in] inColors : Array of RGBcolors (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \param[in] inU : Array of u params (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \param[in] inV : Array of v params (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \return return 0 if ok, other value if fails.
	int add_triangles(const Xchg_vector<Xchg_UInt32> * inIndices,const Xchg_vector<Xchg_pnt> * inVertices,const  Xchg_vector<Xchg_dir> * inNormals = NULL,const  Xchg_vector<Xchg_RGB> * inColors = NULL,const  Xchg_vector<Xchg_Float32> * inU = NULL,const  Xchg_vector<Xchg_Float32> * inV = NULL);
	
	//! \brief Get the number of simple triangles kept in the class instance
	//! \return return Number of triangles
	Xchg_Size_t get_nb_triangles() const;

	//! \brief Get pointer of triangle indices of i-th triangle.
	//! \param[in] inI : triangle to get (use get_nbtriangles to see how many triangles there is)
	//! \brief Use the result as an array (check example below)
	//! \b Sample:
	//! \code 
	//! const Xchg_UInt32* t = get_triangle_indices(5);   // get the 5th triangle indices
	//! printf("indices are : %d %d %d\n",t[0],t[1],t[2]);
	//! \endcode
	//! \return return Number of triangles
	const Xchg_UInt32* get_triangle_indices(Xchg_Size_t inI) const;

	//! \brief Get the V-th vertex of the inI-th triangle.
	//! \param[in] inI : triangle to get (use get_nbtriangles to see how many triangles there is)
	//! \param[in] inV : Vertex to get (0, 1 or 2)
	//! \return The vertex.
	Xchg_pnt get_triangle_vertex(Xchg_Size_t inI,Xchg_Size_t inV) const;

	//! \brief Get the center normal of the inI-th triangle.
	//! \param[in] inI : triangle to get (use get_nbtriangles to see how many triangles there is)
	//! \return the normal of the triangle .
	Xchg_dir get_triangle_normal(Xchg_Size_t inI) const;

	//! \brief Reverse the i-th triangle of the set of triangle, so that the normal will be inversed.
	//! \param[in] inI : triangle to reverse (use get_nbtriangles to see how many triangles there is)
	void reverse_triangle(Xchg_Size_t inI);

// trianglestrip
	//! \brief Add one triangle strip
	//! \brief On this form, indices MUST match an existing vertex inside Xchg_Mesh class
	//! \param[in] inIndices : Array of indices. Must be a at least 3.
	//! \return return 0 if ok, other value if fails.
	int add_triangle_strip(const Xchg_vector<Xchg_UInt32> * inIndices);

	int reserve_triangle_strip(Xchg_Size_t nb);

	//! \brief Add one triangle strip and the corresponding vertices
	//! \param[in] inIndices : Array of indices. Must be at least 3.
	//! \param[in] inVertices : Array of vertices. They will be added into the mesh. inIndices values are indices of this table
	//! \param[in] inNormals : Array of normals (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \param[in] inColors : Array of RGBcolors (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \param[in] inU : Array of u params (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \param[in] inV : Array of v params (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \return return 0 if ok, other value if fails.
	int add_triangle_strip(const Xchg_vector<Xchg_UInt32> * inIndices,const Xchg_vector<Xchg_pnt> * inVertices,const Xchg_vector<Xchg_dir> * inNormals = NULL,const  Xchg_vector<Xchg_RGB> * inColors = NULL,const  Xchg_vector<Xchg_Float32> * inU = NULL,const  Xchg_vector<Xchg_Float32> * inV = NULL);

	//! \brief Get the number of triangle strips kept in the class instance
	//! \return return Number of triangle strips
	Xchg_Size_t get_nbtriangle_strips() const;

	//! \brief Get inI-th triangle strip
	//! \param[in] inI : indice of the triangle strip to get.
	//! \return return an array of indices.
	const Xchg_vector<Xchg_UInt32>* get_triangle_strip_indices(Xchg_Size_t inI) const;

// trianglefan
	//! \brief Add one triangle fan
	//! \brief On this form, indices MUST match an existing vertex inside Xchg_Mesh class
	//! \param[in] inIndices : Array of indices. Must be a at least 3.
	//! \return return 0 if ok, other value if fails.
	int add_triangle_fan(const Xchg_vector<Xchg_UInt32> * inIndices);

	//! \brief Add one triangle fan and the corresponding vertices
	//! \param[in] inIndices : Array of indices. Must be at least 3.
	//! \param[in] inVertices : Array of vertices. They will be added into the mesh. inIndices values are indices of this table
	//! \param[in] inNormals : Array of normals (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \param[in] inColors : Array of RGBcolors (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \param[in] inU : Array of u params (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \param[in] inV : Array of v params (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \return return 0 if ok, other value if fails.
	int add_triangle_fan(const Xchg_vector<Xchg_UInt32> * inIndices,const  Xchg_vector<Xchg_pnt> * inVertices,const  Xchg_vector<Xchg_dir> * inNormals = NULL,const  Xchg_vector<Xchg_RGB> * inColors = NULL,const  Xchg_vector<Xchg_Float32> * inU = NULL,const  Xchg_vector<Xchg_Float32> * inV = NULL);

	//! \brief Get the number of triangle fans kept in the class instance
	//! \param[in] inI : indice of the triangle fan to get.
	//! \return return Number of triangle fans
	Xchg_Size_t get_nbtriangle_fans() const;

	//! \brief Get inI-th triangle fan
	//! \return return an array of indices.
	const Xchg_vector<Xchg_UInt32>* get_triangle_fan_indices(Xchg_Size_t inI) const;

// polygons
	//! \brief Add one polygon
	//! \brief poygon MUST be convex
	//! \brief On this form, indices MUST match an existing vertex inside Xchg_Mesh class
	//! \param[in] inIndices : Array of indices. Must be a at least 3.
	//! \return return 0 if ok, other value if fails.
	int add_polygon(const Xchg_vector<Xchg_UInt32> * inIndices);

	//! \brief Add one polygon and the corresponding vertices
	//! \brief poygon MUST be convex
	//! \param[in] inIndices : Array of indices. Must be at least 3.
	//! \param[in] inVertices : Array of vertices. They will be added into the mesh. inIndices values are indices of this table
	//! \param[in] inNormals : Array of normals (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \param[in] inColors : Array of RGBcolors (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \param[in] inU : Array of u params (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \param[in] inV : Array of v params (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \return return 0 if ok, other value if fails.
	int add_polygon(const Xchg_vector<Xchg_UInt32> * inIndices,const  Xchg_vector<Xchg_pnt> * inVertices,const  Xchg_vector<Xchg_dir> * inNormals = NULL,const  Xchg_vector<Xchg_RGB> * inColors = NULL,const  Xchg_vector<Xchg_Float32> * inU = NULL,const  Xchg_vector<Xchg_Float32> * inV = NULL);

	//! \brief Get the number of polygons kept in the class instance
	//! \return return Number of polygons
	Xchg_Size_t get_nbpolygons() const;

	//! \brief Get inI-th polygon
	//! \param[in] inI : indice of the polygon to get.
	//! \return return an array of indices.
	const Xchg_vector<Xchg_UInt32>* get_polygon_indices(Xchg_Size_t inI) const;


	Xchg_dir get_polygon_normal(Xchg_Size_t inI) const;

// mockups
	//! \brief Add one mockup
	//! \brief On this form, indices MUST match an existing vertex inside Xchg_Mesh class
	//! \param[in] inIndices : Array of Array of indices (ie : Array of polygons). The first polygon is the outer limit of the mockups, the others are inner loops
	//! \brief Note : Polygons MUST be convex
	//! \brief Note : Face is ALWAYS on the left of polygon. It means that for outer polygon, the sens is counterclockwise, for the other, sense is clockwise
	//! \return return 0 if ok, other value if fails.
	int add_mockup(const Xchg_vector<Xchg_vector<Xchg_UInt32> > * inIndices);

	//! \brief Add one polygon and the corresponding vertices
	//! \brief poygon MUST be convex
	//! \param[in] inIndices : Array of Array of indices (ie : Array of polygons). The first polygon is the outer limit of the mockups, the others are inner loops
	//! \brief Note : Polygons MUST be convex
	//! \brief Note : Face is ALWAYS on the left of polygon. It means that for outer polygon, the sens is counterclockwise, for the other, sense is clockwise
	//! \param[in] inVertices : Array of vertices. They will be added into the mesh. inIndices values are indices of this table
	//! \param[in] inNormals : Array of normals (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \param[in] inColors : Array of RGBcolors (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \param[in] inU : Array of u params (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \param[in] inV : Array of v params (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \return return 0 if ok, other value if fails.
	int add_mockup(const Xchg_vector<Xchg_vector<Xchg_UInt32> > * inIndices, const Xchg_vector<Xchg_pnt> * inVertices, const Xchg_vector<Xchg_dir> * inNormals = NULL, const Xchg_vector<Xchg_RGB> * inColors = NULL, const Xchg_vector<Xchg_Float32> * inU = NULL, const Xchg_vector<Xchg_Float32> * inV = NULL);

	int add_mockups(const Xchg_vector<Xchg_vector<Xchg_vector<Xchg_UInt32> > >* inIndices, const Xchg_vector<Xchg_pnt> * inVertices, const Xchg_vector<Xchg_dir> * inNormals = NULL, const Xchg_vector<Xchg_RGB> * inColors = NULL, const Xchg_vector<Xchg_Float32> * inU =NULL , const Xchg_vector<Xchg_Float32> * inV = NULL);

	//! \brief Get the number of mockups kept in the class instance
	//! \return return Number of mockups
	Xchg_Size_t get_nbmockups() const;

	//! \brief Get inI-th mockup
	//! \return return an array of indices.
	const Xchg_vector<Xchg_vector<Xchg_UInt32> >* get_mockup_indices(Xchg_Size_t inI) const;

	Xchg_dir get_mockup_normal(Xchg_Size_t inI) const;

// polylines

	//! \brief Add one polyline
	//! \brief On this form, indices MUST match an existing vertex inside Xchg_Mesh class
	//! \param[in] inIndices : Array of indices. Must be a at least 3.
	//! \return return 0 if ok, other value if fails.
	int add_polyline(const Xchg_vector<Xchg_UInt32> * inIndices);

	//! \brief Add one polyline and the corresponding vertices
	//! \param[in] inIndices : Array of indices. Must be at least 2.
	//! \param[in] inVertices : Array of vertices. They will be added into the mesh. inIndices values are indices of this table
	//! \param[in] inNormals : Array of normals (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \param[in] inColors : Array of RGBcolors (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \param[in] inU : Array of u params (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \param[in] inV : Array of v params (or NULL is not specified). If specified, size must be the same than inVertices.
	//! \return return 0 if ok, other value if fails.
	int add_polyline(const Xchg_vector<Xchg_UInt32> * inIndices,const  Xchg_vector<Xchg_pnt> * inVertices,const  Xchg_vector<Xchg_dir> * inNormals = NULL,const  Xchg_vector<Xchg_RGB> * inColors = NULL,const  Xchg_vector<Xchg_Float32> * inU = NULL,const  Xchg_vector<Xchg_Float32> * inV = NULL);

	void reserve_polylines(Xchg_Size_t nb);
	//! \brief Get the number of polylines kept in the class instance
	//! \return return Number of polylines
	Xchg_Size_t get_nbpolylines() const;

	//! \brief Get inI-th polygon
	//! \param[in] inI : indice of the polygon to get.
	//! \return return an array of indices.
	const Xchg_vector<Xchg_UInt32>* get_polyline_indices(Xchg_Size_t inI) const;

// points

	int add_point(Xchg_UInt32 inpnt);
	int add_points(const Xchg_vector<Xchg_UInt32> * pnts);
	int add_points(const Xchg_vector<Xchg_UInt32> * inIndices,const  Xchg_vector<Xchg_pnt> * inVertices,const  Xchg_vector<Xchg_dir> * inNormals = NULL,const  Xchg_vector<Xchg_RGB> * inColors = NULL,const  Xchg_vector<Xchg_Float32> * inU = NULL,const  Xchg_vector<Xchg_Float32> * inV = NULL);
	Xchg_Size_t get_nbpoints() const;
	Xchg_UInt32 get_point_indice(Xchg_Size_t inI) const;


// subinfos

	int add_subinfo(Xchg_InfoPtr& ininf);
	Xchg_Size_t get_nbsubinfos() const;
	Xchg_InfoPtr& get_subinfo(Xchg_Size_t inI);
	const Xchg_InfoPtr& get_subinfo(Xchg_Size_t inI) const;

	//! \brief Reduce memory cost of the Xchg_mesh, to call after adding all geometry
	//! \brief The Xchg_mesh class hold internal containers that keep more memory not to reallocate each data enqueuing (like reserve of a std::vector)
	//! \brief This function remove all reserves to free some useless memory
	void reduce();

   //! \brief Merge a face into the current face
	//! \param[in] inMeshface : Mesh face to append to the current face.
   void merge(Xchg_mesh_face* inMeshface);

// additionnal infos
	//! \brief acces to class info of mesh_face
   Xchg_InfoPtr& info();
   //! \brief acces to class info of mesh_face
   const Xchg_InfoPtr& info() const;

	//! \brief Set an Id to the face
	//! \param inId : set an Id to the face
	void set_id(int inId);
	//! \brief Return the face Id
	int get_id();

	//! \brief Set a color to the face. Sometimes, one needs to set a color to a face, not the vertices.
	//! \param inColor : RGB color
	void set_face_color(const Xchg_RGB& inColor);
	//! \brief Return the RGB face color
	Xchg_RGB get_face_color();

	//! \brief Return the RGB face color (indice)
	int get_face_icolor();
	//! \brief Set a color to the face (indexed). Sometimes, one needs to set a color to a face, not the vertices.
	//! \param inColor : indice of a RGB color
	void set_face_icolor(int inColor);

	//! \brief Return the current face type
	XCHG_MESH_TYPE_FACE get_face_type();
	//! \brief Set a face type
	//! \param inType : face type. Can be XCHG_MESH_CYLINDER, XCHG_MESH_PLANE or XCHG_MESH_OTHERSRF
	void set_face_type(XCHG_MESH_TYPE_FACE inType);

	//! \brief Return the mesh_circle_data of the current face. If it doesn't exist, NULL is returned.
	Xchg_mesh_circle_data get_mesh_circle_data();
	//! \brief Set a mesh_circle_data object to the face, in order to add circular datas (for cylinders)
	//! \param inCircle A Xchg_mesh_circle_data object, filled.
	void set_mesh_circle_data(const Xchg_mesh_circle_data& inCircle);

	//! \brief Some interfaces need to keep a normal for a specific type of face. Use this to retrieve a global face normal
	Xchg_dir get_face_normal();
	//! \brief Some interfaces need to keep a normal for a specific type of face. Use this to store a global face normal
	void set_face_normal(const Xchg_dir&);

	//! \brief Get a name for the face
	Xchg_string get_face_name();
	//! \brief Set a name for the face
	//! \param inName : Name to set.
	void set_face_name(const Xchg_string& inName);

	virtual Xchg_Size_t GetSize() const; 
};


#endif // _MESH_MESH_FACE_HPP_

