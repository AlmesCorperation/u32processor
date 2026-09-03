#ifndef U32_H
#define U32_H

#include <stdint.h>
#include <stdbool.h>

#define REG_COUNT 256
#define MEM_SIZE  (1 << 16)

#define CSR_STATUS      0xF0 
#define CSR_EXC_CAUSE   0xF1 
#define CSR_EXC_ADDR    0xF2 
#define CSR_PTBR        0xF3 

typedef enum {
    FMT_PP   = 0x0,
    FMT_P    = 0x1,
    FMT_PPOP = 0x2,
    FMT_POP  = 0x3
} U32_Format;

typedef enum {
    RP16_EXC_NONE = 0,
    RP16_EXC_PAGE_FAULT = 1,
    RP16_EXC_ACCESS_FAULT = 2,
    RP16_EXC_ILLEGAL_INSTRUCTION = 3
} Rp16Exception;

typedef struct {
    uint16_t page_table_base_reg;
    bool     mmu_enabled;
} Rp16_MMU;

typedef struct {
    uint16_t regs[REG_COUNT];
    uint16_t pc;              
    uint8_t  memory[MEM_SIZE]; 
    bool     running;
    Rp16_MMU mmu;             
} U32_CPU;

#ifdef __cplusplus
extern "C" {
#endif
    // Assembly Engines
    uint32_t u32_decode_and_pack_asm(uint16_t instruction);
    uint16_t u32_translate_english_asm(uint32_t action_id, uint8_t reg_index);

    void u32_init(U32_CPU *cpu);
    void u32_step(U32_CPU *cpu);
    
    void rp16_init(Rp16_MMU *mmu);
    uint16_t rp16_translate_address(Rp16_MMU *mmu, uint8_t *physical_memory, uint16_t virtual_address, bool is_write, Rp16Exception *out_exc);
    void rp16_map_page(uint8_t *physical_memory, uint16_t pt_base, uint8_t virt_page, uint8_t phys_frame, uint16_t flags);
    
    void u32_demux_exception_router(U32_CPU *cpu, Rp16Exception exc, uint16_t fault_addr);
    void u32_55016_multiplexer_dispatch(uint16_t instruction, uint8_t packed_reg_idx, uint16_t reg_val);
#ifdef __cplusplus
}
#endif

#endif
