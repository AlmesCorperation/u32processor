## VM32 Runtime & Display Modes

VM32 includes two distinct execution environments powered by the OpenBIOS firmware bridge (`arch_init()`):

### 1. QEMU-Style Terminal Monitor (`run`)
Spawns an interactive terminal window providing a live retro dashboard governed by OpenBIOS:
- **Firmware Boot Handover**: Calls `arch_init()`, loads the binary into physical memory at `0x0000`, and invokes `u32_boot_entry` to drive execution.
- **Real-Time Tracking**: Program Counter (PC) monitoring, core register inspection (`R1` through `R4`, `STATUS`, `CAUSE`, `ADDR` CSRs), and physical memory hex dumps (`0x0000` to `0x0020`).
- **Controls**: Interactive stepping (`s`), execution continuation (`c`), and window exit (`q`).

### 2. SDL2 VGA Visual Mode (`visual`)
Launches a dedicated graphical pixel-based framebuffer window:
- **Firmware-Backed Display**: Initializes the OpenBIOS bridge context before spinning up the SDL2 pipeline.
- **Framebuffer Rendering**: Streams live visual updates scaled from VM memory space at a locked frame rate (`SDL_Delay(16)`), executing instructions frame-by-frame via `u32_step(&vm)` under the active MMU context.
