# explore-xchg-source

在文档和源码库中查找 Xchg 格式和 STEP 转换的相关信息。

## 功能

此 skill 用于查找 X 格式和 STEP 转换的实现细节，按以下优先级：

1. **优先查找项目文档**（已总结的转换规则）
2. **其次查找源码库**（详细实现）

## 查找顺序

### 1. 项目文档（优先）

- `docs/step_to_x_to_pk_process.md` - STEP↔Xchg↔PK 转换规则分析
  - 包含 XOR 运算规则
  - 方向标志处理
  - 拓扑层次映射

- `docs/implementation_issues.md` - 已知问题和解决方案
  - PK→Xchg 转换中的问题
  - 修正方案和代码示例

### 2. 源码库（文档中找不到时）

**exchange_base** (`D:\workspace\exchange_base`) - Xchg 格式库
- X→PK 转换逻辑
- 关键文件：`src/parasolid/xchg_body_convertto_pk.cpp`
- 关键函数：`GetOrderedCoedges`, `FinAddChildren`, `FaceAddChildren`

**amxt_stp** (`D:\workspace\amxt_stp`) - STEP 读取库
- STEP→X 转换逻辑
- 关键文件：`src/step/Translate_*.cpp`, `GeomBuilder.cpp`

## 使用方法

```
/explore-xchg-source <搜索内容>
```

### 示例

搜索特定函数或类：
```
/explore-xchg-source GetOrderedCoedges
/explore-xchg-source FinAddChildren
/explore-xchg-source ConvertToPKBody
```

搜索特定概念：
```
/explore-xchg-source loop orientation
/explore-xchg-source face sense
/explore-xchg-source XOR 运算
```

## 实现逻辑

当调用此 skill 时：

1. **第一步**：在 `docs/step_to_x_to_pk_process.md` 中搜索关键词
2. **第二步**：如果文档中没有，在 `docs/implementation_issues.md` 中搜索
3. **第三步**：如果仍未找到，在源码库中搜索：
   - 使用 Grep 工具查找匹配的代码片段
   - 使用 Read 工具读取相关文件
4. **第四步**：提供代码分析和解释

## 参考资料 (Reference)

### 项目文档

| 文档 | 路径 | 内容 |
|------|------|------|
| 转换规则分析 | `docs/step_to_x_to_pk_process.md` | STEP↔Xchg↔PK 完整转换规则，XOR 运算逻辑 |
| 实现问题 | `docs/implementation_issues.md` | PK→Xchg 转换的已知问题和解决方案 |
| 项目指南 | `CLAUDE.md` | 项目架构、构建系统、关键概念 |

### 源码库

| 库 | 路径 | 关键文件 |
|---|------|---------|
| exchange_base | `D:\workspace\exchange_base` | `src/parasolid/xchg_body_convertto_pk.cpp` |
| amxt_stp | `D:\workspace\amxt_stp` | `src/step/Translate_*.cpp`, `GeomBuilder.cpp` |

### 关键概念速查

| 概念 | 文档位置 | 源码位置 |
|------|---------|---------|
| GetOrderedCoedges | `step_to_x_to_pk_process.md` 第3.4节 | `xchg_body_convertto_pk.cpp:925` |
| XOR 方向运算 | `step_to_x_to_pk_process.md` 第2.2节 | 多处使用 |
| Face 方向计算 | `step_to_x_to_pk_process.md` 第3.3节 | `xchg_body_convertto_pk.cpp:2391` |
| Loop orientation | `implementation_issues.md` 问题2 | `xchg_body_convertto_pk.cpp:817` |
| Region 类型 | `step_to_x_to_pk_process.md` 第3.2节 | `xchg_body_convertto_pk.cpp:2200` |

## 常用查询

### X→PK 转换相关
- `GetOrderedCoedges` - Coedge 排序逻辑（错误 -40 的根源）
- `FinAddChildren` - Fin 到 PK 的转换
- `FaceAddChildren` - Face 到 PK 的转换
- `LoopAddChildren` - Loop 到 PK 的转换

### STEP→X 转换相关
- `Translate_Shell` - Shell 转换逻辑
- `Translate_Face` - Face 转换逻辑
- `Translate_Loop` - Loop 转换逻辑
- `GeomBuilder` - 几何构建器

### 方向计算
- `orientation` - 方向标志
- `sense` - 方向感知
- `XOR` - 异或运算逻辑
- `finalOrientation` - 最终方向计算

## 注意事项

- 优先使用项目文档，文档已经总结了关键规则
- 源码库是只读的参考代码，不应修改
- 用于理解官方实现逻辑，指导 PK→X 转换的实现
- 重点关注方向计算和拓扑转换的规则
