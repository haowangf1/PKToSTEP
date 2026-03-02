# Step1 实施方案：PK_BODY 到 AMCAXExchangeBase 中间格式转换

## 目标
将 Parasolid 的 PK_BODY 数据结构转换为 AMCAXExchangeBase 定义的中间数据格式，包括拓扑关系和几何数据的完整转换。

## 核心任务分解

### 1. 拓扑结构提取与转换

#### 1.1 Parasolid 拓扑层次结构
```
PK_BODY                          (最高级拓扑实体)
  └─ PK_REGION                   (>=1, PK保证regions[0]一定是infinite void region)
      └─ PK_SHELL                (>=1, 连通的有向face集合)
          ├─ PK_FACE             (>=0, 面 + orientation标志)
          │   └─ PK_LOOP         (>=0, face的边界，0个loop=封闭面如完整球面)
          │       └─ PK_FIN      (有序排列，nose-to-tail, 是edge在loop中的有向使用)
          │           └─ PK_EDGE (有界曲线段)
          │               └─ PK_VERTEX (0/1/2个, 点)
          ├─ PK_EDGE (wireframe) (Shell可直接包含wireframe edge，无fin)
          └─ PK_VERTEX (acorn)   (Shell可直接包含一个acorn vertex)
```

#### 1.2 AMCAXExchangeBase 拓扑层次结构
```
Xchg_Body                        (最高级，Create()创建)
  └─ Xchg_Lump                   (Create(parentBody), 对应PK_REGION, manifold子部分)
      ├─ AddOuterShell()          (一个outer shell)
      ├─ AddInnerShell()          (零或多个inner shell)
      ├─ AddOpenShell()           (一个open shell)
      └─ AddWireShell()           (wire shell)
          └─ Xchg_Shell           (Create(parentBody), 面的集合)
              └─ AddFace(face, orientation)
                  └─ Xchg_Face    (Create(parentBody), 关联Surface几何)
                      ├─ AddOuterLoop()   (外环)
                      ├─ AddInnerLoop()   (内环/孔)
                      └─ AddVertexLoop()  (顶点环)
                          └─ Xchg_Loop    (Create(parentBody), 包含有序Coedge)
                              └─ AddCoedge(coedge, orientation)
                                  └─ Xchg_Coedge (Create(parentBody), 关联UV曲线)
                                      └─ SetEdge()
                                          └─ Xchg_Edge (Create(parentBody), 关联3D曲线)
                                              ├─ SetStartVertex()
                                              └─ SetEndVertex()
                                                  └─ Xchg_Vertex (Create(parentBody), 关联Point几何)
```

#### 1.3 PK 与 ExchangeBase 拓扑父子关系详细映射

##### 1.3.1 整体对应关系表

| PK 实体 | PK 父子关系 | Xchg 实体 | Xchg 父子关系 | 映射说明 |
| --- | --- | --- | --- | --- |
| PK_BODY | 包含 >=1 个 PK_REGION | Xchg_Body | 包含 >=1 个 Xchg_Lump | Body 1:1 映射 |
| PK_REGION | 属于 Body，包含 >=1 个 Shell | Xchg_Lump | 属于 Body，包含 Shell | solid Region → Lump，void region无需转换到x格式 |
| PK_SHELL | 属于 Region，包含 Face+orientation | Xchg_Shell | 属于 Lump，包含 Face+orientation | Shell 1:1 映射，需设置 outer/inner/open |
| PK_FACE | 属于 Shell(可属于2个)，包含 Loop | Xchg_Face | 属于 Shell，包含 Loop | Face 1:1 映射 |
| PK_LOOP | 属于 Face，包含有序 Fin | Xchg_Loop | 属于 Face，包含有序 Coedge | Loop 1:1 映射 |
| PK_FIN | 属于 Loop+Edge，有sense标志 | Xchg_Coedge | 属于 Loop，关联 Edge | Fin → Coedge 是关键映射 |
| PK_EDGE | 包含 0-2 个 Vertex，包含 >=0 个 Fin | Xchg_Edge | 包含 Start/End Vertex | Edge 1:1 映射，需去重共享 |
| PK_VERTEX | 零维点 | Xchg_Vertex | 零维点 | Vertex 1:1 映射，需去重共享 |

##### 1.3.2 关键差异与处理策略

**差异1：PK_REGION vs Xchg_Lump**
- PK_BODY 总有一个**无穷 void region**。**PK 保证 `PK_BODY_ask_regions` 返回的第一个 region（`regions[0]`）一定是 infinite void region**，后续 region 为 solid 或 bounded void region。
- Xchg (STEP/BREP) 中**不需要单独表示 void region**。Xchg→PK 转换时，PK 会自动根据 outer shell 推导 infinite void region（outer shell 的 face 反面即为 void region 的 shell），根据 inner shell 推导 bounded void region。
- **策略**：一个 Body 只创建**一个 Xchg_Lump**，通过 shell 的 outer/inner 属性来表达拓扑关系：
  - Solid body / **General body**：跳过 infinite void region；solid region 的 shell → `AddOuterShell`；bounded void region（空腔）的 shell → `AddInnerShell`
  - **注意**：通过 `Xchg_Node::ConvertToPKBody()` 转换得到的 PK_BODY，其 `PK_BODY_ask_type` 返回的是 `PK_BODY_type_general_c`（5605）而非 `PK_BODY_type_solid_c`（5601），因此必须将 `general_c` 与 `solid_c` 同等处理。
  - Sheet body：只处理 infinite void region，其 shell → `AddOpenShell`
  - Wire body：只处理 infinite void region，其 shell → `AddWireShell`
  - Acorn body：只处理 infinite void region，其 shell → `AddWireShell`
  - 不需要对 face 做跨 region 去重（因为不再遍历同一 face 的两个 region）

