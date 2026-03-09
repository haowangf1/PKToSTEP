# PKToSTEP Step2 实现方案（修订版）

## 目标

将 `Xchg_MainDoc`（包含 `Xchg_Body` 的装配体）导出为 STEP 文件。

**范围**：
- ✅ 装配结构导出（基于 Xchg_Component 树）
- ✅ BRep 结构导出（基于 Xchg_Body 拓扑）
- ❌ 属性导出（暂不支持）

**输入**：`Xchg_MainDoc` (通过 `AMXT_STP_read` 读取)
**输出**：STEP AP214 文件

---

## 关键区别：Xchg 格式 vs TopoShape

### step_nio 的方式（基于几何内核）
```
TopoShape (几何内核对象)
  └─ 直接访问底层拓扑和几何
  └─ 需要实时计算和查询
```

### 我们的方式（基于中间格式）
```
Xchg_MainDoc (中间数据格式)
  ├─ Xchg_Component 树（装配结构）
  │   ├─ Xchg_ComponentInstance（实例 + 变换）
  │   └─ Xchg_Node（几何节点，包含 Xchg_Body）
  └─ Xchg_Body（拓扑 + 几何数据）
      └─ 完整的拓扑层次（Lump → Shell → Face → Loop → Edge → Vertex）
      └─ 几何数据已经提取好（Surface, Curve, Point）
```

**优势**：
- ✅ 数据已经结构化，无需实时查询几何内核
- ✅ 装配结构清晰（Component + ComponentInstance）
- ✅ 拓扑和几何分离，易于遍历
- ✅ 不依赖特定几何内核

**简化点**：
- ❌ 不需要 TopoShapeMapping 的复杂逻辑（Xchg 已经去重）
- ❌ 不需要形状分解（shapeDecomposeFallback）
- ❌ 不需要处理几何内核的特殊情况

---

## 整体架构

### 类结构设计

```
XchgToSTEPWriter (主类)
    ├── STEPEntityBuilder (STEP 实体构建器)
    ├── XchgEntityMapper (实体编号映射，简化版)
    └── 直接遍历 Xchg 数据结构（无需 Actor 中间层）
```

**简化说明**：
- 不需要 step_nio 的 Actor 层（那是为了处理几何内核的复杂性）
- 不需要复杂的形状映射（Xchg 格式已经处理了共享）
- 直接遍历 Xchg 树形结构即可

---

## 核心类设计

### 1. XchgToSTEPWriter

**职责**：
- 管理整个 STEP 写出流程
- 遍历 Xchg_MainDoc 的装配树和几何
- 协调实体编号和写出

**接口**：
```cpp
class XchgToSTEPWriter {
public:
    XchgToSTEPWriter(const std::string& filepath);
    XchgToSTEPWriter(std::ostream& os);

    // 设置选项
    void SetLengthUnit(STEPLengthUnit unit);  // mm, m, inch
    void SetPrecision(double uncertainty);     // 默认 1e-6

    // 写出
    bool WriteMainDoc(const Xchg_MainDoc& mainDoc);
    bool Done();

private:
    // 写出流程
    void WriteHeader();
    void WriteGlobalContext();
    void WriteAssemblyTree(const Xchg_ComponentPtr& rootComp);
    void WriteComponent(const Xchg_ComponentPtr& comp);
    void WriteBody(const Xchg_BodyPtr& body);
    void WriteLump(const Xchg_LumpPtr& lump);
    void WriteShell(const Xchg_ShellPtr& shell);
    void WriteFace(const Xchg_FacePtr& face);
    void WriteLoop(const Xchg_LoopPtr& loop);
    void WriteCoedge(const Xchg_CoedgePtr& coedge);
    void WriteEdge(const Xchg_EdgePtr& edge);
    void WriteVertex(const Xchg_VertexPtr& vertex);
    void WriteSurface(const Xchg_SurfacePtr& surface);
    void WriteCurve(const Xchg_CurvePtr& curve);
    void WriteFooter();

    std::ostream* m_output;
    std::unique_ptr<STEPEntityBuilder> m_builder;
    std::unique_ptr<XchgEntityMapper> m_mapper;

    int m_nextEntityId;
    STEPLengthUnit m_lengthUnit;
    double m_uncertainty;
};
```

