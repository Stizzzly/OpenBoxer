// Independently authored stdcall color-call bridge. Arguments are raw float32
// bit patterns; the final semantically-void driver's EAX residue is observed.
.text
.globl _lighting_color_bits
_lighting_color_bits:
    pushl %ebp
    movl %esp, %ebp
    pushl 24(%ebp)
    pushl 20(%ebp)
    pushl 16(%ebp)
    pushl 12(%ebp)
    call *8(%ebp)
    leave
    ret