**差异2：PK_SHELL 类型 vs Xchg_Shell 添加方式**
- PK 中 Shell 属于 Region，通过 `PK_SHELL_ask_type()` 获取类型：
  - `PK_SHELL_type_acorn_c` → acorn vertex shell
  - `PK_SHELL_type_wireframe_c` → wireframe edge shell
  - `PK_SHELL_type_wireframe_free_c` → face shell（无wireframe edge）
  - `PK_SHELL_type_mixed_c` → 混合shell
- ExchangeBase 中 Shell 通过不同方法添加到 Lump：
  - `Xchg_Lump::AddOuterShell()` → 外壳（solid body的外表面）
  - `Xchg_Lump::AddInnerShell()` → 内壳（solid body中的空腔）
  - `Xchg_Lump::AddOpenShell()` → 开放壳（sheet body）
  - `Xchg_Lump::AddWireShell()` → 线框壳（wire body）
- **策略（按 region 类型+shell 类型综合判断）**：
  - Solid/General body 的 solid region shell → `lump->AddOuterShell(shell)`
  - Bounded void region（空腔）的 shell → `lump->AddInnerShell(shell)`
  - Infinite void region 的 shell（仅 non-solid body）：
    - Sheet body → `lump->AddOpenShell(shell)`
    - Wire/Acorn body → `lump->AddWireShell(shell)`
  - Wireframe/Acorn shell → `lump->AddWireShell(shell)`
  - **Solid/General body 中跳过 infinite void region**（其 face 与 solid region 共享，不需要重复表示）

**差异3：PK_FIN vs Xchg_Coedge**
- PK 中 **Fin** 是 Edge 在 Loop 中的有向使用，连接 Loop 和 Edge。
  - Fin 有 sense（正向/反向），表示 Fin 方向是否与 Edge 方向一致
  - Fin 按顺序排列在 Loop 中（nose-to-tail）
  - 通过 `PK_FIN_ask_edge()` 获取关联的 Edge
  - 通过 `PK_FIN_ask_oriented_curve()` 获取 Fin 方向与 curve 的关系
- ExchangeBase 中 **Coedge** 功能等同于 Fin：
  - Coedge 有 orientation（`SetOrientation()`），表示 Coedge 方向是否与 Edge 方向一致
  - Coedge 可关联 UV 曲线（`SetGeom()`）
  - 通过 `SetEdge()` 关联 Edge
- **策略**：每个 PK_FIN → 一个 Xchg_Coedge
  - Fin 的 sense → Coedge 的 orientation
  - 通过 `PK_FIN_ask_edge()` 获取 edge 后，查找/创建对应的 Xchg_Edge

**差异4：Edge/Vertex 共享**
- PK 中一个 Edge 可被多个 Fin 引用（即被多个 Face/Loop 共享）
- ExchangeBase 中同样：一个 Xchg_Edge 可被多个 Xchg_Coedge 引用
- **策略**：使用 `pk_to_xchg_id_map_` 确保同一个 PK_EDGE 只创建一个 Xchg_Edge
  - 转换 Fin→Coedge 时，先查 map 中 edge 是否已转换
  - Vertex 同理，确保共享的 vertex 只创建一次

**差异5：Face 方向 (orientation) — 需要精确映射**
- PK 有两个独立的 orientation：
  1. **Shell-Face orientation**：`PK_SHELL_ask_oriented_faces()` 返回 `orients[i]`
     - `PK_LOGICAL_true`：face normal **指向 shell 所围体积内部**（对 solid region 的 shell = solid 内部）
     - `PK_LOGICAL_false`：face normal 指向 shell 所围体积外部
  2. **Face-Surface orientation**：`PK_FACE_ask_oriented_surf()` 返回 `face_orient`
     - `PK_LOGICAL_true`：face normal 与 surface natural normal **同向**
     - `PK_LOGICAL_false`：face normal 与 surface natural normal **反向**
- ExchangeBase 约定：
  - `Xchg_Face` normal **始终等于** surface normal（geometrical orientation always True）
  - `Xchg_Shell::AddFace(face, orientation)`：`XCHG_TRUE` 表示 face material 与 shell material **同侧**
    - 对 outer shell：material（solid）在 shell 内部，`XCHG_TRUE` → face normal **指向外部**
- **Shell-Face orientation 的正确映射**：
  - Xchg face normal = surface normal（固定）
  - 当 `face_orient = true`：Xchg face normal = PK face normal
  - 当 `face_orient = false`：Xchg face normal = -PK face normal
  - Xchg `XCHG_TRUE` 要求 face normal 指向外部（对 outer shell），即 PK face normal 指向外部
  - PK `orients[i] = false` → face normal 指向外部（`true` 为指向内部）
  - **综合得出：`xchg_orient = (orients[i] != face_orient)`**
    - `orients[i]=false, face_orient=true`：Xchg face normal 朝外 → `XCHG_TRUE` ✓
    - `orients[i]=true, face_orient=false`：Xchg face normal = -PK face normal 朝外 → `XCHG_TRUE` ✓
    - `orients[i]=true, face_orient=true`：Xchg face normal 朝内 → `XCHG_FALSE` ✓
    - `orients[i]=false, face_orient=false`：Xchg face normal 朝内 → `XCHG_FALSE` ✓

#### 1.4 关键 Parasolid API 函数（详细）

**Body 级别：**
- `PK_BODY_ask_type(body, &body_type)` → 获取 body 类型 (solid/sheet/wire/acorn/empty/general)
- `PK_BODY_ask_regions(body, &n_regions, &regions)` → 获取所有 region（**PK 保证 regions[0] 一定是 infinite void region**）
- `PK_BODY_ask_topology(body, options, &n_topols, &topols, &classes, &n_relations, &parents, &children, &senses)` → 一次性获取完整拓扑（可选方案）

**Region 级别：**
- `PK_REGION_ask_shells(region, &n_shells, &shells)` → 获取 region 的所有 shell
- `PK_REGION_ask_type(region, options, &results)` → 获取 region 类型（solid/void）

