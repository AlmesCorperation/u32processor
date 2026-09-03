## System Architecture & Memory Model

### 1. Instruction Set Architecture (ISA)
The U32 architecture utilizes a clean 16-bit instruction word split into dual 8-bit chunks:
- **Pre-chunk (PRE)**: Defines the operation category, instruction family, or primary command action.
- **Post-chunk (POST)**: Specifies target registers, immediate values, or execution modifiers.

### 2. Memory Management (RP16 MMU)
Memory access is governed by the RP16 Memory Management Unit, ensuring bounded physical memory mapping and runtime protection checks.
- Any illegal or out-of-bounds access triggers a hardware fault trap routed directly through the **OpenBIOS exception vector (`u32_demux_exception_router`)**, capturing the fault address and setting status CSRs.

### 3. The 55016 Bus Multiplexer
The platform uses the 55016 hardware multiplexer to bridge CPU execution logic with external peripheral states, handling register dispatches and data streaming cleanly between subsystems.

### 4. Native Firmware Layer (OpenBIOS Integration)
Instead of relying on external binaries, OpenBIOS is integrated natively via `openbios/arch/u32`:
- **Boot Authority (`u32_boot_entry`)**: On system startup (`0x0000`), control is handed over to OpenBIOS to initialize environment vectors, configure states, and drive the execution loop.