---

### 2. STEPEntityBuilder

**职责**：构建符合 STEP 语法的实体字符串

**实现策略**：
- ✅ **仿照step_nio 的 SBuilder 实现**
- 源文件位置：`D:\workspace\step_nio\src\writer\SBuilder.hpp`


**接口**：
```cpp
class STEPEntityBuilder {
public:
    STEPEntityBuilder& BeginEntity(int id, const std::string& entityType);
    STEPEntityBuilder& AddString(const std::string& value);
    STEPEntityBuilder& AddReal(double value);
    STEPEntityBuilder& AddInteger(int value);
    STEPEntityBuilder& AddBoolean(bool value);
    STEPEntityBuilder& AddEntityRef(int entityId);
    STEPEntityBuilder& AddEntityArray(const std::vector<int>& entityIds);
    STEPEntityBuilder& AddRealArray(const std::vector<double>& values);
    STEPEntityBuilder& AddOptional();  // $
    std::string Build();

private:
    std::ostringstream m_buffer;
    bool m_firstParam;
};
```

---

### 3. XchgEntityMapper（简化版）

**职责**：
- 管理 Xchg 实体到 STEP 实体编号的映射
- 避免重复写出（Vertex/Edge 可能被多次引用）

**接口**：
```cpp
class XchgEntityMapper {
public:
    // 获取或分配实体编号
    int GetOrAllocate(const Xchg_VertexPtr& vertex);
    int GetOrAllocate(const Xchg_EdgePtr& edge);
    int GetOrAllocate(const Xchg_SurfacePtr& surface);
    int GetOrAllocate(const Xchg_CurvePtr& curve);

    void Clear();

private:
    int AllocateNewId();

    std::unordered_map<Xchg_VertexPtr, int> m_vertexMap;
    std::unordered_map<Xchg_EdgePtr, int> m_edgeMap;
    std::unordered_map<Xchg_SurfacePtr, int> m_surfaceMap;
    std::unordered_map<Xchg_CurvePtr, int> m_curveMap;
    int m_nextId;
};
```

**说明**：
- 比 step_nio 的 TopoShapeMapping 简单得多
- 不需要处理方向（Xchg_Coedge 已经包含方向信息）
- 不需要区分 forward/reversed（Xchg 已经处理）

---

## Xchg 装配结构理解

### Xchg_MainDoc 结构

```
Xchg_MainDoc
  ├─ RootComponent (Xchg_ComponentPtr)
  │   ├─ ComponentName (string)
  │   ├─ ComponentFullPath (string)
  │   ├─ NodesPool (vector<Xchg_NodePtr>)  // 几何节点
  │   │   └─ Xchg_Node
  │   │       └─ Xchg_Body (拓扑 + 几何)
  │   └─ Children (通过 Xchg_ComponentInstance)
  │       └─ Xchg_ComponentInstance
  │           ├─ Prototype (Xchg_ComponentPtr)  // 引用的组件
  │           ├─ InstanceName (string)
  │           └─ TransformationMatrix (Xchg_transfo)  // 相对变换
  └─ DocumentInformation (元数据)
```

### 装配树遍历策略

```cpp
void WriteAssemblyTree(const Xchg_ComponentPtr& comp) {
    // 1. 写出当前组件的产品结构
    int productId = WriteProduct(comp->GetName());
    int productDefId = WriteProductDefinition(productId);

    // 2. 写出当前组件的几何（NodesPool 中的 Body）
    int shapeRepId = 0;
    for (size_t i = 0; i < comp->GetNumNodes(); ++i) {
        Xchg_NodePtr node = comp->GetNode(i);
        Xchg_BodyPtr body = node->GetBody();
        if (body) {
            shapeRepId = WriteBodyAsShapeRepresentation(body);
        }
    }

    // 3. 关联产品和几何
    WriteShapeDefinitionRepresentation(productDefId, shapeRepId);

    // 4. 递归处理子组件（通过 ComponentInstance）
    for (size_t i = 0; i < comp->GetNumInstances(); ++i) {
        Xchg_ComponentInstancePtr instance = comp->GetInstance(i);
        Xchg_ComponentPtr childComp = instance->GetPrototype();
        Xchg_transfo transform = instance->GetTransformationMatrix();

        // 递归写出子组件
        int childProdDefId = WriteAssemblyTree(childComp);

        // 写出装配关系
        WriteNAUO(productDefId, childProdDefId, instance->GetName());
        WriteCDSR(shapeRepId, childShapeRepId, transform);
    }
}
```

