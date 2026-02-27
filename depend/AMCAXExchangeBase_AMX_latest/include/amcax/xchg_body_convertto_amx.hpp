#ifndef _XCHG_BODY_CONVERT_TO_AMX_HPP_
#define _XCHG_BODY_CONVERT_TO_AMX_HPP_

#include <map>

#include "amx_kernel.h"
#include "base/xchg_type.hpp"
#include "topology/xchg_body.hpp"

typedef ID AMX_ID;

enum class AMX_TYPE_ENUM {
    AMX_BODY,
    AMX_INF_VOID_REGION,
    AMX_SOLID_REGION,
    AMX_VOID_REGION,
    AMX_FRONT_FACE_SHELL,
    AMX_BACK_FACE_SHELL,
    AMX_OPEN_FACE_SHELL,
    AMX_EDGE_SHELL,
    AMX_CURVE_SHELL,
    AMX_VERTEX_SHELL,
    AMX_POINT_SHELL,
    AMX_FACE,
    AMX_LOOP,
    AMX_FIN,
    AMX_EDGE,
    AMX_CURVE_EDGE,
    AMX_VERTEX,
    AMX_POINT_VERTEX,
};

struct AMXTopo
{
    AMX_TYPE_ENUM _Type;
    Xchg_EntityPtr _Entity;
    Xchg_Size_t _Index;

    // for std::map
    Xchg_bool operator<(const AMXTopo& other) const
    {
        if (this->_Type != other._Type)
        {
            return this->_Type < other._Type;
        }
        else if (this->_Entity != other._Entity)
        {
            return this->_Entity < other._Entity;
        }
        return this->_Index < other._Index;
    }

    static AMXTopo Body(const Xchg_BodyPtr& inBody);
    static AMXTopo InfVoidRegion(const Xchg_BodyPtr& inBody);
    static AMXTopo SolidRegion(const Xchg_LumpPtr& inLump);
    static AMXTopo VoidRegion(const Xchg_ShellPtr& inShell);
    static AMXTopo FrontFaceShell(const Xchg_ShellPtr& inShell);
    static AMXTopo BackFaceShell(const Xchg_ShellPtr& inShell);
    static AMXTopo OpenFaceShell(const Xchg_ShellPtr& inShell);
    static AMXTopo EdgeShell(const Xchg_ShellPtr& inShell, const Xchg_Size_t& inEdgeSetIndex);
    static AMXTopo CurveShell(const Xchg_ShellPtr& inShell, const Xchg_Size_t& inCurveSetIndex);
    static AMXTopo VertexShell(const Xchg_VertexPtr& inVertex);
    static AMXTopo PointShell(const Xchg_PointPtr& inPoint);
    static AMXTopo Face(const Xchg_FacePtr& inFace);
    static AMXTopo Loop(const Xchg_LoopPtr& inLoop);
    static AMXTopo Fin(const Xchg_CoedgePtr& inFin);
    static AMXTopo Edge(const Xchg_EdgePtr& inEdge);
    static AMXTopo CurveEdge(const Xchg_CurvePtr& inCurve);
    static AMXTopo Vertex(const Xchg_VertexPtr& inVertex);
    static AMXTopo PointVertex(const Xchg_PointPtr& inPoint);
};

