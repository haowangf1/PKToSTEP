---
name: add-converter-function
description: 在 PKToSTEP 项目中添加新的几何或拓扑转换函数（ConvertXxxSurface、ConvertXxxCurve、ConvertXxxTopo 等）。当需要扩展转换器支持新的 PK 几何类型或拓扑结构时使用。
---

# 添加新转换函数

## 核心文件

- 实现：`src/pk_to_xchg_converter.cpp`
- 接口：`include/pk_to_xchg_converter.hpp`

## 函数模板

### 几何转换（Surface/Curve）

```cpp
STEPExport_ErrorCode PkToXchgConverter::ConvertXxxSurface(
    PK_SURF_t pk_surf, std::shared_ptr<Xchg_Surface>& xchg_surf)
{
    // 1. 检查缓存（去重）
    auto it = pk_surface_map_.find(pk_surf);
    if (it != pk_surface_map_.end()) {
        xchg_surf = it->second;
        return STEP_OK;
    }

    // 2. 调用 PK API 获取几何参数
    PK_XXX_sf_t sf;
    PK_ERROR_code_t err = PK_XXX_ask(pk_surf, &sf);
    if (err != PK_ERROR_no_errors_c) return STEP_ERR_KERNEL;

    // 3. 创建 Xchg 几何对象
    auto surf = std::make_shared<Xchg_XxxSurface>();
    // ... 设置参数 ...

    // 4. 存入缓存
    pk_surface_map_[pk_surf] = surf;
    xchg_surf = surf;
    return STEP_OK;
}
```

### 拓扑转换

```cpp
STEPExport_ErrorCode PkToXchgConverter::ConvertXxx(
    PK_XXX_t pk_xxx, std::shared_ptr<Xchg_Xxx>& xchg_xxx)
{
    // PK API 内存用 PKMemGuard 管理
    int n_items = 0;
    PK_YYY_t* items = nullptr;
    PK_XXX_ask_yyy(pk_xxx, &n_items, &items);
    PKMemGuard guard;
    guard.ptr = items;

    // ... 转换逻辑 ...
    return STEP_OK;
}
```

## 检查清单

添加新函数后逐项确认：

- [ ] 几何函数：是否接入缓存 map（`pk_surface_map_` 或 `pk_curve_map_`）
- [ ] 拓扑函数：共享实体（Edge/Vertex）是否走去重逻辑
- [ ] PK API 分配的内存是否用 `PKMemGuard` 管理
- [ ] 返回类型是 `STEPExport_ErrorCode`，错误向上传播
- [ ] 日志用 `log_callback_`，禁止 cout/printf
- [ ] 在 `.hpp` 中声明新函数
- [ ] 在调用处的 switch/if 分支中注册新类型

## 开发流程

1. 查询 PK API 文档（用 `query-pk-api` skill）了解函数签名
2. 参考同类型的已有函数（如 `ConvertPlaneSurface`、`ConvertLineCurve`）
3. 按模板实现，填入参数设置逻辑
4. 构建验证（用 `build-and-test` skill）

## 缓存 Map 速查

| Map | 键 | 值 |
|-----|----|----|
| `pk_surface_map_` | `PK_SURF_t` | `Xchg_Surface` |
| `pk_curve_map_` | `PK_CURVE_t` | `Xchg_Curve` |
| `pk_face_map_` | `PK_FACE_t` | `Xchg_Face` |
| `pk_edge_map_` | `PK_EDGE_t` | `Xchg_Edge` |
| `pk_vertex_map_` | `PK_VERTEX_t` | `Xchg_Vertex` |
