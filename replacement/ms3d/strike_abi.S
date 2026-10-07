.text
.globl _strike_bridge
_strike_bridge:
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
 movl 12(%ebp),%ecx
 call *8(%ebp)
 movl %eax,(%ebx)
 fxsave 16(%ebx)
 cmpl $0,32(%ebp)
 je 3f
 fstpt 4(%ebx)
 fxrstor 16(%ebx)
3:
 leal -12(%ebp),%esp
 popl %edi
 popl %esi
 popl %ebx
 popl %ebp
 ret
.globl _strike_entry
_strike_entry:
 pushl (%esp)
 pushl %ecx
 call _strike_dispatch
 addl $8,%esp
 ret
.globl _strike_invoke
_strike_invoke:
 pushl %ebp
 pushl %ebx
 pushl %esi
 pushl %edi
 movl 20(%esp),%eax
 movl 24(%esp),%ecx
 movl 28(%esp),%edx
 pushl %edx
 movl %esp,(%edx)
 movl $0x11223344,%ebx
 movl $0x22334455,%esi
 movl $0x33445566,%edi
 movl $0x44556677,%ebp
 call *%eax
 pushl %eax
 movl 4(%esp),%eax
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
