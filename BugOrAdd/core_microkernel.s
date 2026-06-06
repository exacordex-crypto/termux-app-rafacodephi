
.global mk_scalar
.global mk_vector
.global mk_memory
.global mk_branch
.global mk_mix

mk_scalar:
    push {r4-r7, lr}
    mov r4, r0
1:
    add r5, r5, r6
    eor r5, r5, r7
    mul r5, r5, r6
    subs r4, r4, #1
    bgt 1b
    pop {r4-r7, pc}

mk_vector:
    push {r4-r11, lr}
    mov r4, r0
    vdup.32 q0, r1
    vdup.32 q1, r2
1:
    vadd.i32 q0, q0, q1
    vmul.i32 q1, q1, q0
    veor q0, q0, q1
    subs r4, r4, #1
    bgt 1b
    pop {r4-r11, pc}

mk_memory:
    push {r4-r11, lr}
    mov r4, r0
    ldr r5, =buffer
1:
    ldr r6, [r5, #0]
    ldr r7, [r5, #64]
    ldr r8, [r5, #512]
    ldr r9, [r5, #1024]
    add r6, r6, r7
    eor r6, r6, r8
    mul r6, r6, r9
    subs r4, r4, #1
    bgt 1b
    pop {r4-r11, pc}

mk_branch:
    push {r4-r7, lr}
    mov r4, r0
1:
    cmp r4, #0
    beq 2f
    tst r4, #1
    beq 3f
    add r5, r5, #1
    b 4f
3:
    sub r5, r5, #1
4:
    subs r4, r4, #1
    b 1b
2:
    pop {r4-r7, pc}

mk_mix:
    push {r4-r11, lr}
    mov r4, r0
1:
    add r5, r5, r6
    ldr r7, [r0, #32]
    eor r5, r5, r7
    mul r5, r5, r7
    vmov s0, r5
    vadd.f32 s0, s0, s0
    subs r4, r4, #1
    bgt 1b
    pop {r4-r11, pc}

