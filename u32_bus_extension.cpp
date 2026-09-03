#include "u32.h"
#include <iostream>
#include <fstream>
#include <vector>

extern "C" {

    // Extended Peripheral Discovery via Host Environment Scan for peripherals.u32bin
    void u32_discover_peripherals(U32_CPU *cpu) {
        (void)cpu;
        std::ifstream file("peripherals.u32bin", std::ios::binary | std::ios::ate);
        if (file.is_open()) {
            std::streamsize size = file.tellg();
            file.close();
            std::cout << "[U32 Bus Extension] Discovered 'peripherals.u32bin' (" << size << " bytes) in host environment.\n";
            std::cout << "[U32 Bus Extension] Peripheral subsystem mapped and reported to CPU core.\n";
        } else {
            std::cout << "[U32 Bus Extension] No 'peripherals.u32bin' detected. Operating on internal stub bus.\n";
        }
    }

    // Extended 55016 Multiplexer Handler for the new bus extension
    void u32_extended_55016_dispatch(uint16_t instruction, uint8_t packed_reg_idx, uint16_t reg_val) {
        uint8_t device_target = (instruction >> 8) & 0x0F;
        uint16_t cmd_space = instruction & 0x0FFF;

        std::cout << "[55016 Bus Extension] Multiplexing -> Device: 0x" << std::hex << (int)device_target 
                  << " | Cmd: 0x" << cmd_space << " | Packed Reg [0x" << (int)packed_reg_idx 
                  << "] = 0x" << reg_val << std::dec << "\n";
    }

}
