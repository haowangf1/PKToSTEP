# Parasolid PK API 参考文档（从内网文档提炼）

## 文档访问方式

**文档服务器地址：** `http://192.168.180.50/docs/parasolid/`

**访问方式（PowerShell）：**
```powershell
# 获取页面内容
$r = Invoke-WebRequest -Uri "http://192.168.180.50/docs/parasolid/<页面路径>" -UseBasicParsing -TimeoutSec 10
# 清理 HTML 标签
$text = $r.Content -replace '<[^>]+>', ' ' -replace '&nbsp;', ' ' -replace '\s+', ' '
```

**文档结构：**
- 主页: `http://192.168.180.50/docs/parasolid/` (frameset 页面)
- PK Reference 索引: `http://192.168.180.50/docs/parasolid/pk_index.html`
- Functional Description 索引: `http://192.168.180.50/docs/parasolid/fd_index.html`
- API 函数文档: `http://192.168.180.50/docs/parasolid/headers/pk_<函数名>.html`
- 数据类型文档: `http://192.168.180.50/docs/parasolid/headers/pk_<类型名>.html`
- FD 章节: `http://192.168.180.50/docs/parasolid/chapters/fd_chap.<编号>.html`
  - Model Structure: `fd_chap.015.html`
  - Body Types: `fd_chap.016.html`
  - Geometry: `fd_chap.018.html`
  - B-Curves And B-Surfaces: `fd_chap.019.html`

---

## 一、基础数据类型

### 1.1 PK_VECTOR_t — 三维向量
```c
struct PK_VECTOR_s {
    double coord[3];
};
typedef struct PK_VECTOR_s PK_VECTOR_t;
```

### 1.2 PK_AXIS1_sf_t — 单轴定位（点+方向）
```c
struct PK_AXIS1_sf_s {
    PK_VECTOR_t  location;   // 笛卡尔坐标点
    PK_VECTOR1_t axis;       // 方向
};
typedef struct PK_AXIS1_sf_s PK_AXIS1_sf_t;
```
对应 STEP 的 `axis1_placement`。

### 1.3 PK_AXIS2_sf_t — 双轴定位（点+法向+参考方向）
```c
struct PK_AXIS2_sf_s {
    PK_VECTOR_t  location;       // 笛卡尔坐标点
    PK_VECTOR1_t axis;           // 法向/轴方向
    PK_VECTOR1_t ref_direction;  // 参考方向（与 axis 正交）
};
typedef struct PK_AXIS2_sf_s PK_AXIS2_sf_t;
```
对应 STEP 的 `axis2_placement`。所有解析曲面和曲线的参数化都基于此结构。

在曲面/曲线参数化中的通用约定：
- `x = ref_direction`
- `y = axis × ref_direction`（叉积）
- `z = axis`

### 1.4 PK_TOPOL_sense_t — 拓扑 sense
```c
typedef int PK_TOPOL_sense_t;
PK_TOPOL_sense_none_c       // sense 无关
PK_TOPOL_sense_positive_c   // 正向
PK_TOPOL_sense_negative_c   // 反向
```

---

## 二、拓扑 API

### 2.1 Body 级别

#### PK_BODY_ask_type — 获取 body 类型
```c
PK_ERROR_code_t PK_BODY_ask_type(
    PK_BODY_t       body,        // 输入: body
    PK_BODY_type_t *body_type    // 输出: body 类型
);
```
**PK_BODY_type_t 枚举值：**

| 值 | 说明 |
|---|------|
| `PK_BODY_type_empty_c` | 空 body，仅含一个无界 void region |
| `PK_BODY_type_minimum_c` | 0维流形体，含单个孤立顶点（acorn 特例） |
| `PK_BODY_type_acorn_c` | 0维流形体，含两个或以上孤立顶点 |
| `PK_BODY_type_wire_c` | 1维流形体，含一个或多个连通边集（任意顶点处不超过2条边） |
| `PK_BODY_type_sheet_c` | 2维流形体，含一个或多个连通面集（任意边处不超过2个面） |
| `PK_BODY_type_solid_c` | 3维流形体，含一个或多个不相交的实体 region。所有面构成 solid 和 void region 之间的边界 |
| `PK_BODY_type_general_c` | 非流形体和/或混合拓扑维度体 |
| `PK_BODY_type_compound_c` | 复合体，包含零个或多个子体 |
| `PK_BODY_type_unspecified_c` | 未指定 |

#### PK_BODY_ask_regions — 获取 body 的所有 region
```c
PK_ERROR_code_t PK_BODY_ask_regions(
    PK_BODY_t     body,       // 输入: body
    int          *n_regions,  // 输出: region 数量 (>= 1)
    PK_REGION_t **regions     // 输出: region 数组（可选，传 NULL 只获取数量）
);
```
**重要：`regions[0]` 总是无穷 void region（body 的无限外部空间）。**

对于不同 body 类型的 region 结构：
- **Solid body**: 1个无穷void region + N个solid region + M个有界void region（空腔）
- **Sheet body**: 1个无穷void region（包含sheet的shell） + 闭合部分有额外bounded void region
- **Wire body**: 1个无穷void region（包含wire的shell）
- **Acorn body**: 1个无穷void region（包含acorn的shell）

