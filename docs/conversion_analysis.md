# STEP ↔ Xchg ↔ PK_BODY 转换规则分析

## 概述

本文档总结了 STEP 文件与 Parasolid PK_BODY 之间通过 Xchg 中间格式转换的关键规则，基于对 amxt_stp 和 exchange_base 库源码的深入分析。

---

## 一、转换链路

```
STEP 文件 ──读取──> Xchg 格式 ──转换──> PK_BODY
         (amxt_stp)          (exchange_base)
```

**逆向转换（用户实现）**：
```
PK_BODY ──转换──> Xchg 格式 ──导出──> STEP 文件
       (pk_to_xchg_converter)  (未实现)
```

---

## 二、STEP → Xchg 转换规则

### 2.1 拓扑层次映射

| STEP 实体 | Xchg 实体 | 说明 |
|-----------|-----------|------|
| MANIFOLD_SOLID_BREP | Xchg_Body + Xchg_Lump | Body 包含一个或多个 Lump |
| CLOSED_SHELL (outer) | Xchg_Shell (IsOuter=true) | 通过 AddOuterShell 添加 |
| CLOSED_SHELL (inner) | Xchg_Shell (IsOuter=false) | 通过 AddInnerShell 添加 |
| OPEN_SHELL | Xchg_Shell | 通过 AddOpenShell 添加 |
| ADVANCED_FACE | Xchg_Face | 包含几何和拓扑信息 |
| FACE_OUTER_BOUND | Xchg_Loop (IsOuter=true) | 外环 |
| FACE_BOUND | Xchg_Loop (IsOuter=false) | 内环（孔） |
| ORIENTED_EDGE | Xchg_Coedge | Edge 在 Loop 中的有向使用 |
| EDGE_CURVE | Xchg_Edge | 包含起点、终点和曲线几何 |
| VERTEX_POINT | Xchg_Vertex | 包含点几何 |

### 2.2 方向标志处理（核心算法）

**XOR 运算规则**：
```cpp
final_orientation = !(_parent_orientation ^ current_sense)
```

**应用场景**：

1. **Shell 中的 Face 方向**：
```cpp
// Translate_Shell.cpp:103
bool final_orientation = !(_orientation ^ sense);
// _orientation: Shell 的方向（outer=true）
// sense: Face 的 same_sense 标志
```

2. **Face_Bound 的 Loop 方向**：
```cpp
// Translate_Loop.cpp:180
bool orientation = !(fb.v_orientation ^ _sense);
// fb.v_orientation: STEP 中 Face_Bound 的 orientation
// _sense: 从上层传递的 sense
```

3. **Oriented_Edge 的 Coedge 方向**：
```cpp
// Translate_Coedge.cpp:89
_coedge->SetOrientation(oe.v_orientation);
// 直接使用 STEP 的 orientation
```

### 2.3 几何转换

**Surface 转换示例（PLANE）**：
```cpp
// GeomBuilder.cpp:1796-1831
Xchg_pnt Origin;      // 从 AXIS2_PLACEMENT_3D 提取
Xchg_dir Normal;      // Z 轴方向
Xchg_dir UDirection;  // X 轴方向（可选）

planePtr = Xchg_PlaneSurface::Create(Origin, Normal, UDirection);
```

**Curve 转换示例（LINE）**：
```cpp
Xchg_pnt Origin;      // 起点
Xchg_dir Direction;   // 方向向量

linePtr = Xchg_Line::Create(Origin, Direction);
```

---

## 三、Xchg → PK_BODY 转换规则

### 3.1 Region 创建规则（关键映射）

**Body → Regions 的映射结构**：
```
PK_BODY
├── InfVoidRegion (infinite void, 必须存在，第一个 region)
│   ├── FrontFaceShell (outer closed shells)
│   ├── OpenFaceShell (open shells)
│   └── WireShell (wire/edge shells)
├── SolidRegion (每个 Lump 对应一个)
│   └── BackFaceShell (outer + inner shells)
└── VoidRegion (每个 inner closed shell 对应一个)
    └── FrontFaceShell
```

**Shell 分类规则**：
```cpp
// xchg_body_convertto_pk.cpp:349-400

if (shell->IsClosed()) {
    if (shell->IsOuter()) {
        // Outer closed shell
        if (lump_index == 0) {
            // 第一个 lump 的 outer shell
            InfVoidRegion.FrontFaceShell.AddFaces(shell);
            SolidRegion.BackFaceShell.AddFaces(shell);
        } else {
            // 其他 lump 的 outer shell
            SolidRegion.BackFaceShell.AddFaces(shell);
        }
    } else {
        // Inner closed shell (空腔)
        VoidRegion.Create();
        VoidRegion.FrontFaceShell.AddFaces(shell);
        SolidRegion.BackFaceShell.AddFaces(shell);
    }
} else {
    // Open shell
    InfVoidRegion.OpenFaceShell.AddFaces(shell);
}
```

### 3.2 方向（Sense）转换规则