class BodyConvertToAMX
{
protected:
    // PK topo to PK id maps
    std::map<AMXTopo, AMX_ID> _Body;
    std::map<AMXTopo, AMX_ID> _InfVoidRegion;
    std::map<AMXTopo, AMX_ID> _SolidRegions;
    std::map<AMXTopo, AMX_ID> _VoidRegions;
    std::map<AMXTopo, AMX_ID> _FrontFaceShells;
    std::map<AMXTopo, AMX_ID> _BackFaceShells;
    std::map<AMXTopo, AMX_ID> _OpenFaceShells;
    std::map<AMXTopo, AMX_ID> _EdgeShells;
    std::map<AMXTopo, AMX_ID> _CurveShells;
    std::map<AMXTopo, AMX_ID> _VertexShells;
    std::map<AMXTopo, AMX_ID> _PointShells;
    std::map<AMXTopo, AMX_ID> _Faces;
    std::map<AMXTopo, AMX_ID> _Loops;
    std::map<AMXTopo, AMX_ID> _Fins;
    std::map<AMXTopo, AMX_ID> _Edges;
    std::map<AMXTopo, AMX_ID> _CurveEdges;
    std::map<AMXTopo, AMX_ID> _Vertices;
    std::map<AMXTopo, AMX_ID> _PointVertices;
    std::map<AMXTopo, Xchg_bool> _FacesOrientation;
    // all PK topos
    Xchg_vector<AMXTopo> _AMXTopos;
    // topo classes, relations and senses
    Xchg_vector<Xchg_Int32> _Classes;
    Xchg_vector<Xchg_Int32> _Parents;
    Xchg_vector<Xchg_Int32> _Children;
    Xchg_vector<Xchg_Int32> _Senses;
    // PK id
    AMX_ID _ID;
    // PK BodyPtr
    Xchg_BodyPtr _BodyPtr;

    // topo's id
    AMX_ID GetNewID();

    // create topo
    Xchg_bool CreateBody(const Xchg_BodyPtr& inBody);
    Xchg_bool CreateInfVoidRegion(const Xchg_BodyPtr& inBody);
    Xchg_bool CreateSolidRegion(const Xchg_LumpPtr& inLump);
    Xchg_bool CreateVoidRegion(const Xchg_ShellPtr& inShell);
    Xchg_bool CreateFrontFaceShell(const Xchg_ShellPtr& inShell);
    Xchg_bool CreateBackFaceShell(const Xchg_ShellPtr& inShell);
    Xchg_bool CreateOpenFaceShell(const Xchg_ShellPtr& inShell);
    Xchg_bool CreateEdgeShell(const Xchg_ShellPtr& inShell, const Xchg_Size_t& inEdgeSetIndex);
    Xchg_bool CreateCurveShell(const Xchg_ShellPtr& inShell, const Xchg_Size_t& inCurveSetIndex);
    Xchg_bool CreateVertexShell(const Xchg_VertexPtr& inVertex);
    Xchg_bool CreatePointShell(const Xchg_PointPtr& inPoint);
    Xchg_bool CreateFace(const Xchg_FacePtr& inFace);
    Xchg_bool CreateLoop(const Xchg_LoopPtr& inLoop);
    Xchg_bool CreateFin(const Xchg_CoedgePtr& inFin);
    Xchg_bool CreateEdge(const Xchg_EdgePtr& inEdge);
    Xchg_bool CreateCurveEdge(const Xchg_CurvePtr& inCurve);
    Xchg_bool CreateVertex(const Xchg_VertexPtr& inVertex);
    Xchg_bool CreatePointVertex(const Xchg_PointPtr& inPoint);

