#ifndef _MESH_MESH_VERTEX_HPP_
#define _MESH_MESH_VERTEX_HPP_

#include "base/xchg_export.hpp"
#include "base/xchg_type.hpp"
#include "base/xchg_pnt.hpp"
#include "base/xchg_dir.hpp"
#include "base/xchg_rgb.hpp"
#include "base/xchg_transfo.hpp"

/* Class mesh_vertex : vertex data */

//! \class Xchg_mesh_vertex
//! \brief For internal use.
//!
//!    This class gathers all the datas related to a vertex.
class XCHG_API Xchg_mesh_vertex
{
protected:
	Xchg_Float32 vertex[3];
public:
	Xchg_mesh_vertex();
	Xchg_mesh_vertex(const Xchg_pnt * inVertex);
	virtual ~Xchg_mesh_vertex();
	virtual Xchg_pnt GetVertex() const;
	virtual Xchg_dir GetNormal() const;
	virtual Xchg_RGB GetRGB() const;
	virtual Xchg_Float32 GetU() const;
	virtual Xchg_Float32 GetV() const;
	virtual Xchg_Int32 SetVertex(const Xchg_pnt&);
	virtual Xchg_Int32 SetNormal(const Xchg_dir&);
	virtual Xchg_Int32 SetRGB(const Xchg_RGB&);
	virtual Xchg_Int32 SetU(Xchg_Float32);
	virtual Xchg_Int32 SetV(Xchg_Float32);
	virtual Xchg_Int32 transform(Xchg_transfo&);
	virtual Xchg_Size_t GetSize() const ;
};

class XCHG_API Xchg_mesh_vertex_norm:public Xchg_mesh_vertex
{
protected:
	Xchg_Float32 normal[3];
public:
	Xchg_mesh_vertex_norm();
	Xchg_mesh_vertex_norm(const Xchg_pnt * inVertex,const Xchg_dir * inNorm);
	Xchg_dir GetNormal() const;
	Xchg_Int32 SetNormal(const Xchg_dir&);
	Xchg_Int32 transform(Xchg_transfo&);
	 Xchg_Size_t GetSize() const ;
};

class XCHG_API Xchg_mesh_vertex_norm_color:public Xchg_mesh_vertex_norm
{
protected:
	Xchg_RGB color;
public:
	Xchg_mesh_vertex_norm_color();
	Xchg_mesh_vertex_norm_color(const Xchg_pnt * inVertex,const Xchg_dir * inNorm,const Xchg_RGB* inRGB);
	Xchg_RGB GetRGB() const;
	Xchg_Int32 SetRGB(const Xchg_RGB&);
	 Xchg_Size_t GetSize() const ;
};

class XCHG_API Xchg_mesh_vertex_norm_color_uv:public Xchg_mesh_vertex_norm_color
{
protected:
	Xchg_Float32 u,v;
public:
	Xchg_mesh_vertex_norm_color_uv();
	Xchg_mesh_vertex_norm_color_uv(const Xchg_pnt * inVertex,const Xchg_dir * inNorm,const Xchg_RGB* inRGB,Xchg_Float32 inU,Xchg_Float32 inV);
	Xchg_Float32 GetU() const;
	Xchg_Float32 GetV() const;
	Xchg_Int32 SetU(Xchg_Float32);
	Xchg_Int32 SetV(Xchg_Float32);
	 Xchg_Size_t GetSize() const ;
};

#endif // _MESH_MESH_VERTEX_HPP_

