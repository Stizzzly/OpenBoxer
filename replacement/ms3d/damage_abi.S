.text
.globl _damage_guard_probe
_damage_guard_probe:
 pushl %ebp
 movl %esp,%ebp
 movl 28(%ebp),%edx
 fxsave (%edx)
 fnstenv 512(%edx)
 fxrstor (%edx)
 fldenv 512(%edx)
 pushl 24(%ebp)
 pushl 20(%ebp)
 pushl 16(%ebp)
 pushl 12(%ebp)
 pushl 8(%ebp)
 call _damage_guard_check
 addl $20,%esp
 movzbl %al,%eax
 movl 28(%ebp),%edx
 movl %eax,1088(%edx)
 fxsave 544(%edx)
 fnstenv 1056(%edx)
 fxrstor 544(%edx)
 fldenv 1056(%edx)
 popl %ebp
 ret
.globl _damage_bridge
_damage_bridge:
 pushl %ebp
 movl %esp,%ebp
 pushl %ebx
 pushl %esi
 pushl %edi
 movl 28(%ebp),%ebx
 movl 20(%ebp),%edi
 movl 16(%ebp),%esi
 testl %edi,%edi
 jz 2f
1:
 decl %edi
 pushl (%esi,%edi,4)
 testl %edi,%edi
 jnz 1b
2:
 movl 36(%ebp),%edx
 testl %edx,%edx
 jz 5f
 movups 0(%edx),%xmm0
 movups 16(%edx),%xmm1
 movups 32(%edx),%xmm2
 movups 48(%edx),%xmm3
 movups 64(%edx),%xmm4
 movups 80(%edx),%xmm5
 movups 96(%edx),%xmm6
 movups 112(%edx),%xmm7
5:
 movl 12(%ebp),%ecx
 call *8(%ebp)
 movl %eax,(%ebx)
 fxsave 16(%ebx)
 fnstenv 528(%ebx)
 fldenv 528(%ebx)
 cmpl $0,32(%ebp)
 je 3f
 fstpt 4(%ebx)
 fxrstor 16(%ebx)
 fldenv 528(%ebx)
3:
 leal -12(%ebp),%esp
 popl %edi
 popl %esi
 popl %ebx
 popl %ebp
 ret
.globl _damage_candidate_entry
_damage_candidate_entry:
 pushl 4(%esp)
 pushl 4(%esp)
 pushl %ecx
 call _damage_dispatch
 addl $12,%esp
 ret $4
.globl _damage_invoke
_damage_invoke:
 pushl %ebp
 pushl %ebx
 pushl %esi
 pushl %edi
 movl 20(%esp),%eax
 movl 24(%esp),%ecx
 movl 36(%esp),%edx
 testl %edx,%edx
 jz 4f
 fxrstor (%edx)
 fldenv 512(%edx)
4:
 movl 28(%esp),%edx
 pushl 32(%esp)
 pushl %edx
 movl 4(%esp),%edx
 fxsave 32(%edx)
 fnstenv 544(%edx)
 fldenv 544(%edx)
 leal 4(%esp),%ebx
 movl %ebx,(%edx)
 movl $0x11223344,%ebx
 movl $0x22334455,%esi
 movl $0x33445566,%edi
 movl $0x44556677,%ebp
 call *%eax
 pushl %eax
 movl 4(%esp),%eax
 fxsave 576(%eax)
 fnstenv 1088(%eax)
 fldenv 1088(%eax)
 leal 4(%esp),%edx
 movl %edx,4(%eax)
 movl %ebx,8(%eax)
 movl %esi,12(%eax)
 movl %edi,16(%eax)
 movl %ebp,20(%eax)
 movl (%esp),%edx
 movl %edx,24(%eax)
 popl %eax
 addl $4,%esp
 popl %edi
 popl %esi
 popl %ebx
 popl %ebp
 ret

.globl _damage_ai_guard_probe
_damage_ai_guard_probe:
 pushl %ebp
 movl %esp,%ebp
 movl 28(%ebp),%edx
 fxsave (%edx)
 fnstenv 512(%edx)
 fxrstor (%edx)
 fldenv 512(%edx)
 pushl 24(%ebp)
 pushl 20(%ebp)
 pushl 16(%ebp)
 pushl 12(%ebp)
 pushl 8(%ebp)
 call _damage_ai_guard_check
 addl $20,%esp
 movzbl %al,%eax
 movl 28(%ebp),%edx
 movl %eax,1088(%edx)
 fxsave 544(%edx)
 fnstenv 1056(%edx)
 fxrstor 544(%edx)
 fldenv 1056(%edx)
 popl %ebp
 ret

.globl _damage_continuation_guard_probe
_damage_continuation_guard_probe:
 pushl %ebp
 movl %esp,%ebp
 movl 28(%ebp),%edx
 fxsave (%edx)
 fnstenv 512(%edx)
 fxrstor (%edx)
 fldenv 512(%edx)
 pushl 24(%ebp)
 pushl 20(%ebp)
 pushl 16(%ebp)
 pushl 12(%ebp)
 pushl 8(%ebp)
 call _damage_continuation_guard_check
 addl $20,%esp
 movzbl %al,%eax
 movl 28(%ebp),%edx
 movl %eax,1088(%edx)
 fxsave 544(%edx)
 fnstenv 1056(%edx)
 fxrstor 544(%edx)
 fldenv 1056(%edx)
 popl %ebp
 ret
