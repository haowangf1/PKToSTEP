# Xchg→STEP 导出代码全面审计方案

## 背景
项目已通过 OCC 验证工具发现并修复了多个导出 bug（NURBS 控制点转置、EDGE_CURVE same_sense 硬编码、BREP_WITH_VOIDS 格式）。现在需要系统性审查所有导出代码，找出剩余的硬编码值、缺失数据和标准合规问题。

---

## A. 硬编码值修复

| # | 文件 | 行号 | 问题 | 修复方案 |
|---|------|------|------|----------|
| A1 | xchg_to_step_geometry.cpp | 557,578 | NURBS Curve `closed` 硬编码 `.F.` | 改用 `nurbs->IsClosed()` |
| A2 | xchg_to_step_geometry.cpp | 557,578 | NURBS Curve `self_intersect` 硬编码 `.F.` | 保持 `.F.`（Xchg 无此 API，安全默认值） |
| A3 | xchg_to_step_topology.cpp | 58,73,80 | Shell 闭合状态硬编码 `true` | 改用 `shell->IsClosed()` |
| A4 | xchg_to_step_topology.cpp | 159 | 外环判断硬编码 `i==0` | 改用 `loop->IsOuter()` |
| A5 | xchg_to_step_topology.cpp | 202 | FACE_BOUND orientation 硬编码 `true` | 需确认：查 step_nio 参考实现中此值来源 |

## B. 缺失几何类型

| # | 类型 | STEP 实体 | Xchg 头文件 | 优先级 |
|---|------|-----------|-------------|--------|
| B1 | RevolutionSurface | SURFACE_OF_REVOLUTION | xchg_revolutionsurface.hpp | P1 - 常见 |
| B2 | LinearExtrusionSurface | SURFACE_OF_LINEAR_EXTRUSION | xchg_linearextrusionsurface.hpp | P1 - 常见 |
| B3 | OffsetSurface | OFFSET_SURFACE | xchg_offsetsurface.hpp | P2 |
| B4 | OffsetCurve | OFFSET_CURVE_3D | xchg_offsetcurve.hpp | P3 |

## C. 拓扑逻辑问题

| # | 文件 | 问题 | 影响 | 修复方案 |
|---|------|------|------|----------|
| C1 | topology.cpp:339 | Point 去重逻辑冗余：`!HasMapping \|\| id==GetMapping` 恒为 true | 功能正确但代码混乱 | 简化去重逻辑 |
| C3 | topology.cpp:301-303 | GetVertex 失败时 `//TODO` 无处理 | 可能写出无效 EDGE_CURVE | 添加错误处理，跳过无效 edge |

## D. 数值精度问题

| # | 文件 | 问题 | 修复方案 |
|---|------|------|----------|
| D1 | geometry.cpp:300-301 | writeRealArr 直接用 `<<` 输出 double，精度不受控 | 改用 builder 的 AddReal 格式化，或统一精度 |

---

## 实施步骤

### Step 1: 修复硬编码值（A1, A3, A4, A5） ✅
- A1: NURBS Curve closed 标志 → `nurbs->IsClosed()` ✅
- A3: Shell 闭合 → `shell->IsClosed()` ✅
- A4: 外环判断 → `loop->IsOuter()` ✅
- A5: FACE_BOUND orientation → `loop->GetOrientation()` ✅
- A2: self_intersect 保持 `.F.`（Xchg 无此 API）— 无需修改

### Step 2: 修复拓扑逻辑（C1, C3） ✅
- C1: Point 去重逻辑简化，先检查 HasMapping 再 GetOrAllocate ✅
- C3: Edge GetVertex 失败时打印警告并跳过 ✅

### Step 3: 实现缺失几何类型（B1-B4） ✅
- B1: WriteRevolutionSurface → SURFACE_OF_REVOLUTION + AXIS1_PLACEMENT ✅
- B2: WriteLinearExtrusionSurface → SURFACE_OF_LINEAR_EXTRUSION + VECTOR ✅
- B3: WriteOffsetSurface → OFFSET_SURFACE ✅
- B4: WriteOffsetCurve → OFFSET_CURVE_3D ✅

### Step 5: 精度问题（D1）
---

## 验证方法

每个 Step 完成后：
1. `cmake --build build/Debug --config Debug` 编译
2. 对 `resource/` 目录已知文件运行导出
3. `python scripts/batch_test.py` 批量对比
4. 对失败文件用 `python scripts/face_check.py` 定位问题
5. 回归确认之前通过的文件仍然通过

---

## 关键文件
- `src/xchg_to_step_topology.cpp` - 拓扑导出
- `src/xchg_to_step_geometry.cpp` - 几何导出
- `src/xchg_to_step_writer.cpp` - Writer 主逻辑
- `exchange_base/include/topology/` - Xchg 拓扑 API
- `exchange_base/include/geom/` - Xchg 几何 API
- `step_nio/src/writer/STEPWriter_Actor.cpp` - 参考实现
