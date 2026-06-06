.global core_entropy
.type core_entropy, %function

core_entropy:
    push {r4-r12, lr}

    mov r4, r0

    mov r5, #0x12345678
    mov r6, #0x87654321
    mov r7, #0xF00DBEEF
    mov r8, #0x0BADCAFE

loop:
    eor r5, r5, r5, lsl #13
    eor r5, r5, r5, lsr #17
    eor r5, r5, r5, lsl #5

    eor r6, r6, r6, lsl #11
    eor r6, r6, r6, lsr #19

    add r7, r7, r5
    eor r7, r7, r6

    add r8, r8, r7
    eor r8, r8, r5

    subs r4, r4, #1
    bne loop

    pop {r4-r12, pc}
