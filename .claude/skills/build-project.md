# Build Project Skill

使用 Conan + CMake 构建 PKToSTEP 项目。

## 构建流程

1. 切换到 build 目录（如果不存在则创建）
2. 运行 Conan 安装依赖（如果需要）
3. 运行 CMake 配置
4. 运行 CMake 构建
5. 报告构建结果和可执行文件位置

## 参数

- `clean` (可选): 是否清理后重新构建，默认为 false
- `config` (可选): 构建配置，默认为 `Release`
  - 可选值: `Release`, `Debug`

## 使用示例

```
/build-project
/build-project clean
/build-project Debug
```

## 执行步骤

1. 如果指定 clean，删除 build 目录并重新创建
2. `cd build`
3. `conan install .. --build=missing` (如果 conanfile.txt 有变化或首次构建)
4. `cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=<config>`
5. `cmake --build . --config <config>`
6. 报告可执行文件位置: `build/bin/PKToSTEP`

## 依赖项

- Conan 包管理器
- CMake 3.15+
- C++17 编译器
- 依赖库: pskernel, amcax-exchange-base, amcax-amxt_stp, gtest