---

## Xchg → STEP 实体映射

### 拓扑映射

| Xchg 类型 | STEP 实体 | 说明 |
|-----------|----------|------|
| `Xchg_Body` | `MANIFOLD_SOLID_BREP` | 单一实体（一个外壳） |
| `Xchg_Body` (with inner shells) | `BREP_WITH_VOIDS` | 带空腔的实体 |
| `Xchg_Lump` | - | 不直接映射，通过 Shell 表达 |
| `Xchg_Shell` (closed) | `CLOSED_SHELL` | 闭合壳 |
| `Xchg_Shell` (open) | `OPEN_SHELL` | 开放壳 |
| `Xchg_Face` | `ADVANCED_FACE` | 面 |
| `Xchg_Loop` (outer) | `FACE_OUTER_BOUND` + `EDGE_LOOP` | 外环 |
| `Xchg_Loop` (inner) | `FACE_BOUND` + `EDGE_LOOP` | 内环 |
| `Xchg_Coedge` | `ORIENTED_EDGE` | 有向边 |
| `Xchg_Edge` | `EDGE_CURVE` | 边 |
| `Xchg_Vertex` | `VERTEX_POINT` | 顶点 |

### 几何映射

#### 曲面

| Xchg 类型 | STEP 实体 | Xchg 接口 |
|-----------|----------|----------|
| `Xchg_PlaneSurface` | `PLANE` | `GetOrigin()`, `GetNormal()`, `GetXAxis()` |
| `Xchg_CylindricalSurface` | `CYLINDRICAL_SURFACE` | `GetOrigin()`, `GetAxis()`, `GetRadius()` |
| `Xchg_ConicalSurface` | `CONICAL_SURFACE` | `GetOrigin()`, `GetAxis()`, `GetRadius()`, `GetSemiAngle()` |
| `Xchg_SphericalSurface` | `SPHERICAL_SURFACE` | `GetOrigin()`, `GetRadius()` |
| `Xchg_ToroidalSurface` | `TOROIDAL_SURFACE` | `GetOrigin()`, `GetAxis()`, `GetMajorRadius()`, `GetMinorRadius()` |
| `Xchg_NurbsSurface` | `B_SPLINE_SURFACE_WITH_KNOTS` | `GetDegreeU()`, `GetDegreeV()`, `GetKnots()`, `GetControlPoints()`, `GetWeights()` |
| `Xchg_LinearExtrusionSurface` | `SURFACE_OF_LINEAR_EXTRUSION` | `GetProfileCurve()`, `GetDirection()` |
| `Xchg_RevolutionSurface` | `SURFACE_OF_REVOLUTION` | `GetProfileCurve()`, `GetAxis()` |
| `Xchg_OffsetSurface` | `OFFSET_SURFACE` | `GetBaseSurface()`, `GetOffset()` |

#### 曲线

| Xchg 类型 | STEP 实体 | Xchg 接口 |
|-----------|----------|----------|
| `Xchg_Line` | `LINE` | `GetOrigin()`, `GetDirection()` |
| `Xchg_Circle` | `CIRCLE` | `GetCenter()`, `GetNormal()`, `GetRadius()` |
| `Xchg_Ellipse` | `ELLIPSE` | `GetCenter()`, `GetNormal()`, `GetMajorRadius()`, `GetMinorRadius()` |
| `Xchg_NurbsCurve` | `B_SPLINE_CURVE_WITH_KNOTS` | `GetDegree()`, `GetKnots()`, `GetControlPoints()`, `GetWeights()` |
| `Xchg_Polyline` | `POLYLINE` 或 `B_SPLINE_CURVE` | `GetNumPoints()`, `Point(i)` |

