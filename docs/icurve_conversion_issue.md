# icurve 转换问题记录

## 问题描述

step->x->pk过程中不会主动创建icurve，但是x->pk过程中调用了PK的接口，生成的PKBody中是有icurve的，那么在PKBody转回X格式过程中，我需要处理这个icurve变为x格式基本curve
在 PK→Xchg 转换中，目前通过 `PK_CURVE_make_approx` 近似为基本曲线类型（如 bcurve）。当前实现会遇到两个问题：

1. **"too short" 错误**：使用空 interval `{0}` 导致 PK 认为曲线太短而拒绝近似
2. **944 错误**：使用 `PK_CURVE_ask_interval` 查询实际 interval 后，虽然近似成功，但后续 Xchg→PK 转换时报错 -944（edge 的 vertex 和 curve 不匹配）

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

`src/pk_to_xchg_converter.cpp` 第 692-710 行，`ConvertCurve()` 函数中的 `PK_CLASS_icurve` 分支。

## 尝试过的解决方案

### 方案 1：使用空 interval（当前代码）

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

### 方案 2：查询实际 interval（已尝试，失败）

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

**失败原因**：
- `PK_CURVE_make_approx` 成功执行，生成了近似曲线
- 但在后续 Xchg→PK 往返转换时，`ConvertToPKBody()` 报错 -944
- 错误 -944 含义：edge 的 vertex 位置与 curve 的端点不匹配
- **根本原因**：`PK_CURVE_make_approx` 生成的近似曲线参数域与原始 icurve 不同
  - 根据 PK 文档，`PK_CURVE_make_approx` 返回的 `new_interval` 是 `[0, length]`（弧长参数化）
  - 原始 icurve 的参数范围可能是任意的（如 `[5.2, 18.7]`）
  - 近似后的 bcurve 参数范围被重新映射到 `[0, 曲线长度]`
  - edge 存储的 start/end 参数值（如 `[5.2, 10.3]`）在新曲线的参数域 `[0, length]` 上已经无效
  - 导致 edge 的 start/end vertex 对应的参数值在新曲线上计算出的位置不正确

## 可能的解决方向

### 方向 1：调整 tolerance 和选项

尝试不同的 `opts.tolerance` 值或其他 `PK_CURVE_make_approx_o_t` 选项，确保近似曲线的参数域与原始曲线一致。

### 方向 2：建立参数映射关系（推荐）

**关键发现**：根据 PK 文档搜索结果，`PK_CURVE_make_approx` 返回的 `new_interval` 是 `[0, length]`（弧长参数化），这意味着参数域被完全重新映射。

**解决思路**：
1. 在 `ConvertCurve()` 中近似 icurve 后，记录原始 interval 和新 interval 的映射关系
2. 在 `ConvertEdge()` 中，将 edge 的参数范围从旧参数域映射到新参数域
3. 参数映射公式：`new_param = (old_param - old_min) / (old_max - old_min) * new_length`
4. 或者，在 Xchg 中存储映射信息，让 X→PK 转换时能正确处理

**实现挑战**：
- 需要在 `ConvertCurve()` 和 `ConvertEdge()` 之间传递映射信息
- 可能需要在 `pk_edge_map_` 中存储额外的参数映射数据
- 需要处理多个 edge 共享同一条 icurve 的情况

### 方向 3：使用 edge 的实际参数范围

不使用 `PK_CURVE_ask_interval` 返回的完整曲线范围，而是从 edge 本身查询其使用的参数范围（`PK_EDGE_ask_curve_interval`），只近似 edge 实际使用的部分。

**问题**：多个 edge 可能共享同一条 icurve 的不同部分，这会导致重复近似和去重失败。

### 方向 4：保留 icurve 不转换

如果 Xchg 格式支持交线曲线或通用曲线类型，可以考虑不进行近似，直接保留原始几何信息（如两个曲面的引用）。

**问题**：需要确认 Xchg 格式是否支持非标准曲线类型。

## PK 文档搜索结果

### PK_CURVE_make_approx API 关键信息

来源：`http://192.168.180.50/docs/parasolid/headers/pk_curve_make_approx.html`

- **curve_interval 参数**：如果提供的参数 interval 超出曲线范围，会报错 `PK_ERROR_bad_parameter`
- **返回的 new_interval**：对于 'arclength' 参数化，返回的 interval 是 `[0, length]`
- **推荐 tolerance**：0.0001 到 0.00001，更小的值可能导致过多顶点
- **描述**：创建一个与原曲线重合（在给定容差内）的弧长参数化曲线

### Geometry 章节关键信息

来源：`http://192.168.180.50/docs/parasolid/chapters/fd_chap.018.html`

- icurve 没有显式参数形式，基于折线近似的参数化
- 参数化是连续且一阶可微的，但二阶导数通常不连续
- 一阶导数的大小接近 1.0（单位速度参数化）

## 待验证的 API

- `PK_EDGE_ask_curve_interval` - 查询 edge 使用的曲线参数范围
- `PK_CURVE_find_point` - 在曲线上查找最接近给定点的参数值
- `PK_CURVE_parameterise_vector` - 将 3D 点映射到曲线参数
- `PK_CURVE_make_approx_o_t` 的其他选项字段

## 测试用例

需要找到包含 icurve 的 STEP 文件进行测试，当前已知会触发此问题的文件：
- （待补充具体文件名）

## 状态

**未解决** - 需要进一步研究 PK API 文档和测试不同方案。

**下一步建议**：尝试方向 2（建立参数映射关系），这是最有可能解决 944 错误的方案。
