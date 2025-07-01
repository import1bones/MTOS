#![no_std]
#![no_main]

//! Hello World - Simple userspace application demonstrating basic MTOS functionality

use mtos_runtime::*;

fn main() -> i32 {
    // Print hello message
    if let Err(_) = println("Hello from MTOS userspace!") {
        return -1;
    }
    
    // Show process information
    let pid = getpid();
    let pid_str = format_u32(pid);
    if let Err(_) = print("Process ID: ") {
        return -1;
    }
    if let Err(_) = println(&pid_str) {
        return -1;
    }
    
    // Demonstrate system calls
    if let Err(_) = println("Testing memory allocation...") {
        return -1;
    }
    
    match malloc(1024) {
        Ok(ptr) => {
            if let Err(_) = println("Memory allocation successful!") {
                return -1;
            }
            
            // Use the memory briefly
            unsafe {
                *ptr = 0x42;
                if *ptr == 0x42 {
                    if let Err(_) = println("Memory write/read test passed!") {
                        return -1;
                    }
                }
            }
            
            if let Err(_) = free(ptr) {
                if let Err(_) = println("Warning: Memory free failed") {
                    return -1;
                }
            } else {
                if let Err(_) = println("Memory freed successfully!") {
                    return -1;
                }
            }
        }
        Err(_) => {
            if let Err(_) = println("Memory allocation failed!") {
                return -1;
            }
            return -1;
        }
    }
    
    // Test sleep
    if let Err(_) = println("Sleeping for 100ms...") {
        return -1;
    }
    
    if let Err(_) = sleep_ms(100) {
        if let Err(_) = println("Sleep failed!") {
            return -1;
        }
    }
    
    if let Err(_) = println("Hello world complete!") {
        return -1;
    }
    
    0
}

mtos_main!(main);
