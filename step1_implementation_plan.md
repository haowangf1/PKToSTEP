# Step1 实施方案：PK_BODY 到 AMCAXExchangeBase 中间格式转换

## 目标
将 Parasolid 的 PK_BODY 数据结构转换为 AMCAXExchangeBase 定义的中间数据格式，包括拓扑关系和几何数据的完整转换。

## 核心任务分解

### 1. 拓扑结构提取与转换

#### 1.1 Parasolid 拓扑层次结构
```
PK_BODY                          (最高级拓扑实体)
  └─ PK_REGION                   (>=1, 第一个总是无穷void region)
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
|---------|------------|----------|-------------|---------|
| PK_BODY | 包含 >=1 个 PK_REGION | Xchg_Body | 包含 >=1 个 Xchg_Lump | Body 1:1 映射 |
| PK_REGION | 属于 Body，包含 >=1 个 Shell | Xchg_Lump | 属于 Body，包含 Shell | **所有 Region → Lump**，包括 void region |
| PK_SHELL | 属于 Region，包含 Face+orientation | Xchg_Shell | 属于 Lump，包含 Face+orientation | Shell 1:1 映射，需设置 outer/inner/open |
| PK_FACE | 属于 Shell(可属于2个)，包含 Loop | Xchg_Face | 属于 Shell，包含 Loop | Face 1:1 映射 |
| PK_LOOP | 属于 Face，包含有序 Fin | Xchg_Loop | 属于 Face，包含有序 Coedge | Loop 1:1 映射 |
| PK_FIN | 属于 Loop+Edge，有sense标志 | Xchg_Coedge | 属于 Loop，关联 Edge | **Fin → Coedge** 是关键映射 |
| PK_EDGE | 包含 0-2 个 Vertex，包含 >=0 个 Fin | Xchg_Edge | 包含 Start/End Vertex | Edge 1:1 映射，**需去重共享** |
| PK_VERTEX | 零维点 | Xchg_Vertex | 零维点 | Vertex 1:1 映射，**需去重共享** |

##### 1.3.2 关键差异与处理策略

**差异1：PK_REGION vs Xchg_Lump**
- Xchg_Lump 文档明确说："aligning with Parasolid Region"，即与 PK_REGION 对齐。
- PK_BODY 总有一个**无穷 void region**（regions[0]）。
- **策略（无损转换）**：遍历 `PK_BODY_ask_regions()` 返回的**所有** regions，每个 region → 一个 Xchg_Lump。
  - 使用 `PK_REGION_is_solid()` 判断 region 类型
  - **所有 region 都要转换**，不跳过任何 region
  - Solid body：solid region → Lump（其 shell 作为 outer shell）；bounded void region → Lump（其 shell 作为 inner shell）；infinite void region → Lump（其 shell 是 solid 外边界的反向视角）
  - Sheet body：void region 包含 sheet shell → Lump + AddOpenShell
  - Wire body：void region 包含 wireframe shell → Lump + AddWireShell
  - 注意：PK 中每个 face 属于 2 个 shell（来自 face 两侧的 region），面对象需 map 去重，同一个 Xchg_Face 可能被两个 Xchg_Shell 引用

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
  - Solid region 的 shell：
    - 该 shell 是 solid region 的边界 → `lump->AddOuterShell(shell)`
  - Bounded void region 的 shell：
    - 该 shell 是空腔的边界 → `lump->AddInnerShell(shell)`
  - Infinite void region 的 shell（含 face 的）：
    - Solid body 中：该 shell 是 solid 外边界从 void 侧看 → `lump->AddOuterShell(shell)`
      （与 solid region 的 outer shell 引用相同 face，但 orientation 相反）
    - Sheet body 中：该 shell 包含 sheet face → `lump->AddOpenShell(shell)`
  - Wireframe shell：`lump->AddWireShell(shell)`
  - Acorn shell：`lump->AddWireShell(shell)`（只含 vertex）

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
  1. **Shell-Face orientation**：`PK_SHELL_ask_oriented_faces()` 返回
     - `PK_LOGICAL_true`：face normal 指向 shell 内部（shell 所属 region 方向）
     - `PK_LOGICAL_false`：face normal 指向 shell 外部
  2. **Face-Surface orientation**：`PK_FACE_ask_oriented_surf()` 返回
     - `PK_LOGICAL_true`：face normal 平行于 surface natural normal
     - `PK_LOGICAL_false`：face normal 反平行于 surface natural normal
- ExchangeBase 约定：
  - `Xchg_Face`："Geometrical orientation relative to basis surface is **always True**"
    → 即 Xchg_Face normal **始终等于** basis surface normal
  - `Xchg_Shell::AddFace(face, orientation)`："XCHG_TRUE if face material and shell material are on same side"
- **策略（无损映射，不丢弃任何 orientation 信息）**：
  - **Face-Surface orientation 的处理**：
    - PK `face_orient == true` → face normal = surface normal → 直接使用 surface，无需处理
    - PK `face_orient == false` → face normal = -surface normal → 由于 ExchangeBase 要求 face normal = surface normal，**需要反转 surface**（翻转法向/反转参数化）然后设置到 Xchg_Face
    - 反转 surface 后，face normal 方向不变，但 surface normal 方向翻转为与 face normal 一致
  - **Shell-Face orientation 的映射**：
    - PK shell_orient 描述的是 face normal 与 shell 内部的关系
    - ExchangeBase shell_orient 描述的是 material side 的关系
    - 由于反转 surface 改变了 Xchg_Face normal 的含义（现在 = 原 surface normal 的反向 = 原 face normal），shell-face orientation 需要综合 PK 的 shell_orient 和 face_orient 来计算：
      `xchg_shell_orient = (pk_shell_orient == pk_face_orient)`
    - 即：如果 PK 中 shell_orient 和 face_orient 同号 → XCHG_TRUE，异号 → XCHG_FALSE
  - **Face 去重**：同一个 PK_FACE 在两个 shell 中出现（face 两侧的 region），需用 map 去重，确保只创建一个 Xchg_Face，但在两个 Xchg_Shell 中以不同 orientation 引用

#### 1.4 关键 Parasolid API 函数（详细）

**Body 级别：**
- `PK_BODY_ask_type(body, &body_type)` → 获取 body 类型 (solid/sheet/wire/acorn/empty/general)
- `PK_BODY_ask_regions(body, &n_regions, &regions)` → 获取所有 region（第一个是无穷void region）
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
|-------------------|----------------------|---------|
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

##### 2.2 Region → Lump 转换（无损：所有 region 都转换）
```
输入: PK_BODY_t pk_body, Xchg_BodyPtr xchg_body
步骤:
1. PK_BODY_ask_regions(pk_body, &n_regions, &regions)
   → regions[0] 总是无穷 void region
   → 所有 region 都需要转换，不跳过任何一个

