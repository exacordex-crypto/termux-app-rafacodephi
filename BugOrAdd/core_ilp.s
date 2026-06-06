.global core_ilp
.type core_ilp, %function

core_ilp:
    push {r4-r7, lr}

    mov r4, r0

loop:
    add r5, r5, r6
    add r7, r7, r8
    add r9, r9, r10
    add r11, r11, r12

    subs r4, r4, #1
    bne loop

    pop {r4-r7, pc}
