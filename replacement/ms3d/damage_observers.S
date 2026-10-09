.text
.globl _damage_observer0
_damage_observer0:
 pushl $0
 jmp _damage_observer_common
.globl _damage_script0
_damage_script0:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $0
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer1
_damage_observer1:
 pushl $1
 jmp _damage_observer_common
.globl _damage_script1
_damage_script1:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $1
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer2
_damage_observer2:
 pushl $2
 jmp _damage_observer_common
.globl _damage_script2
_damage_script2:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $2
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer3
_damage_observer3:
 pushl $3
 jmp _damage_observer_common
.globl _damage_script3
_damage_script3:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $3
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer4
_damage_observer4:
 pushl $4
 jmp _damage_observer_common
.globl _damage_script4
_damage_script4:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $4
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer5
_damage_observer5:
 pushl $5
 jmp _damage_observer_common
.globl _damage_script5
_damage_script5:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $5
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer6
_damage_observer6:
 pushl $6
 jmp _damage_observer_common
.globl _damage_script6
_damage_script6:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $6
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer7
_damage_observer7:
 pushl $7
 jmp _damage_observer_common
.globl _damage_script7
_damage_script7:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $7
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer8
_damage_observer8:
 pushl $8
 jmp _damage_observer_common
.globl _damage_script8
_damage_script8:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $8
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer9
_damage_observer9:
 pushl $9
 jmp _damage_observer_common
.globl _damage_script9
_damage_script9:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $9
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer10
_damage_observer10:
 pushl $10
 jmp _damage_observer_common
.globl _damage_script10
_damage_script10:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $10
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer11
_damage_observer11:
 pushl $11
 jmp _damage_observer_common
.globl _damage_script11
_damage_script11:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $11
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer12
_damage_observer12:
 pushl $12
 jmp _damage_observer_common
.globl _damage_script12
_damage_script12:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $12
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer13
_damage_observer13:
 pushl $13
 jmp _damage_observer_common
.globl _damage_script13
_damage_script13:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $13
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer14
_damage_observer14:
 pushl $14
 jmp _damage_observer_common
.globl _damage_script14
_damage_script14:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $14
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer15
_damage_observer15:
 pushl $15
 jmp _damage_observer_common
.globl _damage_script15
_damage_script15:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $15
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer16
_damage_observer16:
 pushl $16
 jmp _damage_observer_common
.globl _damage_script16
_damage_script16:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $16
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer17
_damage_observer17:
 pushl $17
 jmp _damage_observer_common
.globl _damage_script17
_damage_script17:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $17
 call _damage_script_dispatch
 addl $12,%esp
 ret $12
.globl _damage_observer18
_damage_observer18:
 pushl $18
 jmp _damage_observer_common
.globl _damage_script18
_damage_script18:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $18
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer19
_damage_observer19:
 pushl $19
 jmp _damage_observer_common
.globl _damage_script19
_damage_script19:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $19
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer20
_damage_observer20:
 pushl $20
 jmp _damage_observer_common
.globl _damage_script20
_damage_script20:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $20
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer21
_damage_observer21:
 pushl $21
 jmp _damage_observer_common
.globl _damage_script21
_damage_script21:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $21
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer22
_damage_observer22:
 pushl $22
 jmp _damage_observer_common
.globl _damage_script22
_damage_script22:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $22
 call _damage_script_dispatch
 addl $12,%esp
 ret $12
.globl _damage_observer23
_damage_observer23:
 pushl $23
 jmp _damage_observer_common
.globl _damage_script23
_damage_script23:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $23
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer24
_damage_observer24:
 pushl $24
 jmp _damage_observer_common
.globl _damage_script24
_damage_script24:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $24
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer25
_damage_observer25:
 pushl $25
 jmp _damage_observer_common
.globl _damage_script25
_damage_script25:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $25
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer26
_damage_observer26:
 pushl $26
 jmp _damage_observer_common
.globl _damage_script26
_damage_script26:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $26
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer27
_damage_observer27:
 pushl $27
 jmp _damage_observer_common
.globl _damage_script27
_damage_script27:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $27
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer28
_damage_observer28:
 pushl $28
 jmp _damage_observer_common
.globl _damage_script28
_damage_script28:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $28
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer29
_damage_observer29:
 pushl $29
 jmp _damage_observer_common
.globl _damage_script29
_damage_script29:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $29
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer30
_damage_observer30:
 pushl $30
 jmp _damage_observer_common
.globl _damage_script30
_damage_script30:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $30
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer31
_damage_observer31:
 pushl $31
 jmp _damage_observer_common
.globl _damage_script31
_damage_script31:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $31
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer32
_damage_observer32:
 pushl $32
 jmp _damage_observer_common
.globl _damage_script32
_damage_script32:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $32
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer33
_damage_observer33:
 pushl $33
 jmp _damage_observer_common
.globl _damage_script33
_damage_script33:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $33
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer34
_damage_observer34:
 pushl $34
 jmp _damage_observer_common
.globl _damage_script34
_damage_script34:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $34
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer35
_damage_observer35:
 pushl $35
 jmp _damage_observer_common
.globl _damage_script35
_damage_script35:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $35
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer36
_damage_observer36:
 pushl $36
 jmp _damage_observer_common
.globl _damage_script36
_damage_script36:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $36
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer37
_damage_observer37:
 pushl $37
 jmp _damage_observer_common
.globl _damage_script37
_damage_script37:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $37
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer38
_damage_observer38:
 pushl $38
 jmp _damage_observer_common
.globl _damage_script38
_damage_script38:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $38
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer39
_damage_observer39:
 pushl $39
 jmp _damage_observer_common
