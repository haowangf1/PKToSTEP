---
name: build-and-test
description: 构建 PKToSTEP 项目并运行往返测试验证转换结果。当需要编译项目、运行测试、或排查构建/运行错误时使用。
---

# 构建与测试

## 标准流程

### Step 1：确认目录
```powershell
cd d:\workspace\PKToSTEP\build\Debug
```

### Step 2：直接编译（优先）
```powershell
cmake --build . --config Debug
```

### Step 3：编译失败时才安装依赖
仅当编译报"找不到头文件/库"时执行：
```powershell
cd d:\workspace\PKToSTEP
conan install . --build=missing
cd build\Debug
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
cmake --build . --config Debug
```

### Step 4：运行往返测试（必须带 DLL 路径）
```powershell
$env:PATH = "C:\Users\adminstrator\.conan2\p\amcax01d02fb16add4\p\bin\Debug;C:\Users\adminstrator\.conan2\p\amcax724d8e7c73c89\p\bin\Debug;C:\Users\adminstrator\.conan2\p\psker7f7426e452be7\p\bin;$env:PATH"
cd d:\workspace\PKToSTEP
.\build\Debug\Debug\PKToSTEP.exe 2>&1 | Tee-Object -FilePath .\xt\test_output.txt
```
重定向 stderr+stdout，便于分析输出。**不设置 PATH 会导致 DLL 找不到（exit -1073741515），程序实际不运行。**

### Step 5：解读对比表格输出

程序现在输出一个 `CompareXchgBodyTopo` 对比表格，直接标注两个 Xchg_Body 的差异：

```
==============================================================================
  TOPOLOGY COMPARISON: STEP->Xchg (reference)  vs  PK->Xchg (ours)
==============================================================================
  Lumps: REF=1    OURS=1
  Lump[0]
    shells: REF=1    OURS=1
    Shell[0]: closed REF=1 OURS=1  outer REF=1 OURS=1  faces REF=6 OURS=6
      Face[0]: shellOrient REF=0 OURS=1  <<<DIFF>>>  loops REF=1 OURS=1  surfType REF=Plane OURS=Plane
        surfParams:
          REF : Plane O=(0.0000,0.0000,0.0000) N=[1.0000,0.0000,-0.0000]
          OURS: Plane O=(0.0000,0.0000,0.0050) N=[0.0000,0.0000,1.0000]  <<<DIFF>>>
        Loop[0]: outer REF=1 OURS=1  orient REF=0 OURS=1  <<<DIFF>>>  coedges REF=4 OURS=4
          CE[0]:
            inLoop    : REF=0  OURS=1  <<<DIFF>>>
            ceOrient  : REF=0  OURS=0
            sameSense : REF=1  OURS=1
            curveType : REF=Line          OURS=Line
            uvCrvType : REF=-             OURS=-
            startV    : REF=(0.0000,0.0000,0.0000)  OURS=(0.0000,0.0000,0.0050)  <<<DIFF>>>
            endV      : REF=(0.0000,0.0000,0.0050)  OURS=(0.0000,0.0100,0.0050)  <<<DIFF>>>
```

**`<<<DIFF>>>` 标记所有不一致项。无标记 = 与参考一致。**

**各层级对比项说明**：

| 层级 | 对比字段 | 含义 |
|------|---------|------|
| Shell | `closed` / `outer` | 壳是否闭合 / 是否外壳 |
| Shell | `faces` 数量 | 该壳包含的面数 |
| Face | `shellOrient` | 面在壳中的朝向（面法线与壳外法线同向=1） |
| Face | `surfType` | 曲面类型（Plane/Cylinder/Sphere/NURBS…） |
| Face | `surfParams` | 曲面关键参数（Origin、Normal/Axis、Radius 等） |
| Loop | `outer` | 是否外环（1=外环） |
| Loop | `orient` | 环的方向（STEP中1=CCW=正向） |
| Loop | `coedges` 数量 | 该环包含的 coedge 数 |
| Coedge | `inLoop` | coedge 与所属 loop 的方向一致性（AddCoedge第2参数） |
| Coedge | `ceOrient` | coedge 自身方向 |
| Coedge | `sameSense` | edge 的几何曲线方向（1=正向） |
| Coedge | `curveType` | 3D 边的曲线类型（Line/Circle/NURBS…） |
| Coedge | `uvCrvType` | 2D UV 参数曲线类型（若无则为`-`） |
| Coedge | `startV` / `endV` | 边的起点/终点坐标 |

**快速定位差异的策略**：

1. 先看 Lumps/shells/faces 数量 — 数字不同说明结构层次已出错
2. 数量一致但有 `<<<DIFF>>>` — 逐层向下找到第一个差异层
3. `shellOrient` 差异 → 检查 `ConvertShell` 中的 `same` 计算公式
4. `surfParams` 差异（Origin/Normal 不同）→ Face 顺序不同，拓扑等价但枚举顺序有差异（无功能影响）
5. `inLoop` 全部为 OURS=1 REF=0 → REF 来自 STEP 直读（默认值），OURS 来自 PK（已修正为显式设1），属已知差异
6. `startV/endV` 不同 → Face 不对应（REF 和 OURS 的 Face 枚举顺序不同），需结合 surfParams 确认是否同一面

## 编译错误判断

| 错误特征 | 原因 | 处理 |
|---------|------|------|
| `cannot open source file "xxx.h"` | 缺少依赖头文件 | 执行 conan install 重新配置 |
| `unresolved external symbol` | 链接库缺失 | 检查 CMakeLists.txt 链接项 |
| `C2039: "GetXxx": 不是 "Xchg_Yyy" 的成员` | 调用了不存在的方法 | 查对应头文件确认正确方法名 |

## 运行时错误判断

| 输出特征 | 原因 | 处理 |
|---------|------|------|
| exit code `-1073741515 (0xc0000135)` + 无输出 | DLL 找不到 | 设置 PATH（见 Step 4） |
| `[Error] PKToXchgConverter::Convert failed` | 转换器返回错误码 | 查看 `[Converter]` 行的具体错误 |
| `[Error] ConvertToPKBody failed` | Xchg→PK 反向转换失败 | 拓扑数据有误，分析对比表格中的差异 |
| `[Warn] No body nodes found` | STEP 文件读取异常 | 检查 STEP 文件路径 |
| 对比表格中 `shells/faces` 数量 `<<<DIFF>>>` | 结构层次转换错误 | 根据不一致的层次定位代码 |
| 对比表格中只有 orientation 差异无 DIFF | 已知可接受差异 | 确认 roundtrip 成功即可 |