**Shell 级别：**
- `PK_SHELL_ask_oriented_faces(shell, &n_faces, &faces, &orients)` → 获取 shell 中的面及其朝向
- `PK_SHELL_ask_type(shell, &shell_type)` → 获取 shell 类型（acorn/wireframe/wireframe_free/mixed）
- `PK_SHELL_ask_wireframe_edges(shell, ...)` → 获取 shell 中的 wireframe edge
- `PK_SHELL_ask_acorn_vertex(shell, &vertex)` → 获取 acorn shell 的顶点

**Face 级别：**
- `PK_FACE_ask_loops(face, &n_loops, &loops)` → 获取 face 的所有 loop（不区分 outer/inner）
- `PK_FACE_ask_surf(face, &surf)` → 获取 face 的曲面（可能为 null）
- `PK_FACE_ask_oriented_surf(face, &surf, &orientation)` → 获取 face 的曲面及朝向标志

**Loop 级别：**
- `PK_LOOP_ask_fins(loop, &n_fins, &fins)` → 获取 loop 中有序排列的 fin（nose-to-tail）
- `PK_LOOP_ask_type(loop, &loop_type)` → 判断 loop 类型：
  - `PK_LOOP_type_outer_c` → 外环
  - `PK_LOOP_type_inner_c` → 内环（孔）
  - `PK_LOOP_type_vertex_c` → 顶点环
  - `PK_LOOP_type_wire_c` → 线框环
  - `PK_LOOP_type_winding_c` → 绕环（周期面上）
  - `PK_LOOP_type_inner_sing_c` → 内奇异环（如锥面顶部）
  - `PK_LOOP_type_likely_outer_c` / `PK_LOOP_type_likely_inner_c` → 双周期面上

**Fin 级别：**
- `PK_FIN_ask_edge(fin, &edge)` → 获取 fin 对应的 edge
- `PK_FIN_is_positive(fin, &is_positive)` → **判断 fin 是否与 edge 同向**（推荐用于计算 Coedge orientation）
- `PK_FIN_ask_curve(fin, &curve)` → 获取 fin 级别的 curve（tolerant edge 的 SP-curve）
- `PK_FIN_ask_oriented_curve(fin, &curve, &orientation)` → 获取 fin 的 curve 及方向
  - `PK_LOGICAL_true`：fin 方向平行于 curve 切线
  - `PK_LOGICAL_false`：fin 方向反平行于 curve 切线
- `PK_FIN_ask_next_in_loop(fin, &next)` → 获取 loop 中的下一个 fin
- `PK_FIN_ask_type(fin, &fin_type)` → 获取 fin 类型（normal/wire/biwire）

**Edge 级别：**
- `PK_EDGE_ask_vertices(edge, vertices[2])` → 获取起始/终止 vertex（ring edge 两个都是 null）
- `PK_EDGE_ask_curve(edge, &curve)` → 获取 edge 的 3D curve（可能为 null）
- `PK_EDGE_ask_fins(edge, &n_fins, &fins)` → 获取 edge 的所有 fin（按空间顺序排列）
- `PK_EDGE_ask_oriented_curve(edge, &curve, &orientation)` → 获取 edge 的 curve 及方向

**Vertex 级别：**
- `PK_VERTEX_ask_point(vertex, &point)` → 获取 vertex 的 point（可能为 null）

### 2. 几何数据提取与转换

#### 2.1 曲面类型映射

| Parasolid 曲面类型 | AMCAXExchangeBase 类型 | 转换函数 |
|-------------------|----------------------|---------|
| PK_SURF_plane_c | Xchg_PlaneSurface | 提取原点+法向+U/V方向 |
| PK_SURF_cyl_c | Xchg_CylindricalSurface | 提取轴线+半径 |
| PK_SURF_cone_c | Xchg_ConicalSurface | 提取顶点+轴线+半角 |
| PK_SURF_sphere_c | Xchg_SphericalSurface | 提取中心+半径 |
| PK_SURF_torus_c | Xchg_ToroidalSurface | 提取中心+轴线+大半径+小半径 |
| PK_SURF_spun_c | Xchg_RevolutionSurface | 提取旋转轴+母线 |
| PK_SURF_swept_c | Xchg_LinearExtrusionSurface | 提取截面曲线+拉伸方向 |
| PK_SURF_bsurf_c | Xchg_NurbsSurface | 提取控制点+节点+权重+阶数 |
| PK_SURF_offset_c | Xchg_OffsetSurface | 提取基础曲面+偏移距离 |

**关键 API：**
- `PK_SURF_ask_class()` - 获取曲面类型
- `PK_SURF_ask_plane()` - 获取平面参数
- `PK_SURF_ask_cyl()` - 获取圆柱面参数
- `PK_SURF_ask_bsurf()` - 获取 B-spline 曲面参数
- 等等...

#### 2.2 曲线类型映射

| Parasolid 曲线类型 | AMCAXExchangeBase 类型 | 转换函数 |
| --- | --- | --- |
| PK_CURVE_line_c | Xchg_Line | 提取起点+方向 |
| PK_CURVE_circle_c | Xchg_Ellipse (圆形) | 提取中心+半径+法向 |
| PK_CURVE_ellipse_c | Xchg_Ellipse | 提取中心+长短轴 |
| PK_CURVE_bcurve_c | Xchg_NurbsCurve | 提取控制点+节点+权重+阶数 |
| PK_CURVE_conic_c | Xchg_Conic/Hyperbola/Parabola | 根据类型转换 |
| PK_CURVE_offset_c | Xchg_OffsetCurve | 提取基础曲线+偏移 |

**关键 API：**
- `PK_CURVE_ask_class()` - 获取曲线类型
- `PK_CURVE_ask_line()` - 获取直线参数
- `PK_CURVE_ask_circle()` - 获取圆参数
- `PK_CURVE_ask_bcurve()` - 获取 B-spline 曲线参数
- 等等...

