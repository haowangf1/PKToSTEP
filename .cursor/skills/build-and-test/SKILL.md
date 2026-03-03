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

### Step 4：运行往返测试（捕获完整输出）
```powershell
./bin/Release/PKToSTEP.exe 2>&1 | Tee-Object -FilePath ../xt/test_output.txt
```
重定向 stderr+stdout，便于分析输出。

### Step 5：验证测试结果

程序会输出**两次** `DumpXchgBodyTopo`：
1. `STEP->Xchg (reference)`：从 STEP 文件直接读入的参考拓扑
2. `PK->Xchg (ours)`：经过我们转换器的拓扑

**对比检查项**：

| 检查项 | 参考值来源 | 通过标准 |
|--------|-----------|---------|
| Lumps 数量 | reference dump | 与 ours 完全一致 |
| 每个 Shell 的 faces 数量 | reference dump | 与 ours 完全一致 |
| Shell closed/outer 标志 | reference dump | 与 ours 完全一致 |
| 每个 Face 的 loops 数量 | reference dump | 与 ours 完全一致 |
| 每个 Loop 的 coedges 数量 | reference dump | 与 ours 完全一致 |
| Vertex 坐标 | reference dump | 误差 < 1e-6 |
| xmt 文件生成 | 文件系统 | `xt/hollow_cube_roundtrip.xmt_txt` 存在且非空 |

**快速判断**：先对比两个 dump 的 Lumps/Shell/Face/Loop/Coedge 数字是否完全相同，数字不一致直接定位问题层次。

## 编译错误判断

| 错误特征 | 原因 | 处理 |
|---------|------|------|
| `cannot open source file "xxx.h"` | 缺少依赖头文件 | 执行 conan install 重新配置 |
| `unresolved external symbol` | 链接库缺失 | 检查 CMakeLists.txt 链接项 |

## 运行时错误判断

| 输出特征 | 原因 | 处理 |
|---------|------|------|
| `[Error] PKToXchgConverter::Convert failed` | 转换器返回错误码 | 查看 `[Converter]` 行的具体错误 |
| `[Error] ConvertToPKBody failed` | Xchg→PK 反向转换失败 | 拓扑数据有误，对比两次 dump |
| `[Warn] No body nodes found` | STEP 文件读取异常 | 检查 STEP 文件路径 |
| `PK_ERROR_xxx` 在 Converter 日志 | PK API 调用参数错误 | 查询对应 PK API 文档 |
| 两次 dump 数字不一致 | 拓扑转换逻辑错误 | 根据不一致的层次定位代码 |
