# STEP 写出架构总结（基于 step_nio）

本文档总结了 `D:\workspace\step_nio\src\writer` 中 STEP 文件写出的架构和实现方式，作为 PKToSTEP 项目 Step2 的参考。

## 整体架构

### 核心类结构

```
STEPWriter (外观类)
    ├── OutputImpl (输出流管理)
    └── STEPWriter_Actor (核心写出逻辑)
        ├── TopoShapeMapping (形状去重映射)
        └── SBuilder (STEP 实体构建器)
```

### 写出流程

```
1. Init()
   ├── writeHead()           # ISO-10303-21 头部
   ├── writeApplication()    # APPLICATION_CONTEXT
   └── writeAxis()           # AXIS2_PLACEMENT_3D (全局坐标系)

2. WriteShape() / WriteShapes()
   └── addProductTree()      # 添加产品树到内部缓存
       └── correctTree()     # 修正缺失的 location 信息

3. Done()
   ├── done()                # 写出所有产品树和几何
   │   └── writeProductTree()
   │       ├── writePTreeImpl()        # 产品结构
   │       └── writeShapeRepresentation() # 几何表示
   ├── writeCDSR()           # CONTEXT_DEPENDENT_SHAPE_REPRESENTATION (装配关系)
   ├── writeGGGR()           # 全局单位和精度
   ├── writeMDGPR()          # 机械设计相关
   ├── writeGroups()         # 组
   ├── writeLayers()         # 图层
   └── writeFoot()           # ENDSEC
```

## 关键组件详解

### 1. STEPWriter (外观类)

**职责**：
- 提供简洁的用户接口
- 管理输出流（文件或 ostream）
- 协调 Actor 的工作流程

**主要接口**：
```cpp
STEPWriter(const char* filepath);
STEPWriter(std::ostream& os);

void Init();  // 写头部
bool WriteShape(const TopoShape& shape);
bool WriteShape(const std::shared_ptr<STEPProduct>& product);
bool WriteShapes(const std::vector<std::shared_ptr<STEPProduct>>& products);
bool Done();  // 完成写出

void SetOutputUnit(STEPLengthUnit unit);
void SetOptions(const STEPOptions& options);
void AddGroup(const STEPGroup& group);
void AddLayer(const STEPLayer& layer);
```

**支持的产品类型**：
- `STEPProduct` - 基本产品（形状 + 名称）
- `STEPStyledProduct` - 带样式的产品（颜色、材质）
- `STEPLabelProduct` - 基于 Label 的产品（属性、装配）

---

### 2. STEPWriter_Actor (核心逻辑)

**职责**：
- 实现 STEP 实体的写出逻辑
- 管理实体编号（LabelNumber）
- 处理形状去重和复用
- 构建装配树结构

**关键数据结构**：

#### 产品树缓存
```cpp
std::vector<PUnion> treenodes;  // 待写出的产品树
std::map<std::shared_ptr<void>, std::pair<int, int>> productMapper;  // 产品去重
```

#### 装配关系记录
```cpp
struct CDSRBundle {
    Frame3 axis;              // 相对变换
    int relating_sr;          // 父级 shape_representation
    int related_sr;           // 子级 shape_representation
    int relating_pd;          // 父级 product_definition
    int related_pd;           // 子级 product_definition
    AssemblyUsageOccurrence auo;  // 装配使用关系
};
std::vector<CDSRBundle> cdsrRecords;
```

#### 形状复用映射
```cpp
TopoShapeMapping mapper;  // Vertex/Edge 去重
std::vector<std::pair<TopoShape, int>> shapeRepLabels;     // ABSR (solid)
std::vector<std::pair<TopoShape, int>> msbShapeRepLabels;  // MSB (manifold_solid_brep)
std::vector<std::pair<TopoShape, int>> bwvShapeRepLabels;  // BWV (brep_with_voids)
std::vector<std::pair<TopoShape, int>> gsShapeRepLabels;   // GS (geometric_set)
std::vector<std::pair<TopoShape, int>> sbsmShapeRepLabels; // SBSM (shell_based_surface_model)
```

---

### 3. 产品结构写出

#### 产品树层次结构

```
PRODUCT
  └─ PRODUCT_DEFINITION_FORMATION
      └─ PRODUCT_DEFINITION
          └─ PRODUCT_DEFINITION_SHAPE
              └─ SHAPE_DEFINITION_REPRESENTATION
                  └─ SHAPE_REPRESENTATION (ABSR/MSSR/GBWSR)
                      └─ representation_item (几何)
```

#### 装配关系表达

