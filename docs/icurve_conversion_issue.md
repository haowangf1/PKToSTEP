# icurve 转换问题记录

## 问题描述

step->x->pk过程中不会主动创建icurve，但是x->pk过程中调用了PK的接口，生成的PKBody中是有icurve的，那么在PKBody转回X格式过程中，我需要处理这个icurve变为x格式基本curve
在 PK→Xchg 转换中，目前通过 `PK_CURVE_make_approx` 近似为基本曲线类型（如 bcurve）。当前实现会遇到两个问题：

1. **"too short" 错误**：使用空 interval `{0}` 导致 PK 认为曲线太短而拒绝近似
2. **944 错误**：为了解决 "too short" 错误，使用 `PK_CURVE_ask_interval` 查询 icurve 的实际 interval 并传给 `PK_CURVE_make_approx`，虽然近似成功，但后续 Xchg→PK 往返转换时报错 -944（edge 的 vertex 和 curve 不匹配）

## icurve产生原因和解释
icurve are created when surfaces are intersected and the intersection cannot be represented by one of the analytic curves.
icurve 的本质：
  - icurve 是两个曲面相交产生的曲线，没有像直线（y=kx+b）或圆（x²+y²=r²）那样的显式数学公式
  - Parasolid 内部用折线（polyline）近似表示它，然后在折线上建立参数化

关键特性：
  1. 参数范围不固定：不同的 icurve 有不同的参数范围，不像标准曲线（如圆的参数范围是 [0, 2π]）
  2. C¹ 连续但非 C² 连续：
    - 曲线本身和切线方向连续（C¹）
    - 但曲率（二阶导数）可能突变（非 C²），因为底层是折线近似
  3. 周期性问题：闭合的 icurve（如两个圆柱相交形成的椭圆）在周期边界处切线可能不连续
  4. 参数稳定性：即使曲线被延伸（如修剪操作），原有点的参数值不变，新增部分会扩展参数范围
  5. 单位速度参数化：一阶导数 ≈ 1.0，意味着参数变化 Δt ≈ 曲线长度变化 Δs，这使得参数更直观


## 当前代码位置

`src/pk_to_xchg_converter.cpp` 第 692-720 行，`ConvertCurve()` 函数中的 `PK_CLASS_icurve` 分支。

## 尝试过的解决方案

### 方案 1：使用空 interval

```cpp
else if (curve_class == PK_CLASS_icurve) {
    PK_CURVE_make_approx_o_t opts;
    PK_CURVE_make_approx_o_m(opts);
    opts.tolerance = 1e-6;

    PK_CURVE_t approx_curve = PK_ENTITY_null;
    PK_INTERVAL_t interval = {0}, new_interval = {0};  // 空 interval
    PK_LOGICAL_t exact = PK_LOGICAL_false;
    PK_ERROR_code_t err = PK_CURVE_make_approx(pk_curve, interval, &opts, &approx_curve, &new_interval, &exact);

    if (err == PK_ERROR_no_errors && approx_curve != PK_ENTITY_null) {
        rc = ConvertCurve(approx_curve, &result);
    } else {
        Log("Failed to approximate icurve");
        return STEP_OK;
    }
}
```

**失败原因**：对于某些 icurve，空 interval 导致 PK 认为曲线 "too short" 而拒绝近似。

---

### 方案 2：查询实际 interval（当前代码）

**目的**：解决方案 1 的 "too short" 错误

```cpp
else if (curve_class == PK_CLASS_icurve) {
    // 先查询 icurve 的实际参数范围
    PK_INTERVAL_t interval;
    PK_ERROR_code_t err = PK_CURVE_ask_interval(pk_curve, &interval);
    if (err != PK_ERROR_no_errors) {
        Log("Failed to query icurve interval");
        return STEP_OK;
    }

    PK_CURVE_make_approx_o_t opts;
    PK_CURVE_make_approx_o_m(opts);
    opts.tolerance = 1e-6;

    PK_CURVE_t approx_curve = PK_ENTITY_null;
    PK_INTERVAL_t new_interval = {0};
    PK_LOGICAL_t exact = PK_LOGICAL_false;
    err = PK_CURVE_make_approx(pk_curve, interval, &opts, &approx_curve, &new_interval, &exact);

    if (err == PK_ERROR_no_errors && approx_curve != PK_ENTITY_null) {
        rc = ConvertCurve(approx_curve, &result);
    } else {
        char buf[256];
        snprintf(buf, sizeof(buf), "Failed to approximate icurve (error %d, interval [%g, %g])",
                 err, interval.value[0], interval.value[1]);
        Log(buf);
        return STEP_OK;
    }
}
```

