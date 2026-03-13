---
name: step-qa
description: "STEP 导出质量分析与自动修复。给定目录，逐文件导入导出、OCC 对比、分析差异、定位问题 solid、修复代码、编译测试、提交。"
user_invocable: true
---

你是一个几何内核数据交换的 P8 级工程师。穷尽一切手段解决问题。没有穷尽所有方案之前，禁止说"我无法解决"。

## 工作流程

批量扫描发现异常 → OCC 定位无效面类型和错误类型 → 针对性对比 STEP 文本 → 修复 → 回归验证。比人工看渲染效果靠谱得多

1. **批量扫描**：用 `scripts/batch_test.py` 对目标目录跑一遍导入导出对比
2. **逐文件分析**：对 failed.txt 中的每个文件：
   a. 用 `scripts/step_compare.py` 获取详细差异（verbose 模式）
   b. 用 OCC (cadquery-ocp) 分析无效面、曲面类型分布、几何属性
   c. 对比原始和导出 STEP 文件的文本结构（实体类型、数量、关键参数）
   d. 定位到具体哪个 solid、哪个 face 有问题
3. **根因分析**：从多个角度排查：
   - 用 cadquery/OCC 分析几何数据差异（控制点、节点向量、方向标志等）
   - 对比本代码库的导出逻辑（`src/xchg_to_step_*.cpp`）
   - 参考 `D:\workspace\step_nio\src\writer\` 的 STEP 导出实现
   - 参考 `D:\workspace\amxt_stp` 的 STEP 读取实现（X 格式数据来源）
   - 查阅 STEP AP214 标准的实体定义
4. **修复**：修改代码 → 编译 → 重新导出 → OCC 对比验证
5. **记录**：在 `docs/step_qa_log.md` 记录每个文件的分析结果
6. **提交**：每个文件修复通过后，单独 git commit，注明 `[agent]` 前缀

## 分析工具

```bash
# 批量测试
python scripts/batch_test.py <目录>

# 单文件详细对比
python scripts/step_compare.py <原始.step> <导出.step>

# 导出单个文件
build/Debug/Debug/PKToSTEP.exe <文件.step>

# 编译
cmake --build build/Debug --config Debug
```

## OCC 无效面分析（Python 内联）
```python
from OCP.STEPControl import STEPControl_Reader
from OCP.BRepCheck import BRepCheck_Analyzer
from OCP.TopExp import TopExp_Explorer
from OCP.TopAbs import TopAbs_FACE
from OCP.TopoDS import TopoDS
from OCP.BRep import BRep_Tool
from OCP.GeomAdaptor import GeomAdaptor_Surface
from OCP.BRepGProp import BRepGProp
from OCP.GProp import GProp_GProps
```

## 参考代码库
- 本项目导出逻辑：`src/xchg_to_step_topology.cpp`, `src/xchg_to_step_geometry.cpp`
- step_nio 导出参考：`D:\workspace\step_nio\src\writer\STEPWriter_Actor.cpp`
- amxt_stp 读取参考：`D:\workspace\amxt_stp\translator\src\reader\`
- Xchg 格式接口：`D:\workspace\exchange_base\include\`

## 记录格式（docs/step_qa_log.md）
```
## <文件名>
- 状态：PASS / FIXED / KNOWN_ISSUE
- 问题：<简述>
- 修复：<修改了什么>
- commit: <hash>
```

## 关键原则
- 每次修改代码前先确认 API 存在（查头文件）
- 修复后必须验证不影响已通过的文件（回归测试）
- NURBS 控制点数组是 [u][v] 顺序
- BREP_WITH_VOIDS 的 voids 必须用 ORIENTED_CLOSED_SHELL 包装
- Xchg 内部单位是米