```
NEXT_ASSEMBLY_USAGE_OCCURRENCE (NAUO)
  ├─ relating: 父级 PRODUCT_DEFINITION
  └─ related:  子级 PRODUCT_DEFINITION

CONTEXT_DEPENDENT_SHAPE_REPRESENTATION (CDSR)
  ├─ relating: 父级 SHAPE_REPRESENTATION
  ├─ related:  子级 SHAPE_REPRESENTATION
  └─ ITEM_DEFINED_TRANSFORMATION (相对变换)
```

---

### 4. 几何表示写出

#### Shape Representation 类型

| 类型 | STEP 实体 | 用途 | 适用形状 |
|------|----------|------|---------|
| **ABSR** | ADVANCED_BREP_SHAPE_REPRESENTATION | 实体模型 | Solid, CompSolid |
| **MSSR** | MANIFOLD_SURFACE_SHAPE_REPRESENTATION | 曲面模型 | Shell, Face |
| **GBWSR** | GEOMETRICALLY_BOUNDED_WIREFRAME_SHAPE_REPRESENTATION | 线框模型 | Wire, Edge |
| **CGR** | CONSTRUCTIVE_GEOMETRY_REPRESENTATION | 构造几何 | 特殊情况 |

#### Representation Item 类型

**实体表示**：
- `MANIFOLD_SOLID_BREP` - 单一实体（一个外壳）
- `BREP_WITH_VOIDS` - 带空腔的实体（外壳 + 内壳）
- `SHELL_BASED_SURFACE_MODEL` - 基于壳的曲面模型

**拓扑元素**：
- `CLOSED_SHELL` / `OPEN_SHELL`
- `ADVANCED_FACE`
- `FACE_BOUND` (outer/inner)
- `EDGE_LOOP`
- `ORIENTED_EDGE`
- `EDGE_CURVE`
- `VERTEX_POINT`

**几何元素**：
- Surface: `PLANE`, `CYLINDRICAL_SURFACE`, `CONICAL_SURFACE`, `SPHERICAL_SURFACE`, `TOROIDAL_SURFACE`, `B_SPLINE_SURFACE_WITH_KNOTS`, `SURFACE_OF_LINEAR_EXTRUSION`, `SURFACE_OF_REVOLUTION`, `OFFSET_SURFACE`
- Curve: `LINE`, `CIRCLE`, `ELLIPSE`, `PARABOLA`, `HYPERBOLA`, `B_SPLINE_CURVE_WITH_KNOTS`, `TRIMMED_CURVE`, `OFFSET_CURVE`, `COMPOSITE_CURVE`
- Point: `CARTESIAN_POINT`

---

### 5. 形状去重机制（TopoShapeMapping）

**目的**：避免重复写出相同的 Vertex 和 Edge，减小文件大小。

**机制**：
```cpp
class TopoShapeMapping {
    std::unordered_map<TopoShape, int> vertexMap;
    std::unordered_map<TopoShape, std::pair<int, int>> edgeMap;  // (forward, reversed)

    void Fill(const TopoShape& shape);  // 预扫描收集所有 vertex/edge
    int VertexNum(const TopoShape& vertex, int expectedNum);  // 返回已有编号或新编号
    int EdgeNum(const TopoShape& edge, int expectedNum);      // 考虑方向
};
```

**使用流程**：
1. `Fill(shape)` - 遍历形状，收集所有 vertex/edge
2. 写出时调用 `VertexNum()` / `EdgeNum()`
3. 如果已写出，返回已有编号；否则返回新编号并记录

**注意**：
- Face、Shell、Compound 不复用（每次都写新的）
- Edge 区分方向（forward/reversed 分别记录）

---

### 6. SBuilder (STEP 实体构建器)

**职责**：构建符合 STEP 语法的实体字符串。

**示例用法**：
```cpp
SBuilder sb;
sb.wrSimEnt(id, StepType(StepEntityKeyword::STEP_Product))
  .wrStr(sid)
  .wrStr(name)
  .wrStr(desc)
  .wrEntArr(products);
ost << sb;  // 输出: #123=PRODUCT('id','name','desc',(#1,#2));
```

**主要方法**：
- `wrSimEnt(id, type)` - 开始一个简单实体
- `wrStr(s)` - 写字符串 `'s'`
- `wrEnt(id)` - 写实体引用 `#id`
- `wrEntArr(ids)` - 写实体数组 `(#1,#2,#3)`
- `wrOpt()` - 写可选值 `$`
- `wrReal(v)` - 写实数
- `wrInt(i)` - 写整数
- `wrBool(b)` - 写布尔值 `.T.` / `.F.`

