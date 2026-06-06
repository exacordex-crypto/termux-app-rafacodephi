.global core_neon
.type core_neon, %function

core_neon:
    push {r4-r7, lr}

    mov r4, r0

loop:
    vadd.i32 q0, q0, q1
    vadd.i32 q2, q2, q3
    veor q4, q4, q5

    subs r4, r4, #1
    bne loop

    pop {r4-r7, pc}
