.syntax unified
.arch armv7-a
.fpu neon
.text
.global _start
.align 2

_start:
    @ 1. Alocação de 4KB na pilha
    sub sp, sp, #4096
    mov r4, sp

    @ 2. Constantes Phi e Sqrt2
    movw r0, #0xB851
    movt r0, #0x3FCE
    vmov s0, r0
    vdup.32 q0, d0[0]

    movw r1, #0x04F3
    movt r1, #0x3FB5
    vmov s1, r1
    vdup.32 q1, d1[0]

    @ 3. Iterações
    mov r6, #65536

    @ 4. Habilita contador de ciclos (PMCCNTR)
    mrc p15, 0, r2, c9, c14, 0
    orr r2, r2, #1
    mcr p15, 0, r2, c9, c14, 0
    mov r2, #0x80000000
    mcr p15, 0, r2, c9, c12, 1
    mov r2, #0
    mcr p15, 0, r2, c9, c13, 0
    mov r2, #1
    mcr p15, 0, r2, c9, c12, 0

    @ 5. Leitura do ciclo inicial
    mrc p15, 0, r7, c9, c13, 0

.L_LOOP:
    vld1.8 {q2, q3}, [r4]
    veor q4, q2, q3
    vcvt.f32.u32 q5, q4
    vmul.f32 q6, q5, q0
    vmul.f32 q7, q6, q1
    vcvt.u32.f32 q4, q7
    vst1.8 {q4, q5}, [r4]
    subs r6, r6, #1
    bne .L_LOOP

    @ 6. Leitura do ciclo final
    mrc p15, 0, r8, c9, c13, 0
    sub r7, r8, r7

    @ Desabilita contador
    mov r2, #0
    mcr p15, 0, r2, c9, c12, 0

    @ 7. Imprime cabeçalho
    mov r0, #1
    ldr r1, =msg_header
    ldr r2, =msg_header_len
    mov r7, #4
    svc #0

    @ 8. Converte ciclos para string
    adr r1, num_buffer
    mov r0, r7
    bl uint32_to_str

    @ 9. Imprime número
    mov r2, r0
    mov r0, #1
    adr r1, num_buffer
    mov r7, #4
    svc #0

    @ 10. Imprime rodapé
    mov r0, #1
    ldr r1, =msg_footer
    ldr r2, =msg_footer_len
    mov r7, #4
    svc #0

    @ 11. Exit
    mov r7, #1
    mov r0, #0
    svc #0

@ ============================================================
@ Função uint32_to_str (sem udiv)
@ ============================================================
uint32_to_str:
    push {r4-r7, lr}
    mov r4, r1
    add r5, r1, #10
    mov r6, #10

1:  mov r3, #0
2:  cmp r0, r6
    blo 3f
    sub r0, r0, r6
    add r3, r3, #1
    b 2b
3:  add r3, r3, #'0'
    strb r3, [r1], #1
    cmp r0, #0
    bne 1b

    sub r5, r1, #1
    mov r1, r4
4:  cmp r1, r5
    bge 5f
    ldrb r3, [r1]
    ldrb r7, [r5]
    strb r7, [r1], #1
    strb r3, [r5], #-1
    b 4b
5:  sub r0, r1, r4
    pop {r4-r7, pc}

.section .data
msg_header:      .ascii "[RAFAELIA-ARMv7] Ciclos gastos: "
msg_header_len = . - msg_header
msg_footer:      .ascii "\n☯ Execucao concluida.\n"
msg_footer_len = . - msg_footer

.section .bss
num_buffer:      .space 16