#### PK_BODY_ask_topology — 一次性获取完整拓扑定义
```c
PK_ERROR_code_t PK_BODY_ask_topology(
    PK_BODY_t                     body,         // 输入: body
    const PK_BODY_ask_topology_o_t *options,    // 输入: 选项（是否包含 fin/frame）
    int                          *n_topols,     // 输出: 拓扑实体数量
    PK_TOPOL_t                  **topols,       // 输出: 拓扑实体数组
    PK_CLASS_t                  **classes,      // 输出: 每个实体的类型
    int                          *n_relations,  // 输出: 关系数量
    int                         **parents,      // 输出: 父实体索引数组
    int                         **children,     // 输出: 子实体索引数组
    PK_TOPOL_sense_t            **senses        // 输出: 关系 sense 数组
);
```
返回的拓扑关系格式与 `PK_BODY_create_topology_2` 相同。

**拓扑父子关系表：**

| 实体 | 子实体 | 子实体数量 | 父实体 | 父实体数量 |
|------|--------|-----------|--------|-----------|
| Body | Region | >= 1 | - | - |
| Region | Shell | >= 1 | Body | 1 |
| Shell | Face | >= 1, Edge >= 1, Vertex 1 | Region | 1 |
| Face | Loop | >= 1 | Shell | 2 |
| Loop | Edge/Fin >= 1, Vertex 1 | | Face, Edge | >= 1 |
| Fin | Edge 1 | | Loop, Edge | <= 2 |
| Edge | Vertex | <= 2 | Loop/Fin/Shell | 1, Loop >= 1, Fin >= 1 |
| Vertex | - | 0 | Edge/Loop/Shell | >= 1 |

**Sense 含义：**

| 父-子关系 | positive_c | negative_c |
|----------|-----------|------------|
| Body → Region | region 是 solid | region 是 void |
| Shell → Face | face normal 指向 shell 内部 | face normal 指向 shell 外部 |
| Loop/Fin → Edge | edge 方向与 fin/loop 方向一致 | edge 方向与 fin/loop 方向相反 |
| Edge → Fin/Loop | edge 方向与 fin/loop 方向一致 | edge 方向与 fin/loop 方向相反 |

### 2.2 Region 级别

#### PK_REGION_ask_shells — 获取 region 的 shell
```c
PK_ERROR_code_t PK_REGION_ask_shells(
    PK_REGION_t  region,    // 输入: region
    int         *n_shells,  // 输出: shell 数量 (>= 0)
    PK_SHELL_t **shells     // 输出: shell 数组（可选）
);
```
region 的 outer shell 不做区分。region 可能没有 shell（代表全部空间）。

#### PK_REGION_is_solid — 判断 region 是 solid 还是 void
```c
PK_ERROR_code_t PK_REGION_is_solid(
    PK_REGION_t   region,     // 输入: region
    PK_LOGICAL_t *is_solid    // 输出: true=solid, false=void
);
```

#### PK_REGION_ask_type — 获取 region 类型
```c
PK_ERROR_code_t PK_REGION_ask_type(
    PK_REGION_t               region,   // 输入: region
    const PK_REGION_ask_type_o_t *options,
    PK_REGION_ask_type_r_t   *results   // 输出: 类型结果
);
```

### 2.3 Shell 级别

#### PK_SHELL_ask_type — 获取 shell 类型
```c
PK_ERROR_code_t PK_SHELL_ask_type(
    PK_SHELL_t       shell,       // 输入: shell
    PK_SHELL_type_t *shell_type   // 输出: shell 类型
);
```
**PK_SHELL_type_t 枚举值：**

| 值 | 说明 |
|---|------|
| `PK_SHELL_type_acorn_c` | shell 只有一个 acorn vertex |
| `PK_SHELL_type_wireframe_c` | shell 有一条或多条 wireframe edge（无face） |
| `PK_SHELL_type_wireframe_free_c` | shell 有一个或多个 face（无wireframe edge） |
| `PK_SHELL_type_mixed_c` | shell 既有 wireframe edge 也有 face |

#### PK_SHELL_ask_oriented_faces — 获取 shell 中的面及朝向
```c
PK_ERROR_code_t PK_SHELL_ask_oriented_faces(
    PK_SHELL_t     shell,     // 输入: shell
    int           *n_faces,   // 输出: face 数量 (>= 0)
    PK_FACE_t    **faces,     // 输出: face 数组（可选）
    PK_LOGICAL_t **orients    // 输出: 朝向数组（可选）
);
```
**朝向含义：**
- `PK_LOGICAL_true`: face normal 指向 shell 内部（即指向 shell 所属 region 的方向）
- `PK_LOGICAL_false`: face normal 指向 shell 外部

**注意：** 一个 face 最多出现两次（正反两面），以相反的 orientation 出现。

