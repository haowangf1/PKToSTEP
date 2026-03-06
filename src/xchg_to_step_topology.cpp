#include "../include/xchg_to_step_writer.hpp"
#include "../include/step_entity_builder.hpp"
#include "../include/xchg_entity_mapper.hpp"

#include "topology/xchg_body.hpp"
#include "topology/xchg_lump.hpp"
#include "topology/xchg_shell.hpp"
#include "topology/xchg_face.hpp"
#include "topology/xchg_loop.hpp"
#include "topology/xchg_coedge.hpp"
#include "topology/xchg_edge.hpp"
#include "topology/xchg_vertex.hpp"
#include "geom/surface/xchg_surface.hpp"
#include "geom/curve/xchg_curve.hpp"
#include "geom/xchg_point.hpp"

#include <iostream>
#include <vector>



// 在 xchg_to_step_writer.cpp 中声明的辅助成员
// 这里需要访问 m_builder, m_mapper, m_output

int XchgToSTEPWriter::WriteBody(const Xchg_BodyPtr& body) {
    if (!body) return 0;

    // 获取 Lump 数量
    Xchg_Size_t numLumps = body->GetNumLumps();
    if (numLumps == 0) {
        std::cerr << "[Warning] Body has no lumps" << std::endl;
        return 0;
    }

    // 目前只处理第一个 Lump
    Xchg_LumpPtr lump;
    Xchg_ErrorStatus err = body->GetLump(0, lump);
    if (err != XCHG_OK || !lump) {
        std::cerr << "[Error] Failed to get lump" << std::endl;
        return 0;
    }

    // 获取 Shell 数量
    Xchg_Size_t numShells = lump->GetNumShells();
    if (numShells == 0) {
        std::cerr << "[Warning] Lump has no shells" << std::endl;
        return 0;
    }

    // 判断是 MANIFOLD_SOLID_BREP 还是 BREP_WITH_VOIDS
    bool hasVoids = (numShells > 1);

    if (!hasVoids) {
        // MANIFOLD_SOLID_BREP (单一外壳)
        Xchg_ShellPtr shell;
        lump->GetShell(0, shell);

        int shellId = WriteShell(shell, true);  // 假设是闭合的

        int bodyId = m_mapper->AllocateNewId();
        std::string entity = m_builder->BeginEntity(bodyId, "MANIFOLD_SOLID_BREP")
            .AddString("")
            .AddEntityRef(shellId)
            .Build();
        WriteEntity(entity);

        return bodyId;
    } else {
        // BREP_WITH_VOIDS (外壳 + 内壳)
        Xchg_ShellPtr outerShell;
        lump->GetShell(0, outerShell);

        int outerShellId = WriteShell(outerShell, true);

        // 收集内壳
        std::vector<int> voidShellIds;
        for (Xchg_Size_t i = 1; i < numShells; ++i) {
            Xchg_ShellPtr innerShell;
            lump->GetShell(i, innerShell);
            int innerShellId = WriteShell(innerShell, true);
            voidShellIds.push_back(innerShellId);
        }

        int bodyId = m_mapper->AllocateNewId();
        std::string entity = m_builder->BeginEntity(bodyId, "BREP_WITH_VOIDS")
            .AddString("")
            .AddEntityRef(outerShellId)
            .AddEntityArray(voidShellIds)
            .Build();
        WriteEntity(entity);

        return bodyId;
    }
}

