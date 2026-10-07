.text
.globl _frames_candidate_advance@4
_frames_candidate_advance@4:
 pushl %ecx
 pushl 8(%esp)
 call _frames_candidate_advance_dispatch
 addl $8,%esp
 ret $4
.globl _frames_entry@4
_frames_entry@4:
 movl (%esp),%eax
 pushl %eax
 pushl %ecx
 pushl 12(%esp)
 call _frames_dispatch
 addl $12,%esp
 ret $4
.globl _frames_observe@4
_frames_observe@4:
 movl (%esp),%eax
 pushl %eax
 pushl %ecx
 pushl 12(%esp)
 call _frames_observe_dispatch
 addl $12,%esp
 ret $4