#### PK_SHELL_ask_wireframe_edges — 获取 wireframe edge
```c
PK_ERROR_code_t PK_SHELL_ask_wireframe_edges(
    PK_SHELL_t  shell,
    int        *n_edges,
    PK_EDGE_t **edges
);
```

#### PK_SHELL_ask_acorn_vertex — 获取 acorn vertex
```c
PK_ERROR_code_t PK_SHELL_ask_acorn_vertex(
    PK_SHELL_t    shell,    // 输入: shell
    PK_VERTEX_t  *vertex    // 输出: acorn vertex 或 PK_ENTITY_null
);
```
仅当 shell 类型为 `PK_SHELL_type_acorn_c` 时返回有效 vertex。

#### PK_SHELL_ask_region — 获取 shell 所属 region
```c
PK_ERROR_code_t PK_SHELL_ask_region(
    PK_SHELL_t   shell,    // 输入: shell
    PK_REGION_t *region     // 输出: 所属 region
);
```

### 2.4 Face 级别

#### PK_FACE_ask_loops — 获取 face 的 loop
```c
PK_ERROR_code_t PK_FACE_ask_loops(
    PK_FACE_t   face,      // 输入: face
    int        *n_loops,    // 输出: loop 数量 (>= 0)
    PK_LOOP_t **loops       // 输出: loop 数组（可选）
);
```
**注意：** outer loop 不做区分，需用 `PK_LOOP_ask_type()` 判断或用 `PK_FACE_find_outer_loop()` 查找。
0个 loop 表示完整封闭面（如完整球面）。

#### PK_FACE_ask_surf — 获取 face 的曲面
```c
PK_ERROR_code_t PK_FACE_ask_surf(
    PK_FACE_t  face,   // 输入: face
    PK_SURF_t *surf     // 输出: 曲面（可能为 PK_ENTITY_null）
);
```

#### PK_FACE_ask_oriented_surf — 获取 face 的曲面及朝向
```c
PK_ERROR_code_t PK_FACE_ask_oriented_surf(
    PK_FACE_t     face,          // 输入: face
    PK_SURF_t    *surf,          // 输出: 曲面（可能为 PK_ENTITY_null）
    PK_LOGICAL_t *orientation    // 输出: face 相对于 surface 的朝向
);
```
**朝向含义：**
- `PK_LOGICAL_true`: face normal 平行于 surface normal
- `PK_LOGICAL_false`: face normal 反平行于 surface normal（anti-parallel）
- 无 surface 时，orientation 值不确定

#### PK_FACE_find_outer_loop — 查找 face 的 outer loop
```c
PK_ERROR_code_t PK_FACE_find_outer_loop(
    PK_FACE_t                         face,        // 输入: face
    const PK_FACE_find_outer_loop_o_t *options,     // 输入: 选项
    PK_LOOP_t                        *outer_loop    // 输出: outer loop 或 PK_ENTITY_null
);
```
**注意：**
- 无 loop（如完整球面）返回 PK_ENTITY_null
- face 无几何面时报错 PK_ERROR_missing_geom
- 参数接缝未 imprint 时返回 PK_ENTITY_null

### 2.5 Loop 级别

#### PK_LOOP_ask_type — 获取 loop 类型
```c
PK_ERROR_code_t PK_LOOP_ask_type(
    PK_LOOP_t       loop,       // 输入: loop
    PK_LOOP_type_t *loop_type   // 输出: loop 类型
);
```
**PK_LOOP_type_t 枚举值（完整）：**

| 值 | 说明 | 面类型 |
|---|------|-------|
| `PK_LOOP_type_vertex_c` | 顶点环，只有顶点没有边 | 任意 |
| `PK_LOOP_type_wire_c` | 线框环，无内部（如来自 wire body） | 任意 |
| `PK_LOOP_type_outer_c` | 简单外周环 | 非周期面 |
| `PK_LOOP_type_inner_c` | 简单内孔环 | 非周期面 |
| `PK_LOOP_type_winding_c` | 绕环（在周期面上绕一圈） | 柱面/环面等周期面 |
| `PK_LOOP_type_inner_sing_c` | 围绕曲面奇异点的内孔环（如锥面顶部） | 锥面等退化周期面 |
| `PK_LOOP_type_likely_outer_c` | 双闭合面上的可能外环 | 双周期面 |
| `PK_LOOP_type_likely_inner_c` | 双闭合面上的可能内孔 | 双周期面 |
| `PK_LOOP_type_unclear_c` | 无法确定（含一个极点的周期退化面，或 facet 几何面） | 特殊退化面 |
| `PK_LOOP_type_error_c` | 无效 loop 或算法失败 | - |

**非周期面规则：** 每个 face 有一个 `outer_c`（外周环）和任意数量的 `inner_c`（孔）。

**单周期面规则（柱面等）：** 需要一对 `winding_c` 或一个 `outer_c` 来定义面的范围。可含任意数量 `inner_c`。

**退化周期面规则（锥面等）：** 只有一个 `outer_c`。绕奇异点的孔是 `inner_sing_c`。

**双周期面规则（环面等）：** 使用 `winding_c`、`likely_outer_c`、`likely_inner_c`。

