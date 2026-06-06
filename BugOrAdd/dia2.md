; ============================================================================
; VECTRA BENCHMARK INDUSTRIAL – x86-64 Assembly (CORRIGIDO)
; Correções:
;   ✓ print_string preserva ponteiro
;   ✓ print_decimal seguro
;   ✓ CRC32 corrigido
;   ✓ mix_buffer sem OOB
;   ✓ sum/sum_sq 64-bit
;   ✓ RDTSC serializado
;   ✓ média/desvio inicial
;
; Compilar:
;   nasm -f elf64 benchmark_x86_64_fixed.asm -o benchmark_x86_64_fixed.o
;   ld -e main benchmark_x86_64_fixed.o -o benchmark_x86_64_fixed
; ============================================================================

bits 64
default rel

; ============================================================================
; CONFIG
; ============================================================================
N_SAMPLES       equ 56
BUFFER_SIZE     equ 4096
CRC32_POLY      equ 0xEDB88320

SYS_WRITE       equ 1
SYS_EXIT        equ 60

STDOUT          equ 1

; ============================================================================
; DATA
; ============================================================================
section .data align=64

cpu_has_avx2        dq 0
cpu_has_sse4_2      dq 0
cpu_has_popcnt      dq 0

samples_count       dq 0

stat_sum            dq 0
stat_sum_sq         dq 0

stat_min            dd 0xFFFFFFFF
stat_max            dd 0

cycles_before       dq 0
cycles_after        dq 0

str_header db 10
db "==============================================",10
db " VECTRA BENCHMARK INDUSTRIAL x86-64",10
db "==============================================",10,0

str_cpuid db "[CPUID]",10,0
str_avx2 db "  AVX2 OK",10,0
str_sse42 db "  SSE4.2 OK",10,0
str_popcnt db "  POPCNT OK",10,0

str_collect db 10,"[COLLECT]",10,0
str_stats db 10,"[STATS]",10,0

str_samples db "Samples: ",0
str_mean db "Mean: ",0
str_min db "Min: ",0
str_max db "Max: ",0

newline db 10,0

; ============================================================================
; BSS
; ============================================================================
section .bss align=64

work_buffer         resb BUFFER_SIZE
samples             resd N_SAMPLES

tmp_decimal         resb 64

; ============================================================================
; TEXT
; ============================================================================
section .text

global main

; ============================================================================
; MAIN
; ============================================================================
main:

    push rbp
    mov rbp, rsp

    ; --------------------------------
    ; SERIALIZED RDTSC (START)
    ; --------------------------------
    xor eax, eax
    cpuid
    rdtsc

    shl rdx, 32
    or rax, rdx

    mov [cycles_before], rax

    ; --------------------------------
    ; HEADER
    ; --------------------------------
    lea rdi, [str_header]
    call print_string

    ; --------------------------------
    ; CPUID
    ; --------------------------------
    lea rdi, [str_cpuid]
    call print_string

    call cpuid_detect

    ; --------------------------------
    ; COLLECT
    ; --------------------------------
    lea rdi, [str_collect]
    call print_string

    call init_buffer
    call collect_samples

    ; --------------------------------
    ; STATS
    ; --------------------------------
    lea rdi, [str_stats]
    call print_string

    call compute_basic_stats
    call print_report

    ; --------------------------------
    ; SERIALIZED RDTSC (END)
    ; --------------------------------
    rdtscp
    shl rdx, 32
    or rax, rdx

    mov [cycles_after], rax

    xor eax, eax
    leave
    ret

; ============================================================================
; CPUID DETECT
; ============================================================================
cpuid_detect:

    push rbx

    ; SSE4.2 / POPCNT
    mov eax, 1
    xor ecx, ecx
    cpuid

    test ecx, (1 << 20)
    jz .skip_sse

    mov qword [cpu_has_sse4_2], 1

    lea rdi, [str_sse42]
    call print_string

.skip_sse:

    test ecx, (1 << 23)
    jz .skip_pop

    mov qword [cpu_has_popcnt], 1

    lea rdi, [str_popcnt]
    call print_string

.skip_pop:

    ; AVX2
    mov eax, 7
    xor ecx, ecx
    cpuid

    test ebx, (1 << 5)
    jz .done

    mov qword [cpu_has_avx2], 1

    lea rdi, [str_avx2]
    call print_string

.done:

    pop rbx
    ret

; ============================================================================
; INIT BUFFER
; ============================================================================
init_buffer:

    push rax
    push rcx
    push rdi

    lea rdi, [work_buffer]

    mov rcx, BUFFER_SIZE / 8

    mov rax, 0x9E3779B97F4A7C15

.loop:

    mov [rdi], rax

    add rax, 0x517CC1B727220A95

    add rdi, 8

    loop .loop

    pop rdi
    pop rcx
    pop rax

    ret

; ============================================================================
; COLLECT SAMPLES
; ============================================================================
collect_samples:

    push rbx
    push rcx
    push rdi

    xor ecx, ecx

