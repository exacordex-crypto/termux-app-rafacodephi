.arm                           @ força modo ARM (não Thumb)
.global core_run
.type core_run, %function

core_run:
    push {r4-r11, lr}

    mov r4, r0                  @ r4 = iteracoes

    @ Carrega sementes via literal pool (funciona sempre)
    ldr r5, =0x12345678
    ldr r6, =0x87654321
    ldr r7, =0xF00DBEEF
    ldr r8, =0x0BADCAFE
    ldr r9, =0xCAFEBABE
    ldr r10,=0xDEADBEEF
    ldr r11,=0x13579BDF
    ldr r12,=0x2468ACE0

    @ Endereço simbólico de "pinos" (array na memória)
    ldr r0, =pinos
    mov r1, #0                  @ contador de pinos

loop:
    @ Retroalimentação caótica entre registradores
    eor r5, r5, r5, lsl #13
    eor r5, r5, r5, lsr #17
    eor r5, r5, r5, lsl #5

    eor r6, r6, r6, lsl #11
    eor r6, r6, r6, lsr #19
    eor r6, r6, r6, lsl #3

    add r7, r7, r5
    eor r7, r7, r6
    mul r7, r7, r11

    add r8, r8, r6
    eor r8, r8, r5
    mul r8, r8, r12

    eor r9, r9, r7
    add r9, r9, r8
    mul r9, r9, r5

    eor r10, r10, r8
    add r10, r10, r7
    mul r10, r10, r6

    eor r11, r11, r9
    add r11, r11, r10

    eor r12, r12, r10
    add r12, r12, r9

    @ Escreve um valor nos "pinos" (simula GPIO)
    str r5, [r0, r1, lsl #2]    @ armazena r5 no pino[contador]
    add r1, r1, #1
    cmp r1, #8
    blt skip
    mov r1, #0                   @ reinicia contador
skip:

    subs r4, r4, #1
    bgt loop

    pop {r4-r11, pc}

.data
.align 4
pinos:
    .space 32                    @ 8 palavras (32 bytes)