#### PK_LOOP_ask_fins — 获取 loop 中有序排列的 fin
```c
PK_ERROR_code_t PK_LOOP_ask_fins(
    PK_LOOP_t  loop,     // 输入: loop
    int       *n_fins,   // 输出: fin 数量 (>= 0)
    PK_FIN_t **fins      // 输出: fin 数组（可选）
);
```
**Fin 排列规则（极重要）：**
- Fin 按 **nose-to-tail** 顺序排列
- 每个 fin 的终点与下一个 fin 的起点重合
- 最后一个 fin 的终点与第一个 fin 的起点重合（闭合）
- 沿 fin 方向看，face normal 朝上时，**fin 所属 loop 的 face 在左侧**

### 2.6 Fin 级别

#### PK_FIN_ask_edge — 获取 fin 所属的 edge
```c
PK_ERROR_code_t PK_FIN_ask_edge(
    PK_FIN_t   fin,    // 输入: fin
    PK_EDGE_t *edge    // 输出: 所属 edge
);
```

#### PK_FIN_is_positive — 判断 fin 是否与 edge 同向
```c
PK_ERROR_code_t PK_FIN_is_positive(
    PK_FIN_t      fin,          // 输入: fin
    PK_LOGICAL_t *is_positive   // 输出: true = fin 与 edge 同向
);
```
**这是计算 Coedge orientation 的最直接方式。**

#### PK_FIN_ask_oriented_curve — 获取 fin 的 curve 及方向
```c
PK_ERROR_code_t PK_FIN_ask_oriented_curve(
    PK_FIN_t      fin,          // 输入: fin
    PK_CURVE_t   *curve,        // 输出: curve（可能为 PK_ENTITY_null）
    PK_LOGICAL_t *orientation   // 输出: fin 方向与 curve 切线的关系
);
```
- `PK_LOGICAL_true`: fin 方向平行于 curve 切线
- `PK_LOGICAL_false`: fin 方向反平行于 curve 切线
- 无 curve（如非 tolerant fin）时 curve 为 null，orientation 不确定

**注意：** 非 tolerant fin 通常没有自己的 curve。tolerant edge 的几何信息附在每个 fin 上（SP-curve）。

#### PK_FIN_ask_next_in_loop — 获取 loop 中的下一个 fin
```c
PK_ERROR_code_t PK_FIN_ask_next_in_loop(
    PK_FIN_t  fin,    // 输入: fin
    PK_FIN_t *next    // 输出: loop 中的下一个 fin
);
```
可与 `PK_LOOP_ask_first_fin()` 配合遍历 loop 中的 fin。排列是循环的。

#### PK_FIN_ask_type — 获取 fin 类型
```c
PK_ERROR_code_t PK_FIN_ask_type(
    PK_FIN_t       fin,       // 输入: fin
    PK_FIN_type_t *fin_type   // 输出: fin 类型
);
```
**PK_FIN_type_t 枚举值：**

| 值 | 说明 |
|---|------|
| `PK_FIN_type_wire_c` | fin 的 edge 在同一 face 中有另一个 fin，且两个 fin 在同一 loop 中 |
| `PK_FIN_type_biwire_c` | fin 的 edge 在同一 face 中有另一个 fin，但在不同 loop 中 |
| `PK_FIN_type_normal_c` | fin 的 edge 在同一 face 中没有其他 fin |

### 2.7 Edge 级别

#### PK_EDGE_ask_vertices — 获取 edge 的起止 vertex
```c
PK_ERROR_code_t PK_EDGE_ask_vertices(
    PK_EDGE_t   edge,         // 输入: edge
    PK_VERTEX_t vertices[2]   // 输出: [0]=起始vertex, [1]=终止vertex
);
```
**Ring edge：** 如果 edge 是一条完整的周期曲线（如完整圆），两个 vertex 都是 `PK_ENTITY_null`。

#### PK_EDGE_ask_curve — 获取 edge 的 3D curve
```c
PK_ERROR_code_t PK_EDGE_ask_curve(
    PK_EDGE_t   edge,    // 输入: edge
    PK_CURVE_t *curve    // 输出: curve（可能为 PK_ENTITY_null）
);
```
**无 curve 的情况：** edge 可能是 tolerant 的，此时 curve 几何附在每个 fin 上。

#### PK_EDGE_ask_oriented_curve — 获取 edge 的 curve 及方向
```c
PK_ERROR_code_t PK_EDGE_ask_oriented_curve(
    PK_EDGE_t     edge,          // 输入: edge
    PK_CURVE_t   *curve,         // 输出: curve（可能为 PK_ENTITY_null）
    PK_LOGICAL_t *orientation    // 输出: edge 方向与 curve 切线的关系
);
```
- `PK_LOGICAL_true`: edge 方向平行于 curve 切线
- `PK_LOGICAL_false`: edge 方向反平行于 curve 切线