---

## 写出策略

### 1. 两阶段写出

**阶段 1：收集（addProductTree）**
- 用户调用 `WriteShape()` 时，产品树被添加到 `treenodes` 缓存
- 不立即写出，允许批量处理

**阶段 2：写出（Done）**
- 遍历 `treenodes`，依次写出每个产品树
- 写出产品结构（PRODUCT → PRODUCT_DEFINITION）
- 写出几何表示（SHAPE_REPRESENTATION → 拓扑 → 几何）
- 最后写出装配关系（CDSR）

### 2. 形状分解策略

```cpp
void shapeDecomposeFallback(const TopoShape& shape,
                            std::vector<TopoShape>& absr,   // 实体
                            std::vector<TopoShape>& mssr,   // 曲面
                            std::vector<TopoShape>& gbwsr); // 线框
```

**规则**：
- Solid/CompSolid → ABSR
- Shell/Face → MSSR
- Wire/Edge → GBWSR
- Vertex → GBWSR (作为 VERTEX_POINT)

### 3. 装配树遍历

**递归遍历**：
```cpp
void writeProductTree(std::ostream& ost, const PUnion& pTree, ShapeMapAttr& label_map) {
    // 1. 写出当前产品
    auto [pd_id, sr_id] = writePTreeImpl(ost, pTree, label_map);

    // 2. 递归写出子产品
    for (auto& child : pTree.children) {
        auto [child_pd, child_sr] = writeProductTree(ost, child, label_map);

        // 3. 记录装配关系
        CDSRBundle bundle;
        bundle.relating_pd = pd_id;
        bundle.related_pd = child_pd;
        bundle.relating_sr = sr_id;
        bundle.related_sr = child_sr;
        bundle.axis = child.transform;
        cdsrRecords.push_back(bundle);
    }
}
```

---

## STEP 文件结构

### 完整文件结构

```
ISO-10303-21;
HEADER;
FILE_DESCRIPTION(('STEP AP214'),'2;1');
FILE_NAME('output.step','2024-03-06T10:00:00',(''),(''),'','','');
FILE_SCHEMA(('AUTOMOTIVE_DESIGN'));
ENDSEC;

DATA;
/* 全局上下文 */
#1=APPLICATION_CONTEXT('automotive_design');
#2=AXIS2_PLACEMENT_3D('',#3,#4,#5);
#3=CARTESIAN_POINT('',(0.,0.,0.));
#4=DIRECTION('',(0.,0.,1.));
#5=DIRECTION('',(1.,0.,0.));

/* 产品结构 */
#10=PRODUCT('','Part1','',());
#11=PRODUCT_DEFINITION_FORMATION('','',#10);
#12=PRODUCT_DEFINITION('','',#11,#13);
#13=PRODUCT_DEFINITION_CONTEXT('',#1,'');
#14=PRODUCT_DEFINITION_SHAPE('','',#12);
#15=SHAPE_DEFINITION_REPRESENTATION(#14,#16);

/* 几何表示 */
#16=ADVANCED_BREP_SHAPE_REPRESENTATION('',(#17,#2),#100);
#17=MANIFOLD_SOLID_BREP('',#18);
#18=CLOSED_SHELL('',(#19));
#19=ADVANCED_FACE('',(#20),#30,.T.);
#20=FACE_OUTER_BOUND('',#21,.T.);
#21=EDGE_LOOP('',(#22));
#22=ORIENTED_EDGE('',*,*,#23,.T.);
#23=EDGE_CURVE('',#24,#25,#26,.T.);
#24=VERTEX_POINT('',#27);
#25=VERTEX_POINT('',#28);
#26=LINE('',#27,#29);
#27=CARTESIAN_POINT('',(0.,0.,0.));
#28=CARTESIAN_POINT('',(1.,0.,0.));
#29=VECTOR('',#4,1.);
#30=PLANE('',#2);

/* 装配关系 */
#200=NEXT_ASSEMBLY_USAGE_OCCURRENCE('','','',#12,#112,'');
#201=CONTEXT_DEPENDENT_SHAPE_REPRESENTATION(#202,#203);
#202=REPRESENTATION_RELATIONSHIP('','',#16,#116);
#203=ITEM_DEFINED_TRANSFORMATION('','',#2,#204);
#204=AXIS2_PLACEMENT_3D('',#205,#4,#5);
#205=CARTESIAN_POINT('',(10.,0.,0.));

/* 全局单位和精度 */
#100=GEOMETRIC_REPRESENTATION_CONTEXT(3);
#101=GLOBAL_UNCERTAINTY_ASSIGNED_CONTEXT((#102));
#102=UNCERTAINTY_MEASURE_WITH_UNIT(...);
#103=GLOBAL_UNIT_ASSIGNED_CONTEXT((#104,#105,#106));
#104=(LENGTH_UNIT()NAMED_UNIT(*)SI_UNIT(.MILLI.,.METRE.));
#105=(NAMED_UNIT(*)PLANE_ANGLE_UNIT()SI_UNIT($,.RADIAN.));
#106=(NAMED_UNIT(*)SI_UNIT($,.STERADIAN.)SOLID_ANGLE_UNIT());

ENDSEC;
END-ISO-10303-21;
```

