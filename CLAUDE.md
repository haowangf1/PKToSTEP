# CLAUDE.md

总是用中文回答

本文件为 Claude Code (claude.ai/code) 在此代码库中工作时提供指导。

## 开发规则

### Parasolid API 调用规范

**关键规则**：调用任何 PK API 前，必须先查阅头文件确认 API 存在和签名正确。

**PK 头文件位置**：
```
~/.conan2/p/psker*/p/include/parasolid_kernel.h
```

**查找方法**：
```bash
# 搜索 API 函数
grep -i "PK_CURVE_make" ~/.conan2/p/psker*/p/include/parasolid_kernel.h

# 查看函数签名和参数
grep -A 10 "PK_CURVE_make_approx" ~/.conan2/p/psker*/p/include/parasolid_kernel.h
```

**禁止**：
- ❌ 凭记忆或猜测编写 PK API 调用
- ❌ 编造不存在的 API 函数
- ❌ 假设 API 签名

**必须**：
- ✅ 先查头文件，确认 API 存在
- ✅ 复制正确的函数签名和参数结构
- ✅ 使用正确的选项宏初始化（如 `PK_CURVE_make_approx_o_m`）

## 项目概述

PKToSTEP 是一个 Parasolid PK_BODY 到 STEP 文件的转换器。它通过中间格式 Xchg (AMCAXExchangeBase) 将 Parasolid 几何模型转换为 STEP 格式。

**转换链路**：
```
PK_BODY → Xchg 格式 → STEP 文件
         (本项目)    (未来工作)
```

**当前实现**：PK_BODY → Xchg 转换，并与参考的 STEP→Xchg→PK 往返转换进行拓扑对比。

## 构建系统

### 快速构建（Debug）

使用 `/build-debug` skill 或手动运行：

```bash
# 检查依赖并在需要时安装
if [ ! -f "build/Debug/conan_toolchain.cmake" ]; then
    conan install . -of=build/Debug -s build_type=Debug --build=missing
fi

# 配置和构建
cmake -S . -B build/Debug -G "Visual Studio 17 2022" \
    -DCMAKE_TOOLCHAIN_FILE=build/Debug/conan_toolchain.cmake \
    -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
cmake --build build/Debug --config Debug
```

**输出**：`build/Debug/Debug/PKToSTEP.exe`

### 依赖项（Conan）

- `amcax-exchange-base/0.0.1.121` - Xchg 格式库
- `pskernel/36.0` - Parasolid 内核
- `amcax-amxt_stp/0.0.1.39` - STEP 读取器
- `gtest/1.14.0` - 测试框架

**重要**：首次构建后必须将 DLL 复制到 `build/Debug/Debug/`：
- `AMCAXExchangeBase.dll`
- `AMXT_STP.dll`
- `pskernel.dll`

`/build-debug` skill 会自动处理这些。

## 架构

### 核心转换器：`PKToXchgConverter`

**位置**：`include/pk_to_xchg_converter.hpp`，`src/pk_to_xchg_converter.cpp`

通过层次化遍历将 Parasolid 拓扑和几何转换为 Xchg 格式：

```
PK_BODY
  └─ PK_REGION (solid/void)
      └─ PK_SHELL
          └─ PK_FACE
              └─ PK_LOOP (outer/inner)
                  └─ PK_FIN (coedge)
                      └─ PK_EDGE
                          └─ PK_VERTEX
```

**关键方法**：
- `Convert()` - 入口点，将 PK_BODY 转换为 Xchg_Body
- `ConvertRegions()` - 处理 solid/void region，确定 shell 类型
- `ConvertShell()` - 转换 shell 并计算 face 方向
- `ConvertFace()` - 转换 face，将方向传递给 loop
- `ConvertLoop()` - 转换 loop，根据 face 设置方向
- `ConvertFin()` - 将 fin (coedge) 转换为 Xchg_Coedge
- `ConvertEdge()` - 转换 edge 并去重
- `ConvertVertex()` - 转换 vertex 并去重

### 关键方向逻辑

**Face 方向**（在 `ConvertShell` 中）：
```cpp
// BackFaceShell (solid region)：需要额外反转
xchg_orient = !(orients[i] ^ face_orient);

// FrontFaceShell (void region)：原始逻辑
xchg_orient = (orients[i] != face_orient);
```

**Loop 方向**（在 `ConvertLoop` 中）：
```cpp
// Loop 方向与 face 在 shell 中的方向一致
xchg_loop->SetOrientation(face_orientation ? XCHG_TRUE : XCHG_FALSE);
```

