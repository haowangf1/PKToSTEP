# STEP QA Log - bugstepfiles

批量测试目录：`D:\workspace\resource\bugstepfiles`
测试时间：2026-03-13
工具：OCC (cadquery-ocp) 几何对比

## 批量扫描结果

（待运行 bugstepfiles 完整扫描）

---

## 逐文件分析记录

### 0065829.step
- 状态：FIXED
- 问题：37个无效面（Cylinder 21, Torus 9, Plane 7），体积差10.9%，OCC报 UnorientableShape
- 根因：`WriteEdge()` 中 EDGE_CURVE 的 same_sense 硬编码为 `.T.`，未读取 `edge->GetSameSense()`
- 修复：`xchg_to_step_topology.cpp:319` 改为 `.AddBoolean(edge->GetSameSense())`
- 修复后：179 faces, 816 edges, volume误差<0.0001%, ALL MATCH