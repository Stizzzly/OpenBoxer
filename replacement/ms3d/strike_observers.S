.text
.globl _strike_observer0
_strike_observer0:
 leal 4(%esp),%eax
 pushl (%esp)
 pushl %eax
 pushl %ecx
 pushl $0
 call _strike_observer_dispatch
 addl $16,%esp
 ret $12
.globl _strike_observer1
_strike_observer1:
 leal 4(%esp),%eax
 pushl (%esp)
 pushl %eax
 pushl %ecx
 pushl $1
 call _strike_observer_dispatch
 addl $16,%esp
 ret $4
.globl _strike_observer2
_strike_observer2:
 leal 4(%esp),%eax
 pushl (%esp)
 pushl %eax
 pushl %ecx
 pushl $2
 call _strike_observer_dispatch
 addl $16,%esp
 ret $4
.globl _strike_observer3
_strike_observer3:
 leal 4(%esp),%eax
 pushl (%esp)
 pushl %eax
 pushl %ecx
 pushl $3
 call _strike_observer_dispatch
 addl $16,%esp
 ret $4
.globl _strike_observer4
_strike_observer4:
 leal 4(%esp),%eax
 pushl (%esp)
 pushl %eax
 pushl %ecx
 pushl $4
 call _strike_observer_dispatch
 addl $16,%esp
 ret
.globl _strike_observer5
_strike_observer5:
 leal 4(%esp),%eax
 pushl (%esp)
 pushl %eax
 pushl %ecx
 pushl $5
 call _strike_observer_dispatch
 addl $16,%esp
 ret
.globl _strike_observer6
_strike_observer6:
 leal 4(%esp),%eax
 pushl (%esp)
 pushl %eax
 pushl %ecx
 pushl $6
 call _strike_observer_dispatch
 addl $16,%esp
 ret $4
.globl _strike_observer7
_strike_observer7:
 leal 4(%esp),%eax
 pushl (%esp)
 pushl %eax
 pushl %ecx
 pushl $7
 call _strike_observer_dispatch
 addl $16,%esp
 ret $4
.globl _strike_observer8
_strike_observer8:
 leal 4(%esp),%eax
 pushl (%esp)
 pushl %eax
 pushl %ecx
 pushl $8
 call _strike_observer_dispatch
 addl $16,%esp
 ret
.globl _strike_observer9
_strike_observer9:
 leal 4(%esp),%eax
 pushl (%esp)
 pushl %eax
 pushl %ecx
 pushl $9
 call _strike_observer_dispatch
 addl $16,%esp
 ret
.globl _strike_observer10
_strike_observer10:
 leal 4(%esp),%eax
 pushl (%esp)
 pushl %eax
 pushl %ecx
 pushl $10
 call _strike_observer_dispatch
 addl $16,%esp
 ret
.globl _strike_observer11
_strike_observer11:
 leal 4(%esp),%eax
 pushl (%esp)
 pushl %eax
 pushl %ecx
 pushl $11
 call _strike_observer_dispatch
 addl $16,%esp
 ret
.globl _strike_observer12
_strike_observer12:
 leal 4(%esp),%eax
 pushl (%esp)
 pushl %eax
 pushl %ecx
 pushl $12
 call _strike_observer_dispatch
 addl $16,%esp
 ret $4
.globl _strike_observer13
_strike_observer13:
 leal 4(%esp),%eax
 pushl (%esp)
 pushl %eax
 pushl %ecx
 pushl $13
 call _strike_observer_dispatch
 addl $16,%esp
 ret
.globl _strike_observer14
_strike_observer14:
 leal 4(%esp),%eax
 pushl (%esp)
 pushl %eax
 pushl %ecx
 pushl $14
 call _strike_observer_dispatch
 addl $16,%esp
 ret
.globl _strike_observer15
_strike_observer15:
 leal 4(%esp),%eax
 pushl (%esp)
 pushl %eax
 pushl %ecx
 pushl $15
 call _strike_observer_dispatch
 addl $16,%esp
 ret
.globl _strike_observer16
_strike_observer16:
 leal 4(%esp),%eax
 pushl (%esp)
 pushl %eax
 pushl %ecx
 pushl $16
 call _strike_observer_dispatch
 addl $16,%esp
 ret
.globl _strike_observer17
_strike_observer17:
 leal 4(%esp),%eax
 pushl (%esp)
 pushl %eax
 pushl %ecx
 pushl $17
 call _strike_observer_dispatch
 addl $16,%esp
 ret
.globl _strike_observer18
_strike_observer18:
 leal 4(%esp),%eax
 pushl (%esp)
 pushl %eax
 pushl %ecx
 pushl $18
 call _strike_observer_dispatch
 addl $16,%esp
 ret
