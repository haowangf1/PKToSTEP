# PKToSTEP 项目指南

## 项目概述

PKToSTEP 是一个两阶段的 CAD 格式转换器，用于将 Parasolid (PK_BODY) 模型转换为 STEP 文件：

- **Step1 (已完成)**: PK_BODY → AMCAXExchangeBase 中间格式
  - 完整的拓扑关系转换（Body/Region/Shell/Face/Loop/Edge/Vertex）
  - 完整的几何数据转换（Surface/Curve/Point）
  - 实体去重机制确保共享几何只创建一次

- **Step2 (计划中)**: AMCAXExchangeBase → STEP 文件导出

## 关键设计决策

### 拓扑映射规则

#### Void Region 处理
- **PK 保证**: `PK_BODY_ask_regions` 返回的 `regions[0]` 一定是 infinite void region
- **转换策略**:
  - **Solid/General body**: 跳过 infinite void region，只处理 solid region 和 bounded void region（空腔）
  - **Sheet/Wire/Acorn body**: 只处理 infinite void region
- **原因**: Xchg (STEP/BREP) 不需要显式表示 void region，PK 会根据 outer/inner shell 自动推导

#### General Body 等同 Solid Body
- **重要**: `Xchg_Node::ConvertToPKBody()` 返回的 PK_BODY 类型是 `PK_BODY_type_general_c` (5605)，而非 `PK_BODY_type_solid_c` (5601)
- **处理**: 必须将 `general_c` 与 `solid_c` 同等对待，使用相同的转换逻辑
- **代码位置**: `pk_to_xchg_converter.cpp` 第 93 行有相关注释

#### Face 方向映射
- **公式**: `xchg_orient = (orients[i] != face_orient)`
- **原因**: 确保 Face 法向在转换后保持正确方向
- **代码位置**: `ConvertFace` 函数

#### 实体去重
- **机制**: 使用 `unordered_map` 缓存已转换的实体
  - `pk_face_map_`: Face 去重
  - `pk_edge_map_`: Edge 去重（共享边只创建一次）
  - `pk_vertex_map_`: Vertex 去重（共享顶点只创建一次）
  - `pk_surface_map_`: Surface 几何去重
  - `pk_curve_map_`: Curve 几何去重
- **重要性**: 确保拓扑共享关系正确，避免重复创建实体

### Shell 类型映射

| PK Region 类型 | PK Shell 类型 | Xchg 添加方式 |
|---------------|--------------|--------------|
| Solid region | 任意 | `lump->AddOuterShell(shell)` |
| Bounded void region (空腔) | 任意 | `lump->AddInnerShell(shell)` |
| Infinite void region (Sheet body) | 任意 | `lump->AddOpenShell(shell)` |
| Infinite void region (Wire/Acorn) | 任意 | `lump->AddWireShell(shell)` |

## 编码约定

### 错误处理
- **返回类型**: 所有转换函数返回 `STEPExport_ErrorCode`
- **错误码** (定义在 `xchg_error_code.hpp`):
  - `STEP_OK` (0): 成功
  - `STEP_ERR_KERNEL` (1): PK kernel 错误
  - `STEP_ERR_NULL_ARGUMENT` (2): 空指针参数
  - `STEP_ERR_UNSUPPORTED_TYPE` (3): 不支持的类型
  - `STEP_ERR_INTERNAL` (4): 内部错误
- **错误传播**: 错误码向上传播，调用者检查返回值

### 内存管理
- **PK API 内存**: 使用 `PKMemGuard` RAII 包装自动释放
  ```cpp
  struct PKMemGuard {
      void* ptr = nullptr;
      ~PKMemGuard() { if (ptr) PK_MEMORY_free(ptr); }
  };
  ```
- **使用示例**:
  ```cpp
  PKMemGuard guard;
  PK_FACE_ask_loops(face, &n_loops, &loops);
  guard.ptr = loops;  // 自动释放
  ```

### 日志机制
- **回调函数**: 通过 `SetLogCallback()` 注入日志函数
- **避免**: 不要在转换器内部硬编码 `std::cout` 或 `printf`
- **使用**: `if (log_callback_) log_callback_("message");`

## 文件组织

### 核心代码
- `src/pk_to_xchg_converter.cpp` (1103 行): 核心转换逻辑
- `include/pk_to_xchg_converter.hpp`: 转换器接口
- `include/xchg_error_code.hpp`: 错误码定义
- `src/main.cpp`: 测试入口，包含往返测试和拓扑 dump 函数

