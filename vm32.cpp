#include "u32.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <SDL2/SDL.h>

extern "C" {
    void arch_init(void);
    void u32_bind_openbios_cpu(U32_CPU *cpu);
    void u32_boot_entry(U32_CPU *cpu, uint16_t entry_vector);
}

extern "C" {
    int rs_verify_and_load_openbios(const char *bios_path, unsigned char *buffer, size_t max_size);
    void rs_print_qol_telemetry(unsigned short pc, int running, unsigned short reg_zero_val);
}

extern "C" {
    void u32_bind_openbios_cpu(U32_CPU *cpu);
    void u32_boot_entry(U32_CPU *cpu, uint16_t entry_vector);
    void u32_run_fortran_assembler(const char* filename);

    // Declared extern so they resolve to u32_peripherals.c without multiple definitions
    void u32_demux_exception_router(U32_CPU *cpu, Rp16Exception exc, uint16_t fault_addr);
    void u32_55016_multiplexer_dispatch(uint16_t instruction, uint8_t packed_reg_idx, uint16_t reg_val);
}

class VM32_Machine {
private:
    U32_CPU vm;

    bool is_valid_hex(const std::string& str) {
        if (str.empty()) return false;
        for (char c : str) {
            char upper = std::toupper(c);
            if (!((upper >= '0' && upper <= '9') || (upper >= 'A' && upper <= 'F'))) return false;
        }
        return true;
    }

    std::string to_lower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
        return s;
    }

    bool load_iso_binary(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cout << "[VM32 Error] Could not open file: " << filepath << "\n";
            return false;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        if (size > (std::streamsize)sizeof(vm.memory)) {
            std::cout << "[VM32 Error] Image size (" << size << " bytes) exceeds VM physical memory limit!\n";
            return false;
        }

        if (file.read((char*)vm.memory, size)) {
            std::cout << "[QEMU Storage] Successfully mounted image (" << size << " bytes) at physical address 0x0000.\n";
            return true;
        }

        std::cout << "[VM32 Error] Failed to read binary contents.\n";
        return false;
    }

    void render_qemu_display(const std::string& iso_name) {
        std::cout << "\033[2J\033[1;1H";
        std::cout << "+--------------------------------------------------------------------+\n";
        std::cout << "| QEMU Virtual Machine Monitor [Architecture: U32 / OpenBIOS IEEE1275] |\n";
        std::cout << "+--------------------------------------------------------------------+\n";
        std::cout << "| Loaded File: " << std::left << std::setw(53) << iso_name << " |\n";
        std::cout << "| CPU Status : " << std::left << std::setw(53) << (vm.running ? "RUNNING (Active)" : "HALTED") << " |\n";
        std::cout << "| Program Counter (PC): 0x" << std::hex << std::setw(4) << std::setfill('0') << vm.pc 
                  << "                             |\n";
        std::cout << "+--------------------------------------------------------------------+\n";
        std::cout << "| CORE REGISTERS & CSRs:                                             |\n";
        std::cout << "|    R1: 0x" << std::setw(4) << vm.regs[1] << "  R2: 0x" << std::setw(4) << vm.regs[2] 
                  << "  R3: 0x" << std::setw(4) << vm.regs[3] << "  R4: 0x" << std::setw(4) << vm.regs[4] << "         |\n";
        std::cout << "|    STATUS: 0x" << std::setw(4) << vm.regs[CSR_STATUS] 
                  << "  CAUSE: 0x" << std::setw(4) << vm.regs[CSR_EXC_CAUSE] 
                  << "  ADDR: 0x" << std::setw(4) << vm.regs[CSR_EXC_ADDR] << " |\n";
        std::cout << "+--------------------------------------------------------------------+\n";
        std::cout << "| PHYSICAL MEMORY DUMP (0x0000 - 0x0020):                            |\n" << std::dec;
        
        std::cout << "| ";
        for (int i = 0; i < 32; ++i) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)vm.memory[i] << " ";
            if ((i + 1) % 16 == 0 && i < 31) std::cout << "|\n| ";
        }
        std::cout << " |\n";
        std::cout << "+--------------------------------------------------------------------+\n";
        std::cout << " [QEMU Controls] (s)tep execution | (c)ontinue | (q)uit window\n";
    }