#### PK_EDGE_ask_fins — 获取 edge 的所有 fin
```c
PK_ERROR_code_t PK_EDGE_ask_fins(
    PK_EDGE_t  edge,     // 输入: edge
    int       *n_fins,   // 输出: fin 数量 (>= 0)
    PK_FIN_t **fins      // 输出: fin 数组（可选）
);
```
**Fin 排列顺序：** 沿 edge 方向看，后续 fin 按**顺时针**排列（空间方向）。

**Edge 类型（按 fin 数量）：**

| 类型 | fin 数量 | 说明 |
|------|---------|------|
| wireframe | 0 | 无 fin，父级是 shell |
| laminar | 1 | 一个 fin（开放边） |
| normal | 2（sense相反） | 两个面共享的常规边 |
| general | 2（sense相同）或其他 | 非流形边（如内部分割面的边） |

#### PK_EDGE_ask_type — 获取 edge 类型（多维分类）
```c
PK_ERROR_code_t PK_EDGE_ask_type(
    PK_EDGE_t            edge,        // 输入: edge
    PK_EDGE_ask_type_t  *edge_types   // 输出: edge 类型结构
);
```
返回结构包含三个独立分类：

**vertex_type（按顶点）：**
| 值 | 说明 |
|---|------|
| `PK_EDGE_type_open_c` | 两个不同的 vertex |
| `PK_EDGE_type_closed_c` | 起止 vertex 相同 |
| `PK_EDGE_type_ring_c` | 无 vertex（完整周期曲线） |

**enclosure（按周围空间）：**
| 值 | 说明 |
|---|------|
| `PK_enclosure_inside_c` | 完全被 solid region 包围 |
| `PK_enclosure_outside_c` | 完全被 void region 包围 |
| `PK_enclosure_on_c` | 同时邻接 solid 和 void region |

**fins_type（按 fin 数量）：**
| 值 | 说明 |
|---|------|
| `PK_EDGE_type_wireframe_c` | 无 fin |
| `PK_EDGE_type_laminar_c` | 1个 fin |
| `PK_EDGE_type_normal_c` | 2个 fin，sense 相反 |
| `PK_EDGE_type_general_c` | 其他 fin 组合 |

### 2.8 Vertex 级别

#### PK_VERTEX_ask_point — 获取 vertex 的 point
```c
PK_ERROR_code_t PK_VERTEX_ask_point(
    PK_VERTEX_t  vertex,   // 输入: vertex
    PK_POINT_t  *point     // 输出: point（可能为 PK_ENTITY_null）
);
```

---

## 三、几何 API

### 3.1 Point

#### PK_POINT_ask — 获取 point 坐标
```c
PK_ERROR_code_t PK_POINT_ask(
    PK_POINT_t     point,      // 输入: point
    PK_POINT_sf_t *point_sf    // 输出: point standard form
);
```
**PK_POINT_sf_t：**
```c
struct PK_POINT_sf_s {
    PK_VECTOR_t position;    // 笛卡尔坐标位置 (x, y, z)
};
```

### 3.2 曲线 (Curve)

#### 3.2.1 PK_LINE — 直线
```c
PK_ERROR_code_t PK_LINE_ask(PK_LINE_t line, PK_LINE_sf_t *line_sf);
```
**PK_LINE_sf_t：**
```c
struct PK_LINE_sf_s {
    PK_AXIS1_sf_t basis_set;  // location + direction
};
```
**参数化：** `P(t) = location + t * direction`

#### 3.2.2 PK_CIRCLE — 圆
```c
PK_ERROR_code_t PK_CIRCLE_ask(PK_CIRCLE_t circle, PK_CIRCLE_sf_t *circle_sf);
```
**PK_CIRCLE_sf_t：**
```c
struct PK_CIRCLE_sf_s {
    PK_AXIS2_sf_t basis_set;  // 中心、平面法向、t=0 方向
    double        radius;     // 半径 (> 0)
};
```
**参数化：**
```
C = location, x = ref_direction, y = axis × ref_direction, R = radius
P(t) = C + R·(cos(t)·x + sin(t)·y)     (0 <= t < 2π)
```

#### 3.2.3 PK_ELLIPSE — 椭圆
```c
PK_ERROR_code_t PK_ELLIPSE_ask(PK_ELLIPSE_t ellipse, PK_ELLIPSE_sf_t *ellipse_sf);
```
**PK_ELLIPSE_sf_t：**
```c
struct PK_ELLIPSE_sf_s {
    PK_AXIS2_sf_t basis_set;  // 中心、平面法向、长轴方向
    double        R1;         // 长半径 (> 0)
    double        R2;         // 短半径 (> 0, R1 > R2)
};
```
**参数化：**
```
C = location, x = ref_direction (长轴), y = axis × ref_direction
P(t) = C + R1·cos(t)·x + R2·sin(t)·y     (0 <= t < 2π)
```
**注意：** R1 必须大于 R2。