2. 遍历所有 regions[0..n_regions-1]:
   对每个 region:
     a. PK_REGION_is_solid(region, &is_solid) → 判断 solid/void
     b. Xchg_Lump::Create(xchg_body) → 创建 Lump
     c. xchg_body->AddLump(lump)
     d. PK_REGION_ask_shells(region, &n_shells, &shells)
     e. 遍历 shells:
        - PK_SHELL_ask_type(shell, &shell_type) → 获取 shell 类型
        - ConvertShell(shell) → xchg_shell
        - 根据 region 类型 + shell 类型决定添加方式:

3. Shell 添加方式判断:

   ═══ Solid Region 的 Shell ═══
   - 该 shell 是 solid region 的边界
   - lump->AddOuterShell(xchg_shell)

   ═══ Bounded Void Region 的 Shell（空腔）═══
   - 该 shell 是空腔的边界
   - lump->AddInnerShell(xchg_shell)

   ═══ Infinite Void Region 的 Shell ═══
   根据 shell 类型:
   - wireframe_free/mixed (含 face):
     - Solid body 中: lump->AddOuterShell(xchg_shell)
       → 这些 face 与 solid region shell 的 face 是同一批（PK 中每个 face 属于 2 个 shell）
       → 用 face map 去重，确保同一 PK_FACE 只创建一个 Xchg_Face
     - Sheet body 中: lump->AddOpenShell(xchg_shell)
   - wireframe (只有 edge): lump->AddWireShell(xchg_shell)
   - acorn (只有 vertex): lump->AddWireShell(xchg_shell)

   ═══ Acorn Body ═══
   - 只有 void region，shell 中只有 vertex
   - lump->AddWireShell(xchg_shell)

   ═══ General Body ═══
   - 可能混合多种 shell 类型
   - 逐 shell 根据 PK_SHELL_ask_type() 判断并选择对应的 Add 方法

