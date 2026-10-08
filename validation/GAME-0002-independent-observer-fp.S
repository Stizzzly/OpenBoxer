.text
.globl _audit_target
_audit_target:
 fldt precise
 movl $0xaabbccdd,%eax
 stc
 ret $4
.globl _audit_probe
_audit_probe:
 pushl %ebp
 movl %esp,%ebp
 pushl %ebx
 pushl %esi
 pushl %edi
 fninit
 fldcw cw
 ldmxcsr mx
 fld1
 fldz
 movl $0x11223344,%ebx
 movl $0x22334455,%esi
 movl $0x33445566,%edi
 movl $0x12345678,%ecx
 pushl $0x87654321
 call *8(%ebp)
 pushfl
 pushl %eax
 movl 12(%ebp),%edx
 fxsave (%edx)
 fnstenv 512(%edx)
 popl %eax
 movl %eax,544(%edx)
 popl %eax
 movl %eax,548(%edx)
 movl %ebx,552(%edx)
 movl %esi,556(%edx)
 movl %edi,560(%edx)
 movl %esp,%eax
 leal -12(%ebp),%ecx
 subl %ecx,%eax
 movl %eax,564(%edx)
 movl %ebp,568(%edx)
 fninit
 popl %edi
 popl %esi
 popl %ebx
 popl %ebp
 ret
.data
cw: .short 0x027f
mx: .long 0x00001fa0
precise: .long 1,0x80000000
 .short 0x3fff
