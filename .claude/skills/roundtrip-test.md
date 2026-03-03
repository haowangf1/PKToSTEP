# Roundtrip Test Skill

执行 PKToSTEP 往返测试流程，验证转换的正确性。

## 测试流程

1. 检查项目是否已构建，如果没有则先构建
2. 运行 PKToSTEP 处理指定的 STEP 文件
3. 检查 xt/ 目录下生成的 .xmt_txt 文件
4. 报告测试结果（文件大小、是否成功生成）

## 参数

- `step_file` (可选): STEP 文件名，默认为 `hollow_cube.step`
  - 可选值: `hollow_cube.step`, `cube214.step`, `cylinder214.step`

## 使用示例

```
/roundtrip-test
/roundtrip-test cube214.step
/roundtrip-test cylinder214.step
```

## 执行步骤

1. 切换到 build 目录
2. 检查可执行文件是否存在
3. 运行: `./bin/PKToSTEP ../resource/<step_file>`
4. 验证输出文件: `../xt/<basename>_roundtrip.xmt_txt`
5. 报告结果