**Region 级别**：
```cpp
// BodyAddChildren, line 1222-1245
Body → InfVoidRegion:  PK_TOPOL_sense_negative_c
Body → SolidRegion:    PK_TOPOL_sense_positive_c
Body → VoidRegion:     PK_TOPOL_sense_negative_c
```

**Shell 级别**：
```cpp
// FrontFaceShellAddChildren, line 1409-1428
FrontFaceShell → Face: PK_TOPOL_sense_positive_c  (始终正向)
BackFaceShell → Face:  PK_TOPOL_sense_negative_c  (始终反向)
OpenFaceShell → Face:  根据 Xchg_Shell::AddFace 的 orientation 参数
```

**Face 级别**：
```cpp
// FaceAddChildren, line 1544
Face → Loop: PK_TOPOL_sense_none_c  (无方向)
```

**Loop 级别**：
```cpp
// LoopAddChildren, line 1577
Loop → Fin: PK_TOPOL_sense_none_c  (无方向)
```

**Fin（Coedge）级别（核心算法）**：
```cpp
// FinAddChildren, line 1584-1622
Xchg_bool coedgeOrientation = inCoedge->GetOrientation();
Xchg_bool loopOrientation;   // 从 loop->GetCoedge(i, coedge, loopOrientation) 获取

PK_TOPOL_sense_t sense =
    !(coedgeOrientation ^ loopOrientation) ?
        PK_TOPOL_sense_positive_c :
        PK_TOPOL_sense_negative_c;

// XOR 逻辑：
// - 两个方向相同（都是 true 或都是 false）→ positive
// - 两个方向不同 → negative
```

**Edge 级别**：
```cpp
// EdgeAddChildren, line 1639-1640
Edge → Vertex: PK_TOPOL_sense_none_c  (无方向)
```

### 3.3 Face 方向计算

**Face 的 sense 由两个因素决定**：
```cpp
// xchg_body_convertto_pk.cpp:2391-2430

// 1. Face 在 Shell 中的 orientation（从 Shell::AddFace 获取）
Xchg_bool face_orientation;
shell->GetFace(i, face, face_orientation);

// 2. Shell 的类型（Front/Back/Open）
ShellType shell_type;

// 最终 sense 计算：
PK_TOPOL_sense_t face_sense =
    !(face_orientation ^ (shell_type == BackFaceShell)) ?
        PK_TOPOL_sense_positive_c :
        PK_TOPOL_sense_negative_c;

// 逻辑：
// - FrontFaceShell: face_orientation 直接决定 sense
// - BackFaceShell: face_orientation 反转后决定 sense
```

---

## 四、PK_BODY → Xchg 转换规则（逆向）

### 4.1 拓扑层次映射

| PK 实体 | Xchg 实体 | 转换规则 |
|---------|-----------|----------|
| PK_BODY | Xchg_Body | 1:1 映射 |
| PK_REGION (solid) | Xchg_Lump | 跳过 infinite void region |
| PK_REGION (void) | 不转换 | Xchg 通过 inner shell 表达空腔 |
| PK_SHELL | Xchg_Shell | 根据 region 类型判断 outer/inner |
| PK_FACE | Xchg_Face | 1:1 映射，需要去重 |
| PK_LOOP | Xchg_Loop | 1:1 映射 |
| PK_FIN | Xchg_Coedge | Fin → Coedge 是关键映射 |
| PK_EDGE | Xchg_Edge | 1:1 映射，需要去重 |
| PK_VERTEX | Xchg_Vertex | 1:1 映射，需要去重 |

### 4.2 Region 处理规则

**关键判断逻辑**：
```cpp
// pk_to_xchg_converter.cpp:93-120

PK_BODY_type_t body_type;
PK_BODY_ask_type(pk_body, &body_type);

if (body_type == PK_BODY_type_solid_c ||
    body_type == PK_BODY_type_general_c) {

    // Solid/General body:
    // - 跳过 regions[0]（infinite void region）
    // - regions[1..n] 中：
    //   - solid region → Lump + OuterShell
    //   - bounded void region → Lump + InnerShell

} else if (body_type == PK_BODY_type_sheet_c) {
    // Sheet body:
    // - 只处理 regions[0]（infinite void region）
    // - shell → Lump + OpenShell
}
```

### 4.3 方向映射规则

**Face 方向计算**：
```cpp
// pk_to_xchg_converter.cpp:299-322

// 1. 获取 Face 在 Shell 中的方向
PK_LOGICAL_t* orients;  // Face 在 Shell 中的方向数组
PK_SHELL_ask_faces(pk_shell, &n_faces, &faces, &orients);

// 2. 获取 Face 自身的方向
PK_LOGICAL_t face_orient;
PK_FACE_ask_oriented_surf(faces[i], &surf, &face_orient);

// 3. 计算最终方向（XOR 逻辑）
Xchg_bool xchg_orient = (orients[i] != face_orient);

// 4. 添加到 Shell
shell->AddFace(xchg_face, xchg_orient);
```

