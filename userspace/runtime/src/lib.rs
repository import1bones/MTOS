#![no_std]

//! MTOS Runtime Library
//! 
//! Provides basic runtime support for userspace applications in MTOS.
//! This includes system call interfaces, memory management, and basic I/O.

use core::panic::PanicInfo;
use core::arch::asm;
use heapless::String;

/// System call numbers for MTOS
#[repr(u32)]
#[derive(Copy, Clone, Debug)]
pub enum SysCall {
    Exit = 0,
    Print = 1,
    Read = 2,
    Write = 3,
    GetPid = 4,
    Sleep = 5,
    Malloc = 6,
    Free = 7,
    SendMessage = 8,
    ReceiveMessage = 9,
}

/// System call interface
#[inline(always)]
pub unsafe fn syscall0(call: SysCall) -> isize {
    let ret: isize;
    asm!(
        "int 0x80",
        in("eax") call as u32,
        lateout("eax") ret,
        options(nostack, preserves_flags)
    );
    ret
}

#[inline(always)]
pub unsafe fn syscall1(call: SysCall, arg1: usize) -> isize {
    let ret: isize;
    asm!(
        "int 0x80",
        in("eax") call as u32,
        in("ebx") arg1,
        lateout("eax") ret,
        options(nostack, preserves_flags)
    );
    ret
}

#[inline(always)]
pub unsafe fn syscall2(call: SysCall, arg1: usize, arg2: usize) -> isize {
    let ret: isize;
    asm!(
        "int 0x80",
        in("eax") call as u32,
        in("ebx") arg1,
        in("ecx") arg2,
        lateout("eax") ret,
        options(nostack, preserves_flags)
    );
    ret
}

#[inline(always)]
pub unsafe fn syscall3(call: SysCall, arg1: usize, arg2: usize, arg3: usize) -> isize {
    let ret: isize;
    asm!(
        "int 0x80",
        in("eax") call as u32,
        in("ebx") arg1,
        in("ecx") arg2,
        in("edx") arg3,
        lateout("eax") ret,
        options(nostack, preserves_flags)
    );
    ret
}

/// Print a string to the console
pub fn print(s: &str) -> Result<(), i32> {
    let result = unsafe {
        syscall2(SysCall::Print, s.as_ptr() as usize, s.len())
    };
    
    if result >= 0 {
        Ok(())
    } else {
        Err(result as i32)
    }
}

/// Print a string with a newline
pub fn println(s: &str) -> Result<(), i32> {
    print(s)?;
    print("\n")
}

/// Exit the current process
pub fn exit(code: i32) -> ! {
    unsafe {
        syscall1(SysCall::Exit, code as usize);
    }
    // Should never reach here
    loop {}
}

/// Get current process ID
pub fn getpid() -> u32 {
    unsafe {
        syscall0(SysCall::GetPid) as u32
    }
}

/// Sleep for specified milliseconds
pub fn sleep_ms(ms: u32) -> Result<(), i32> {
    let result = unsafe {
        syscall1(SysCall::Sleep, ms as usize)
    };
    
    if result >= 0 {
        Ok(())
    } else {
        Err(result as i32)
    }
}

/// Allocate memory
pub fn malloc(size: usize) -> Result<*mut u8, i32> {
    let result = unsafe {
        syscall1(SysCall::Malloc, size)
    };
    
    if result > 0 {
        Ok(result as *mut u8)
    } else {
        Err(result as i32)
    }
}

/// Free memory
pub fn free(ptr: *mut u8) -> Result<(), i32> {
    let result = unsafe {
        syscall1(SysCall::Free, ptr as usize)
    };
    
    if result >= 0 {
        Ok(())
    } else {
        Err(result as i32)
    }
}

/// Send a message via IPC
pub fn send_message(dest_pid: u32, msg: &[u8]) -> Result<(), i32> {
    let result = unsafe {
        syscall3(SysCall::SendMessage, dest_pid as usize, msg.as_ptr() as usize, msg.len())
    };
    
    if result >= 0 {
        Ok(())
    } else {
        Err(result as i32)
    }
}

/// Receive a message via IPC
pub fn receive_message(buffer: &mut [u8]) -> Result<(u32, usize), i32> {
    let result = unsafe {
        syscall2(SysCall::ReceiveMessage, buffer.as_mut_ptr() as usize, buffer.len())
    };
    
    if result >= 0 {
        let sender_pid = (result >> 16) as u32;
        let msg_len = (result & 0xFFFF) as usize;
        Ok((sender_pid, msg_len))
    } else {
        Err(result as i32)
    }
}

/// Default panic handler for userspace applications
#[panic_handler]
fn panic(info: &PanicInfo) -> ! {
    if let Some(s) = info.payload().downcast_ref::<&str>() {
        let _ = println(&format!("PANIC: {}", s));
    } else {
        let _ = println("PANIC: (no message)");
    }
    
    if let Some(location) = info.location() {
        let _ = println(&format!("  at {}:{}", location.file(), location.line()));
    }
    
    exit(-1);
}

/// Global allocator interface (stub for now)
pub struct MTOSAllocator;

unsafe impl core::alloc::GlobalAlloc for MTOSAllocator {
    unsafe fn alloc(&self, layout: core::alloc::Layout) -> *mut u8 {
        match malloc(layout.size()) {
            Ok(ptr) => ptr,
            Err(_) => core::ptr::null_mut(),
        }
    }

    unsafe fn dealloc(&self, ptr: *mut u8, _layout: core::alloc::Layout) {
        let _ = free(ptr);
    }
}

#[global_allocator]
static ALLOCATOR: MTOSAllocator = MTOSAllocator;

/// Application entry point macro
#[macro_export]
macro_rules! mtos_main {
    ($main:expr) => {
        #[no_mangle]
        pub extern "C" fn _start() -> ! {
            let result = $main();
            $crate::exit(result);
        }
    };
}

// Helper formatting functions (since we can't use std::fmt)
pub fn format_u32(value: u32) -> String<32> {
    let mut result = String::new();
    if value == 0 {
        result.push('0').ok();
        return result;
    }
    
    let mut val = value;
    let mut digits = heapless::Vec::<u8, 32>::new();
    
    while val > 0 {
        digits.push((val % 10) as u8 + b'0').ok();
        val /= 10;
    }
    
    for digit in digits.iter().rev() {
        result.push(*digit as char).ok();
    }
    
    result
}

/// Simple string formatting function
pub fn format(template: &str, value: u32) -> String<64> {
    let mut result = String::new();
    let value_str = format_u32(value);
    
    // Simple string substitution (replace first {} with value)
    let mut chars = template.chars();
    while let Some(ch) = chars.next() {
        if ch == '{' {
            // Look for closing brace
            if let Some(next_ch) = chars.next() {
                if next_ch == '}' {
                    // Insert the value
                    for value_ch in value_str.chars() {
                        result.push(value_ch).ok();
                    }
                } else {
                    // Not a placeholder, add both characters
                    result.push(ch).ok();
                    result.push(next_ch).ok();
                }
            } else {
                result.push(ch).ok();
            }
        } else {
            result.push(ch).ok();
        }
    }
    
    result
}