**结果**：
- ✅ 成功解决了 "too short" 错误，`PK_CURVE_make_approx` 执行成功
- ❌ 但引入了新问题：后续 Xchg→PK 往返转换时报错 -944

**失败原因分析**：

经过深入分析 PK API 文档和代码，944 错误的根本原因是：

1. **PK_CURVE_make_approx 的近似精度不足**：
   - 虽然参数域重新映射到 [0, length]（弧长参数化）
   - 但参数域变化本身不影响曲线在 3D 空间中的几何形状
   - 真正的问题是：tolerance = 1e-6 可能导致近似曲线与原始 icurve 的几何偏差过大

2. **Xchg→PK 转换时的 vertex 检查**：
   - 根据 `PK_EDGE_attach_curves` API 文档，PK 会检查 vertex 的 3D 点是否在 curve 上
   - 错误 `PK_ERROR_bad_vertex`：A curve has failed the vertex checking
   - 如果近似曲线与原始曲线偏差超过容差，vertex 的 3D 点可能不在新曲线上
   - 导致 vertex 检查失败 → 944 错误

3. **Edge 与参数区间的关系**：
   - Edge 通过 `PK_EDGE_ask_geometry` 可以查询其在 curve 上使用的参数区间 `[t_start, t_end]`
   - `PK_EDGE_attach_curves` 可以通过 options 指定参数区间
   - 但参数域变化不是 944 错误的直接原因，因为 PK 主要检查的是 3D 几何匹配

## 可能的解决方向

### 方向 1：减小 tolerance（推荐优先尝试）

将 `opts.tolerance` 从 `1e-6` 减小到 `1e-8` 或 `1e-10`，确保近似曲线与原始 icurve 的几何偏差足够小，使 vertex 能通过 PK 的几何检查。

**优点**：
- 实现简单，只需修改一个参数
- 直接解决几何精度问题

**缺点**：
- 可能导致近似曲线的控制点数量增加
- 性能可能略有下降

### 方向 2：调整 PK_CURVE_make_approx 的其他选项

查看 `PK_CURVE_make_approx_o_t` 结构的其他选项字段，可能有控制近似质量或参数化方式的选项。

### 方向 3：禁用 vertex 检查（不推荐）

在 Xchg→PK 转换时，使用 `PK_EDGE_attach_curves` 的 options 禁用 vertex 检查（`vx_checking = PK_check_vx_on_cu_none_c`）。

**问题**：这只是绕过检查，不解决根本问题，可能导致后续几何操作出错。

### 方向 4：保留 icurve 不转换

如果 Xchg 格式支持交线曲线或通用曲线类型，可以考虑不进行近似，直接保留原始几何信息。

**问题**：需要确认 Xchg 格式是否支持非标准曲线类型。

## PK 文档搜索结果

### PK_CURVE_make_approx API 关键信息

来源：`http://192.168.180.50/docs/parasolid/headers/pk_curve_make_approx.html`

- **curve_interval 参数**：如果提供的参数 interval 超出曲线范围，会报错 `PK_ERROR_bad_parameter`
- **返回的 new_interval**：对于 'arclength' 参数化，返回的 interval 是 `[0, length]`
- **推荐 tolerance**：0.0001 到 0.00001，更小的值可能导致过多顶点
- **描述**：创建一个与原曲线重合（在给定容差内）的弧长参数化曲线

### PK_EDGE_attach_curves API 关键信息

来源：Parasolid 头文件

- **PK_ERROR_bad_vertex**：A curve has failed the vertex checking (if enabled)
- **PK_ERROR_bad_interval**：The specified interval extends beyond the curve's parameter range
- **options.intervals**：可以指定 edge 在 curve 上使用的参数区间
- **options.vx_checking**：控制 vertex 检查模式

### Geometry 章节关键信息

来源：`http://192.168.180.50/docs/parasolid/chapters/fd_chap.018.html`

- icurve 没有显式参数形式，基于折线近似的参数化
- 参数化是连续且一阶可微的，但二阶导数通常不连续
- 一阶导数的大小接近 1.0（单位速度参数化）

## 待验证的 API

- `PK_CURVE_make_approx_o_t` 的其他选项字段
- `PK_EDGE_attach_curves` 的 vx_checking 选项

## 测试用例

需要找到包含 icurve 的 STEP 文件进行测试，当前已知会触发此问题的文件：
- （待补充具体文件名）

## 状态

**未解决** - 需要测试不同的 tolerance 值。

**下一步建议**：
1. 优先尝试方向 1：将 tolerance 改为 1e-8 或 1e-10，测试是否解决 944 错误
2. 如果方向 1 有效，找到合适的 tolerance 平衡点（精度 vs 性能）
3. 如果方向 1 无效，再考虑其他方向