    // extract topo
    Xchg_ErrorStatus ExtractBody(const Xchg_BodyPtr& inBody);
    Xchg_ErrorStatus ExtractInfVoidRegion(const Xchg_BodyPtr& inBody);
    Xchg_ErrorStatus ExtractSolidRegion(const Xchg_LumpPtr& inLump);
    Xchg_ErrorStatus ExtractVoidRegion(const Xchg_ShellPtr& inShell);
    Xchg_ErrorStatus ExtractFrontFaceShell(const Xchg_ShellPtr& inShell);
    Xchg_ErrorStatus ExtractBackFaceShell(const Xchg_ShellPtr& inShell);
    Xchg_ErrorStatus ExtractOpenFaceShell(const Xchg_ShellPtr& inShell);
    Xchg_ErrorStatus ExtractWireShell(const Xchg_ShellPtr& inShell);
    Xchg_ErrorStatus ExtractEdgeShell(const Xchg_ShellPtr& inShell, const Xchg_Size_t& inEdgeSetIndex);
    Xchg_ErrorStatus ExtractCurveShell(const Xchg_ShellPtr& inShell, const Xchg_Size_t& inCurveSetIndex);
    Xchg_ErrorStatus ExtractVertexShell(const Xchg_VertexPtr& inVertex);
    Xchg_ErrorStatus ExtractPointShell(const Xchg_PointPtr& inPoint);
    Xchg_ErrorStatus ExtractFace(const Xchg_FacePtr& inFace, Xchg_bool orientation);
    Xchg_ErrorStatus ExtractLoop(const Xchg_LoopPtr& inLoop);
    Xchg_ErrorStatus ExtractFin(const Xchg_CoedgePtr& inFin);
    Xchg_ErrorStatus ExtractEdge(const Xchg_EdgePtr& inEdge);
    Xchg_ErrorStatus ExtractEdgeFromCurve(const Xchg_CurvePtr& inCurve);
    Xchg_ErrorStatus ExtractVertex(const Xchg_VertexPtr& inVertex);
    Xchg_ErrorStatus ExtractVertexFromPoint(const Xchg_PointPtr& inPoint);

    // calculate coedges's order in a loop, get a ordered coedges array
    XchgErrorStatus GetOrderedCoedges(const Xchg_LoopPtr& inLoop, Xchg_vector<Xchg_CoedgePtr>& OutCoedges);

    // add topo's relation (parent->child and sense)
    void AddRelation(AMX_ID inParentID, AMX_ID inChildID, Xchg_Int32 inSense);

    // extract relations
    virtual Xchg_ErrorStatus ExtractAllRelations();
    virtual Xchg_ErrorStatus BodyAddChildren(const Xchg_BodyPtr& inBody);
    virtual Xchg_ErrorStatus InfVoidRegionAddChildren(const Xchg_BodyPtr& inBody);
    virtual Xchg_ErrorStatus SolidRegionAddChildren(const Xchg_LumpPtr& inLump);
    virtual Xchg_ErrorStatus VoidRegionAddChildren(const Xchg_ShellPtr& inShell);
    virtual Xchg_ErrorStatus FrontFaceShellAddChildren(const Xchg_ShellPtr& inShell);
    virtual Xchg_ErrorStatus BackFaceShellAddChildren(const Xchg_ShellPtr& inShell);
    virtual Xchg_ErrorStatus OpenFaceShellAddChildren(const Xchg_ShellPtr& inShell);
    virtual Xchg_ErrorStatus EdgeShellAddChildren(const Xchg_ShellPtr& inShell, const Xchg_Size_t& inEdgeSetIndex);
    virtual Xchg_ErrorStatus CurveShellAddChildren(const Xchg_ShellPtr& inShell, const Xchg_Size_t& inCurveSetIndex);
    virtual Xchg_ErrorStatus VertexShellAddChildren(const Xchg_VertexPtr& inVertex);
    virtual Xchg_ErrorStatus PointShellAddChildren(const Xchg_PointPtr& inPoint);
    virtual Xchg_ErrorStatus FaceAddChildren(const Xchg_FacePtr& inFace);
    virtual Xchg_ErrorStatus LoopAddChildren(const Xchg_LoopPtr& inLoop);
    virtual Xchg_ErrorStatus FinAddChildren(const Xchg_CoedgePtr& inCoedge);
    virtual Xchg_ErrorStatus EdgeAddChildren(const Xchg_EdgePtr& inEdge);

