.global core_scalar
.type core_scalar, %function

core_scalar:
    push {r4-r7, lr}

    mov r4, r0

loop:
    add r5, r5, r6   @ dependency chain
    add r5, r5, r6
    add r5, r5, r6

    subs r4, r4, #1
    bne loop

    pop {r4-r7, pc}