#### 2.3 点几何
- `PK_POINT_ask()` - 获取点坐标

### 3. 实施步骤

#### 阶段 1：框架搭建
1. 创建转换器类 `PKToXchgConverter`
2. 实现 ID 映射机制 (PK_ENTITY_t tag → Xchg_TopologicalEntityPtr)
3. 实现错误处理和日志记录
4. 实现 PK 内存管理辅助（PK 返回的数组需要 `PK_MEMORY_free` 释放）

#### 阶段 2：拓扑转换（详细流程）

##### 2.1 Body 转换
```
输入: PK_BODY_t pk_body
输出: Xchg_BodyPtr xchg_body

步骤:
1. PK_BODY_ask_type(pk_body, &body_type) → 获取 body 类型
2. Xchg_Body::Create() → 创建 Xchg_Body
3. 设置 body 类型映射:
   PK_BODY_type_solid_c   → Xchg_BodyType_Solid
   PK_BODY_type_sheet_c   → Xchg_BodyType_Sheet
   PK_BODY_type_wire_c    → Xchg_BodyType_Wire
   PK_BODY_type_acorn_c   → Xchg_BodyType_Acorn
   PK_BODY_type_empty_c   → Xchg_BodyType_Empty
   PK_BODY_type_general_c → Xchg_BodyType_General
4. xchg_body->SetBodyType(mapped_type)
```

##### 2.2 Region → Lump 转换（Xchg 不需要 void region）
```
输入: PK_BODY_t pk_body, Xchg_BodyPtr xchg_body
步骤:
1. PK_BODY_ask_regions(pk_body, &n_regions, &regions)
   → regions[0] 总是无穷 void region

2. 创建一个 Lump:
   Xchg_Lump::Create(xchg_body) → lump
   xchg_body->AddLump(lump)

3. 遍历 regions，根据 body 类型选择性处理:

   ═══ Solid Body ═══
   - 跳过 infinite void region（regions[0]）
   - Solid region 的 shell → lump->AddOuterShell(xchg_shell)
   - Bounded void region（空腔）的 shell → lump->AddInnerShell(xchg_shell)
   - 注意：不需要处理 infinite void region，Xchg→PK 转换时会自动
     根据 outer shell 推导 void region（使用 face 的反面）

   ═══ Sheet Body ═══
   - 只处理 infinite void region
   - 其 shell → lump->AddOpenShell(xchg_shell)

   ═══ Wire / Acorn Body ═══
   - 只处理 infinite void region
   - 其 shell → lump->AddWireShell(xchg_shell)

   ═══ General Body ═══
   - 逐 shell 根据 PK_SHELL_ask_type() 判断并选择对应的 Add 方法

注意：由于不再遍历同一 face 的两个 region，不存在 face 跨 shell 重复引用的问题。
Face 去重仍用于同一 shell 内可能出现的共享 face（虽然少见）。
```

##### 2.3 Shell 转换
```
输入: PK_SHELL_t pk_shell, Xchg_BodyPtr xchg_body
输出: Xchg_ShellPtr xchg_shell

步骤:
1. PK_SHELL_ask_type(pk_shell, &shell_type) → 获取 shell 类型
2. Xchg_Shell::Create(xchg_body) → 创建 Shell
3. 根据 shell 类型处理:

   ═══ wireframe_free (有 face 无 wireframe edge) 或 mixed ═══
   a. PK_SHELL_ask_oriented_faces(pk_shell, &n_faces, &faces, &orients)
   b. 遍历 faces:
      - ConvertFace(faces[i]) → xchg_face（从 map 去重，同一 PK_FACE 只创建一次）
      - 计算 ExchangeBase shell-face orientation:
        PK_FACE_ask_oriented_surf(faces[i], &surf, &face_orient)
        xchg_orient = (orients[i] == face_orient) ? XCHG_TRUE : XCHG_FALSE
      - xchg_shell->AddFace(xchg_face, xchg_orient)

   ═══ mixed shell 额外处理 ═══
   a. PK_SHELL_ask_wireframe_edges(pk_shell, ...) → 获取 wireframe edge
   b. 转换 edge → Xchg_Edge（查 map 去重）
   c. xchg_shell->AddWire(edges)

   ═══ wireframe (有 edge 无 face) ═══
   a. PK_SHELL_ask_wireframe_edges(pk_shell, ...) → 获取 wireframe edge
   b. 转换 edge → Xchg_Edge
   c. xchg_shell->AddWire(edges)

   ═══ acorn (只有一个 vertex) ═══
   a. PK_SHELL_ask_acorn_vertex(pk_shell, &vertex) → 获取 acorn vertex
   b. ConvertVertex(vertex) → xchg_vertex
   c. xchg_shell->AddWire({vertex})

4. 设置 shell 的 closed/outer 信息:
   - xchg_shell->SetClosedInfo(...)
   - xchg_shell->SetOuterInfo(...)
```