**Coedge inLoop**（在 `ConvertLoop` 中）：
```cpp
// 简化为 XCHG_TRUE 以确保 GetOrderedCoedges 成功
// 复杂的 XOR 逻辑会导致 X→PK 往返转换中的 -40 错误
xchg_loop->AddCoedge(xchg_coedge, XCHG_TRUE);
```

### 主程序流程

**位置**：`src/main.cpp`

1. 读取 STEP 文件 → Xchg_Body（参考）
2. 转换 Xchg_Body → PK_BODY
3. 转换 PK_BODY → Xchg_Body（我们的实现）
4. 使用 `XchgTopoCompare::Compare()` 比较拓扑
5. 往返测试：Xchg_Body → PK_BODY → 验证

### 拓扑比较

**位置**：`include/xchg_topo_compare.hpp`

比较两个 Xchg_Body 实例并报告以下差异：
- Face shellOrient（face 在 shell 中的方向）
- Loop orientation
- Coedge inLoop 标志
- 几何参数（surfaces、curves、vertices）

## 关键概念

### Xchg 格式方向规则

**来自 X→PK 转换**（`exchange_base/src/parasolid/xchg_body_convertto_pk.cpp`）：

```cpp
// GetOrderedCoedges 使用此规则确定边的方向：
finalOrientation = !(inLoop ^ coedgeOrientation);

// 如果 finalOrientation 错误，coedge 无法排列成
// 首尾相连的链，导致错误 -40 (XCHG_ERROR_UNDEFINED)
```

**关键**：`AddCoedge()` 中的 `inLoop` 参数必须正确设置，以确保 `GetOrderedCoedges` 能在 X→PK 转换期间将 coedge 排列成闭合 loop。

### Region 类型和 Shell 分类

- **SolidRegion**：材料区域 → BackFaceShell（面朝内）
- **InfVoidRegion**：无限空隙 → FrontFaceShell（面朝外）
- **VoidRegion**：空腔 → 需要特殊处理

### 去重

使用 `pk_edge_map_` 和 `pk_vertex_map_` 对 Edge 和 Vertex 进行去重，以确保共享的拓扑实体被重用。

## 文档

- `docs/step_to_x_to_pk_process.md` - STEP↔Xchg↔PK 转换规则的详细分析
- `docs/implementation_issues.md` - PK→Xchg 转换中的已知问题和解决方案
- `docs/icurve_conversion_issue.md` - icurve 转换问题记录（未解决）

## 测试

当前测试：往返验证，将 STEP→Xchg→PK→Xchg 与参考进行比较。

`resource/` 中的测试文件：
- `cube214.step` - 简单立方体（6 个面）
- `hollow_cube.step` - 带空腔的立方体（30 个面，内环）
- `cylinder214.step` - 圆柱几何


## 常见问题

### 错误 -40：GetOrderedCoedges 失败

**原因**：`AddCoedge()` 中的 `inLoop` 参数不正确，导致 coedge 的起点/终点顺序错误。

**解决方案**：对 `inLoop` 参数使用 `XCHG_TRUE`（简化方法，有效）。

### 缺少 DLL

**症状**："cannot open shared object file" 错误

**解决方案**：从 Conan 缓存复制 DLL 到 `build/Debug/Debug/`：
```bash
cp ~/.conan2/p/amcax*/p/bin/Debug/*.dll build/Debug/Debug/
cp ~/.conan2/p/psker*/p/bin/*.dll build/Debug/Debug/
```

### Face/Loop 方向不匹配

**原因**：`ConvertShell` 或 `ConvertLoop` 中的方向计算不正确。

**检查**：比较测试输出中的 `shellOrient` 和 `orient` 字段。REF 和 OURS 之间应该匹配。

## 可用 Skills

- `/build-debug` - 配置和构建 Debug 版本，自动管理依赖
- `/check-pk-api` - 从内部服务器查询 Parasolid API 文档

## 测试step资源文件地址
- "D:\workspace\resource\r1.0.1"
### Tolerant Edge 和 SPcurve 处理（重要）

**PK 中的 Tolerant Edge**：
- Tolerant edge 本身没有 3D curve（`PK_EDGE_ask_curve` 返回 null）
- 其几何信息存储在使用该 edge 的 fin 中，以 SPcurve（surface parameter curve）形式表达
- SPcurve 是 2D bcurve 嵌入在 surface 的参数空间 (u,v) 中