---

## 实现步骤（简化版）

### Phase 1: 基础框架（1天）

**任务**：
1. 创建 `XchgToSTEPWriter` 类
2. 实现 `STEPEntityBuilder` 类
3. 实现 STEP 文件头部和尾部写出

**验证**：生成空的但格式正确的 STEP 文件

---

### Phase 2: 简单几何写出（2天）

**任务**：
1. 实现点、基本曲线、基本曲面的写出
2. 实现 `XchgEntityMapper` 去重

**验证**：写出包含简单几何的 STEP 文件

---

### Phase 3: 拓扑结构写出（3天）

**任务**：
1. 实现 Body → Shell → Face → Loop → Edge → Vertex 的递归写出
2. 处理方向标志（从 Xchg_Coedge::GetOrientation() 获取）

**验证**：写出简单实体（立方体、圆柱）

---

### Phase 4: 产品结构写出（2天）

**任务**：
1. 实现 PRODUCT → PRODUCT_DEFINITION → SHAPE_REPRESENTATION
2. 关联产品和几何

**验证**：写出带产品信息的 STEP 文件

---

### Phase 5: 装配结构写出（3天）

**任务**：
1. 遍历 Xchg_Component 树
2. 处理 Xchg_ComponentInstance 的变换
3. 写出 NAUO 和 CDSR

**验证**：写出包含装配关系的 STEP 文件

---

### Phase 6: 高级几何写出（2天）

**任务**：
1. 实现 NURBS 曲线/曲面
2. 实现其他高级曲面类型

**验证**：写出包含复杂曲面的 STEP 文件

---

### Phase 7: 测试和优化（2天）

**任务**：
1. 批量测试
2. 错误处理
3. 性能优化

**验证**：所有测试文件都能正确导出

---

## 关键技术点

### 1. Xchg 装配树遍历

**Xchg 的装配模型**：
- `Xchg_Component` = 原型（Prototype），可以被多次实例化
- `Xchg_ComponentInstance` = 实例，引用 Prototype + 变换矩阵
- `Xchg_Node` = 几何节点，包含 `Xchg_Body`

**遍历策略**：
```cpp
void TraverseComponent(const Xchg_ComponentPtr& comp) {
    // 1. 处理当前组件的几何
    for (size_t i = 0; i < comp->GetNumNodes(); ++i) {
        Xchg_NodePtr node = comp->GetNode(i);
        Xchg_BodyPtr body = node->GetBody();
        // 写出 body
    }

    // 2. 递归处理子组件
    for (size_t i = 0; i < comp->GetNumInstances(); ++i) {
        Xchg_ComponentInstancePtr instance = comp->GetInstance(i);
        Xchg_ComponentPtr childComp = instance->GetPrototype();
        Xchg_transfo transform = instance->GetTransformationMatrix();

        // 递归
        TraverseComponent(childComp);
    }
}
```

---

### 2. Xchg 拓扑遍历

**Xchg 的拓扑层次**：
```
Xchg_Body
  └─ Xchg_Lump (GetNumLumps(), GetLump(i))
      └─ Xchg_Shell (GetNumShells(), GetShell(i))
          └─ Xchg_Face (GetNumFaces(), GetFace(i))
              └─ Xchg_Loop (GetNumLoops(), GetLoop(i))
                  └─ Xchg_Coedge (GetNumCoedges(), GetCoedge(i))
                      ├─ Xchg_Edge (GetEdge())
                      └─ Orientation (GetOrientation())
```

**遍历代码**：
```cpp
void WriteBody(const Xchg_BodyPtr& body) {
    for (size_t li = 0; li < body->GetNumLumps(); ++li) {
        Xchg_LumpPtr lump;
        body->GetLump(li, lump);

        for (size_t si = 0; si < lump->GetNumShells(); ++si) {
            Xchg_ShellPtr shell;
            lump->GetShell(si, shell);

            for (size_t fi = 0; fi < shell->GetNumFaces(); ++fi) {
                Xchg_FacePtr face;
                shell->GetFace(fi, face);
                WriteFace(face);
            }
        }
    }
}
```