    // point, curve, surface
    virtual Xchg_Int32 ConvertToAMXPoint(const Xchg_PointPtr& inPoint, Xchg_Int32& OutPKPoint);
    virtual Xchg_Int32 ConvertToAMXCurve(const Xchg_CurvePtr& inCurve, Xchg_Int32& OutPKCurve);
    virtual Xchg_Int32 ConvertToAMXSurf(const Xchg_SurfacePtr& inSurf, Xchg_Int32& OutPKSurf);
    // curves
    virtual Xchg_Int32 ConvertToAMXLine(const Xchg_LinePtr& inLine, Xchg_Int32& OutPKLine);
    virtual Xchg_Int32 ConvertToAMXPolyline(const Xchg_PolylinePtr& inPolyline, Xchg_Int32& OutPKPolyline);
    virtual Xchg_Int32 ConvertToAMXEllipse(const Xchg_EllipsePtr& inEllipse, Xchg_Int32& OutPKEllipse);
    virtual Xchg_Int32 ConvertToAMXCircle(const Xchg_EllipsePtr& inCircle, Xchg_Int32& OutPKCircle);
    virtual Xchg_Int32 ConvertToAMXHyperbola(const Xchg_HyperbolaPtr& inHyperbola, Xchg_Int32& OutPKHyperbola);
    virtual Xchg_Int32 ConvertToAMXParabola(const Xchg_ParabolaPtr& inParabola, Xchg_Int32& OutPKParabola);
    virtual Xchg_Int32 ConvertToAMXNurbsCurve(const Xchg_NurbsCurvePtr& inNurbsCurve, Xchg_Int32& OutPKNurbsCurve);
    virtual Xchg_Int32 ConvertToAMXNurbsCurve(const Xchg_CompositeCurvePtr& inCompositeCurve,
                                              Xchg_Int32& OutPKNurbsCurve);
    // TODO offset curve
    // surfaces
    virtual Xchg_Int32 ConvertToAMXPlane(const Xchg_PlaneSurfacePtr& inPlaneSurface, Xchg_Int32& OutPKPlane);
    virtual Xchg_Int32 ConvertToAMXCyl(const Xchg_CylindricalSurfacePtr& inCylindricalSurface, Xchg_Int32& OutPKCyl);
    virtual Xchg_Int32 ConvertToAMXCone(const Xchg_ConicalSurfacePtr& inConicalSurface, Xchg_Int32& OutPKCone);
    virtual Xchg_Int32 ConvertToAMXSphere(const Xchg_SphericalSurfacePtr& inSphericalSurface, Xchg_Int32& OutPKSphere);
    virtual Xchg_Int32 ConvertToAMXTorus(const Xchg_ToroidalSurfacePtr& inToroidalSurface, Xchg_Int32& OutPKTorus);
    virtual Xchg_Int32 ConvertToAMXNurbsSurf(const Xchg_NurbsSurfacePtr& inNurbsSurface, Xchg_Int32& OutPKBSurf);
    virtual Xchg_Int32 ConvertToAMXOffSet(const Xchg_OffsetSurfacePtr& inOffsetSurface, Xchg_Int32& OutPKOffset);
    virtual Xchg_Int32 ConvertToAMXSwept(const Xchg_LinearExtrusionSurfacePtr& inLinearExtrusionSurface,
                                         Xchg_Int32& OutPKSwept);
    virtual Xchg_Int32 ConvertToAMXSpun(const Xchg_RevolutionSurfacePtr& inRevolutionSurface, Xchg_Int32& OutPKSpun);

    // transform geom
    virtual Xchg_Int32 TransformGeom(Xchg_Int32 inGeom, Xchg_transfoPtr inTransfo, Xchg_Int32& OutResultGeom);

    // print topos array and relations for test
    virtual void PrintToposAndRelations();

public:
    // main convert api
    virtual Xchg_Int32 ConvertBody(Xchg_Int32& outBodyTag);

    BodyConvertToAMX(const Xchg_BodyPtr& inBody);
    ~BodyConvertToAMX();
    BodyConvertToAMX(const BodyConvertToAMX& inOther) = delete;
    BodyConvertToAMX(BodyConvertToAMX&& inOther) = delete;
    BodyConvertToAMX& operator=(const BodyConvertToAMX& inOther) = delete;
    BodyConvertToAMX& operator=(BodyConvertToAMX&& inOther) = delete;
};

#endif