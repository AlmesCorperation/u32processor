use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int, c_ushort};
use std::fs::File;
use std::io::Read;
use std::path::Path;

#[repr(C)]
pub struct RsCpuSnapshot {
    pub pc: c_ushort,
    pub running: c_int,
    pub active_registers: c_int,
}

/// QoL Feature: Advanced OpenBIOS & Firmware Loader Verification
#[no_mangle]
pub extern "C" fn rs_verify_and_load_openbios(bios_path: *const c_char, buffer: *mut u8, max_size: usize) -> c_int {
    if bios_path.is_null() || buffer.is_null() {
        return -1;
    }

    let c_str = unsafe { CStr::from_ptr(bios_path) };
    let path_str = match c_str.to_str() {
        Ok(s) => s,
        Err(_) => return -2,
    };

    println!("[Rust U32 Subsystem] Scanning for OpenBIOS image at: {}", path_str);

    if !Path::new(path_str).exists() {
        println!("[Rust U32 Subsystem Error] OpenBIOS firmware file not found on disk.");
        return -3;
    }

    let mut file = match File::open(path_str) {
        Ok(f) => f,
        Err(_) => return -4,
    };

    let mut temp_buf = Vec::new();
    match file.read_to_end(&mut temp_buf) {
        Ok(bytes_read) => {
            if bytes_read > max_size {
                println!("[Rust U32 Subsystem Error] OpenBIOS binary exceeds memory allocation limit!");
                return -5;
            }
            unsafe {
                std::ptr::copy_nonoverlapping(temp_buf.as_ptr(), buffer, bytes_read);
            }
            println!("[Rust U32 Subsystem] OpenBIOS successfully loaded into memory slot ({} bytes).", bytes_read);
            bytes_read as c_int
        }
        Err(_) => -6,
    }
}

/// QoL Feature: High-Fidelity Machine Telemetry Dashboard
#[no_mangle]
pub extern "C" fn rs_print_qol_telemetry(pc: c_ushort, running: c_int, reg_zero_val: c_ushort) {
    println!("┌────────────────────────────────────────────────────────┐");
    println!("│                U32 VIRTUAL MACHINE TELEMETRY           │");
    println!("├────────────────────────────────────────────────────────┤");
    println!("│ Program Counter (PC) : 0x{:04X}                      │", pc);
    println!("│ Core Execution Status: {:<20}          │", if running == 1 { "RUNNING (Active)" } else { "HALTED / STOPPED" });
    println!("│ Ground Register [0x00] : 0x{:04X} (Hardwired Anchor)   │", reg_zero_val);
    println!("└────────────────────────────────────────────────────────┘");
}