int XchgToSTEPWriter::WriteShell(const Xchg_ShellPtr& shell, bool isClosed) {
    if (!shell) return 0;

    // 收集所有 Face
    std::vector<int> faceIds;
    Xchg_Size_t numFaces = shell->GetNumFaces();

    for (Xchg_Size_t i = 0; i < numFaces; ++i) {
        Xchg_FacePtr face;
        Xchg_bool faceOrientation;
        shell->GetFace(i, face, faceOrientation);

        if (face) {
            int faceId = WriteFace(face);
            if (faceId > 0) {
                faceIds.push_back(faceId);
            }
        }
    }

    if (faceIds.empty()) {
        std::cerr << "[Warning] Shell has no valid faces" << std::endl;
        return 0;
    }

    // 写出 CLOSED_SHELL 或 OPEN_SHELL
    int shellId = m_mapper->AllocateNewId();
    std::string entityType = isClosed ? "CLOSED_SHELL" : "OPEN_SHELL";
    std::string entity = m_builder->BeginEntity(shellId, entityType)
        .AddString("")
        .AddEntityArray(faceIds)
        .Build();
    WriteEntity(entity);

    return shellId;
}

int XchgToSTEPWriter::WriteFace(const Xchg_FacePtr& face) {
    if (!face) return 0;

    // 获取 Face 的几何（Surface）
    Xchg_SurfacePtr surface = face->GetGeom();
    int surfaceId = WriteSurface(surface);

    // 收集所有 Loop
    std::vector<int> boundIds;
    Xchg_Size_t numLoops = face->GetNumLoops();

    for (Xchg_Size_t i = 0; i < numLoops; ++i) {
        Xchg_LoopPtr loop;
        Xchg_ErrorStatus err = face->GetLoop(i, loop);

        if (err == XCHG_OK && loop) {
            bool isOuter = (i == 0);  // 第一个 loop 通常是外环
            int boundId = WriteLoop(loop, isOuter);
            if (boundId > 0) {
                boundIds.push_back(boundId);
            }
        }
    }

    if (boundIds.empty()) {
        std::cerr << "[Warning] Face has no valid loops" << std::endl;
        return 0;
    }

    // 写出 ADVANCED_FACE
    int faceId = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(faceId, "ADVANCED_FACE")
        .AddString("")
        .AddEntityArray(boundIds)
        .AddEntityRef(surfaceId)
        .AddBoolean(true)  // same_sense，通常为 true
        .Build();
    WriteEntity(entity);

    return faceId;
}

int XchgToSTEPWriter::WriteLoop(const Xchg_LoopPtr& loop, bool isOuter) {
    if (!loop) return 0;

    // 写出 EDGE_LOOP
    int edgeLoopId = WriteEdgeLoop(loop);
    if (edgeLoopId == 0) {
        return 0;
    }

    // 写出 FACE_OUTER_BOUND 或 FACE_BOUND
    int boundId = m_mapper->AllocateNewId();
    std::string entityType = isOuter ? "FACE_OUTER_BOUND" : "FACE_BOUND";
    std::string entity = m_builder->BeginEntity(boundId, entityType)
        .AddString("")
        .AddEntityRef(edgeLoopId)
        .AddBoolean(true)  // orientation，通常为 true
        .Build();
    WriteEntity(entity);

    return boundId;
}

int XchgToSTEPWriter::WriteEdgeLoop(const Xchg_LoopPtr& loop) {
    if (!loop) return 0;

    // 收集所有 Coedge (ORIENTED_EDGE)
    std::vector<int> orientedEdgeIds;
    Xchg_Size_t numCoedges = loop->GetNumCoedges();

    for (Xchg_Size_t i = 0; i < numCoedges; ++i) {
        Xchg_CoedgePtr coedge;
        Xchg_bool coedgeOrientation;
        Xchg_ErrorStatus err = loop->GetCoedge(i, coedge, coedgeOrientation);

        if (err == XCHG_OK && coedge) {
            int orientedEdgeId = WriteCoedge(coedge);
            if (orientedEdgeId > 0) {
                orientedEdgeIds.push_back(orientedEdgeId);
            }
        }
    }

    if (orientedEdgeIds.empty()) {
        std::cerr << "[Warning] Loop has no valid coedges" << std::endl;
        return 0;
    }

    // 写出 EDGE_LOOP
    int edgeLoopId = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(edgeLoopId, "EDGE_LOOP")
        .AddString("")
        .AddEntityArray(orientedEdgeIds)
        .Build();
    WriteEntity(entity);

    return edgeLoopId;
}

