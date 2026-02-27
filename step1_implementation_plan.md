# Step1 实施方案：PK_BODY 到 AMCAXExchangeBase 中间格式转换

## 目标
将 Parasolid 的 PK_BODY 数据结构转换为 AMCAXExchangeBase 定义的中间数据格式，包括拓扑关系和几何数据的完整转换。

## 核心任务分解

### 1. 拓扑结构提取与转换

#### 1.1 Parasolid 拓扑层次结构
```
PK_BODY
  └─ PK_REGION (可选，用于实体)
      └─ PK_SHELL
          └─ PK_FACE
              └─ PK_LOOP
                  └─ PK_FIN (Face-Edge连接)
                      └─ PK_EDGE
                          └─ PK_VERTEX
```

#### 1.2 AMCAXExchangeBase 拓扑层次结构
```
Xchg_Body
  └─ Xchg_Lump
      └─ Xchg_Shell (outer/inner)
          └─ Xchg_Face
              └─ Xchg_Loop (outer/inner)
                  └─ Xchg_Coedge
                      └─ Xchg_Edge
                          └─ Xchg_Vertex
```

#### 1.3 关键 Parasolid API 函数

**Body 级别：**
- `PK_BODY_ask_topology()` - 获取 body 的拓扑信息
- `PK_BODY_ask_type()` - 获取 body 类型 (solid/sheet/wire/acorn)

**Shell 级别：**
- `PK_SHELL_ask_faces()` - 获取 shell 中的所有 face
- `PK_SHELL_ask_oriented()` - 判断 shell 是否为 outer shell

**Face 级别：**
- `PK_FACE_ask_loops()` - 获取 face 的所有 loop
- `PK_FACE_ask_surf()` - 获取 face 的几何曲面
- `PK_FACE_ask_oriented()` - 获取 face 的方向

**Loop 级别：**
- `PK_LOOP_ask_fins()` - 获取 loop 中的所有 fin (edge使用)
- `PK_LOOP_ask_type()` - 判断 loop 类型 (outer/inner/vertex)

**Edge 级别：**
- `PK_EDGE_ask_vertices()` - 获取 edge 的起点和终点 vertex
- `PK_EDGE_ask_curve()` - 获取 edge 的几何曲线
- `PK_FIN_ask_edge_sense()` - 获取 edge 在 loop 中的方向

**Vertex 级别：**
- `PK_VERTEX_ask_point()` - 获取 vertex 的几何点坐标

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
2. 实现 ID 映射机制 (PK tag → Xchg_ID)
3. 实现错误处理和日志记录

#### 阶段 2：拓扑转换
1. **Body 转换**
   - 创建 `Xchg_Body`
   - 设置 body 类型 (根据 PK_BODY_type_t 映射到 Xchg_BodyType)
   - 设置容差

2. **Lump/Shell 转换**
   - 遍历 PK_BODY 的 regions 和 shells
   - 创建对应的 `Xchg_Lump` 和 `Xchg_Shell`
   - 区分 outer shell 和 inner shell

3. **Face 转换**
   - 遍历每个 shell 的 faces
   - 创建 `Xchg_Face`
   - 设置 face 方向
   - 关联几何曲面

4. **Loop 转换**
   - 遍历每个 face 的 loops
   - 创建 `Xchg_Loop`
   - 区分 outer loop 和 inner loop
   - 处理 vertex loop (退化情况)

5. **Edge/Coedge 转换**
   - 遍历每个 loop 的 fins
   - 创建 `Xchg_Edge` 和 `Xchg_Coedge`
   - 设置 edge 方向 (same_sense)
   - 关联几何曲线
   - 建立 coedge 之间的连接关系

6. **Vertex 转换**
   - 创建 `Xchg_Vertex`
   - 设置顶点坐标
   - 建立 edge-vertex 关系

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
    // ID 映射表
    std::map<PK_ENTITY_t, Xchg_ID> pk_to_xchg_id_map_;
    std::map<Xchg_ID, PK_ENTITY_t> xchg_to_pk_id_map_;

    // 当前转换的 body
    Xchg_BodyPtr current_body_;

public:
    // 主转换函数
    Xchg_BodyPtr ConvertBody(PK_BODY_t pk_body);

private:
    // 拓扑转换
    void ConvertTopology(PK_BODY_t pk_body);
    Xchg_LumpPtr ConvertRegion(PK_REGION_t pk_region);
    Xchg_ShellPtr ConvertShell(PK_SHELL_t pk_shell);
    Xchg_FacePtr ConvertFace(PK_FACE_t pk_face);
    Xchg_LoopPtr ConvertLoop(PK_LOOP_t pk_loop);
    Xchg_EdgePtr ConvertEdge(PK_EDGE_t pk_edge);
    Xchg_VertexPtr ConvertVertex(PK_VERTEX_t pk_vertex);

    // 几何转换
    Xchg_SurfacePtr ConvertSurface(PK_SURF_t pk_surf);
    Xchg_CurvePtr ConvertCurve(PK_CURVE_t pk_curve);
    Xchg_PointPtr ConvertPoint(PK_POINT_t pk_point);

    // 具体几何类型转换
    Xchg_PlaneSurfacePtr ConvertPlaneSurface(PK_SURF_t pk_surf);
    Xchg_CylindricalSurfacePtr ConvertCylindricalSurface(PK_SURF_t pk_surf);
    Xchg_NurbsSurfacePtr ConvertNurbsSurface(PK_SURF_t pk_surf);
    // ... 其他曲面类型

    Xchg_LinePtr ConvertLine(PK_CURVE_t pk_curve);
    Xchg_EllipsePtr ConvertCircle(PK_CURVE_t pk_curve);
    Xchg_NurbsCurvePtr ConvertNurbsCurve(PK_CURVE_t pk_curve);
    // ... 其他曲线类型

    // 辅助函数
    Xchg_ID RegisterEntity(PK_ENTITY_t pk_entity, Xchg_ID xchg_id);
    Xchg_ID GetXchgID(PK_ENTITY_t pk_entity);
};
```

### 5. 注意事项

1. **方向一致性**
   - Parasolid 和 AMCAXExchangeBase 的方向约定可能不同
   - 需要仔细处理 face normal、edge sense、loop orientation

2. **退化情况**
   - 退化边 (degenerated edge)
   - 退化面 (degenerated face)
   - Vertex loop

3. **容差处理**
   - 保持 Parasolid 的容差信息
   - 传递到 AMCAXExchangeBase

4. **内存管理**
   - AMCAXExchangeBase 使用智能指针 (SmartPtr)
   - 注意 Parasolid 内存的释放

5. **错误处理**
   - Parasolid API 返回错误码
   - AMCAXExchangeBase 使用 Xchg_ErrorStatus
   - 需要统一的错误处理机制

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