#### 3.2.4 PK_BCURVE — B-spline 曲线
```c
PK_ERROR_code_t PK_BCURVE_ask(PK_BCURVE_t bcurve, PK_BCURVE_sf_t *bcurve_sf);
```
**PK_BCURVE_sf_t（关键字段）：**
```c
struct PK_BCURVE_sf_s {
    int             degree;          // 阶数 = order - 1
    int             n_vertices;      // 控制点数量
    int             vertex_dim;      // 每个顶点的维度：2(UV曲线)/3(3D多项式)/4(3D有理)
    PK_LOGICAL_t    is_rational;     // 是否有理
    double         *vertex;          // 控制点数组
    PK_BCURVE_form_t form;           // 曲线形状
    int             n_knots;         // 不同节点值的数量
    int            *knot_mult;       // 每个节点的重复度
    double         *knot;            // 节点值（单调递增）
    PK_knot_type_t  knot_type;       // 节点类型枚举
    PK_LOGICAL_t    is_periodic;     // 是否周期性
    PK_LOGICAL_t    is_closed;       // 是否闭合
    PK_self_intersect_t self_intersecting;  // 自交信息
};
```
**与 STEP 的关键区别：**
- **vertex_dim**: PK 用 2/3/4；STEP 的 cartesian_point 用 1/2/3
- **is_rational**: PK 用此标志 + vertex_dim=4；STEP 用独立子类型 `rational_b_spline_curve`
- **vertex 数组格式**:
  - 多项式 3D: `[x1, y1, z1, x2, y2, z2, ...]`
  - 有理 3D: `[x1·w1, y1·w1, z1·w1, w1, x2·w2, ...]`（**坐标乘以权重**）
  - **STEP 不乘权重**，权重单独存放
- **knot**: PK 存储不重复节点值 + 重复度数组；STEP 存储完整的节点向量

#### 3.2.5 PK_OFFSET — 偏移曲线
参见 `PK_OFFSET_ask()` 和 `PK_OFFSET_sf_t`。

### 3.3 曲面 (Surface)

#### 3.3.1 PK_PLANE — 平面
```c
PK_ERROR_code_t PK_PLANE_ask(PK_PLANE_t plane, PK_PLANE_sf_t *plane_sf);
```
**PK_PLANE_sf_t：**
```c
struct PK_PLANE_sf_s {
    PK_AXIS2_sf_t basis_set;   // 点 + 法向 + 参考方向
};
```
**参数化：**
```
P = location, x = ref_direction, y = axis × ref_direction
S(u, v) = P + u·x + v·y
```

#### 3.3.2 PK_CYL — 圆柱面
```c
PK_ERROR_code_t PK_CYL_ask(PK_CYL_t cylinder, PK_CYL_sf_t *cyl_sf);
```
**PK_CYL_sf_t：**
```c
struct PK_CYL_sf_s {
    PK_AXIS2_sf_t basis_set;   // location + 轴方向 + u=0 方向
    double        radius;      // 半径 (> 0)
};
```
**参数化：**
```
P = location, x = ref_direction, y = axis × ref_direction, z = axis, R = radius
S(u, v) = P + R·(cos(u)·x + sin(u)·y) + v·z     (0 <= u < 2π)
```
法向指向远离轴线方向。

#### 3.3.3 PK_CONE — 锥面
```c
PK_ERROR_code_t PK_CONE_ask(PK_CONE_t cone, PK_CONE_sf_t *cone_sf);
```
**PK_CONE_sf_t：**
```c
struct PK_CONE_sf_s {
    PK_AXIS2_sf_t basis_set;     // location + 轴方向 + u=0 方向
    double        radius;        // 半径 (>= 0)
    double        semi_angle;    // 半角（弧度，严格在 0 到 π/2 之间）
};
```
**参数化：**
```
P = location, x = ref_direction, y = axis × ref_direction, z = axis
R = radius, w = semi_angle
S(u, v) = P + (R + v·tan(w))·(cos(u)·x + sin(u)·y) + v·z
    (0 <= u < 2π, -R·cot(w) <= v < ∞)
```
**注意：**
- axis 方向是圆半径增大的方向（即指向远离顶点的方向）
- 只表示半锥面（不像 STEP 的完整锥面定义包含两个半锥面）

#### 3.3.4 PK_SPHERE — 球面
```c
PK_ERROR_code_t PK_SPHERE_ask(PK_SPHERE_t sphere, PK_SPHERE_sf_t *sphere_sf);
```
**PK_SPHERE_sf_t：**
```c
struct PK_SPHERE_sf_s {
    PK_AXIS2_sf_t basis_set;   // 中心 + 北极方向 + u=0 方向
    double        radius;      // 半径 (> 0)
};
```
**参数化：**
```
C = location, x = ref_direction, y = axis × ref_direction, z = axis, R = radius
S(u, v) = C + R·cos(v)·(cos(u)·x + sin(u)·y) + R·sin(v)·z
    (0 <= u < 2π, -π/2 <= v < π/2)
```
法向指向远离球心方向。