##### 2.4 Face 转换（带去重）
```
输入: PK_FACE_t pk_face, Xchg_BodyPtr xchg_body
输出: Xchg_FacePtr xchg_face

步骤:
0. 检查 pk_face_map_：如果已转换则直接返回（Face 被 2 个 Shell 共享）

1. Xchg_Face::Create(xchg_body) → 创建 Face
2. 获取并设置几何曲面:
   a. PK_FACE_ask_oriented_surf(pk_face, &pk_surf, &surf_orient)
   b. 如果 pk_surf == PK_ENTITY_null → 无几何（仍创建拓扑 Face，不跳过）
   c. 如果 pk_surf != PK_ENTITY_null:
      - 如果 surf_orient == PK_LOGICAL_true:
        → ConvertSurface(pk_surf) → xchg_surface（正向，直接使用）
      - 如果 surf_orient == PK_LOGICAL_false:
        → ConvertSurface(pk_surf) → xchg_surface
        → 需要创建反向 surface（翻转法向/反转参数化）
        → 注意：反向 surface 不能与正向共享 map 缓存，需单独存储
        → 或使用 Xchg_Face::ReverseNormal() 如果 ExchangeBase 支持
      - xchg_face->SetGeom(xchg_surface)

3. 获取并转换 loops:
   a. PK_FACE_ask_loops(pk_face, &n_loops, &loops)
   b. 如果 n_loops == 0 → 完整封闭面（如完整球面），仍然合法
   c. 遍历 loops:
      - PK_LOOP_ask_type(loops[i], &loop_type) → 获取 loop 类型
      - ConvertLoop(loops[i]) → xchg_loop
      - 根据 loop_type 添加到 face:
        PK_LOOP_type_outer_c / likely_outer_c → xchg_face->AddOuterLoop(xchg_loop)
        PK_LOOP_type_inner_c / likely_inner_c / inner_sing_c → xchg_face->AddInnerLoop(xchg_loop)
        PK_LOOP_type_vertex_c → xchg_face->AddVertexLoop(vertex)
        PK_LOOP_type_winding_c → 需要判断是外环还是内环（通常成对出现）
        PK_LOOP_type_wire_c → 线框环，作为 inner loop 或单独处理

4. 存入 map: pk_face_map_[pk_face] = xchg_face
```

##### 2.5 Loop 转换
```
输入: PK_LOOP_t pk_loop, Xchg_BodyPtr xchg_body
输出: Xchg_LoopPtr xchg_loop

步骤:
1. Xchg_Loop::Create(xchg_body) → 创建 Loop
2. PK_LOOP_ask_type(pk_loop, &loop_type)
3. 设置 loop 属性:
   - xchg_loop->SetOuterInfo(loop_type == outer_c ? XCHG_TRUE : XCHG_FALSE)
   - xchg_loop->SetOrientation(XCHG_TRUE)  // PK 中 loop 方向使得 face 在左侧

4. 特殊情况 — Vertex Loop:
   a. PK_LOOP_ask_type == PK_LOOP_type_vertex_c
   b. PK_LOOP_ask_vertices(pk_loop, ...) → 获取 vertex
   c. ConvertVertex(vertex) → xchg_vertex
   d. xchg_loop->SetVertexLoop(xchg_vertex)
   e. 返回

5. 正常情况 — Fin Loop:
   a. PK_LOOP_ask_fins(pk_loop, &n_fins, &fins)
      → fins 按 nose-to-tail 顺序排列
   b. 遍历 fins[0..n_fins-1]:
      - ConvertFin(fins[i]) → xchg_coedge, coedge_orientation
      - xchg_loop->AddCoedge(xchg_coedge, coedge_orientation)
```

##### 2.6 Fin → Coedge 转换（核心映射）
```
输入: PK_FIN_t pk_fin, Xchg_BodyPtr xchg_body
输出: Xchg_CoedgePtr xchg_coedge

步骤:
1. Xchg_Coedge::Create(xchg_body) → 创建 Coedge
2. 获取 fin 关联的 edge:
   a. PK_FIN_ask_edge(pk_fin, &pk_edge)
   b. 查找 map: 如果 pk_edge 已转换 → 获取已有的 xchg_edge
      否则 → ConvertEdge(pk_edge) → xchg_edge，并存入 map
   c. xchg_coedge->SetEdge(xchg_edge)
   d. xchg_edge->AddCoedge(xchg_coedge)  // 建立反向引用

3. 设置 coedge 的 orientation (coedge 方向与 edge 方向的关系):
   → 使用 PK_FIN_is_positive()，这是最直接且可靠的方式
   a. PK_FIN_is_positive(pk_fin, &is_positive)
   b. is_positive == true → fin 与 edge 同向 → xchg_coedge->SetOrientation(XCHG_TRUE)
      is_positive == false → fin 与 edge 反向 → xchg_coedge->SetOrientation(XCHG_FALSE)
   注意: 不要用 PK_FIN_ask_oriented_curve + PK_EDGE_ask_oriented_curve 比较的方式，
         因为 tolerant edge/fin 可能没有 curve（返回 null），此时 orientation 不确定

4. 设置 UV 曲线（不可省略，tolerant edge 的几何依赖于此）:
   - PK_FIN_ask_curve(pk_fin, &fin_curve) → 获取 fin 级别的 curve
   - 如果 fin_curve != PK_ENTITY_null:
     → 转换 fin_curve → xchg_uv_curve
     → xchg_coedge->SetGeom(xchg_uv_curve)
   - 对于 tolerant edge（edge 无 3D curve），fin 上的 SP-curve 是该边在对应 face 上的
     唯一几何表达，丢弃会导致几何信息不完整

注意: Fin 在 Loop 中的顺序 = Coedge 在 Loop 中的添加顺序
      Fin 是 nose-to-tail 排列，保证了 Loop 的闭合性
```

