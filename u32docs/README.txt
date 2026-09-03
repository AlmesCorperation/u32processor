kU32 Architecture Platform Documentation
Primary License: AGPLv3

Overview:
A custom-built, polyglot 16-bit systems architecture, assembler, and virtual machine environment written in C, C++, Fortran, and Assembly under the AGPLv3 license.

Features:

Custom 16-bit ISA with a dual 8-bit chunk instruction format defined in u32.h.

RP16 MMU for clean memory management and exception routing via OpenBIOS.

55016 Bus Multiplexer for hardware-software device routing and register dispatches.

Two-pass Fortran assembler (u32asm) for label resolution and control-flow logic.

Dual runtime modes: Terminal monitoring and an SDL2 pixel-based VGA visual window.

Prerequisites & Setup (Ubuntu/Debian):
sudo apt update
sudo apt install build-essential gfortran libsdl2-dev -y

Building the Project:
make clean
make

Quick Start Guide:

Assemble a Source File:
Compile your human-readable U32 source code (.u32) into a native binary (.u32bin):
./u32asm program.u32

Launch the VM32 Runtime:
Boot your compiled binary inside the virtual machine environment:
./vm32 program.u32bin

Launch the SDL2 VGA Visual Window:
Fire up the graphical pixel-based framebuffer window with your binary:
./vm32 program.u32bin visual
