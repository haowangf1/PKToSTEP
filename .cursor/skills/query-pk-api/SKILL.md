---
name: query-pk-api
description: 查询 Parasolid PK API 内网文档，获取函数签名、参数说明和使用注意事项。当需要了解 PK_xxx 函数用法、PK 数据类型定义、或 Parasolid 功能描述时使用。
---

# 查询 PK API 文档

## 文档服务器

base URL: `http://192.168.180.50/docs/parasolid/`

## URL 规则

| 查询类型 | URL 格式 | 示例 |
|---------|---------|------|
| API 函数 | `headers/pk_<函数名>.html` | `headers/pk_BODY_ask_topology.html` |
| 数据类型 | `headers/pk_<类型名>.html` | `headers/pk_BODY_type_t.html` |
| FD 章节 | `chapters/fd_chap.<编号>.html` | `chapters/fd_chap.016.html` |

**FD 章节编号速查**：
- `015` Model Structure（拓扑结构）
- `016` Body Types（body 类型）
- `018` Geometry（几何）
- `019` B-Curves And B-Surfaces

## 查询步骤

1. 用 WebFetch 直接访问对应 URL
2. 如果函数名不确定，先访问 `pk_index.html` 搜索关键词
3. 从页面内容中提取：函数签名、参数说明、返回值、注意事项

## 示例

查询 `PK_FACE_ask_loops`：
→ WebFetch `http://192.168.180.50/docs/parasolid/headers/pk_FACE_ask_loops.html`

查询 body 类型常量：
→ WebFetch `http://192.168.180.50/docs/parasolid/headers/pk_BODY_type_t.html`

## 注意

- 函数名大小写敏感，URL 中函数名保持原始大小写
- 页面是 HTML，WebFetch 会自动转换为可读文本
- 如果页面不存在，尝试 `pk_index.html` 搜索正确的函数名
