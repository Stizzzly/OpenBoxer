// Independently designed adapters for the documented ST0 and scalar layout ABI.
.text
.globl _selection_score
_selection_score:
 pushl %ebp
 movl %esp,%ebp
 subl $40,%esp
 movl 12(%ebp),%eax
 movl 16(%ebp),%edx
 movl 0(%eax),%ecx
 movl %ecx,0(%esp)
 movl 4(%eax),%ecx
 movl %ecx,4(%esp)
 movl 8(%eax),%ecx
 movl %ecx,8(%esp)
 movl 12(%eax),%ecx
 movl %ecx,12(%esp)
 movl 0(%edx),%ecx
 movl %ecx,16(%esp)
 movl 4(%edx),%ecx
 movl %ecx,20(%esp)
 movl 8(%edx),%ecx
 movl %ecx,24(%esp)
 movl 12(%edx),%ecx
 movl %ecx,28(%esp)
 call *8(%ebp)
 fsts 36(%esp)
 fstps 0(%esp)
 call *20(%ebp)
 fstps 36(%esp)
 movl 36(%esp),%eax
 leave
 ret
.globl _selection_angle
_selection_angle:
 pushl %ebp
 movl %esp,%ebp
 subl $16,%esp
 flds 12(%ebp)
 fstpl 0(%esp)
 call *8(%ebp)
 fmull 16(%ebp)
 fsts 12(%esp)
 fstps 8(%esp)
 movl 8(%esp),%eax
 leave
 ret