### 测试资源
- `resource/hollow_cube.step`: 主要测试模型（空心立方体，90KB）
- `resource/cube214.step`: 简单立方体（7KB）
- `resource/cylinder214.step`: 圆柱体（3.5KB）

### 输出文件
- `xt/*.xmt_txt`: 往返测试生成的 Parasolid 传输格式文件

### 文档
- `step1_implementation_plan.md` (758 行): 详细的实施方案，包含 API 映射和实现策略
- `pk_api_reference.md`: PK API 文档访问方式

## 构建与测试

### 构建命令
```bash
cd build
conan install .. --build=missing
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
cmake --build . --config Release
```

### 运行往返测试
```bash
cd build
./bin/PKToSTEP ../resource/hollow_cube.step
# 检查 ../xt/hollow_cube_roundtrip.xmt_txt 是否生成
```

### 依赖项 (conanfile.txt)
- `pskernel/36.0`: Parasolid 内核库
- `amcax-exchange-base/0.0.1.121`: 中间格式库
- `amcax-amxt_stp/0.0.1.39`: STEP 格式支持
- `gtest/1.14.0`: Google Test 框架

## 文档访问

### PK API 文档服务器
- **地址**: http://192.168.180.50/docs/parasolid/
- **已配置权限**: `.claude/settings.local.json` 中允许 WebFetch 和 curl 访问
- **常用页面**:
  - API 函数: `http://192.168.180.50/docs/parasolid/headers/pk_<函数名>.html`
  - 数据类型: `http://192.168.180.50/docs/parasolid/headers/pk_<类型名>.html`
  - 功能描述: `http://192.168.180.50/docs/parasolid/chapters/fd_chap.<编号>.html`

### 查询示例
```bash
# 使用 curl 查询 API 文档
curl "http://192.168.180.50/docs/parasolid/headers/pk_BODY_ask_topology.html"
```

## 开发指南

### 修改转换逻辑前
1. 先阅读 `step1_implementation_plan.md` 相关章节
2. 理解 PK 和 Xchg 的拓扑层次差异
3. 检查是否影响实体去重逻辑

### 添加新几何类型
1. 参考现有 `ConvertXxxSurface` 或 `ConvertXxxCurve` 函数
2. 遵循模式：
   - 调用 PK API 获取几何数据
   - 使用 `PKMemGuard` 管理内存
   - 创建对应的 Xchg 几何对象
   - 添加到缓存 map 中

### 调试拓扑问题
- 使用 `main.cpp` 中的 `DumpXchgBodyTopo()` 函数打印拓扑结构
- 对比转换前后的拓扑层次和实体数量

### 不要做的事
- ❌ 破坏实体去重逻辑（会导致拓扑共享关系错误）
- ❌ 跳过 void region 处理规则（会导致转换失败）
- ❌ 修改 Face 方向映射公式（会导致法向错误）
- ❌ 在转换器内部硬编码输出（使用日志回调）

## 已知限制

### 当前不支持
- 多连通 general body（假设单个 lump）- 代码第 93 行
- Surface 方向反转（TODO 注释在第 322 行）
- Parabola 和 Hyperbola 曲线（第 687-689 行）

### 未来扩展
- Step2: AMCAXExchangeBase → STEP 文件导出
- 更多几何类型支持
- 性能优化（大模型处理）

## 测试策略

### 当前测试方式
往返测试：STEP → PK_BODY → Xchg_Body → PK_BODY → XMT 文件 → 可视化检查

### 计划中的自动化测试
- **单元测试**: 每个转换函数独立测试
- **拓扑一致性**: 比较转换前后的 Face/Edge/Vertex 数量
- **几何精度**: 比较顶点坐标（误差 < 1e-6）
- **去重验证**: 验证共享边和顶点只创建一次

## 术语表

| 术语 | 含义 |
|------|------|
| PK | Parasolid Kernel，Siemens 的几何内核 |
| Xchg | AMCAXExchangeBase，中间交换格式 |
| BREP | Boundary Representation，边界表示法 |
| Void Region | 空区域，PK 中表示空间的拓扑实体 |
| Fin | PK 中 Edge 在 Loop 中的有向使用 |
| Coedge | Xchg/STEP 中 Edge 在 Loop 中的有向使用 |
| Roundtrip | 往返转换，用于验证转换的正确性 |
