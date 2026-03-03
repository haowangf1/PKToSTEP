# Check PK API Skill

从内网文档服务器查询 Parasolid API 函数或数据类型的文档。

## 功能

快速查询 PK API 文档，提取关键信息（函数签名、参数说明、返回值、注意事项），无需手动访问网页。

## 参数

- `api_name` (必需): API 函数名或数据类型名
  - 函数示例: `PK_BODY_ask_topology`, `PK_FACE_ask_surf`, `PK_EDGE_ask_curve`
  - 类型示例: `PK_SURF_plane_sf_t`, `PK_CURVE_line_sf_t`, `PK_BODY_type_t`

## 使用示例

```
/check-pk-api PK_BODY_ask_topology
/check-pk-api PK_SURF_plane_sf_t
/check-pk-api PK_FACE_ask_loops
```

## 执行步骤

1. 构造文档 URL: `http://192.168.180.50/docs/parasolid/headers/pk_<api_name>.html`
2. 使用 WebFetch 获取页面内容
3. 提取关键信息:
   - 函数签名或类型定义
   - 参数说明
   - 返回值说明
   - 注意事项和使用建议
4. 以简洁格式呈现（过滤 HTML 标签和无关内容）

## 文档服务器

- **地址**: http://192.168.180.50/docs/parasolid/
- **权限**: 已在 `.claude/settings.local.json` 中配置 WebFetch 访问权限

## 常用 API 分类

### 拓扑查询
- `PK_BODY_ask_topology`: 获取 body 的拓扑结构
- `PK_REGION_ask_shells`: 获取 region 的 shells
- `PK_SHELL_ask_faces`: 获取 shell 的 faces
- `PK_FACE_ask_loops`: 获取 face 的 loops
- `PK_LOOP_ask_fins`: 获取 loop 的 fins
- `PK_FIN_ask_edge`: 获取 fin 的 edge
- `PK_EDGE_ask_vertices`: 获取 edge 的 vertices

### 几何查询
- `PK_FACE_ask_surf`: 获取 face 的 surface 几何
- `PK_EDGE_ask_curve`: 获取 edge 的 curve 几何
- `PK_VERTEX_ask_point`: 获取 vertex 的 point 坐标
- `PK_SURF_ask_class`: 获取 surface 类型
- `PK_CURVE_ask_class`: 获取 curve 类型

### 数据类型
- `PK_SURF_*_sf_t`: Surface 几何数据结构（plane, cylinder, cone, sphere, torus, nurbs 等）
- `PK_CURVE_*_sf_t`: Curve 几何数据结构（line, circle, ellipse, nurbs 等）
- `PK_BODY_type_t`: Body 类型枚举
