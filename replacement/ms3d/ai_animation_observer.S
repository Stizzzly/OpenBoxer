.text
.globl _ai_selector_entry
_ai_selector_entry:
 pushl $-1
 jmp _ai_selector_common
_ai_selector_common:
 pushfl
 pushal
 movl %esp,%ebp
 subl $576,%esp
 andl $-16,%esp
 fxsave (%esp)
 fnstenv 512(%esp)
 fninit
 movl $0x1f80,544(%esp)
 ldmxcsr 544(%esp)
 pushl %esp
 pushl %ebp
 call _ai_selector_before
 addl $8,%esp
 movl %eax,36(%ebp)
 movl $_ai_selector_after,40(%ebp)
 fxrstor (%esp)
 fldenv 512(%esp)
 movl %ebp,%esp
 popal
 popfl
 ret
_ai_selector_after:
 pushl $0
 pushfl
 pushal
 movl %esp,%ebp
 subl $576,%esp
 andl $-16,%esp
 fxsave (%esp)
 fnstenv 512(%esp)
 fninit
 movl $0x1f80,544(%esp)
 ldmxcsr 544(%esp)
 pushl %esp
 pushl %ebp
 call _ai_selector_return
 addl $8,%esp
 movl %eax,36(%ebp)
 fxrstor (%esp)
 fldenv 512(%esp)
 movl %ebp,%esp
 popal
 popfl
 ret