#### 3.3.5 PK_TORUS — 环面
```c
PK_ERROR_code_t PK_TORUS_ask(PK_TORUS_t torus, PK_TORUS_sf_t *torus_sf);
```
**PK_TORUS_sf_t：**
```c
struct PK_TORUS_sf_s {
    PK_AXIS2_sf_t basis_set;      // 中心 + 轴方向 + 参考方向
    double        major_radius;   // 主半径 (!= 0)
    double        minor_radius;   // 副半径 (> 0)
};
```
**参数化：**
```
C = location, x = ref_direction, y = axis × ref_direction, z = axis
R = major_radius, r = minor_radius
S(u, v) = C + (R + r·cos(v))·(cos(u)·x + sin(u)·y) + r·sin(v)·z
    (0 <= u < 2π, -π <= v < π)
```
**自交情况：**
- `major_radius > 0` 且 `<= minor_radius` → apple（苹果形，外表面）
- `major_radius < 0` 且 `major_radius + minor_radius > 0` → lemon（柠檬形，内表面）

#### 3.3.6 PK_BSURF — B-spline 曲面
```c
PK_ERROR_code_t PK_BSURF_ask(PK_BSURF_t bsurf, PK_BSURF_sf_t *bsurf_sf);
```
**PK_BSURF_sf_t（关键字段）：**
```c
struct PK_BSURF_sf_s {
    int             u_degree;          // u 方向阶数
    int             v_degree;          // v 方向阶数
    int             n_u_vertices;      // u 方向控制点数
    int             n_v_vertices;      // v 方向控制点数
    int             vertex_dim;        // 维度：3(多项式) 或 4(有理)
    PK_LOGICAL_t    is_rational;       // 是否有理
    double         *vertex;            // 控制点数组
    PK_BSURF_form_t form;              // 曲面形状
    int             n_u_knots;         // u 方向不同节点数
    int             n_v_knots;         // v 方向不同节点数
    int            *u_knot_mult;       // u 节点重复度
    int            *v_knot_mult;       // v 节点重复度
    double         *u_knot;            // u 节点值
    double         *v_knot;            // v 节点值
    PK_knot_type_t  u_knot_type;       // u 节点类型
    PK_knot_type_t  v_knot_type;       // v 节点类型
    PK_LOGICAL_t    is_u_periodic;     // u 方向周期性
    PK_LOGICAL_t    is_v_periodic;     // v 方向周期性
    PK_LOGICAL_t    is_u_closed;       // u 方向闭合
    PK_LOGICAL_t    is_v_closed;       // v 方向闭合
    PK_self_intersect_t self_intersecting;
    PK_convexity_t  convexity;
};
```
**与 STEP 的关键区别（同 BCURVE）：**
- 有理曲面 vertex_dim=4，坐标乘以权重：`[x1·w1, y1·w1, z1·w1, w1, ...]`
- 控制点按 **v 优先** 排列（v 变化最快，然后 u）
- 节点存储为 不重复值 + 重复度

#### 3.3.7 PK_SPUN — 旋转面 (Revolution Surface)
```c
PK_ERROR_code_t PK_SPUN_ask(PK_SPUN_t spun, PK_SPUN_sf_t *spun_sf);
```
**PK_SPUN_sf_t：**
```c
struct PK_SPUN_sf_s {
    PK_CURVE_t    curve;   // 母线曲线（line/circle/ellipse/bcurve）
    PK_AXIS1_sf_t axis;    // 旋转轴
};
```
**参数化：**
```
C = curve, P = axis.location, A = axis.axis
Z(u) = P + ((C(u) - P)·A)·A     (轴上投影)
R(u) = C(u) - Z(u)               (径向分量)
S(u, v) = Z(u) + R(u)·cos(v) + (A × R(u))·sin(v)
    u = curve 参数范围, v = [0, 2π]
```
**注意：** 母线可以触碰旋转轴端点，但不能穿越。

#### 3.3.8 PK_SWEPT — 拉伸面 (Linear Extrusion Surface)
```c
PK_ERROR_code_t PK_SWEPT_ask(PK_SWEPT_t swept, PK_SWEPT_sf_t *swept_sf);
```
**PK_SWEPT_sf_t：**
```c
struct PK_SWEPT_sf_s {
    PK_CURVE_t    curve;       // 截面曲线（line/circle/ellipse/bcurve）
    PK_VECTOR1_t  direction;   // 拉伸方向
};
```
**参数化：**
```
C = curve, D = direction
S(u, v) = C(u) + v·D
    u = curve 参数范围, v = (-∞, +∞)
```

#### 3.3.9 PK_OFFSET — 偏移曲面
```c
PK_ERROR_code_t PK_OFFSET_ask(PK_OFFSET_t offset, PK_OFFSET_sf_t *offset_sf);
```
**PK_OFFSET_sf_t：**
```c
struct PK_OFFSET_sf_s {
    PK_SURF_t  underlying_surface;   // 基础曲面
    double     offset_distance;      // 偏移距离 (!= 0)
};
```
偏移方向：正值沿 surface normal 方向，负值沿反方向。

---

## 四、方向关系规则（FD Section 14.7）

### 4.1 Face Normal 规则

**Solid body：**
- Face normal 必须指向**远离 solid region** 的方向
- Outer shell 的面：法线指向外部（void region）
- Inner shell 的面（空腔）：法线指向内部（void region）
- "Negative body"（内外翻转）的法线指向 solid，需要 negate 操作