##### 2.7 Edge 转换
```
输入: PK_EDGE_t pk_edge, Xchg_BodyPtr xchg_body
输出: Xchg_EdgePtr xchg_edge

步骤:
1. 检查 map：如果已转换则直接返回（Edge 可能被多个 Fin/Coedge 共享）
2. Xchg_Edge::Create(xchg_body) → 创建 Edge
3. 获取并设置顶点:
   a. PK_EDGE_ask_vertices(pk_edge, vertices[2])
   b. vertices[0] = start vertex，vertices[1] = end vertex
   c. 如果 vertices[0] != PK_ENTITY_null:
      ConvertVertex(vertices[0]) → xchg_start_vertex （查 map 去重）
      xchg_edge->SetStartVertex(xchg_start_vertex)
   d. 如果 vertices[1] != PK_ENTITY_null:
      ConvertVertex(vertices[1]) → xchg_end_vertex （查 map 去重）
      xchg_edge->SetEndVertex(xchg_end_vertex)
   e. 两个都为 null → ring edge（周期曲线如完整圆）

4. 获取并设置几何曲线:
   a. PK_EDGE_ask_curve(pk_edge, &pk_curve) → 可能为 null（tolerant edge）
   b. 如果 pk_curve != PK_ENTITY_null:
      ConvertCurve(pk_curve) → xchg_curve
      xchg_edge->SetGeom(xchg_curve)
   c. 如果 pk_curve == PK_ENTITY_null:
      → tolerant edge，曲线几何在 fin 上（SP-curve），此处 edge 无 3D curve
      → 仍然创建 edge（不跳过），在 ConvertFin 中通过 PK_FIN_ask_curve 获取 fin 级别的曲线
      → 标记: xchg_edge->SetDegenerated(XCHG_TRUE) 如果是退化边

5. 设置 edge 方向 (same_sense):
   a. 如果有 curve:
      PK_EDGE_ask_oriented_curve(pk_edge, &curve, &orient)
      xchg_edge->SetSameSense(orient ? XCHG_TRUE : XCHG_FALSE)
   b. 如果无 curve (tolerant edge):
      xchg_edge->SetSameSense(XCHG_TRUE) → 默认值
      （方向信息将通过 Coedge 的 orientation 来间接保留）

6. 检查是否为退化边:
   a. 如果 pk_curve == PK_ENTITY_null 或 vertices[0] == vertices[1]（两 vertex 重合）:
      xchg_edge->SetDegenerated(XCHG_TRUE)

7. 获取容差:
   a. PK_EDGE_ask_precision(pk_edge, &precision) → 如果有 local precision
   b. xchg_edge->SetTolerance(precision)

8. 存入 map: pk_edge_map_[pk_edge] = xchg_edge
```

##### 2.8 Vertex 转换
```
输入: PK_VERTEX_t pk_vertex, Xchg_BodyPtr xchg_body
输出: Xchg_VertexPtr xchg_vertex

步骤:
1. 检查 map：如果已转换则直接返回（Vertex 可能被多个 Edge 共享）
2. Xchg_Vertex::Create(xchg_body) → 创建 Vertex（无论有无 point 几何都要创建）
3. PK_VERTEX_ask_point(pk_vertex, &pk_point) → 获取 point
4. 如果 pk_point != PK_ENTITY_null:
   a. PK_POINT_ask(pk_point, &position) → 获取坐标 (x, y, z)
   b. 创建 Xchg_Point(position.coord[0], position.coord[1], position.coord[2])
   c. xchg_vertex->SetGeom(xchg_point)
5. 如果 pk_point == PK_ENTITY_null:
   → 仍然保留 vertex（不跳过），只是没有几何信息
   → 这种情况少见但合法（如 tolerant vertex）
6. 获取容差:
   a. PK_VERTEX_ask_precision(pk_vertex, &precision) → 如果有 local precision
   b. xchg_vertex->SetTolerance(precision)
7. 存入 map: pk_vertex_map_[pk_vertex] = xchg_vertex
```

##### 2.9 转换流程总结（调用顺序）
```
ConvertBody(PK_BODY_t)
  ├─ PK_BODY_ask_type() → SetBodyType()
  ├─ PK_BODY_ask_regions() → 遍历所有 region（包括 void region）
  │   ├─ PK_REGION_is_solid() → 判断 solid/void
  │   ├─ Xchg_Lump::Create() → 每个 region 创建一个 Lump
  │   ├─ PK_REGION_ask_shells() → 遍历
  │   │   ├─ ConvertShell()
  │   │   │   ├─ PK_SHELL_ask_oriented_faces() → 遍历
  │   │   │   │   ├─ ConvertFace()
  │   │   │   │   │   ├─ PK_FACE_ask_oriented_surf() → ConvertSurface() → SetGeom()
  │   │   │   │   │   ├─ PK_FACE_ask_loops() → 遍历
  │   │   │   │   │   │   ├─ ConvertLoop()
  │   │   │   │   │   │   │   ├─ PK_LOOP_ask_type()
  │   │   │   │   │   │   │   ├─ PK_LOOP_ask_fins() → 遍历（有序 nose-to-tail）
  │   │   │   │   │   │   │   │   ├─ ConvertFin() → Xchg_Coedge
  │   │   │   │   │   │   │   │   │   ├─ PK_FIN_ask_edge() → ConvertEdge()（查 map 去重）
  │   │   │   │   │   │   │   │   │   │   ├─ PK_EDGE_ask_vertices() → ConvertVertex()（查 map 去重）
  │   │   │   │   │   │   │   │   │   │   ├─ PK_EDGE_ask_curve() → ConvertCurve()
  │   │   │   │   │   │   │   │   │   │   └─ SetStartVertex(), SetEndVertex(), SetGeom()
  │   │   │   │   │   │   │   │   │   ├─ SetEdge(), SetOrientation()
  │   │   │   │   │   │   │   │   │   └─ （可选）SetGeom(uv_curve)
  │   │   │   │   │   │   │   │   └─ AddCoedge(coedge, orientation)
  │   │   │   │   │   │   │   └─ SetOuterInfo(), SetOrientation()
  │   │   │   │   │   │   └─ AddOuterLoop() / AddInnerLoop() / AddVertexLoop()
  │   │   │   │   │   └─ SetTrimUVBox()
  │   │   │   │   └─ AddFace(face, face_orientation)
  │   │   │   └─ SetClosedInfo(), SetOuterInfo()
  │   │   └─ AddOuterShell() / AddInnerShell() / AddOpenShell() / AddWireShell()
  │   └─ AddLump()
  └─ SetTolerance()
```

#### 阶段 3：几何转换
1. **曲面转换函数**
   - 实现各类型曲面的转换函数
   - 处理参数域和边界信息
   - 设置 UV trim box

2. **曲线转换函数**
   - 实现各类型曲线的转换函数
   - 处理参数范围
   - 设置起点终点