---

## 关键设计模式

### 1. 外观模式（Facade）
- `STEPWriter` 隐藏 `STEPWriter_Actor` 的复杂性
- 提供简洁的用户接口

### 2. 建造者模式（Builder）
- `SBuilder` 逐步构建 STEP 实体字符串
- 链式调用，清晰易读

### 3. 享元模式（Flyweight）
- `TopoShapeMapping` 复用 Vertex/Edge
- 减少内存和文件大小

### 4. 模板方法模式
- `WriteShapePrivate<T>()` 模板函数
- 统一处理不同产品类型

---

## 与 Xchg 格式的对应关系

| Xchg 实体 | STEP 实体 | 说明 |
|-----------|----------|------|
| `Xchg_Body` | `MANIFOLD_SOLID_BREP` / `BREP_WITH_VOIDS` | 实体 |
| `Xchg_Shell` | `CLOSED_SHELL` / `OPEN_SHELL` | 壳 |
| `Xchg_Face` | `ADVANCED_FACE` | 面 |
| `Xchg_Loop` | `FACE_BOUND` + `EDGE_LOOP` | 环 |
| `Xchg_Coedge` | `ORIENTED_EDGE` | 有向边 |
| `Xchg_Edge` | `EDGE_CURVE` | 边 |
| `Xchg_Vertex` | `VERTEX_POINT` | 顶点 |
| `Xchg_Surface` | `PLANE` / `CYLINDRICAL_SURFACE` / ... | 曲面 |
| `Xchg_Curve` | `LINE` / `CIRCLE` / `B_SPLINE_CURVE` / ... | 曲线 |
| `Xchg_Point` | `CARTESIAN_POINT` | 点 |

---

## 总结

**step_nio 的 STEP 写出架构特点**：

1. **清晰的分层**：外观类 → Actor → Builder
2. **两阶段写出**：收集 → 批量写出
3. **形状复用**：Vertex/Edge 去重
4. **装配支持**：完整的产品树和 CDSR
5. **可扩展性**：支持多种产品类型和属性

**适用于 PKToSTEP 的经验**：
- 采用类似的两阶段写出策略
- 实现 Xchg → STEP 实体的映射
- 复用 Vertex/Edge 以减小文件
- 支持装配结构的 CDSR 写出
- 使用 Builder 模式简化 STEP 语法生成

---

## 代码库位置

### step_nio STEP 写出实现
- **位置**：`D:\workspace\step_nio\src\writer\`
- **关键文件**：
  - `STEPWriter.cpp` - 外观类实现
  - `STEPWriter_Actor.hpp/cpp` - 核心写出逻辑
  - `TopoShapeMapping.hpp/cpp` - 形状去重
  - `SBuilder.hpp` - STEP 实体构建器
  - `StepEntityKeywordMap.cpp` - STEP 实体关键字映射

### Xchg 格式接口代码库
- **位置**：`D:\workspace\exchange_base\include\`
- **主要模块**：
  - `topology/` - 拓扑类（Xchg_Body, Xchg_Shell, Xchg_Face, Xchg_Loop, Xchg_Edge, Xchg_Vertex）
  - `geom/surface/` - 曲面类（Xchg_PlaneSurface, Xchg_CylindricalSurface, Xchg_NurbsSurface 等）
  - `geom/curve/` - 曲线类（Xchg_Line, Xchg_Circle, Xchg_NurbsCurve, Xchg_Polyline 等）
  - `geom/` - 基础几何类（Xchg_Point, Xchg_Trsf 等）
  - `xchg_main_doc.hpp` - 主文档类（装配体容器）

### Conan 依赖库位置
- **AMCAXExchangeBase**：`~/.conan2/p/amcax*/p/include/`
- **Parasolid**：`~/.conan2/p/psker*/p/include/parasolid_kernel.h`
- **AMXT_STP**：`~/.conan2/p/amcax*/p/include/`
