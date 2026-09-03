.intel_syntax noprefix
.global u32_translate_english_asm
.text

# uint16_t u32_translate_english_asm(uint32_t action_id, uint8_t reg_index)
u32_translate_english_asm:
    mov r8d, edi           # R8D = Action ID
    mov r9b, sil           # R9B = Register Index
    xor ecx, ecx

    cmp r8d, 1
    je  action_inc         # 1 = increment
    cmp r8d, 2
    je  action_dec         # 2 = decrement
    cmp r8d, 3
    je  action_clear       # 3 = clear
    cmp r8d, 4
    je  action_shl         # 4 = shift left
    cmp r8d, 5
    je  action_halt        # 5 = halt cpu
    cmp r8d, 6
    je  action_dispatch    # 6 = dispatch (print via multiplexer)
    jmp action_unknown

action_inc:
    mov cl, 0x41           # Format 'p' (01 << 6) | sub-op 0x01
    jmp assemble_word

action_dec:
    mov cl, 0x42           # Format 'p' (01 << 6) | sub-op 0x02
    jmp assemble_word

action_clear:
    mov cl, 0x43           # Format 'p' (01 << 6) | sub-op 0x03
    jmp assemble_word

action_shl:
    mov cl, 0x45           # Format 'p' (01 << 6) | sub-op 0x05 (Shift Left)
    jmp assemble_word

action_halt:
    mov cl, 0x82           # Format 'ppop' (10 << 6) | sub-op 0x02 (Halt)
    jmp assemble_word

action_dispatch:
    mov cl, 0xC0           # Format 'pop' (11 << 6) | sub-op 0x00 (Multiplexer)
    jmp assemble_word

action_unknown:
    mov cl, 0x00

assemble_word:
    movzx eax, cl
    shl ax, 8
    movzx rdx, r9b
    or  ax, dx
    ret
