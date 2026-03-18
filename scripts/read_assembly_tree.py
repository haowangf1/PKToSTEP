#!/usr/bin/env python3
"""
read_assembly_tree.py - 使用 OCC XCAF 读取 STEP 装配树

【核心概念】
  XCAF 是 OCC 在通用标签树数据库 (CAF) 上扩展的 CAD 数据管理层。
  普通 STEPControl_Reader 只能读出扁平几何体，
  STEPCAFControl_Reader + XCAF 能还原完整装配树（名字/层级/变换）。

【STEP 与 XCAF 对应关系】
  PRODUCT (装配体)             -> [ASM]  IsAssembly=True
  PRODUCT (零件)               -> [PART] IsSimpleShape=True
  NEXT_ASSEMBLY_USAGE_OCCURRENCE -> [INST] IsReference=True
  ITEM_DEFINED_TRANSFORMATION  -> TopLoc_Location (变换矩阵)
  MANIFOLD_SOLID_BREP          -> TopoDS_Solid
  SHELL_BASED_SURFACE_MODEL    -> TopoDS_Shell

用法:
  python scripts/read_assembly_tree.py <step_file>
"""
import sys

# XCAF 相关模块
from OCP.STEPCAFControl import STEPCAFControl_Reader  # 带属性的 STEP 读取器
from OCP.TDocStd import TDocStd_Document              # XCAF 文档容器
from OCP.XCAFDoc import XCAFDoc_DocumentTool          # 获取 ShapeTool
from OCP.TDF import TDF_LabelSequence, TDF_Label      # 标签和标签序列
from OCP.TCollection import TCollection_ExtendedString
from OCP.TDataStd import TDataStd_Name                # 名字属性

# 几何/拓扑相关模块
from OCP.BRepGProp import BRepGProp    # 几何属性计算（体积/面积）
from OCP.GProp import GProp_GProps     # 属性结果容器
from OCP.TopExp import TopExp_Explorer # 拓扑遍历器
from OCP.TopAbs import TopAbs_SOLID, TopAbs_SHELL, TopAbs_FACE


def get_name(label):
    """
    从 TDF_Label 读取名字属性 (TDataStd_Name)。
    XCAF 每个节点是一个 TDF_Label，属性以 Attribute 形式挂在上面。
    FindAttribute() 按 GUID 查找属性，找到返回 True。
    """
    attr = TDataStd_Name()
    if label.FindAttribute(TDataStd_Name.GetID_s(), attr):
        return attr.Get().ToExtString()  # 转为 Python 字符串
    return '(unnamed)'


def get_volume(shape):
    """
    计算 TopoDS_Shape 的体积。
    BRepGProp.VolumeProperties_s() 填充 GProp_GProps，
    Mass() 在体积属性下等于体积值。
    曲面体 (TopoDS_Shell) 体积为 0 但面积不为 0。
    """
    if shape.IsNull():
        return 0.0
    props = GProp_GProps()
    BRepGProp.VolumeProperties_s(shape, props)
    return abs(props.Mass())


def count_topo(shape, ttype):
    """
    统计 shape 中某种拓扑元素数量。
    TopExp_Explorer 遍历方式: More() -> Current() -> Next()
    常用类型: TopAbs_SOLID / TopAbs_SHELL / TopAbs_FACE / TopAbs_EDGE
    """
    n = 0
    exp = TopExp_Explorer(shape, ttype)
    while exp.More():
        n += 1
        exp.Next()
    return n


