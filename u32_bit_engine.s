.intel_syntax noprefix
.global u32_decode_and_pack_asm
.text

# uint32_t u32_decode_and_pack_asm(uint16_t instruction)
u32_decode_and_pack_asm:
    movzx rax, di          # RAX = instruction (16-bit zero-extended)

    mov r8b, al            # R8B = Post-Chunk (instruction & 0xFF)
    shr rax, 8             
    mov r9b, al            # R9B = Pre-Chunk (instruction >> 8)

    mov r10b, r9b
    shr r10b, 6
    and r10b, 0x03         # R10B = Format (0 to 3)

    mov r11b, r9b
    and r11b, 0x3F         # R11B = Sub-Opcode (0 to 63)

    xor eax, eax
    
    movzx rdx, r10b
    shl rdx, 22
    or  rax, rdx

    movzx rdx, r11b
    shl rdx, 16
    or  rax, rdx

    movzx rdx, r9b
    shl rdx, 8
    or  rax, rdx

    movzx rdx, r8b
    or  rax, rdx

    ret
