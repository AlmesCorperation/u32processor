#include "u32.h"
#include <stdio.h>
#include <string.h>

// Standard C external declaration for peripheral discovery
extern void u32_discover_peripherals(U32_CPU *cpu);

// Hardware-level reset and initialization of the U32 silicon core
void u32_init(U32_CPU *cpu) {
    // Clear the 256-register file (0x00 - 0xFF)
    memset(cpu->regs, 0, sizeof(cpu->regs));
    
    // Hardwire register 0x00 to absolute zero (physical ground line)
    cpu->regs[0x00] = 0x0000;
    
    cpu->pc = 0x0000;
    cpu->running = true;
    
    // Clear physical memory banks
    memset(cpu->memory, 0, sizeof(cpu->memory));
    
    // Initialize the RP16 MMU hardware unit
    rp16_init(&cpu->mmu);

    // Hardcode initial Page Table Base Register (PTBR)
    uint16_t pt_base = 0x1000;
    cpu->mmu.page_table_base_reg = pt_base;
    cpu->regs[CSR_PTBR] = pt_base;
    
    // Map initial identity page
    rp16_map_page(cpu->memory, pt_base, 0x00, 0x00, 0x03); 
    
    // Automatically trigger peripheral file scan on boot
    u32_discover_peripherals(cpu);
    
    printf("[U32 Silicon Core] Hardware Reset Complete. 256-Register File Active. MMU Online.\n");
}

// Single hardware clock step / execution cycle
void u32_step(U32_CPU *cpu) {
    if (!cpu->running) return;

    Rp16Exception exc = RP16_EXC_NONE;

    // --- STAGE 1: FETCH (High/Low byte fetch via RP16 MMU page table walk) ---
    uint16_t phys_high = rp16_translate_address(&cpu->mmu, cpu->memory, cpu->pc, false, &exc);
    if (exc != RP16_EXC_NONE) {
        u32_demux_exception_router(cpu, exc, cpu->pc);
        return;
    }

    uint16_t phys_low = rp16_translate_address(&cpu->mmu, cpu->memory, cpu->pc + 1, false, &exc);
    if (exc != RP16_EXC_NONE) {
        u32_demux_exception_router(cpu, exc, cpu->pc + 1);
        return;
    }

    // Latch 16-bit instruction word from memory bus and advance Program Counter
    uint16_t instruction = (cpu->memory[phys_high] << 8) | cpu->memory[phys_low];
    cpu->pc += 2;

    // --- STAGE 2: DECODE (Hardware bitwise extraction of the two chunks) ---
    uint32_t decoded_packet = u32_decode_and_pack_asm(instruction);

    uint8_t post_chunk  = decoded_packet & 0xFF;         // Target Register / Destination Index
    uint8_t pre_chunk   = (decoded_packet >> 8) & 0xFF;    // Pre-processing configuration byte
    uint8_t sub_opcode  = (decoded_packet >> 16) & 0x3F;   // ALU / Control modifier line
    U32_Format format   = (U32_Format)((decoded_packet >> 22) & 0x3);

    // --- STAGE 3: EXECUTE (ALU and Register Transfer) ---
    switch (format) {
        case FMT_PP: // Pre-processing / Immediate Operand Pipeline
            {
                uint8_t imm = pre_chunk & 0x3F;
                switch (sub_opcode) {
                    case 0x01: cpu->regs[post_chunk] += imm; break;
                    case 0x02: cpu->regs[post_chunk] -= imm; break;
                    case 0x03: cpu->regs[post_chunk] &= imm; break;
                    case 0x04: cpu->regs[post_chunk] |= imm; break;
                    case 0x05: cpu->regs[post_chunk] ^= imm; break;
                    case 0x06: cpu->regs[post_chunk] *= imm; break;
                    default: break;
                }
            }
            break;

        case FMT_P: // Unary / Shift Processing Pipeline
            switch (sub_opcode) {
                case 0x01: cpu->regs[post_chunk] += 1; break;
                case 0x02: cpu->regs[post_chunk] -= 1; break;
                case 0x03: cpu->regs[post_chunk] = 0; break;
                case 0x04: cpu->regs[post_chunk] = ~cpu->regs[post_chunk]; break;
                case 0x05: cpu->regs[post_chunk] <<= 1; break;
                case 0x06: cpu->regs[post_chunk] >>= 1; break;
                default: break;
            }
            break;

        case FMT_PPOP: // Control Flow & System Core Operations
            if (sub_opcode == 0x01) { // Absolute jump targeting register address line
                cpu->pc = cpu->regs[post_chunk];
            } else if (sub_opcode == 0x02) { // Hard halt trigger
                cpu->running = false;
                printf("[U32 Silicon Core] Hardware power-down triggered by instruction.\n");
            }
            break;

        case FMT_POP: // 55016 Bus Multiplexer Dispatch
            u32_55016_multiplexer_dispatch(instruction, post_chunk, cpu->regs[post_chunk]);
            break;
    }
    
    // --- STAGE 4: HARDWARE ANCHOR ---
    // Ground register 0x00 unconditionally on every clock edge
    cpu->regs[0x00] = 0x0000;
}