3. **点转换**
   - 简单坐标转换

#### 阶段 4：验证
1. **转换回 PK_BODY**
   - 使用 AMCAXExchangeBase 提供的接口转换回 PK_BODY
   - 可能需要使用 `depend/AMCAXExchangeBase_AMX_latest/include/amcax/xchg_body_convertto_amx.hpp`

2. **序列化到 XT 文件**
   - 使用 Parasolid 的 `PK_PART_transmit()` 保存为 .xt 文件
   - 用 Parasolid viewer 或其他软件验证

3. **拓扑完整性检查**
   - 验证所有拓扑关系是否正确
   - 检查边界一致性
   - 验证方向信息

### 4. 关键数据结构

```cpp
class PKToXchgConverter {
private:
    // 实体映射表（用于 Face/Edge/Vertex 去重共享）
    std::unordered_map<PK_ENTITY_t, Xchg_FacePtr> pk_face_map_;       // Face 被 2 个 Shell 共享
    std::unordered_map<PK_ENTITY_t, Xchg_EdgePtr> pk_edge_map_;       // Edge 被多个 Coedge 共享
    std::unordered_map<PK_ENTITY_t, Xchg_VertexPtr> pk_vertex_map_;   // Vertex 被多个 Edge 共享
    std::unordered_map<PK_ENTITY_t, Xchg_SurfacePtr> pk_surface_map_; // Surface 几何去重
    std::unordered_map<PK_ENTITY_t, Xchg_CurvePtr> pk_curve_map_;     // Curve 几何去重

    Xchg_BodyPtr current_body_;

    // PK 内存管理辅助
    struct PKMemGuard {
        void* ptr;
        ~PKMemGuard() { if (ptr) PK_MEMORY_free(ptr); }
    };

public:
    Xchg_BodyPtr ConvertBody(PK_BODY_t pk_body);

private:
    // 拓扑转换（按层级）
    void ConvertRegionsToLumps(PK_BODY_t pk_body, PK_BODY_type_t body_type);
    Xchg_ShellPtr ConvertShell(PK_SHELL_t pk_shell);
    Xchg_FacePtr ConvertFace(PK_FACE_t pk_face);
    Xchg_LoopPtr ConvertLoop(PK_LOOP_t pk_loop);
    Xchg_CoedgePtr ConvertFin(PK_FIN_t pk_fin);        // Fin → Coedge
    Xchg_EdgePtr ConvertEdge(PK_EDGE_t pk_edge);        // 带去重
    Xchg_VertexPtr ConvertVertex(PK_VERTEX_t pk_vertex); // 带去重

    // 几何转换
    Xchg_SurfacePtr ConvertSurface(PK_SURF_t pk_surf);
    Xchg_CurvePtr ConvertCurve(PK_CURVE_t pk_curve);
    Xchg_PointPtr ConvertPoint(PK_POINT_t pk_point);

    // 具体几何类型转换
    Xchg_SurfacePtr ConvertPlaneSurface(PK_SURF_t pk_surf);
    Xchg_SurfacePtr ConvertCylindricalSurface(PK_SURF_t pk_surf);
    Xchg_SurfacePtr ConvertConicalSurface(PK_SURF_t pk_surf);
    Xchg_SurfacePtr ConvertSphericalSurface(PK_SURF_t pk_surf);
    Xchg_SurfacePtr ConvertToroidalSurface(PK_SURF_t pk_surf);
    Xchg_SurfacePtr ConvertNurbsSurface(PK_SURF_t pk_surf);
    Xchg_SurfacePtr ConvertSweptSurface(PK_SURF_t pk_surf);       // 拉伸面
    Xchg_SurfacePtr ConvertSpunSurface(PK_SURF_t pk_surf);        // 旋转面
    Xchg_SurfacePtr ConvertOffsetSurface(PK_SURF_t pk_surf);

    Xchg_CurvePtr ConvertLineCurve(PK_CURVE_t pk_curve);
    Xchg_CurvePtr ConvertCircleCurve(PK_CURVE_t pk_curve);        // → Xchg_Ellipse
    Xchg_CurvePtr ConvertEllipseCurve(PK_CURVE_t pk_curve);
    Xchg_CurvePtr ConvertNurbsCurve(PK_CURVE_t pk_curve);
    Xchg_CurvePtr ConvertConicCurve(PK_CURVE_t pk_curve);         // → Hyperbola/Parabola
    Xchg_CurvePtr ConvertOffsetCurve(PK_CURVE_t pk_curve);

    // Shell 类型判断辅助
    bool IsOuterShell(PK_SHELL_t pk_shell, PK_REGION_t pk_region, PK_BODY_type_t body_type);
};
```

### 5. 注意事项

#### 5.1 方向一致性（PK 文档 Section 14.7 规则）

**Face Normal 规则（PK）：**
- Solid body: 面法线必须指向远离 solid region 的方向
  - Outer shell 的面法线指向外
  - Inner shell（空腔）的面法线指向内（远离 solid）
- Sheet body: 面法线指向远离 body 的方向，必须一致
- `PK_FACE_ask_oriented_surf()` 返回 face normal 与 surface normal 的关系
  - true → 平行，false → 反平行

**ExchangeBase 规则：**
- `Xchg_Face` 注释："Geometrical orientation relative to basis surface is always True"
  → 意味着 Xchg_Face 的法线总是与其 basis surface 的法线一致
- `Xchg_Shell::AddFace(face, orientation)` 中的 orientation:
  "XCHG_TRUE if face material and shell material are on same side"

**转换策略（无损）：**
- 如果 PK 中 `face_orient == PK_LOGICAL_true`（face normal == surface normal）:
  → 直接使用 surface，无需翻转
- 如果 PK 中 `face_orient == PK_LOGICAL_false`（face normal != surface normal）:
  → 需要创建反向 surface 或使用 `Xchg_Face::ReverseNormal()`
  → **不能简单跳过或忽略此标志**，否则法向信息丢失
