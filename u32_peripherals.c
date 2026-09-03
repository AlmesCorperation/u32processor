#include "u32.h"
#include <stdio.h>

void u32_demux_exception_router(U32_CPU *cpu, Rp16Exception exc, uint16_t fault_addr) {
    cpu->running = false;
    cpu->regs[CSR_EXC_CAUSE] = (uint16_t)exc;
    cpu->regs[CSR_EXC_ADDR] = fault_addr;
    printf("[U32 Exception Router] Trap triggered! Cause: %d, Fault Address: 0x%04X\n", (int)exc, fault_addr);
}

void u32_55016_multiplexer_dispatch(uint16_t instruction, uint8_t packed_reg_idx, uint16_t reg_val) {
    printf("[55016 Bus Multiplexer] Instruction: 0x%04X, Reg[%d] Val: 0x%04X\n", instruction, packed_reg_idx, reg_val);
}