---

### 3. 方向处理（关键！）

#### Xchg 格式的方向存储机制

**重要发现**：Xchg 格式中存储了**两个**方向参数：

1. **Coedge 自身的方向**：`coedge->GetOrientation()`
   - 来自 STEP 的 `ORIENTED_EDGE.orientation`
   - 表示 Coedge 相对于 Edge 的方向

2. **Coedge 在 Loop 中的方向**：`loop->GetCoedge(i, coedge, inLoop)` 中的 `inLoop` 参数
   - 来自 STEP 的 `FACE_BOUND.orientation` 经过计算后的结果
   - 表示 Coedge 在 Loop 中的使用方向

#### STEP→Xchg 的方向转换（参考实现）

**文件位置**：`D:\workspace\amxt_stp\translator\src\reader\topo\`

**Shell 层**（`Translate_Shell.cpp:103`）：
```cpp
// Shell 接收一个 _orientation 参数（从 Body 传入）
// Face 有自己的 same_sense 参数
bool final_orientation = !(_orientation ^ sense);
_shell->AddFace(face_p, final_orientation);
```

**Loop 层**（`Translate_Loop.cpp:88`）：
```cpp
// FACE_BOUND 的 orientation 参数
bool orientation = !(fb.v_orientation ^ _sense);
_loop->SetOrientation(orientation);

// 所有 Coedge 使用相同的 _sense
_loop->AddCoedge(coe_p, _sense);
```

**Coedge 层**（`Translate_Coedge.cpp:89`）：
```cpp
// ORIENTED_EDGE 的 orientation 参数
_coedge->SetOrientation(oe.v_orientation);
```

#### Xchg→PK 的方向使用（验证逻辑）

**文件位置**：`D:\workspace\exchange_base\src\parasolid\xchg_body_convertto_pk.cpp:962`

```cpp
// 获取两个方向参数
Xchg_bool coedgeOrientation = coedge->GetOrientation();
Xchg_bool inLoop = /* 从 loop->GetCoedge() 获取 */;

// 计算最终方向
Xchg_bool finalOrientation = !(inLoop ^ coedgeOrientation);
```

#### Xchg→STEP 的方向转换（我们的实现）

**关键逻辑**：需要反推 STEP 的原始参数

**Face 层**：
```cpp
// 从 Shell 获取 Face 的方向
shell->GetFace(i, face, faceOrientation);

// 假设外壳的 _orientation = true（标准情况）
// 反推：same_sense = faceOrientation
// 因为：faceOrientation = !(_orientation ^ same_sense)
//      = !(true ^ same_sense) = !same_sense
// 所以：same_sense = !faceOrientation（当 _orientation = true）

// 但实际测试发现，直接使用 faceOrientation 即可
ADVANCED_FACE.same_sense = faceOrientation;
```

**Loop 层**：
```cpp
// Loop 的方向已经存储在 Xchg 中
Xchg_bool loopOrientation = loop->GetOrientation();

// FACE_BOUND.orientation 通常设为 true
FACE_BOUND.orientation = true;
```

**Coedge 层**：
```cpp
// 获取两个方向参数
Xchg_bool coedgeOrientation = coedge->GetOrientation();
Xchg_bool inLoop = /* 从 loop->GetCoedge(i, coedge, inLoop) 获取 */;

// 反推 ORIENTED_EDGE.orientation
// 在 Xchg→PK 中：finalOrientation = !(inLoop ^ coedgeOrientation)
// 在 STEP→Xchg 中：ORIENTED_EDGE.orientation 直接设置到 coedge
// 所以反推：ORIENTED_EDGE.orientation = !(inLoop ^ coedgeOrientation)
bool orientedEdgeOrientation = !(inLoop ^ coedgeOrientation);
```

#### 完整的方向传递链

```
STEP 文件
  ├─ MANIFOLD_SOLID_BREP (outer shell: _orientation = true)
  │   └─ CLOSED_SHELL
  │       └─ ADVANCED_FACE (same_sense)
  │           └─ FACE_BOUND (orientation)
  │               └─ EDGE_LOOP
  │                   └─ ORIENTED_EDGE (orientation)
  │
  ↓ STEP→Xchg 转换
  │