**Sheet body：**
- Face normal 必须指向远离 body 的方向，且保持一致

### 4.2 Surface Natural Normal

各曲面的自然法线方向：

| 曲面 | 自然法线方向 |
|------|-----------|
| Plane | 沿 axis (法向) |
| Cylinder | 远离轴线 |
| Cone | 远离轴线 |
| Sphere | 远离球心 |
| Torus | 远离主轴圆 |
| 参数化面 | dU × dV 方向 |

### 4.3 Face Orientation Flag

`PK_FACE_ask_oriented_surf()` 返回的 `orientation` 标志：
- `PK_LOGICAL_true` → face normal 平行于 surface normal
- `PK_LOGICAL_false` → face normal 反平行于 surface normal

要获取某点处的 face normal，需要调用 `PK_FACE_ask_oriented_surf()` 和 `PK_SURF_eval_with_normal()` 并根据 orientation 标志调整。

### 4.4 Loop / Fin / Edge 方向

- **Loop 前进方向**：从 face normal 上方俯视，face 在 loop 前进方向的**左侧**
- **Fin 方向** = Loop 方向（fin 在 loop 中 nose-to-tail 排列）
- **Edge 方向**与 Fin 方向的关系由 `PK_FIN_is_positive()` 确定

### 4.5 Edge/Fin Orientation Flag

`PK_EDGE_ask_oriented_curve()` 和 `PK_FIN_ask_oriented_curve()` 返回的 `orientation`：
- `PK_LOGICAL_true` → edge/fin 方向平行于 curve 切线（参数递增方向）
- `PK_LOGICAL_false` → edge/fin 方向反平行于 curve 切线

---

## 五、内存管理

所有 PK 函数返回的数组（`faces`, `shells`, `loops`, `fins`, `edges`, `vertices`, `regions` 等）
都需要调用 `PK_MEMORY_free()` 释放：

```c
PK_FACE_t *faces = NULL;
int n_faces = 0;
PK_SHELL_ask_oriented_faces(shell, &n_faces, &faces, &orients);
// 使用 faces ...
PK_MEMORY_free(faces);
PK_MEMORY_free(orients);
```

`PK_BCURVE_sf_t` 和 `PK_BSURF_sf_t` 中的 `vertex`, `knot`, `knot_mult` 指针
也是 PK 分配的内存，使用后需释放。

---

## 六、常用查询函数快速索引

| 函数 | 用途 |
|------|------|
| `PK_BODY_ask_type` | 获取 body 类型 |
| `PK_BODY_ask_regions` | 获取 body 的所有 region |
| `PK_BODY_ask_topology` | 一次性获取完整拓扑 |
| `PK_REGION_ask_shells` | 获取 region 的 shell |
| `PK_REGION_is_solid` | 判断 region solid/void |
| `PK_SHELL_ask_type` | 获取 shell 类型 |
| `PK_SHELL_ask_oriented_faces` | 获取 shell 的面及朝向 |
| `PK_SHELL_ask_wireframe_edges` | 获取 wireframe edge |
| `PK_SHELL_ask_acorn_vertex` | 获取 acorn vertex |
| `PK_FACE_ask_loops` | 获取 face 的 loop |
| `PK_FACE_ask_oriented_surf` | 获取 face 的 surface 及朝向 |
| `PK_FACE_find_outer_loop` | 查找 outer loop |
| `PK_LOOP_ask_type` | 获取 loop 类型 |
| `PK_LOOP_ask_fins` | 获取 loop 中有序 fin |
| `PK_FIN_ask_edge` | 获取 fin 的 edge |
| `PK_FIN_is_positive` | fin 是否与 edge 同向 |
| `PK_FIN_ask_oriented_curve` | 获取 fin 的 curve 及方向 |
| `PK_EDGE_ask_vertices` | 获取 edge 的起止 vertex |
| `PK_EDGE_ask_curve` | 获取 edge 的 3D curve |
| `PK_EDGE_ask_oriented_curve` | 获取 edge 的 curve 及方向 |
| `PK_EDGE_ask_type` | 获取 edge 类型（多维分类） |
| `PK_VERTEX_ask_point` | 获取 vertex 的 point |
| `PK_POINT_ask` | 获取 point 坐标 |
| `PK_PLANE_ask` | 获取平面参数 |
| `PK_CYL_ask` | 获取圆柱面参数 |
| `PK_CONE_ask` | 获取锥面参数 |
| `PK_SPHERE_ask` | 获取球面参数 |
| `PK_TORUS_ask` | 获取环面参数 |
| `PK_LINE_ask` | 获取直线参数 |
| `PK_CIRCLE_ask` | 获取圆参数 |
| `PK_ELLIPSE_ask` | 获取椭圆参数 |
| `PK_BCURVE_ask` | 获取 B-spline 曲线参数 |
| `PK_BSURF_ask` | 获取 B-spline 曲面参数 |
| `PK_SPUN_ask` | 获取旋转面参数 |
| `PK_SWEPT_ask` | 获取拉伸面参数 |
| `PK_OFFSET_ask` | 获取偏移曲面参数 |