def walk(st, label, depth=0):
    """
    递归遍历装配树，打印每个节点信息。

    节点类型:
      [ASM]  - 装配体，IsAssembly=True，有子组件
      [PART] - 叶子零件，IsSimpleShape=True，直接持有 BRep 几何
      [REF]  - 其他引用节点

    装配树结构示例 (aa624.step):
      [ASM]  USB-B-S-X-W-TH              <- 根装配体
        +-[INST] shell  loc=transformed  <- 实例 (NAUO)，含变换矩阵
          [PART] shell                   <- 原型零件，实际几何在此
        +-[INST] contact01  loc=transformed
          [PART] contact01
        +-[INST] contact01  loc=transformed  <- 同一原型的第二个实例
          [PART] contact01               <- 指向同一 PART 标签 (prototype-instance)
    """
    indent = '  ' * depth
    name    = get_name(label)
    is_asm  = st.IsAssembly_s(label)     # 是装配体（有子组件）
    is_part = st.IsSimpleShape_s(label)  # 是叶子零件（有几何）

    # GetShape_s(): 返回该标签对应的 TopoDS_Shape
    #   对于 [ASM]: 整个装配体合并后的形状（含所有子零件，已应用变换）
    #   对于 [PART]: 零件本身的几何
    shape  = st.GetShape_s(label)
    vol    = get_volume(shape)
    solids = count_topo(shape, TopAbs_SOLID)
    shells = count_topo(shape, TopAbs_SHELL)
    faces  = count_topo(shape, TopAbs_FACE)

    tag = '[ASM] ' if is_asm else ('[PART]' if is_part else '[REF] ')
    print(f"{indent}{tag} {name}")
    print(f"{indent}       vol={vol:.4f}  solids={solids}  shells={shells}  faces={faces}")

    if not is_asm:
        return  # 叶子零件无子组件，直接返回

    # 获取子组件实例列表（对应 STEP 里的 NAUO）
    comps = TDF_LabelSequence()
    st.GetComponents_s(label, comps)

    for i in range(comps.Length()):
        inst = comps.Value(i + 1)  # 实例标签（1-based）

        # GetLocation_s(): 获取实例相对父组件的变换矩阵 (TopLoc_Location)
        # 对应 STEP 里 IDT (ITEM_DEFINED_TRANSFORMATION) 的平移/旋转
        # IsIdentity()==True  -> 无变换，放在原点
        # IsIdentity()==False -> 有平移或旋转
        loc = st.GetLocation_s(inst)
        loc_str = 'identity' if loc.IsIdentity() else 'transformed'

        referred = TDF_Label()
        if st.IsReference_s(inst):
            # IsReference=True: 这是一个实例引用
            # GetReferredShape_s(): 找到该实例引用的原型 (prototype) 标签
            # 多个实例可引用同一原型（prototype-instance 模式，节省存储）
            st.GetReferredShape_s(inst, referred)
            inst_shape = st.GetShape_s(inst)  # 实例形状（已应用变换）
            inst_vol   = get_volume(inst_shape)
            ref_name   = get_name(referred)
            print(f"{indent}  +-[INST] {ref_name}  loc={loc_str}  vol={inst_vol:.4f}")
            walk(st, referred, depth + 2)  # 递归进入原型
        else:
            print(f"{indent}  +-[COMP] {get_name(inst)}  loc={loc_str}")
            walk(st, inst, depth + 2)


def main():
    if len(sys.argv) < 2:
        print(f'Usage: python {sys.argv[0]} <step_file>')
        sys.exit(1)

    step_file = sys.argv[1]
    print(f'Reading: {step_file}')
    print('=' * 70)

    # Step 1: 创建 XCAF 文档
    # TDocStd_Document 是 CAF 框架的文档容器，类似带事务的标签树数据库。
    # 'MDTV-CAF' 是固定的文档格式标识符。
    doc = TDocStd_Document(TCollection_ExtendedString('MDTV-CAF'))

    # Step 2: 读取 STEP 文件
    # STEPCAFControl_Reader vs STEPControl_Reader:
    #   STEPControl_Reader   -> 返回扁平 TopoDS_Shape，名字/装配结构丢失
    #   STEPCAFControl_Reader -> 填充 XCAF 文档，保留名字/颜色/装配层级
    reader = STEPCAFControl_Reader()
    reader.SetColorMode(False)  # 不读颜色（加快速度）
    reader.SetNameMode(True)    # 读名字（关键！否则所有名字为空）
    reader.SetLayerMode(False)  # 不读图层
    reader.ReadFile(step_file)  # 解析 STEP 文件
    reader.Transfer(doc)        # 将解析结果填充到 XCAF 文档标签树

    # Step 3: 获取 ShapeTool
    # XCAFDoc_ShapeTool 是 XCAF 里管理形状和装配关系的核心工具。
    # 通过 doc.Main()（文档根标签）获取。主要 API:
    #   IsAssembly_s / IsSimpleShape_s / IsReference_s  -- 判断节点类型
    #   GetComponents_s   -- 获取子实例列表
    #   GetReferredShape_s -- 从实例找原型
    #   GetLocation_s     -- 获取变换矩阵
    #   GetShape_s        -- 获取 TopoDS_Shape
    #   GetFreeShapes     -- 获取顶层根节点
    st = XCAFDoc_DocumentTool.ShapeTool_s(doc.Main())

    # Step 4: 获取顶层根节点
    # GetFreeShapes() 返回所有没有父节点的顶层形状（装配根）
    roots = TDF_LabelSequence()
    st.GetFreeShapes(roots)
    # 某些文件格式导致 GetFreeShapes 返回空，回退用 IsFree_s 过滤
    if roots.Length() == 0:
        all_labels = TDF_LabelSequence()
        st.GetShapes(all_labels)
        for i in range(all_labels.Length()):
            lbl = all_labels.Value(i + 1)
            if st.IsFree_s(lbl):  # IsFree=True 表示没有父节点
                roots.Append(lbl)

    print(f'Top-level shapes: {roots.Length()}')
    print()

    # Step 5: 递归打印装配树
    for i in range(roots.Length()):
        walk(st, roots.Value(i + 1))
        print()


if __name__ == '__main__':
    main()
