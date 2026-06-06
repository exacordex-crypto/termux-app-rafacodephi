@ ============================================================
@ GEO SEED CORE v2 - ARMv7
@ Estado + Memória + Feedback real
@ ============================================================

.global _start

.equ ARENA_SIZE, 65536

.data
arena_buf:
    .space ARENA_SIZE

state_counter:
    .word 0

write_cursor:
    .word 0

.text

_start:
    bl arena_init

main_loop:
    bl tick_state
    bl memory_feedback
    bl write_event_stream
    b main_loop


arena_init:
    ldr r0, =state_counter
    mov r1, #0
    str r1, [r0]

    ldr r0, =write_cursor
    str r1, [r0]
    bx lr


arena_alloc:
    ldr r1, =write_cursor
    ldr r2, [r1]

    add r3, r2, r0
    cmp r3, #ARENA_SIZE
    bge alloc_fail

    str r3, [r1]

    ldr r0, =arena_buf
    add r0, r0, r2
    bx lr

alloc_fail:
    mov r0, #0
    bx lr


tick_state:
    ldr r0, =state_counter
    ldr r1, [r0]

    add r2, r1, r1, lsl #2
    eor r2, r2, r1, lsr #3
    add r2, r2, #17

    str r2, [r0]
    bx lr


write_event_stream:
    ldr r0, =state_counter
    ldr r1, [r0]

    eor r2, r1, r1, lsr #5
    add r2, r2, r1, lsl #1

    mov r0, #4
    bl arena_alloc

    cmp r0, #0
    beq end_write

    str r2, [r0]

end_write:
    bx lr


memory_feedback:
    ldr r0, =write_cursor
    ldr r1, [r0]

    cmp r1, #4
    blt no_feedback

    sub r1, r1, #4

    ldr r2, =arena_buf
    add r2, r2, r1
    ldr r3, [r2]

    ldr r0, =state_counter
    ldr r4, [r0]

    eor r4, r4, r3
    str r4, [r0]

no_feedback:
    bx lr
