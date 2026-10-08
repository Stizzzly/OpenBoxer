.text
.globl _damage_test_full_probe
_damage_test_full_probe:
 pushl %ebp
 movl %esp,%ebp
 movl 12(%ebp),%edx
 fxrstor (%edx)
 movl $0x12345678,%ecx
 pushl $0x87654321
 call *8(%ebp)
 movl 16(%ebp),%edx
 fxsave (%edx)
 fnstenv 512(%edx)
 fxrstor (%edx)
 popl %ebp
 ret
.globl _damage_test_target
_damage_test_target:
 cmpl $0x12345678,%ecx
 jne 1f
 cmpl $0x87654321,4(%esp)
 jne 1f
 fld1
 movl $0xaabbccdd,%eax
 ret $4
1:
 movl $0xbad,%eax
 ret $4
.globl _damage_test_probe
_damage_test_probe:
 pushl %ebp
 movl %esp,%ebp
 pushl %ebx
 pushl %esi
 pushl %edi
 movl $0x11223344,%ebx
 movl $0x22334455,%esi
 movl $0x33445566,%edi
 movl $0x12345678,%ecx
 pushl $0x87654321
 call *8(%ebp)
 movl 12(%ebp),%edx
 movl %eax,0(%edx)
 movl %ebx,4(%edx)
 movl %esi,8(%edx)
 movl %edi,12(%edx)
 movl %esp,%eax
 leal -12(%ebp),%ecx
 subl %ecx,%eax
 movl %eax,16(%edx)
 fstpt 20(%edx)
 fnstcw 30(%edx)
 popl %edi
 popl %esi
 popl %ebx
 popl %ebp
 ret
