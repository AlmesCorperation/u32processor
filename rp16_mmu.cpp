#include "u32.h"
#include <iostream>
#include <iomanip>

#define RP16_PTE_PRESENT   (1 << 0)
#define RP16_PTE_WRITABLE  (1 << 1)
#define RP16_PTE_USER      (1 << 2)
#define RP16_PTE_ACCESSED  (1 << 3)
#define RP16_PTE_DIRTY     (1 << 4)

extern "C" {

    void rp16_init(Rp16_MMU *mmu) {
        mmu->page_table_base_reg = 0x1000; // Default hardware reset PTBR
        mmu->mmu_enabled = true;
        std::cout << "[RP16 MMU] Hardware Memory Management Unit Initialized. Multi-Level Page Walker Online.\n";
    }

    // Hardware Page Table Walker (Guest Virtual Address -> Physical Address Translation)
    uint16_t rp16_translate_address(Rp16_MMU *mmu, uint8_t *physical_memory, uint16_t virtual_address, bool is_write, Rp16Exception *out_exc) {
        *out_exc = RP16_EXC_NONE;

        // If MMU is disabled, pass-through physical addressing (Real Mode)
        if (!mmu->mmu_enabled) {
            return virtual_address;
        }

        // --- STAGE 1: ADDRESS DECOMPOSITION ---
        // Partition 16-bit virtual address into Page Directory Index (top 4 bits), 
        // Page Table Index (middle 4 bits), and Page Offset (bottom 8 bits).
        uint8_t pd_index  = (virtual_address >> 12) & 0x0F;
        uint8_t pt_index  = (virtual_address >> 8)  & 0x0F;
        uint8_t page_offset = virtual_address       & 0xFF;

        // --- STAGE 2: PAGE DIRECTORY WALK ---
        uint32_t pde_address = (uint32_t)mmu->page_table_base_reg + (pd_index * sizeof(uint16_t));
        if (pde_address >= MEM_SIZE - 1) {
            *out_exc = RP16_EXC_ACCESS_FAULT;
            return 0;
        }

        uint16_t pde = (physical_memory[pde_address] << 8) | physical_memory[pde_address + 1];

        // Check Page Directory Entry (PDE) presence
        if (!(pde & RP16_PTE_PRESENT)) {
            *out_exc = RP16_EXC_PAGE_FAULT;
            return 0;
        }

        // --- STAGE 3: PAGE TABLE WALK ---
        // The high byte of the PDE points to the physical frame base of the Page Table
        uint16_t pt_base_frame = (pde & 0xFF00);
        uint32_t pte_address = (uint32_t)pt_base_frame + (pt_index * sizeof(uint16_t));
        
        if (pte_address >= MEM_SIZE - 1) {
            *out_exc = RP16_EXC_ACCESS_FAULT;
            return 0;
        }

        uint16_t pte = (physical_memory[pte_address] << 8) | physical_memory[pte_address + 1];

        // Check Page Table Entry (PTE) presence
        if (!(pte & RP16_PTE_PRESENT)) {
            *out_exc = RP16_EXC_PAGE_FAULT;
            return 0;
        }

        // --- STAGE 4: PERMISSION & PROTECTION CHECKS ---
        if (is_write && !(pte & RP16_PTE_WRITABLE)) {
            *out_exc = RP16_EXC_ACCESS_FAULT;
            return 0;
        }

        // --- STAGE 5: PHYSICAL ADDRESS RESOLUTION ---
        // Combine physical frame base from PTE with the virtual page offset
        uint16_t phys_frame = (pte & 0xFF00);
        uint16_t physical_address = phys_frame | page_offset;

        if (physical_address >= MEM_SIZE) {
            *out_exc = RP16_EXC_ACCESS_FAULT;
            return 0;
        }

        return physical_address;
    }

    // Hardware Page Mapping Utility for Firmware/OS initialization
    void rp16_map_page(uint8_t *physical_memory, uint16_t pt_base, uint8_t virt_page, uint8_t phys_frame, uint16_t flags) {
        uint8_t pd_index = (virt_page >> 4) & 0x0F;
        uint8_t pt_index = virt_page & 0x0F;

        // Ensure Page Directory Exists
        uint32_t pde_address = (uint32_t)pt_base + (pd_index * sizeof(uint16_t));
        if (pde_address >= MEM_SIZE - 1) return;

        uint16_t pde = (physical_memory[pde_address] << 8) | physical_memory[pde_address + 1];
        uint16_t pt_frame;

        if (!(pde & RP16_PTE_PRESENT)) {
            // Allocate a basic fallback page table frame right after the directory
            pt_frame = pt_base + 0x0200; 
            uint16_t new_pde = pt_frame | RP16_PTE_PRESENT | RP16_PTE_WRITABLE;
            physical_memory[pde_address]     = (new_pde >> 8) & 0xFF;
            physical_memory[pde_address + 1] = new_pde & 0xFF;
        } else {
            pt_frame = (pde & 0xFF00);
        }

        // Write Page Table Entry (PTE)
        uint32_t pte_address = (uint32_t)pt_frame + (pt_index * sizeof(uint16_t));
        if (pte_address >= MEM_SIZE - 1) return;

        uint16_t pte = (phys_frame << 8) | (flags & 0x00FF);
        physical_memory[pte_address]     = (pte >> 8) & 0xFF;
        physical_memory[pte_address + 1] = pte & 0xFF;
    }

}
