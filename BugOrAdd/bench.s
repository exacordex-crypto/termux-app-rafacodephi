.arm
.text
.global _start

@ syscalls
.equ SYS_WRITE, 4
.equ SYS_EXIT, 1
.equ SYS_CLOCK_GETTIME, 263
.equ STDOUT, 1
.equ CLOCK_MONOTONIC, 1

_start:
    @ buffer de 32KB na pilha
    sub sp, sp, #32768
    mov r12, sp

    @ timespec (16 bytes) no topo da pilha (após buffer)
    sub sp, sp, #16
    mov r1, sp

    @ tempo inicial
    mov r0, #CLOCK_MONOTONIC
    mov r7, #SYS_CLOCK_GETTIME
    svc 0
    ldr r4, [r1]           @ segundos inicio
    ldr r5, [r1, #4]       @ ns inicio

    @ ---------- LOOP ESCALAR (dependência) ----------
    ldr r6, =20000000
    mov r8, #1
    mov r9, #2
1:
    add r8, r8, r9
    eor r9, r9, r8
    mul r8, r8, r9
    subs r6, r6, #1
    bne 1b

    @ ---------- LOOP ILP (4 lanes independentes) ----------
    ldr r6, =20000000
    mov r8, #1
    mov r9, #2
    mov r10, #3
    mov r11, #4
2:
    add r8, r8, #1
    add r9, r9, #2
    add r10, r10, #3
    add r11, r11, #4
    subs r6, r6, #1
    bne 2b

    @ ---------- LOOP MEMÓRIA (stride 1,8,64) ----------
    ldr r6, =20000000
3:
    ldr r0, [r12, r6, lsl #0]   @ stride 1
    ldr r0, [r12, r6, lsl #3]   @ stride 8
    ldr r0, [r12, r6, lsl #6]   @ stride 64
    subs r6, r6, #1
    bne 3b

    @ ---------- LOOP BRANCH (alternado) ----------
    ldr r6, =20000000
    mov r8, #0
4:
    tst r6, #1
    addne r8, r8, r6
    subeq r8, r8, r6
    subs r6, r6, #1
    bne 4b

    @ ---------- TEMPO FINAL ----------
    mov r0, #CLOCK_MONOTONIC
    mov r7, #SYS_CLOCK_GETTIME
    svc 0
    ldr r6, [r1]           @ segundos fim
    ldr r7, [r1, #4]       @ ns fim
    add sp, sp, #16        @ libera timespec
    add sp, sp, #32768     @ libera buffer

    @ diferença (r6,r7) - (r4,r5)
    sub r6, r6, r4
    sub r7, r7, r5
    cmp r7, #0
    bge 5f
    add r7, r7, #1000000000
    sub r6, r6, #1
5:

    @ ---------- IMPRIMIR "Tempo: " ----------
    adr r0, msg_t
    mov r1, #len_t
    mov r2, #STDOUT
    mov r7, #SYS_WRITE
    svc 0

    @ imprimir segundos (decimal)
    mov r0, r6
    bl print_decimal

    @ imprimir " segundos, "
    adr r0, msg_s
    mov r1, #len_s
    mov r7, #SYS_WRITE
    svc 0

    @ imprimir nanossegundos (decimal)
    mov r0, r7
    bl print_decimal

    @ imprimir " ns\n"
    adr r0, msg_n
    mov r1, #len_n
    mov r7, #SYS_WRITE
    svc 0

    @ ---------- MAPEAMENTO DE ERRO (descomente para crash) ----------
    @ mov r0, #0
    @ ldr r0, [r0]

    @ saída normal
    mov r0, #0
    mov r7, #SYS_EXIT
    svc 0

@ ------------------------------------------------------------
@ print_decimal: imprime inteiro em r0 (sem sinal, decimal)
@ usa subtração por 10 (lento mas seguro)
@ ------------------------------------------------------------
print_decimal:
    push {r4, r5, r6, lr}
    sub sp, sp, #32
    mov r4, sp          @ ponteiro para a pilha de caracteres
    mov r5, #10
    mov r6, r0
    cmp r6, #0
    bne 1f
    mov r0, #'0'
    strb r0, [r4], #1
    b 3f
1:
    @ empilha dígitos (do menos significativo)
2:
    mov r0, r6
    mov r1, #0
    @ divisão manual por 10: r0 = quociente, r1 = resto
    bl div10
    add r1, r1, #'0'
    strb r1, [r4], #1
    mov r6, r0
    cmp r6, #0
    bne 2b
3:
    @ inverter a string
    mov r0, sp
    sub r1, r4, sp      @ tamanho
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
    @ escrever
    mov r0, #STDOUT
    mov r1, sp
    mov r2, r2
    mov r7, #SYS_WRITE
    svc 0
    add sp, sp, #32
    pop {r4, r5, r6, pc}

@ ------------------------------------------------------------
@ div10: r0 = dividendo, retorna r0 = quociente, r1 = resto
@ ------------------------------------------------------------
div10:
    mov r1, #0
    mov r2, #10
1:
    cmp r0, r2
    blt 2f
    sub r0, r0, r2
    add r1, r1, #1
    b 1b
2:
    mov r1, r0          @ resto
    mov r0, r1          @ quociente (errado, na verdade r0 é o resto aqui)
    @ Vamos corrigir: após o loop, r0 contém o resto (<10), r1 contém o quociente.
    @ A troca correta:
    mov r2, r0
    mov r0, r1
    mov r1, r2
    bx lr

@ ------------------------------------------------------------
@ Dados (strings)
@ ------------------------------------------------------------
.data
msg_t: .ascii "Tempo: "
len_t = . - msg_t
msg_s: .ascii " segundos, "
len_s = . - msg_s
msg_n: .ascii " ns\n"
len_n = . - msg_n
