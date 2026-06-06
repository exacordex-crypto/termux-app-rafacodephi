.arm
.text
.global _start

@ syscalls
.equ SYS_WRITE, 4
.equ SYS_EXIT, 1
.equ SYS_CLOCK_GETTIME, 263
.equ STDOUT, 1
.equ CLOCK_MONOTONIC, 1

@ constantes
.equ ITERS,   20000000
.equ SAMPLES, 7

_start:
    @ reserva timespec na pilha
    sub sp, sp, #16
    mov r9, sp

    @ medir kernel escalar
    bl medir_escalar

    @ saída normal
    mov r0, #0
    mov r7, #SYS_EXIT
    svc 0

medir_escalar:
    push {r4-r11, lr}
    sub sp, sp, #(SAMPLES*4)
    mov r10, sp
    mov r11, #0

1:
    mov r0, #CLOCK_MONOTONIC
    mov r1, r9
    mov r7, #SYS_CLOCK_GETTIME
    svc 0
    ldr r4, [r9]
    ldr r5, [r9, #4]

    ldr r6, =ITERS
    mov r0, #1
    mov r1, #2
2:
    add r0, r0, r1
    eor r1, r1, r0
    mul r0, r0, r1
    subs r6, r6, #1
    bne 2b

    mov r0, #CLOCK_MONOTONIC
    mov r1, r9
    mov r7, #SYS_CLOCK_GETTIME
    svc 0
    ldr r6, [r9]
    ldr r7, [r9, #4]
    sub r6, r6, r4
    sub r7, r7, r5
    cmp r7, #0
    bge 3f
    ldr r0, =1000000000
    add r7, r7, r0
    sub r6, r6, #1
3:
    ldr r0, =1000000000
    mul r6, r6, r0
    add r6, r6, r7
    str r6, [r10, r11, lsl #2]
    add r11, r11, #1
    cmp r11, #SAMPLES
    blt 1b

    @ média
    mov r11, #0
    mov r0, #0
4:
    ldr r1, [r10, r11, lsl #2]
    add r0, r0, r1
    add r11, r11, #1
    cmp r11, #SAMPLES
    blt 4b
    mov r1, #SAMPLES
    bl div_u32
    mov r4, r0

    @ desvio
    mov r11, #0
    mov r5, #0
5:
    ldr r1, [r10, r11, lsl #2]
    sub r1, r1, r4
    mul r1, r1, r1
    add r5, r5, r1
    add r11, r11, #1
    cmp r11, #SAMPLES
    blt 5b
    mov r0, r5
    mov r1, #SAMPLES
    bl div_u32
    bl sqrt_u32
    mov r6, r0

    @ imprimir
    adr r0, msg_esc
    bl print_str
    mov r0, r4
    bl print_u32
    adr r0, msg_ns
    bl print_str
    mov r0, r6
    bl print_u32
    adr r0, msg_ns2
    bl print_str

    add sp, sp, #(SAMPLES*4)
    pop {r4-r11, pc}

@ ------------------------------------------------------------
@ Funções auxiliares
@ ------------------------------------------------------------
print_str:
    mov r1, r0
1:
    ldrb r2, [r1], #1
    cmp r2, #0
    bne 1b
    sub r2, r1, r0
    sub r2, r2, #1
    mov r0, #STDOUT
    mov r1, r0
    mov r7, #SYS_WRITE
    svc 0
    bx lr

print_u32:
    push {r4, r5, lr}
    sub sp, sp, #32
    mov r4, sp
    mov r5, #10
    mov r6, r0
    cmp r6, #0
    bne 1f
    mov r0, #'0'
    strb r0, [r4], #1
    b 3f
1:
2:
    mov r0, r6
    bl div10_resto
    add r1, r1, #'0'
    strb r1, [r4], #1
    mov r6, r0
    cmp r6, #0
    bne 2b
3:
    mov r0, sp
    sub r1, r4, sp
    mov r2, r1
    sub r1, r1, #1
    mov r3, #0
4:
    ldrb r5, [r0, r3]
    ldrb r6, [r0, r1]
    strb r6, [r0, r3]
    strb r5, [r0, r1]
    add r3, r3, #1
    sub r1, r1, #1
    cmp r3, r1
    blt 4b
    mov r0, #STDOUT
    mov r1, sp
    mov r2, r2
    mov r7, #SYS_WRITE
    svc 0
    add sp, sp, #32
    pop {r4, r5, pc}

div10_resto:   @ r0 = dividendo, retorna r0 = quociente, r1 = resto
    mov r1, #0
    mov r2, #10
1:
    cmp r0, r2
    blt 2f
    sub r0, r0, r2
    add r1, r1, #1
    b 1b
2:
    mov r2, r0
    mov r0, r1
    mov r1, r2
    bx lr

div_u32:        @ r0 / r1 -> r0 quociente, r1 resto
    mov r2, #0
    mov r3, #32
1:
    lsls r0, r0, #1
    adc  r2, r2, r2
    cmp  r2, r1
    subge r2, r2, r1
    addge r0, r0, #1
    subs r3, r3, #1
    bne 1b
    mov r1, r2
    bx lr

sqrt_u32:       @ r0 = valor, retorna r0 = raiz inteira (aproximada)
    mov r1, #0
    mov r2, #0x4000
1:
    add r3, r1, r2
    mul r3, r3, r3
    cmp r3, r0
    subhi r2, r2, #0x800
    addls r1, r1, r2
    lsr r2, r2, #1
    cmp r2, #0
    bne 1b
    mov r0, r1
    bx lr

.data
msg_esc: .asciz "Escalar: media="
msg_ns:  .asciz " ns, desvio="
msg_ns2: .asciz " ns\n"
