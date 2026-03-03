---
name: build-and-test
description: 构建 PKToSTEP 项目并运行往返测试验证转换结果。当需要编译项目、运行测试、或排查构建/运行错误时使用。
---

# 构建与测试

## 标准流程

### Step 1：确认目录
```powershell
pwd  # 必须确认在 d:\workspace\PKToSTEP\build
```
如果不在 build 目录：`cd d:\workspace\PKToSTEP\build`

### Step 2：直接编译（优先）
```powershell
cmake --build . --config Release
```

### Step 3：编译失败时才安装依赖
仅当编译报"找不到头文件/库"时执行：
```powershell
conan install .. --build=missing
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
cmake --build . --config Release
```

### Step 4：运行往返测试
```powershell
./bin/Release/PKToSTEP.exe ../resource/hollow_cube.step
```

### Step 5：验证输出
检查 `d:\workspace\PKToSTEP\xt\` 目录下是否生成 `hollow_cube_roundtrip.xmt_txt`

## 错误判断

| 错误特征 | 原因 | 处理 |
|---------|------|------|
| `cannot open source file "xxx.h"` | 缺少依赖 | 执行 conan install |
| `unresolved external symbol` | 链接失败 | 检查 CMakeLists.txt |
| `PK_ERROR_xxx` 运行时输出 | PK kernel 错误 | 检查 PK API 调用参数 |
| xmt 文件未生成 | 转换逻辑错误 | 查看控制台错误输出 |
| xmt 文件生成但渲染异常 | 拓扑/几何错误 | 用 `DumpXchgBodyTopo()` 对比拓扑数量 |

## 调试拓扑问题

在 `src/main.cpp` 中启用 `DumpXchgBodyTopo()` 打印拓扑结构，对比转换前后的 Face/Edge/Vertex 数量。