**Coedge 方向计算**：
```cpp
// pk_to_xchg_converter.cpp:487-520

// 1. 获取 Fin 的 sense
PK_TOPOL_sense_t fin_sense;
PK_FIN_ask_sense(pk_fin, &fin_sense);

// 2. 转换为 Xchg orientation
Xchg_bool coedge_orient = (fin_sense == PK_TOPOL_sense_positive_c);

// 3. 设置 Coedge
coedge->SetOrientation(coedge_orient);

// 4. 添加到 Loop（第二个参数是 loop 中的方向）
loop->AddCoedge(coedge, true);  // 通常为 true
```

---

## 五、关键差异点总结

### 5.1 Region 处理

**官方实现（X → PK）**：
- 为每个 inner closed shell 创建独立的 VoidRegion
- 同时在 SolidRegion 和 VoidRegion 中添加该 shell（不同方向）
- InfVoidRegion 包含所有 outer shells 的正面

**用户实现（PK → X）**：
- 跳过 infinite void region
- 只处理 solid region 和 bounded void region
- 不创建 VoidRegion，通过 inner shell 表达空腔

**结论**：用户实现是正确的，因为 Xchg 格式不需要显式的 void region

### 5.2 方向映射

**官方实现（X → PK）**：
```cpp
// Fin sense = XOR(coedge.orientation, loop.orientation)
sense = !(coedgeOrientation ^ loopOrientation) ? positive : negative;
```

**用户实现（PK → X）**：
```cpp
// Coedge orientation = (fin_sense == positive)
coedge_orient = (fin_sense == PK_TOPOL_sense_positive_c);
// Loop orientation = true（固定）
loop->AddCoedge(coedge, true);
```

**问题**：用户实现中 loop orientation 固定为 true，可能导致方向不一致

### 5.3 Face 方向

**官方实现（X → PK）**：
```cpp
// Face sense 由 shell 类型和 face orientation 共同决定
face_sense = !(face_orientation ^ (shell_type == BackFaceShell));
```

**用户实现（PK → X）**：
```cpp
// Face orientation = XOR(shell_orient, face_orient)
xchg_orient = (orients[i] != face_orient);
```

**问题**：用户实现没有考虑 shell 类型（Front/Back），可能导致方向错误

---

## 六、测试结果分析

从测试输出看到的主要差异：

### 6.1 Face 方向系统性反转
```
REF (STEP→X): shellOrient = 0,1,0,1,0,1
OURS (PK→X):  shellOrient = 1,0,1,0,1,0
```

**原因**：
- REF 路径：STEP → X 时，outer shell 的 face 通过 XOR 计算得到正确方向
- OURS 路径：PK → X 时，没有考虑 shell 类型（Front/Back），导致方向反转

### 6.2 Loop 方向不一致
```
REF: orient = 0,0,0,1,0,1
OURS: orient = 1,1,1,1,1,1
```

**原因**：
- REF 路径：Loop orientation 从 STEP 的 Face_Bound.orientation 通过 XOR 计算
- OURS 路径：Loop orientation 可能没有正确设置

### 6.3 Coedge inLoop 标志差异
```
REF: inLoop = 0,0,0,0 (大多为 0)
OURS: inLoop = 1,1,1,1 (大多为 1)
```

**原因**：
- `inLoop` 是 loop->AddCoedge 的第二个参数
- 用户实现固定为 true，而应该根据实际情况计算

---

## 七、建议修正方案

### 7.1 修正 Face 方向映射

```cpp
// 当前实现（错误）
Xchg_bool xchg_orient = (orients[i] != face_orient);

// 建议修正
// 1. 判断 shell 类型（通过 region 类型）
bool is_back_shell = (region_type == solid_region);

// 2. 计算方向
Xchg_bool xchg_orient = !(orients[i] ^ face_orient ^ is_back_shell);
```

### 7.2 修正 Loop 方向

```cpp
// 需要从 PK_LOOP 获取正确的 orientation
// 或者根据 Loop 在 Face 中的位置判断（outer/inner）
Xchg_bool loop_orient = loop->IsOuter() ? true : false;
loop->SetOrientation(loop_orient);
```

### 7.3 修正 Coedge 方向

```cpp
// 当前实现
loop->AddCoedge(coedge, true);  // 固定为 true

// 建议修正
// 需要计算正确的 inLoop 标志
// 可能需要从 PK_FIN 或 PK_LOOP 获取更多信息
Xchg_bool in_loop = /* 计算逻辑 */;
loop->AddCoedge(coedge, in_loop);
```

---

## 八、总结

1. **Region 处理**：用户实现基本正确，跳过 void region 是合理的
2. **方向映射**：这是主要问题所在，需要考虑 shell 类型和多层 XOR 运算
3. **拓扑结构**：基本正确，但方向标志需要修正
4. **几何转换**：从测试看几何数据是正确的，只是顺序和方向不同

**核心问题**：方向映射规则不完整，特别是：
- Face 方向没有考虑 shell 类型
- Loop 方向可能没有正确设置
- Coedge 的 inLoop 标志固定为 true

**下一步**：需要详细对比用户的 pk_to_xchg_converter.cpp 实现，找出具体的代码问题。
