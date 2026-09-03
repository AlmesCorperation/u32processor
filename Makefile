CC = gcc
CXX = g++
FC = gfortran
CARGO = cargo
CFLAGS = -Wall -Wextra -O3 -std=c11 -Iopenbios/include
CXXFLAGS = -Wall -Wextra -O3 -std=c++17 -Iopenbios/include $(shell sdl2-config --cflags)
FCFLAGS = -O3

# Objects required for the VM32 runtime host environment
VM32_OBJS = u32_core.o \
            rp16_mmu.o \
            u32_peripherals.o \
            u32_bus_extension.o \
            u32_assembler_backend.o \
            openbios/arch/u32/context.o \
            openbios/arch/u32/loaders.o \
            vm32.o \
            u32_bit_engine.o \
            u32_english_engine.o

RUST_LIB = rust_bridge/target/release/libu32_rs_bridge.a

all: rust_build vm32 u32asm

# Build the Rust static library via Cargo
rust_build:
	cd rust_bridge && $(CARGO) build --release

# Compile C rules
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ rules
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile Assembly rules
%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

# Compile Fortran rules into an object file
%.o: %.f90
	$(FC) $(FCFLAGS) -c $< -o $@

# Link the main VM32 runtime executable with C++, Fortran, and Rust libraries
vm32: $(VM32_OBJS) rust_build
	$(CXX) $(VM32_OBJS) $(RUST_LIB) -o vm32 -lgfortran -lpthread -ldl -lm $(shell sdl2-config --libs)

# Link the standalone Fortran assembler tool
u32asm: u32asm.o
	$(FC) u32asm.o -o u32asm

clean:
	rm -f $(VM32_OBJS) u32asm.o vm32 u32asm *.u32bin os.iso *.mod
	cd rust_bridge && $(CARGO) clean