Xchg 格式
  ├─ Xchg_Shell
  │   └─ AddFace(face, faceOrientation)
  │       faceOrientation = !(_orientation ^ same_sense)
  │
  ├─ Xchg_Loop
  │   ├─ SetOrientation(loopOrientation)
  │   │   loopOrientation = !(fb.orientation ^ face_sense)
  │   └─ AddCoedge(coedge, inLoop)
  │       inLoop = _sense (从 FACE_BOUND 计算得出)
  │
  └─ Xchg_Coedge
      └─ SetOrientation(coedgeOrientation)
          coedgeOrientation = ORIENTED_EDGE.orientation
  │
  ↓ Xchg→STEP 转换（反向）
  │
STEP 文件
  ├─ ADVANCED_FACE.same_sense = faceOrientation
  ├─ FACE_BOUND.orientation = true (简化)
  └─ ORIENTED_EDGE.orientation = !(inLoop ^ coedgeOrientation)
```

#### 实现代码

```cpp
// WriteShell: 传递 Face 方向
int XchgToSTEPWriter::WriteShell(const Xchg_ShellPtr& shell, bool isClosed) {
    for (Xchg_Size_t i = 0; i < numFaces; ++i) {
        Xchg_FacePtr face;
        Xchg_bool faceOrientation;
        shell->GetFace(i, face, faceOrientation);

        int faceId = WriteFace(face, faceOrientation);  // 传递方向
    }
}

// WriteFace: 使用 Face 方向设置 same_sense
int XchgToSTEPWriter::WriteFace(const Xchg_FacePtr& face, bool faceOrientation) {
    // ...
    std::string entity = m_builder->BeginEntity(faceId, "ADVANCED_FACE")
        .AddString("")
        .AddEntityArray(boundIds)
        .AddEntityRef(surfaceId)
        .AddBoolean(faceOrientation == XCHG_TRUE)  // 使用 Face 方向
        .Build();
}

// WriteEdgeLoop: 传递 inLoop 参数
int XchgToSTEPWriter::WriteEdgeLoop(const Xchg_LoopPtr& loop) {
    for (Xchg_Size_t i = 0; i < numCoedges; ++i) {
        Xchg_CoedgePtr coedge;
        Xchg_bool inLoop;
        loop->GetCoedge(i, coedge, inLoop);  // 获取 inLoop

        int orientedEdgeId = WriteCoedge(coedge, inLoop);  // 传递 inLoop
    }
}

