.syntax unified
.arm

.equ SYS_EXIT,         1
.equ SYS_WRITE,        4
.equ SYS_MMAP2,        192
.equ SYS_CLONE,        120
.equ SYS_WAIT4,        114
.equ SYS_GETTIMEOFDAY, 78

.equ STDOUT,           1
.equ SIGCHLD,          17
.equ N_WORKERS,        4

.text
.align 2
.global _start
.type _start,%function

_start:
    adr r0, msg_boot
    mov r1, #26
    bl ws

    @ mmap shared work+crc area: 0x40000 + 16
    mov r0, #0
    mov r1, #1
    lsl r1, r1, #18          @ 256 KB
    add r1, r1, #16
    mov r2, #3               @ PROT_READ|WRITE
    mov r3, #0x21            @ MAP_SHARED|MAP_ANON
    mvn r4, #0               @ fd=-1
    mov r5, #0
    mov r7, #SYS_MMAP2
    swi #0

    mov r10, r0              @ work base
    mov r11, r10
    mov r1, #1
    lsl r1, r1, #18
    add r11, r11, r1         @ crc result base

    @ fill fibonacci pattern
    mov r4, r10
    mov r5, #1
    lsl r5, r5, #18
    bl fill_fib

    @ start time no stack/global: use current sp scratch
    sub sp, sp, #16
    mov r9, sp               @ start tv
    add r6, sp, #8           @ end tv

    mov r0, r9
    mov r1, #0
    mov r7, #SYS_GETTIMEOFDAY
    swi #0

    mov r8, #0

spawn_loop:
    cmp r8, #N_WORKERS
    beq spawn_done

    @ mmap child stack private 16 KB
    mov r0, #0
    mov r1, #1
    lsl r1, r1, #14
    mov r2, #3
    mov r3, #0x22
    mvn r4, #0
    mov r5, #0
    mov r7, #SYS_MMAP2
    swi #0

    mov r1, #1
    lsl r1, r1, #14
    add r1, r0, r1           @ stack top

    mov r0, #SIGCHLD
    mov r7, #SYS_CLONE
    swi #0

    cmp r0, #0
    beq child_path

    add r8, r8, #1
    b spawn_loop

child_path:
    mov r5, #1
    lsl r5, r5, #16          @ 64 KB per worker

    mov r0, r8
    mul r0, r5, r0
    add r0, r10, r0
    mov r1, r5
    bl crc32_bitwise

    str r0, [r11, r8, lsl #2]

    ldr r0, =msg_worker
    mov r1, #2
    bl ws

    mov r0, r8
    bl print_nibble

    mov r0, #0
    mov r7, #SYS_EXIT
    swi #0

spawn_done:
    mov r8, #N_WORKERS

wait_loop:
    cmp r8, #0
    beq wait_done

    mvn r0, #0
    mov r1, #0
    mov r2, #0
    mov r3, #0
    mov r7, #SYS_WAIT4
    swi #0

    subs r8, r8, #1
    b wait_loop

wait_done:
    mov r0, r6
    mov r1, #0
    mov r7, #SYS_GETTIMEOFDAY
    swi #0

    @ delta usec
    ldmia r6, {r0, r1}
    ldmia r9, {r2, r3}
    sub r0, r0, r2
    sub r1, r1, r3
    movw r4, #0x4240
    movt r4, #0x000F        @ 1,000,000
    mul r0, r0, r4
    add r0, r0, r1

    push {r0}
    adr r0, msg_us
    mov r1, #3
    bl ws
    pop {r0}
    bl print_hex32

    @ xor crc results
    ldr r0, [r11, #0]
    ldr r1, [r11, #4]
    ldr r2, [r11, #8]
    ldr r3, [r11, #12]
    eor r0, r0, r1
    eor r0, r0, r2
    eor r0, r0, r3

    push {r0}
    ldr r0, =msg_crc
    mov r1, #4
    bl ws
    pop {r0}
    bl print_hex32

    ldr r0, =msg_done
    mov r1, #12
    bl ws

    add sp, sp, #16
    mov r0, #0
    mov r7, #SYS_EXIT
    swi #0

fill_fib:
    push {r4,r5,r6,r7,lr}
    mov r6, #1
    mov r7, #1
    mov r0, #0
fib_loop:
    cmp r0, r5
    bge fib_done
    add r2, r6, r7
    uxth r2, r2
    strh r2, [r4, r0]
    mov r6, r7
    mov r7, r2
    add r0, r0, #2
    b fib_loop
fib_done:
    pop {r4,r5,r6,r7,pc}

crc32_bitwise:
    push {r4,r5,r6,r7,r8,lr}
    mvn r6, #0
    mov r4, r0
    mov r5, r1
crc_byte:
    cmp r5, #0
    beq crc_done
    ldrb r7, [r4], #1
    eor r6, r6, r7
    mov r8, #8
crc_bit:
    tst r6, #1
    lsr r6, r6, #1
    eorne r6, r6, #0xED000000
    eorne r6, r6, #0x00B80000
    eorne r6, r6, #0x00008300
    eorne r6, r6, #0x00000020
    subs r8, r8, #1
    bne crc_bit
    subs r5, r5, #1
    b crc_byte
crc_done:
    mvn r0, r6
    pop {r4,r5,r6,r7,r8,pc}

print_nibble:
    push {r4,lr}
    adr r1, hex
    and r0, r0, #0xF
    ldrb r0, [r1, r0]
    sub sp, sp, #8
    strb r0, [sp]
    mov r4, #'\n'
    strb r4, [sp,#1]
    mov r0, sp
    mov r1, #2
    bl ws
    add sp, sp, #8
    pop {r4,pc}

print_hex32:
    push {r4,r5,r6,lr}
    sub sp, sp, #16
    mov r4, r0
    mov r5, sp
    adr r6, hex
    mov r1, #28
hex_loop:
    lsr r0, r4, r1
    and r0, r0, #0xF
    ldrb r0, [r6, r0]
    strb r0, [r5], #1
    subs r1, r1, #4
    bge hex_loop
    mov r0, #'\n'
    strb r0, [r5]
    mov r0, sp
    mov r1, #9
    bl ws
    add sp, sp, #16
    pop {r4,r5,r6,pc}

ws:
    push {lr}
    mov r2, r1
    mov r1, r0
    mov r0, #STDOUT
    mov r7, #SYS_WRITE
    swi #0
    pop {pc}

msg_boot:   .ascii "B3_PIE: MULTICORE+CRC32SW\n"
.equ msg_boot_len, . - msg_boot
msg_worker: .ascii "W:"
.equ msg_worker_len, . - msg_worker
msg_us:     .ascii "US="
.equ msg_us_len, . - msg_us
msg_crc:    .ascii "CRC="
.equ msg_crc_len, . - msg_crc
msg_done:   .ascii "B3_PIE:DONE\n"
.equ msg_done_len, . - msg_done
hex:        .ascii "0123456789ABCDEF"