public:
    VM32_Machine() {
        u32_init(&vm);
        u32_bind_openbios_cpu(&vm);
    }

    void launch_headless_window(const std::string& target_file) {
        u32_init(&vm);
        u32_bind_openbios_cpu(&vm);
        if (!load_iso_binary(target_file)) {
            std::cout << "[QEMU Error] Failed to load file. Press Enter to close window.\n";
            std::cin.get();
            return;
        }

        std::string qemu_cmd;
        while (true) {
            render_qemu_display(target_file);
            std::cout << "qemu-u32> ";
            if (!std::getline(std::cin, qemu_cmd)) break;
            
            std::string qcmd = to_lower(qemu_cmd);
            if (qcmd == "q" || qcmd == "quit") {
                break;
            } else if (qcmd == "s" || qcmd == "step") {
                if (vm.running) {
                    u32_step(&vm);
                } else {
                    std::cout << "\n[QEMU Notice] CPU is halted. Press Enter to close window.\n";
                    std::cin.get();
                    break;
                }
            } else if (qcmd == "c" || qcmd == "run") {
                u32_boot_entry(&vm, vm.pc);
            }
        }
    }

void launch_visual_window(const std::string& target_file) {
        u32_init(&vm);
        u32_bind_openbios_cpu(&vm);
        
        // 1. Initialize the OpenBIOS architecture bridge layer
        arch_init();

        if (!load_iso_binary(target_file)) {
            return;
        }

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cout << "[VGA Error] Could not initialize SDL2: " << SDL_GetError() << "\n";
            return;
        }

        int width = 320;
        int height = 200;
        SDL_Window* window = SDL_CreateWindow("VM32 VGA Visual Display", 
                                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                    width * 2, height * 2, SDL_WINDOW_SHOWN);
        SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, 
                                   SDL_TEXTUREACCESS_STREAMING, width, height);

        uint32_t pixels[320 * 200];
        std::memset(pixels, 0, sizeof(pixels));

        bool running = true;
        SDL_Event e;

        std::cout << "[VM32] Visual window active. OpenBIOS firmware bridge initialized.\n";

        while (running) {
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    running = false;
                }
            }

            if (vm.running) {
                // Steps the CPU frame-by-frame under the OpenBIOS/MMU/bus context
                u32_step(&vm);
            }

            for (int i = 0; i < width * height; i++) {
                uint8_t mem_val = vm.memory[i % sizeof(vm.memory)];
                pixels[i] = (mem_val << 16) | (mem_val << 8) | mem_val; 
            }

            SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);

            SDL_Delay(16);
        }

        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void power_on() {
// Optional: Rust-powered OpenBIOS firmware verification on boot
        unsigned char bios_test_buffer[512];
        int bios_check = rs_verify_and_load_openbios("openbios/arch/u32/openbios.bin", bios_test_buffer, sizeof(bios_test_buffer));
        if (bios_check > 0) {
            std::cout << "[VM32] Rust Bridge: OpenBIOS image verified successfully.\n";
        }

        std::cout << "=========================================\n"
                  << "      VM32 Virtual Machine Monitor       \n"
                  << "      Architecture: U32 / RP16 MMU       \n"
                  << "      Firmware: OpenBIOS (IEEE 1275)     \n"
                  << "=========================================\n"
                  << "Commands:\n"
                  << "  assemble [file.u32]    - Compile source into binary using Fortran engine\n"
                  << "  run [path.u32bin]      - Spawn separate QEMU window running binary\n"
                  << "  visual [path.u32bin]   - Launch SDL2 pixel-based VGA graphics window\n"
                  << "  regs                   - View CPU registers & status\n"
                  << "  english [action] [reg] - Inject inline bytecode command\n"
                  << "  exit                   - Quit monitor\n";

        std::string line;
        while (true) {
            std::cout << "vm32> ";
            if (!std::getline(std::cin, line) || line == "exit") break;
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string cmd, arg;
            ss >> cmd >> arg;
            std::string lcmd = to_lower(cmd);

            if (lcmd == "assemble") {
                if (arg.empty()) {
                    std::cout << "[VM32 Error] error: file not stated\n";
                } else {
                    std::cout << "[VM32] Invoking built-in Fortran assembler for file: " << arg << "...\n";
                    u32_run_fortran_assembler(arg.c_str());
                }
            } else if (lcmd == "run") {
                if (arg.empty()) {
                    std::cout << "[QEMU Error] error: file not stated\n";
                } else {
                    #if defined(__linux__) || defined(__APPLE__)
                    std::string spawn_cmd = "xterm -T 'QEMU-U32 Virtual Machine Display' -e \"bash -c './vm32 --window-mode " + arg + "; read -p \\\"Press Enter to close window...\\\"'\" &";
                    int ret = system(spawn_cmd.c_str());
                    if (ret != 0) {
                        spawn_cmd = "xfce4-terminal --title='QEMU-U32 Virtual Machine Display' -e './vm32 --window-mode " + arg + "' &";
                        ret = system(spawn_cmd.c_str());
                    }
                    if (ret != 0) {
                        spawn_cmd = "gnome-terminal --title='QEMU-U32 Virtual Machine Display' -- ./vm32 --window-mode " + arg;
                        ret = system(spawn_cmd.c_str());
                    }
                    #else
                    std::cout << "[QEMU Error] Window spawning not supported on this platform.\n";
                    #endif
                }
            } else if (lcmd == "visual") {
                if (arg.empty()) {
                    std::cout << "[VGA Error] error: file not stated\n";
                } else {
                    std::string spawn_cmd = "./vm32 --visual-mode " + arg + " &";
                    system(spawn_cmd.c_str());
                }
            } else if (lcmd == "regs") {
                std::cout << "+----------------------------------------+\n"
                          << "| VM32 PC: 0x" << std::hex << std::setw(4) << std::setfill('0') << vm.pc 
                          << " | Status: " << (vm.running ? "RUNNING" : "HALTED") << " |\n"
                          << "+----------------------------------------+\n" << std::dec;
            } else if (lcmd == "english") {
                std::string reg_str;
                ss >> reg_str;
                std::string action_str = to_lower(arg);

                if (action_str.empty() || reg_str.empty() || !is_valid_hex(reg_str)) {
                    std::cout << "[Error] Usage: english [increment|decrement|clear|shift|halt|dispatch] [reg_hex]\n";
                    continue;
                }

                uint32_t action_id = 0;
                if (action_str == "increment" || action_str == "inc") action_id = 1;
                else if (action_str == "decrement" || action_str == "dec") action_id = 2;
                else if (action_str == "clear" || action_str == "clr") action_id = 3;
                else if (action_str == "shift" || action_str == "shl") action_id = 4;
                else if (action_str == "halt") action_id = 5;
                else if (action_str == "dispatch" || action_str == "print") action_id = 6;
                else {
                    std::cout << "[Error] Unknown action keyword.\n";
                    continue;
                }

                uint8_t reg_idx = std::stoul(reg_str, nullptr, 16);
                uint16_t inst = u32_translate_english_asm(action_id, reg_idx);
                uint8_t pre  = (inst >> 8) & 0xFF;
                uint8_t post = inst & 0xFF;

                static uint16_t write_ptr = 0x0020; 
                Rp16Exception exc = RP16_EXC_NONE;
                uint16_t phys_high = rp16_translate_address(&vm.mmu, vm.memory, write_ptr, true, &exc);
                uint16_t phys_low  = rp16_translate_address(&vm.mmu, vm.memory, write_ptr + 1, true, &exc);

                if (exc != RP16_EXC_NONE) {
                    std::cout << "[Error] MMU write fault at address 0x" << std::hex << write_ptr << std::dec << "\n";
                } else {
                    vm.memory[phys_high] = pre;
                    vm.memory[phys_low] = post;
                    std::cout << "[VM32 Compiler] Injected instruction -> 0x" << std::hex << std::setw(4) << std::setfill('0') << inst << std::dec << "\n";
                    write_ptr += 2;
                }
            } else {
                std::cout << "[Error] Unknown command.\n";
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc >= 3 && std::string(argv[1]) == "--window-mode") {
        VM32_Machine machine;
        machine.launch_headless_window(argv[2]);
        return 0;
    }
    if (argc >= 3 && std::string(argv[1]) == "--visual-mode") {
        VM32_Machine machine;
        machine.launch_visual_window(argv[2]);
        return 0;
    }

    VM32_Machine machine;
    machine.power_on();
    return 0;
}