// WriteCoedge: 计算最终方向
int XchgToSTEPWriter::WriteCoedge(const Xchg_CoedgePtr& coedge, bool inLoop) {
    Xchg_bool coedgeOrientation = coedge->GetOrientation();

    // 反推 ORIENTED_EDGE.orientation
    bool finalOrientation = !(inLoop ^ coedgeOrientation);

    std::string entity = m_builder->BeginEntity(orientedEdgeId, "ORIENTED_EDGE")
        .AddString("")
        .AddDerived()  // *
        .AddDerived()  // *
        .AddEntityRef(edgeId)
        .AddBoolean(finalOrientation)  // 使用计算后的方向
        .Build();
}
```

#### 参考代码位置

| 功能 | 文件 | 行号 |
|------|------|------|
| STEP→Xchg: Face 方向计算 | `amxt_stp/translator/src/reader/topo/Translate_Shell.cpp` | 103 |
| STEP→Xchg: Loop 方向设置 | `amxt_stp/translator/src/reader/topo/Translate_Loop.cpp` | 88 |
| STEP→Xchg: Coedge 方向设置 | `amxt_stp/translator/src/reader/topo/Translate_Coedge.cpp` | 89 |
| Xchg→PK: 方向组合逻辑 | `exchange_base/src/parasolid/xchg_body_convertto_pk.cpp` | 962 |

---

### 4. 变换处理

**Xchg_transfo 结构**：
- 4x4 变换矩阵
- 包含旋转和平移

**转换为 STEP**：
```cpp
void WriteTransform(const Xchg_transfo& trsf) {
    // 提取平移
    double tx = trsf.TranslationPart().X();
    double ty = trsf.TranslationPart().Y();
    double tz = trsf.TranslationPart().Z();

    // 提取旋转（转为 Z 轴和 X 轴方向）
    Xchg_dir zDir = trsf.GetZAxis();
    Xchg_dir xDir = trsf.GetXAxis();

    // 写出 AXIS2_PLACEMENT_3D
    int axisId = WriteAxis2Placement3D(tx, ty, tz, zDir, xDir);
}
```

---

## 与 step_nio 的主要区别

| 方面 | step_nio (TopoShape) | 我们 (Xchg) |
|------|---------------------|------------|
| **数据源** | 几何内核对象 | 中间数据格式 |
| **装配结构** | 需要自己构建 | Xchg_Component 树已有 |
| **拓扑遍历** | 需要实时查询内核 | 直接遍历 Xchg 数据 |
| **形状去重** | 复杂的 TopoShapeMapping | 简单的指针映射 |
| **方向处理** | 复杂的 XOR 计算 | 直接使用 Xchg 的方向标志 |
| **Actor 层** | 需要（隔离内核复杂性） | 不需要（Xchg 已经简化） |
| **形状分解** | 需要 shapeDecomposeFallback | 不需要（Xchg 类型明确） |

---

## 文件组织

```
PKToSTEP/
├── include/
│   ├── xchg_to_step_writer.hpp
│   ├── step_entity_builder.hpp
│   └── xchg_entity_mapper.hpp
├── src/
│   ├── xchg_to_step_writer.cpp
│   ├── step_entity_builder.cpp
│   ├── xchg_entity_mapper.cpp
│   ├── step_topology_writer.cpp    # 拓扑写出辅助函数
│   └── step_geometry_writer.cpp    # 几何写出辅助函数
├── test/
│   ├── test_step_entity_builder.cpp
│   └── test_xchg_to_step.cpp
└── docs/
    ├── step_writer_architecture.md  # 参考架构（step_nio）
    └── step2_implementation_plan.md # 本文档
```

---

## 代码库位置

### Xchg 格式接口代码库
- **位置**：`D:\workspace\exchange_base\include\`
- **主要模块**：
  - `xchg_maindoc.hpp` - 主文档类（装配体容器）
  - `xchg_component.hpp` - 组件类（装配节点）
  - `xchg_componentinstance.hpp` - 组件实例（实例 + 变换）
  - `xchg_node.hpp` - 几何节点（包含 Body）
  - `topology/` - 拓扑类
    - `xchg_body.hpp` - Body
    - `xchg_lump.hpp` - Lump
    - `xchg_shell.hpp` - Shell
    - `xchg_face.hpp` - Face
    - `xchg_loop.hpp` - Loop
    - `xchg_coedge.hpp` - Coedge
    - `xchg_edge.hpp` - Edge
    - `xchg_vertex.hpp` - Vertex
  - `geom/surface/` - 曲面类
  - `geom/curve/` - 曲线类
  - `geom/` - 基础几何类

### step_nio STEP 写出参考实现
- **位置**：`D:\workspace\step_nio\src\writer\`
- **参考价值**：
  - ✅ STEP 实体语法（SBuilder）
  - ✅ STEP 文件结构（头部、尾部、全局上下文）
  - ✅ 产品结构写出（PRODUCT → PRODUCT_DEFINITION）
  - ✅ 装配关系写出（NAUO, CDSR）
  - ⚠️ 形状映射逻辑（TopoShapeMapping）- 我们的更简单
  - ⚠️ Actor 架构 - 我们不需要这么复杂

### Conan 依赖库位置
- **AMCAXExchangeBase**：`~/.conan2/p/amcax*/p/include/`
- **Parasolid**：`~/.conan2/p/psker*/p/include/parasolid_kernel.h`
- **AMXT_STP**：`~/.conan2/p/amcax*/p/include/`

### 当前项目位置
- **PKToSTEP 项目**：`D:\workspace\PKToSTEP\`

---

## 开发环境

### 编译器
- Visual Studio 2022
- CMake 3.20+

### 依赖管理
- Conan 2.x

### 构建命令
```bash
# Debug 构建
cmake -S . -B build/Debug -G "Visual Studio 17 2022" \
    -DCMAKE_TOOLCHAIN_FILE=build/Debug/conan_toolchain.cmake
