// Independently authored calling-convention test glue. No original instructions
// are used. The caller's four nonvolatile registers are preserved while sentinels
// are installed around a cdecl function taking three 32-bit argument slots.
.text
.globl _world_abi_probe
_world_abi_probe:
    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    subl $8, %esp
    movl 44(%esp), %eax
    movl %eax, 4(%esp)
    movl %esp, 0(%eax)
    movl 28(%esp), %eax
    pushl 40(%esp)
    pushl 40(%esp)
    pushl 40(%esp)
    movl $0x13579bdf, %ebp
    movl $0x2468ace0, %ebx
    movl $0x55aa55aa, %esi
    movl $0xaa55aa55, %edi
    call *%eax
    addl $12, %esp
    movl %eax, 0(%esp)
    movl 4(%esp), %eax
    movl %esp, 4(%eax)
    movl %ebp, 8(%eax)
    movl %ebx, 12(%eax)
    movl %esi, 16(%eax)
    movl %edi, 20(%eax)
    movl 0(%esp), %ecx
    movl %ecx, 24(%eax)
    movl %ecx, %eax
    addl $8, %esp
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret
