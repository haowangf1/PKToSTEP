# CLAUDE.md

总是用中文回答

本文件为 Claude Code (claude.ai/code) 在此代码库中工作时提供指导。

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