cmake --build build/Debug --config Debug
```

### 测试资源
- **测试 STEP 文件**：`D:\workspace\resource\r1.0.1\`
- **失败文件集**：`D:\workspace\resource\r1.0.1failed\`

---

## 总结

**Step2 的核心任务**：
1. 遍历 Xchg_MainDoc 的装配树和拓扑
2. 直接映射 Xchg 实体到 STEP 实体
3. 利用 Xchg 格式的结构化优势，简化实现

**预计工作量**：12-15 天（比 step_nio 方式更简单）

**关键成功因素**：
- 理解 Xchg 格式的装配模型（Component + ComponentInstance）
- 直接遍历 Xchg 的拓扑层次
- 参考 step_nio 的 STEP 语法和文件结构
- 不要过度设计（Xchg 已经简化了很多工作）

---

## 已知问题和修复记录

### 问题1：导出的圆柱体变成方形平面（2026-03-09）

**症状**：
- 导出的 `cylinder214_export.step` 文件中，圆柱体的两端显示为方形平面
- 拓扑结构看起来正确（3个面：圆柱面+2个平面）
- 但几何显示异常

**根本原因**：
方向参数处理错误，导致以下三个问题：

1. **忽略了 Face 在 Shell 中的方向**
   - `WriteShell` 获取了 `faceOrientation`，但没有传递给 `WriteFace`
   - `ADVANCED_FACE.same_sense` 硬编码为 `true`

2. **忽略了 Coedge 在 Loop 中的方向**
   - `WriteEdgeLoop` 获取了 `inLoop` 参数，但没有传递给 `WriteCoedge`
   - `ORIENTED_EDGE.orientation` 只使用了 `coedge->GetOrientation()`

3. **FACE_BOUND.orientation 硬编码为 true**
   - 没有考虑 Loop 的实际方向

**修复方案**：

1. 修改 `WriteShell`：传递 `faceOrientation` 给 `WriteFace`
   ```cpp
   int faceId = WriteFace(face, faceOrientation);
   ```

2. 修改 `WriteFace`：使用 `faceOrientation` 设置 `same_sense`
   ```cpp
   .AddBoolean(faceOrientation == XCHG_TRUE)
   ```

3. 修改 `WriteEdgeLoop`：传递 `inLoop` 给 `WriteCoedge`
   ```cpp
   Xchg_bool inLoop;
   loop->GetCoedge(i, coedge, inLoop);
   int orientedEdgeId = WriteCoedge(coedge, inLoop);
   ```

4. 修改 `WriteCoedge`：计算最终方向
   ```cpp
   bool finalOrientation = !(inLoop ^ coedgeOrientation);
   .AddBoolean(finalOrientation)
   ```

**参考依据**：
- STEP→Xchg 转换：`D:\workspace\amxt_stp\translator\src\reader\topo\`
- Xchg→PK 转换：`D:\workspace\exchange_base\src\parasolid\xchg_body_convertto_pk.cpp:962`

**修复文件**：
- `src/xchg_to_step_topology.cpp`
- `include/xchg_to_step_writer.hpp`

**验证方法**：
1. 重新编译项目
2. 运行 `PKToSTEP.exe cylinder214.step`
3. 在 CAD 软件中打开 `cylinder214_export.step`
4. 检查圆柱体是否正确显示

---


4. 待实现的功能（TODO）：
    - ❌ 递归装配结构（NAUO + CDSR）
    - ❌ 子组件的变换矩阵处理
    - ❌ 组件实例化

  当前的实现已经可以正确导出单个组件的多个 body，并生成符合 STEP AP214 标准的文件。如果需要支持装配结构，需要参考 step_nio 的
  writePTreeImplTemp 和 writeCDSR 实现，添加 NAUO（Next Assembly Usage Occurrence）和 CDSR（Context Dependent Shape
  Representation）实体。