- Shell 中 face 的 orientation **不能直接传递** `PK_SHELL_ask_oriented_faces` 的结果
  → 因为 PK 和 ExchangeBase 的 orientation 语义不同
  → PK: "face normal 是否指向 shell 内部"
  → ExchangeBase: "face material 和 shell material 是否在同侧"
  → 正确映射: `xchg_shell_orient = (pk_shell_orient == pk_face_orient)`

**Edge/Fin 方向规则（PK）：**
- Loop 方向：面在 loop 的左侧（从面法线上方往下看，沿 loop 前进方向）
- Fin 方向 = Loop 方向（fin 在 loop 中 nose-to-tail 排列）
- `PK_FIN_ask_oriented_curve()` 返回 fin 方向与 curve 切线的关系
- Edge 方向由其 curve 的参数递增方向决定
- `PK_EDGE_ask_oriented_curve()` 返回 edge 方向与 curve 切线的关系

**Coedge Orientation 计算：**
- Coedge orientation = fin 方向是否与 edge 方向一致
- **推荐使用 `PK_FIN_is_positive()`**，直接返回 fin 是否与 edge 同向
  → is_positive == true → XCHG_TRUE
  → is_positive == false → XCHG_FALSE
- **不推荐** 使用 `PK_FIN_ask_oriented_curve()` 与 `PK_EDGE_ask_oriented_curve()` 比较
  → 因为 tolerant edge/fin 可能没有 curve，此时 orientation 返回值不确定

#### 5.2 退化情况

- **退化边 (degenerated edge)**：edge curve 为 null，或两个 vertex 重合
  → `Xchg_Edge::SetDegenerated(XCHG_TRUE)`
  → `Xchg_Coedge::SetDegeneratedFlag()`
- **Ring edge**：PK_EDGE_ask_vertices 返回两个 PK_ENTITY_null，表示周期曲线（如完整圆）
  → Xchg_Edge 不设置 start/end vertex
- **Vertex loop**：PK_LOOP_type_vertex_c，loop 只有一个 vertex 无 edge
  → `Xchg_Face::AddVertexLoop(vertex)` 或 `Xchg_Loop::SetVertexLoop(vertex)`
- **无 surface 的 face**：`PK_FACE_ask_surf()` 可能返回 PK_ENTITY_null
  → **不跳过 face 本身**，创建 Xchg_Face 并处理完整拓扑（loop/coedge/edge/vertex）
  → 仅跳过 `SetGeom()` 调用（无几何可设置）
  → 这种 face 仍然携带拓扑信息，丢弃则破坏完整性

#### 5.3 容差处理

- PK 中 edge 可能有 local precision（tolerant edge）
  → `PK_EDGE_ask_precision(edge, &precision)` 获取
  → `Xchg_Edge::SetTolerance(precision)` 设置到 ExchangeBase
- PK 中 vertex 也可能有 precision
  → `PK_VERTEX_ask_precision(vertex, &precision)` 获取
  → `Xchg_Vertex::SetTolerance(precision)` 设置
- Body 级别的 tolerance
  → `Xchg_Body::SetTolerance(tol)` 设置

#### 5.4 内存管理

- **PK 端**：PK 返回的数组（如 `faces`, `shells`, `fins` 等）需要 `PK_MEMORY_free()` 释放
  → 建议用 RAII 包装器（PKMemGuard）自动管理
- **ExchangeBase 端**：全部使用 `SmartPtr`（引用计数智能指针），无需手动释放
  → 所有 Create() 方法返回 SmartPtr，拷贝安全

#### 5.5 错误处理

- PK 函数返回 `PK_ERROR_code_t`，需检查每个调用的返回值
- ExchangeBase 函数返回 `Xchg_ErrorStatus`（如 `dtkNoError`）
- 建议使用宏封装错误检查：
  ```cpp
  #define PK_CHECK(call) do { \
      PK_ERROR_code_t err = (call); \
      if (err != PK_ERROR_no_errors) { \
          LOG_ERROR("PK error: %d at %s:%d", err, __FILE__, __LINE__); \
          return nullptr; \
      } \
  } while(0)
  ```

#### 5.6 特殊 Body 类型处理（Xchg 不需要 void region）

- **Solid Body**: 一个 Lump。跳过 infinite void region；solid region 的 shell → outer shell；bounded void region（空腔）的 shell → inner shell。Xchg→PK 转换时自动根据 outer shell 推导 void region。
- **Sheet Body**: 一个 Lump。只处理 infinite void region，其 shell → AddOpenShell()
- **Wire Body**: 一个 Lump。只处理 infinite void region，其 shell → AddWireShell()
- **Acorn Body**: 一个 Lump。只处理 infinite void region，其 shell → AddWireShell(vertex)
- **General Body**: 一个 Lump。遍历所有 non-void region 的 shell，逐 shell 根据 `PK_SHELL_ask_type()` 判断类型选择对应 Add 方法
- **Empty Body**: 仅有无穷 void region 且无 shell → 创建空 Lump

### 6. 测试策略

1. **单元测试**
   - 测试单个几何类型的转换
   - 测试简单拓扑结构

2. **集成测试**
   - 测试完整的 body 转换
   - 测试各种 body 类型 (solid/sheet/wire)

3. **回归测试**
   - 转换后再转回 PK_BODY
   - 比较几何和拓扑的一致性

4. **可视化验证**
   - 保存为 .xt 文件
   - 使用 CAD 软件打开验证

### 7. 预期输出

完成 step1 后，应该能够：
1. 输入一个 `PK_BODY_t`
2. 输出一个 `Xchg_BodyPtr`，包含完整的拓扑和几何信息
3. 将 `Xchg_BodyPtr` 转回 `PK_BODY_t` 并保存为 .xt 文件
4. 用 CAD 软件打开 .xt 文件，验证模型正确性