int XchgToSTEPWriter::WriteCoedge(const Xchg_CoedgePtr& coedge) {
    if (!coedge) return 0;

    // 获取 Edge
    Xchg_EdgePtr edge;
    coedge->GetEdge(edge);
    if (!edge) {
        std::cerr << "[Warning] Coedge has no edge" << std::endl;
        return 0;
    }

    // 获取方向
    Xchg_bool orientation = coedge->GetOrientation();

    // 写出 Edge（如果尚未写出）
    int edgeId = WriteEdge(edge);
    if (edgeId == 0) {
        return 0;
    }

    // 写出 ORIENTED_EDGE
    int orientedEdgeId = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(orientedEdgeId, "ORIENTED_EDGE")
        .AddString("")
        .AddDerived()  // *
        .AddDerived()  // *
        .AddEntityRef(edgeId)
        .AddBoolean(orientation == XCHG_TRUE)
        .Build();
    WriteEntity(entity);

    return orientedEdgeId;
}

int XchgToSTEPWriter::WriteEdge(const Xchg_EdgePtr& edge) {
    if (!edge) return 0;

    // 检查是否已经写出
    if (m_mapper->HasMapping(edge)) {
        return m_mapper->GetMapping(edge);
    }

    // 获取起点和终点 Vertex
    Xchg_VertexPtr startVertex;
    Xchg_VertexPtr endVertex;

    Xchg_ErrorStatus err1,err2;

    err1 = edge->GetStartVertex(startVertex);
    err2 = edge->GetEndVertex(endVertex);

    if (err1 != XCHG_OK || err2 != XCHG_OK) {
      //TODO
    }

    int startVertexId = WriteVertex(startVertex);
    int endVertexId = WriteVertex(endVertex);

    // 获取 Edge 的几何（Curve）
    Xchg_CurvePtr curve = edge->GetGeom();
    int curveId = WriteCurve(curve);

    // 写出 EDGE_CURVE
    int edgeId = m_mapper->GetOrAllocate(edge);
    std::string entity = m_builder->BeginEntity(edgeId, "EDGE_CURVE")
        .AddString("")
        .AddEntityRef(startVertexId)
        .AddEntityRef(endVertexId)
        .AddEntityRef(curveId)
        .AddBoolean(true)  // same_sense
        .Build();
    WriteEntity(entity);

    return edgeId;
}

int XchgToSTEPWriter::WriteVertex(const Xchg_VertexPtr& vertex) {
    if (!vertex) return 0;

    // 检查是否已经写出
    if (m_mapper->HasMapping(vertex)) {
        return m_mapper->GetMapping(vertex);
    }

    // 获取 Vertex 的几何（Point）
    Xchg_PointPtr point = vertex->GetGeom();
    int pointId = 0;
    if (point) {
        pointId = m_mapper->GetOrAllocate(point);
        if (!m_mapper->HasMapping(point) || pointId == m_mapper->GetMapping(point)) {
            // 需要写出 Point
            std::string entity = m_builder->BeginEntity(pointId, "CARTESIAN_POINT")
                .AddString("")
                .AddRealArray({point->x(), point->y(), point->z()})
                .Build();
            WriteEntity(entity);
        }
    } else {
        // 如果没有几何，写出原点
        pointId = m_mapper->AllocateNewId();
        std::string entity = m_builder->BeginEntity(pointId, "CARTESIAN_POINT")
            .AddString("")
            .AddRealArray({0.0, 0.0, 0.0})
            .Build();
        WriteEntity(entity);
    }

    // 写出 VERTEX_POINT
    int vertexId = m_mapper->GetOrAllocate(vertex);
    std::string entity = m_builder->BeginEntity(vertexId, "VERTEX_POINT")
        .AddString("")
        .AddEntityRef(pointId)
        .Build();
    WriteEntity(entity);

    return vertexId;
}