.globl _damage_script39
_damage_script39:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $39
 call _damage_script_dispatch
 addl $12,%esp
 ret $12
.globl _damage_observer40
_damage_observer40:
 pushl $40
 jmp _damage_observer_common
.globl _damage_script40
_damage_script40:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $40
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer41
_damage_observer41:
 pushl $41
 jmp _damage_observer_common
.globl _damage_script41
_damage_script41:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $41
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer42
_damage_observer42:
 pushl $42
 jmp _damage_observer_common
.globl _damage_script42
_damage_script42:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $42
 call _damage_script_dispatch
 addl $12,%esp
 ret $12
.globl _damage_observer43
_damage_observer43:
 pushl $43
 jmp _damage_observer_common
.globl _damage_script43
_damage_script43:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $43
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer44
_damage_observer44:
 pushl $44
 jmp _damage_observer_common
.globl _damage_script44
_damage_script44:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $44
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer45
_damage_observer45:
 pushl $45
 jmp _damage_observer_common
.globl _damage_script45
_damage_script45:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $45
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer46
_damage_observer46:
 pushl $46
 jmp _damage_observer_common
.globl _damage_script46
_damage_script46:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $46
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer47
_damage_observer47:
 pushl $47
 jmp _damage_observer_common
.globl _damage_script47
_damage_script47:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $47
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer48
_damage_observer48:
 pushl $48
 jmp _damage_observer_common
.globl _damage_script48
_damage_script48:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $48
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer49
_damage_observer49:
 pushl $49
 jmp _damage_observer_common
.globl _damage_script49
_damage_script49:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $49
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer50
_damage_observer50:
 pushl $50
 jmp _damage_observer_common
.globl _damage_script50
_damage_script50:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $50
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer51
_damage_observer51:
 pushl $51
 jmp _damage_observer_common
.globl _damage_script51
_damage_script51:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $51
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer52
_damage_observer52:
 pushl $52
 jmp _damage_observer_common
.globl _damage_script52
_damage_script52:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $52
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer53
_damage_observer53:
 pushl $53
 jmp _damage_observer_common
.globl _damage_script53
_damage_script53:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $53
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer54
_damage_observer54:
 pushl $54
 jmp _damage_observer_common
.globl _damage_script54
_damage_script54:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $54
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer55
_damage_observer55:
 pushl $55
 jmp _damage_observer_common
.globl _damage_script55
_damage_script55:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $55
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer56
_damage_observer56:
 pushl $56
 jmp _damage_observer_common
.globl _damage_script56
_damage_script56:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $56
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer57
_damage_observer57:
 pushl $57
 jmp _damage_observer_common
.globl _damage_script57
_damage_script57:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $57
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer58
_damage_observer58:
 pushl $58
 jmp _damage_observer_common
.globl _damage_script58
_damage_script58:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $58
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer59
_damage_observer59:
 pushl $59
 jmp _damage_observer_common
.globl _damage_script59
_damage_script59:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $59
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer60
_damage_observer60:
 pushl $60
 jmp _damage_observer_common
.globl _damage_script60
_damage_script60:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $60
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer61
_damage_observer61:
 pushl $61
 jmp _damage_observer_common
.globl _damage_script61
_damage_script61:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $61
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer62
_damage_observer62:
 pushl $62
 jmp _damage_observer_common
.globl _damage_script62
_damage_script62:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $62
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer63
_damage_observer63:
 pushl $63
 jmp _damage_observer_common
.globl _damage_script63
_damage_script63:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $63
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer64
_damage_observer64:
 pushl $64
 jmp _damage_observer_common
.globl _damage_script64
_damage_script64:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $64
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer65
_damage_observer65:
 pushl $65
 jmp _damage_observer_common
.globl _damage_script65
_damage_script65:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $65
 call _damage_script_dispatch
 addl $12,%esp
 ret
.globl _damage_observer66
_damage_observer66:
 pushl $66
 jmp _damage_observer_common
.globl _damage_script66
_damage_script66:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $66
 call _damage_script_dispatch
 addl $12,%esp
 ret $12
.globl _damage_observer67
_damage_observer67:
 pushl $67
 jmp _damage_observer_common
.globl _damage_script67
_damage_script67:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $67
 call _damage_script_dispatch
 addl $12,%esp
 ret $12
.globl _damage_observer68
_damage_observer68:
 pushl $68
 jmp _damage_observer_common
.globl _damage_script68
_damage_script68:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $68
 call _damage_script_dispatch
 addl $12,%esp
 ret $12
.globl _damage_observer69
_damage_observer69:
 pushl $69
 jmp _damage_observer_common
.globl _damage_script69
_damage_script69:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $69
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer70
_damage_observer70:
 pushl $70
 jmp _damage_observer_common
.globl _damage_script70
_damage_script70:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $70
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_observer71
_damage_observer71:
 pushl $71
 jmp _damage_observer_common
.globl _damage_script71
_damage_script71:
 leal 4(%esp),%eax
 pushl %eax
 pushl %ecx
 pushl $71
 call _damage_script_dispatch
 addl $12,%esp
 ret $4
.globl _damage_entry
_damage_entry:
 pushl $-1
 jmp _damage_observer_common
_damage_observer_common:
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
 call _damage_observer_before
 addl $8,%esp
 movl %eax,36(%ebp)
 movl $_damage_observer_after,40(%ebp)
 fxrstor (%esp)
 fldenv 512(%esp)
 movl %ebp,%esp
 popal
 popfl
 ret
_damage_observer_after:
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
 call _damage_observer_return
 addl $8,%esp
 movl %eax,36(%ebp)
 fxrstor (%esp)
 fldenv 512(%esp)
 movl %ebp,%esp
 popal
 popfl
 ret
