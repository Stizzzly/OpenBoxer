.text
.globl _frames_call
_frames_call:
 movl 4(%esp),%edx
 movl 12(%esp),%ecx
 pushl 8(%esp)
 call *%edx
 ret
.globl _frames_invoke
_frames_invoke:
 pushl %ebp
 pushl %ebx
 pushl %esi
 pushl %edi
 subl $8,%esp
 movl 40(%esp),%eax
 movl %eax,(%esp)
 movl %esp,(%eax)
 fnstcw 28(%eax)
 fnstsw 32(%eax)
 stmxcsr 36(%eax)
 movl $0x11223344,%ebx
 movl $0x22334455,%esi
 movl $0x33445566,%edi
 movl $0x44556677,%ebp
 movl 28(%esp),%edx
 movl 36(%esp),%ecx
 pushl 32(%esp)
 call *%edx
 movl (%esp),%edx
 movl %eax,24(%edx)
 movl %esp,4(%edx)
 movl %ebx,8(%edx)
 movl %esi,12(%edx)
 movl %edi,16(%edx)
 movl %ebp,20(%edx)
 fnstcw 40(%edx)
 fnstsw 44(%edx)
 stmxcsr 48(%edx)
 addl $8,%esp
 popl %edi
 popl %esi
 popl %ebx
 popl %ebp
 ret

.globl _frames_fixture_entry@4
_frames_fixture_entry@4:
 pushl %ecx
 pushl 8(%esp)
 call _frames_fixture_dispatch
 addl $8,%esp
 ret $4
.globl _frames_fixture_advance@4
_frames_fixture_advance@4:
 pushl %ecx
 pushl 8(%esp)
 call _frames_fixture_advance_dispatch
 addl $8,%esp
 ret $4