.loop:

    cmp ecx, N_SAMPLES
    jge .done

    lea rsi, [work_buffer]
    call crc32_buffer

    mov [samples + rcx*4], eax

    lea rsi, [work_buffer]
    call mix_buffer

    inc ecx
    jmp .loop

.done:

    mov qword [samples_count], N_SAMPLES

    pop rdi
    pop rcx
    pop rbx

    ret

; ============================================================================
; CRC32 BUFFER
; RSI = buffer
; RETURN: EAX
; ============================================================================
crc32_buffer:

    push rbx
    push rcx
    push rdx
    push rsi

    mov eax, 0xFFFFFFFF
    mov ecx, BUFFER_SIZE

.byte_loop:

    movzx edx, byte [rsi]

    xor eax, edx

    mov ebx, 8

.bit_loop:

    shr eax, 1
    jnc .skip_poly

    xor eax, CRC32_POLY

.skip_poly:

    dec ebx
    jnz .bit_loop

    inc rsi

    dec ecx
    jnz .byte_loop

    not eax

    pop rsi
    pop rdx
    pop rcx
    pop rbx

    ret

; ============================================================================
; MIX BUFFER
; SAFE VERSION
; ============================================================================
mix_buffer:

    push rax
    push rcx
    push rdx

    xor ecx, ecx

.loop:

    cmp ecx, BUFFER_SIZE - 16
    jge .done

    movzx eax, byte [rsi + rcx + 7]
    xor byte [rsi + rcx], al

    movzx eax, byte [rsi + rcx + 11]
    xor byte [rsi + rcx], al

    inc ecx
    jmp .loop

.done:

    pop rdx
    pop rcx
    pop rax

    ret

; ============================================================================
; BASIC STATS
; ============================================================================
compute_basic_stats:

    push rax
    push rbx
    push rcx
    push rdx

    xor r8, r8
    xor r9, r9

    mov dword [stat_min], 0xFFFFFFFF
    mov dword [stat_max], 0

    xor ecx, ecx

.loop:

    cmp ecx, N_SAMPLES
    jge .done

    mov eax, [samples + rcx*4]

    ; MIN
    cmp eax, [stat_min]
    jge .skip_min
    mov [stat_min], eax

.skip_min:

    ; MAX
    cmp eax, [stat_max]
    jle .skip_max
    mov [stat_max], eax

.skip_max:

    ; SUM 64-bit
    movzx rdx, eax
    add r8, rdx

    ; SUM SQ 64-bit
    mov rax, rdx
    imul rax, rax
    add r9, rax

    inc ecx
    jmp .loop

.done:

    mov [stat_sum], r8
    mov [stat_sum_sq], r9

    pop rdx
    pop rcx
    pop rbx
    pop rax

    ret

; ============================================================================
; PRINT REPORT
; ============================================================================
print_report:

    ; Samples
    lea rdi, [str_samples]
    call print_string

    mov rax, [samples_count]
    call print_decimal

    lea rdi, [newline]
    call print_string

    ; Mean
    lea rdi, [str_mean]
    call print_string

    mov rax, [stat_sum]
    mov rbx, N_SAMPLES
    xor rdx, rdx
    div rbx

    call print_decimal

    lea rdi, [newline]
    call print_string

    ; Min
    lea rdi, [str_min]
    call print_string

    mov eax, [stat_min]
    movzx rax, eax

    call print_decimal

    lea rdi, [newline]
    call print_string

    ; Max
    lea rdi, [str_max]
    call print_string

    mov eax, [stat_max]
    movzx rax, eax

    call print_decimal

    lea rdi, [newline]
    call print_string

    ret

; ============================================================================
; PRINT STRING
; RDI = ptr
; ============================================================================
print_string:

    push rax
    push rcx
    push rdx
    push rsi
    push r8

    mov r8, rdi

    xor rcx, rcx

.len_loop:

    cmp byte [r8 + rcx], 0
    je .len_done

    inc rcx
    jmp .len_loop

.len_done:

    mov rax, SYS_WRITE
    mov rdi, STDOUT
    mov rsi, r8
    mov rdx, rcx

    syscall

    pop r8
    pop rsi
    pop rdx
    pop rcx
    pop rax

    ret

; ============================================================================
; PRINT DECIMAL
; RAX = number
; ============================================================================
print_decimal:

    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi

    lea rsi, [tmp_decimal + 63]

    mov byte [rsi], 0

    mov rbx, 10

    dec rsi

    test rax, rax
    jnz .convert

    mov byte [rsi], '0'
    jmp .print

.convert:

.loop:

    xor rdx, rdx

    div rbx

    add dl, '0'

    mov [rsi], dl

    dec rsi

    test rax, rax
    jnz .loop

    inc rsi

.print:

    mov rdi, rsi

    call print_string

    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ret

; ============================================================================
; EOF
; ============================================================================
