# 分析 tank.step 的装配层次结构
import re

with open(r'd:\workspace\PKToSTEP\resource\tank.step', encoding='utf-8', errors='ignore') as f:
    content = f.read()

# 解析所有实体
entities = {}
for m in re.finditer(r'#(\d+)\s*=\s*(\w[^;]*);', content, re.DOTALL):
    eid = int(m.group(1))
    body = m.group(2).strip()
    etype = body.split('(')[0].strip()
    entities[eid] = (etype, body)

# 找所有 NAUO (父子关系)
nauo_list = []
for eid, (etype, body) in entities.items():
    if etype == 'NEXT_ASSEMBLY_USAGE_OCCURRENCE':
        # NAUO('id','name','desc', relating_pd, related_pd, ref)
        refs = re.findall(r'#(\d+)', body)
        if len(refs) >= 2:
            parent_pd = int(refs[0])
            child_pd = int(refs[1])
            nauo_list.append((eid, parent_pd, child_pd))

print(f'NAUO count: {len(nauo_list)}')

# 找所有 PRODUCT_DEFINITION
pd_to_prod = {}
for eid, (etype, body) in entities.items():
    if etype == 'PRODUCT_DEFINITION':
        refs = re.findall(r'#(\d+)', body)
        pd_to_prod[eid] = refs

# 找 PRODUCT 名字
prod_name = {}
for eid, (etype, body) in entities.items():
    if etype == 'PRODUCT':
        m = re.match(r"PRODUCT\s*\(\s*'([^']*)'\s*,\s*'([^']*)'\s*,", body)
        if m:
            prod_name[eid] = m.group(1) or m.group(2)

def get_pd_name(pd_id):
    if pd_id not in pd_to_prod:
        return f'PD#{pd_id}'
    refs = pd_to_prod[pd_id]
    # PD -> PDF -> Product
    for eid, (etype, body) in entities.items():
        if etype == 'PRODUCT_DEFINITION_FORMATION':
            prefs = re.findall(r'#(\d+)', body)
            if prefs and int(prefs[0]) in prod_name:
                # check if this PDF is referenced by PD
                pass
    return f'PD#{pd_id}'

# 建立 parent->children 关系
from collections import defaultdict
children = defaultdict(list)
roots = set(pd_to_prod.keys())
child_set = set()
for eid, parent_pd, child_pd in nauo_list:
    children[parent_pd].append(child_pd)
    child_set.add(child_pd)

root_pds = set(pd_to_prod.keys()) - child_set
print(f'Root PDs: {root_pds}')
print(f'Total PDs: {len(pd_to_prod)}')

# 打印树结构
def print_tree(pd_id, depth=0, visited=None):
    if visited is None:
        visited = set()
    if pd_id in visited:
        print('  '*depth + f'[CYCLE] PD#{pd_id}')
        return
    visited.add(pd_id)
    kids = children.get(pd_id, [])
    print('  '*depth + f'PD#{pd_id}  children={len(kids)}')
    for kid in kids:
        print_tree(kid, depth+1, visited.copy())

for r in sorted(root_pds):
    print_tree(r)
