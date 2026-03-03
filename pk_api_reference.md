# Parasolid PK API 参考文档（从内网文档提炼）

## 文档访问方式

**文档服务器地址：** `http://192.168.180.50/docs/parasolid/`

**访问方式（PowerShell）：**
```powershell
# 获取页面内容
$r = Invoke-WebRequest -Uri "http://192.168.180.50/docs/parasolid/<页面路径>" -UseBasicParsing -TimeoutSec 10
# 清理 HTML 标签
$text = $r.Content -replace '<[^>]+>', ' ' -replace '&nbsp;', ' ' -replace '\s+', ' '
```

**文档结构：**
- 主页: `http://192.168.180.50/docs/parasolid/` (frameset 页面)
- PK Reference 索引: `http://192.168.180.50/docs/parasolid/pk_index.html`
- Functional Description 索引: `http://192.168.180.50/docs/parasolid/fd_index.html`
- API 函数文档: `http://192.168.180.50/docs/parasolid/headers/pk_<函数名>.html`
- 数据类型文档: `http://192.168.180.50/docs/parasolid/headers/pk_<类型名>.html`
- FD 章节: `http://192.168.180.50/docs/parasolid/chapters/fd_chap.<编号>.html`
  - Model Structure: `fd_chap.015.html`
  - Body Types: `fd_chap.016.html`
  - Geometry: `fd_chap.018.html`
  - B-Curves And B-Surfaces: `fd_chap.019.html`
