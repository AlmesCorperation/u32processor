## U32 Two-Pass Assembler (u32asm.f90)

The U32 toolchain features a robust, two-pass assembler written in Fortran (`u32asm.f90`) that translates human-readable assembly source files into raw executable binaries (`os.iso` / `.u32bin`).

### Assembler Design
- **Pass 1 (Symbol Resolution)**: Scans the source file to identify and map all target labels (loop headers, jump vectors, subroutines) into an internal symbol table with calculated memory offsets.
- **Pass 2 (Code Generation)**: Translates instructions and resolved symbols into packed 16-bit binary streams. The resulting binary is loaded at physical address `0x0000`, making it immediately ready for execution handover by the OpenBIOS firmware boot sequence.

### Supported Control Flow Syntax
- `JMP [label]` — Unconditional jump to a target label.
- `JZ [reg], [label]` — Branch/jump if the specified register value evaluates to zero.
- `WHILE` / `FOR` constructs for high-level looping logic.
