## U32 Low-Level Action Opcodes & System Vectors

### Core Action Opcodes (Bytecode / English Engine)
- `0x01` (`INC` / increment) — Increments target register value by 1.
- `0x02` (`DEC` / decrement) — Decrements target register value by 1.
- `0x03` (`CLR` / clear) — Zeroes out the specified register.
- `0x04` (`SHL` / shift) — Performs a bitwise shift operation on the target register.
- `0x05` (`HLT` / halt) — Halts CPU execution and triggers the stop state.
- `0x06` (`DISPATCH` / print) — Dispatches register data to the 55016 bus multiplexer for logging/peripherals.

### Firmware & Exception Vectors
- **Exception Trap Vector**: Routed via `u32_demux_exception_router` to catch RP16 faults and pass control to OpenBIOS diagnostic routines.
- **Boot Entry**: `u32_boot_entry(&vm, 0x0000)` executes the firmware initialization sequence prior to passing control to application code.