注意：Face 去重至关重要
- 在 PK 中，每个 face 被 2 个 shell 引用（face 两侧的 2 个 region 各有一个 shell）
- 转换时需用 pk_face_map_ 确保同一 PK_FACE 只创建一个 Xchg_Face
- 同一 Xchg_Face 被两个 Xchg_Shell 以不同 orientation 引用
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

4. （可选）设置 UV 曲线:
   - 对于 tolerant edge，fin 上可能有 SP-curve
   - PK_FIN_ask_curve(pk_fin, &fin_curve) → 如果不为 null，转换后设置
   - xchg_coedge->SetGeom(uv_curve)

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
    // 实体映射表（用于 Edge/Vertex 去重共享）
    std::unordered_map<PK_ENTITY_t, Xchg_EdgePtr> pk_edge_map_;
    std::unordered_map<PK_ENTITY_t, Xchg_VertexPtr> pk_vertex_map_;
    std::unordered_map<PK_ENTITY_t, Xchg_SurfacePtr> pk_surface_map_;
    std::unordered_map<PK_ENTITY_t, Xchg_CurvePtr> pk_curve_map_;

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

**转换策略：**
- 如果 PK 中 `face_orient == PK_LOGICAL_true`（face normal == surface normal）:
  → 直接使用 surface，无需翻转
- 如果 PK 中 `face_orient == PK_LOGICAL_false`（face normal != surface normal）:
  → 需要翻转 surface normal（使用 `PK_BODY_reverse_orientation` 或在 ExchangeBase 端处理）
  → 或使用 `Xchg_Face::ReverseNormal()`
- Shell 中 face 的 orientation 直接传递 `PK_SHELL_ask_oriented_faces` 的结果

**Edge/Fin 方向规则（PK）：**
- Loop 方向：面在 loop 的左侧（从面法线上方往下看，沿 loop 前进方向）
- Fin 方向 = Loop 方向（fin 在 loop 中 nose-to-tail 排列）
- `PK_FIN_ask_oriented_curve()` 返回 fin 方向与 curve 切线的关系
- Edge 方向由其 curve 的参数递增方向决定
- `PK_EDGE_ask_oriented_curve()` 返回 edge 方向与 curve 切线的关系

**Coedge Orientation 计算：**
- Coedge orientation = fin 方向是否与 edge 方向一致
- 如果 fin_orient == edge_orient → same sense → XCHG_TRUE
- 如果 fin_orient != edge_orient → opposite sense → XCHG_FALSE

#### 5.2 退化情况

- **退化边 (degenerated edge)**：edge curve 为 null，或两个 vertex 重合
  → `Xchg_Edge::SetDegenerated(XCHG_TRUE)`
  → `Xchg_Coedge::SetDegeneratedFlag()`
- **Ring edge**：PK_EDGE_ask_vertices 返回两个 PK_ENTITY_null，表示周期曲线（如完整圆）
  → Xchg_Edge 不设置 start/end vertex
- **Vertex loop**：PK_LOOP_type_vertex_c，loop 只有一个 vertex 无 edge
  → `Xchg_Face::AddVertexLoop(vertex)` 或 `Xchg_Loop::SetVertexLoop(vertex)`
- **无 surface 的 face**：`PK_FACE_ask_surf()` 可能返回 PK_ENTITY_null
  → 跳过几何设置，仅处理拓扑

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

#### 5.6 特殊 Body 类型处理

- **Solid Body**: 最常见，Region → Lump，需要正确区分 outer/inner shell
- **Sheet Body**: 所有 shell 在 void region 中，使用 AddOpenShell()
- **Wire Body**: 只有 edge，无 face，使用 AddWireShell()
- **Acorn Body**: 只有 vertex，无 edge/face，使用 AddWire(vertex)
- **General Body**: 可能混合以上类型，需要逐 shell 判断类型

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


