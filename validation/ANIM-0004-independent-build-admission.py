import json
from pathlib import Path
p=Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer')
m=json.loads((p/'specs/animation/ANIM-0004-harness-metadata.json').read_text())
roles=[dict(state=0,name='LEGS_STAND',return_rva='0x371BE',literal_rva='0x1623E8')]+m['new_roles']
code='#include "action_roles.hpp"\n#include <cstdio>\nint main(){unsigned checks=0,failures=0;constexpr uintptr_t base=0x400000,owner=base+0x17acd0;\n'
for index,r in enumerate(roles):
    caller=r['return_rva']; literal=r['literal_rva']; name=json.dumps(r['name'])
    code+=f'{{int i=action::role(base,base+{caller},base+{literal});if(i!={index}||!action::admit(i,base,owner,true,{name}))++failures;++checks;\n'
    for expr in [f'action::role(base,base+{caller} + 1,base+{literal})!=-1',f'action::role(base,base+{caller},base+{literal} + 1)!=-1',f'action::admit(i,base,owner+4,true,{name})',f'action::admit(i,base,owner,false,{name})','action::admit(i,base,owner,true,"WRONG")','action::admit(i,base,owner,true,nullptr)','action::admit(-1,base,owner,true,"WRONG")','action::admit(8,base,owner,true,"WRONG")']:
        code+=f'if({expr})++failures;++checks;\n'
    code+='}\n'
code+='printf("checks=%u failures=%u\\n",checks,failures);return failures?1:0;}\n'
(p/'validation/ANIM-0004-independent-admission.cpp').write_text(code)