**转换策略**：
- 在 `ConvertFin()` 中检查 fin 是否有 curve（`PK_FIN_ask_oriented_curve`）
- 如果 fin 有 curve，说明对应的 edge 是 tolerant edge（没有 3D curve）
- 将 fin 的 SPcurve 转换为 3D basic curve，并添加到 Xchg edge 中
- 这样确保 Xchg edge 始终有几何信息

**实现位置**：`ConvertFin()` 方法

**关键逻辑**：
```cpp
// 在 ConvertFin 中
PK_CURVE_t fin_curve = PK_ENTITY_null;
PK_FIN_ask_oriented_curve(pk_fin, &fin_curve, &fin_orient);

if (fin_curve != PK_ENTITY_null) {
    // fin 有 curve，说明 edge 是 tolerant edge
    // 将 SPcurve 转换为 3D curve 并设置到 edge
    Xchg_CurvePtr curve_3d;
    ConvertCurve(fin_curve, &curve_3d);  // SPcurve → 3D curve
    if (curve_3d && xchg_edge)
        xchg_edge->SetGeom(curve_3d);
}
```

## Step2: STEP 文件导出
### 代码库位置**Xchg 格式接口**：`D:workspaceexchange_baseinclude`- `topology/` - 拓扑类（Body, Shell, Face, Loop, Edge, Vertex）- `geom/surface/` - 曲面类（Plane, Cylinder, Sphere, NURBS 等）- `geom/curve/` - 曲线类（Line, Circle, NURBS, Polyline 等）- `geom/` - 基础几何（Point, Trsf, Axis）- `xchg_main_doc.hpp` - 主文档类**step_nio 参考实现**：`D:workspacestep_niosrcwriter`- `STEPWriter.cpp` - 外观类- `STEPWriter_Actor.hpp/cpp` - 核心逻辑- `TopoShapeMapping.hpp/cpp` - 形状去重- `SBuilder.hpp` - STEP 实体构建器

### 参考文档

- `docs/step_writer_architecture.md` - STEP 写出架构总结（基于 step_nio）
- `docs/step2_implementation_plan.md` - Step2 实现方案

### 目标

将 `Xchg_MainDoc`（包含 `Xchg_Body` 的装配体）导出为 STEP 文件。

**范围**：
- ✅ 装配结构导出
- ✅ BRep 结构导出
- ❌ 属性导出（暂不支持）

### 核心架构

```
XchgToSTEPWriter (主类)
    ├── STEPEntityBuilder (STEP 实体构建器)
    ├── XchgShapeMapper (形状去重映射)
    └── AssemblyContext (装配上下文)
```

### 关键技术点

1. **实体编号管理**：从 1 开始递增，每个实体唯一编号
2. **形状去重**：Vertex/Edge 复用，减小文件大小
3. **方向处理**：正确映射 Xchg 和 STEP 的方向标志
4. **装配变换**：Xchg_Trsf → AXIS2_PLACEMENT_3D + ITEM_DEFINED_TRANSFORMATION
5. **单位和精度**：支持 mm/m/inch，默认精度 1e-6

### Xchg → STEP 映射

| Xchg 实体 | STEP 实体 |
|-----------|----------|
| `Xchg_Body` | `MANIFOLD_SOLID_BREP` / `BREP_WITH_VOIDS` |
| `Xchg_Shell` | `CLOSED_SHELL` / `OPEN_SHELL` |
| `Xchg_Face` | `ADVANCED_FACE` |
| `Xchg_Loop` | `FACE_BOUND` + `EDGE_LOOP` |
| `Xchg_Coedge` | `ORIENTED_EDGE` |
| `Xchg_Edge` | `EDGE_CURVE` |
| `Xchg_Vertex` | `VERTEX_POINT` |
| `Xchg_Surface` | `PLANE` / `CYLINDRICAL_SURFACE` / `B_SPLINE_SURFACE` / ... |
| `Xchg_Curve` | `LINE` / `CIRCLE` / `B_SPLINE_CURVE` / ... |
| `Xchg_Point` | `CARTESIAN_POINT` |

### 参考实现

参考 `D:\workspace\step_nio\src\writer\` 中的实现：
- `STEPWriter` - 外观类，管理写出流程
- `STEPWriter_Actor` - 核心逻辑，处理产品树和几何
- `TopoShapeMapping` - 形状去重机制
- `SBuilder` - STEP 实体构建器

