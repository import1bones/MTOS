#![no_std]
#![no_main]

//! Simple shell userspace application for MTOS
//! 
//! Demonstrates interactive userspace application development
//! and basic command processing.

use mtos_runtime::{println, getpid, sleep_ms, mtos_main, format_u32};
use heapless::{String, Vec};

fn main() -> i32 {
    println("🐚 MTOS Simple Shell").unwrap();
    println("====================").unwrap();
    println("").unwrap();
    
    let pid = getpid();
    println(&format!("Shell running as PID: {}", format_u32(pid))).unwrap();
    println("").unwrap();
    
    println("📋 Available commands:").unwrap();
    println("  help     - Show this help message").unwrap();
    println("  info     - Show system information").unwrap();
    println("  echo <text> - Echo back the text").unwrap();
    println("  calc <a> <op> <b> - Simple calculator").unwrap();
    println("  sleep <ms> - Sleep for specified milliseconds").unwrap();
    println("  mem      - Test memory allocation").unwrap();
    println("  exit     - Exit the shell").unwrap();
    println("").unwrap();
    
    // Main shell loop (simulated - no real input in this demo)
    println("🔄 Simulating shell session:").unwrap();
    println("").unwrap();
    
    // Simulate some commands
    let demo_commands = [
        "help",
        "info", 
        "echo Hello MTOS!",
        "calc 15 + 27",
        "calc 100 / 7",
        "mem",
        "sleep 500",
        "exit"
    ];
    
    for command in demo_commands.iter() {
        println(&format!("mtos$ {}", command)).unwrap();
        execute_command(command);
        println("").unwrap();
        
        // Small delay between commands for demo effect
        sleep_ms(200).ok();
    }
    
    println("👋 Shell session ended").unwrap();
    0
}

fn execute_command(command: &str) {
    let mut parts = command.split_whitespace();
    
    match parts.next() {
        Some("help") => {
            println("📖 Shell Help:").unwrap();
            println("This is a demonstration shell for MTOS.").unwrap();
            println("It shows how userspace applications can").unwrap();
            println("interact with the kernel through system calls.").unwrap();
        }
        
        Some("info") => {
            println("💻 System Information:").unwrap();
            let pid = getpid();
            println(&format!("Current PID: {}", format_u32(pid))).unwrap();
            println("OS: MTOS (Modular Teaching OS)").unwrap();
            println("Architecture: Educational x86").unwrap();
            println("Userspace Language: Rust").unwrap();
        }
        
        Some("echo") => {
            print("🔊 ");
            for word in parts {
                print(word);
                print(" ");
            }
            println("").unwrap();
        }
        
        Some("calc") => {
            if let Some(a_str) = parts.next() {
                if let Some(op) = parts.next() {
                    if let Some(b_str) = parts.next() {
                        if let (Ok(a), Ok(b)) = (parse_u32(a_str), parse_u32(b_str)) {
                            let result = match op {
                                "+" => Some(a + b),
                                "-" => if a >= b { Some(a - b) } else { None },
                                "*" => Some(a * b),
                                "/" => if b != 0 { Some(a / b) } else { None },
                                "%" => if b != 0 { Some(a % b) } else { None },
                                _ => None
                            };
                            
                            match result {
                                Some(r) => println(&format!("🧮 {} {} {} = {}", 
                                                          format_u32(a), op, format_u32(b), format_u32(r))).unwrap(),
                                None => println("❌ Invalid operation or division by zero").unwrap(),
                            }
                        } else {
                            println("❌ Invalid numbers").unwrap();
                        }
                    } else {
                        println("❌ Missing second number").unwrap();
                    }
                } else {
                    println("❌ Missing operator").unwrap();
                }
            } else {
                println("❌ Missing first number").unwrap();
            }
        }
        
        Some("sleep") => {
            if let Some(ms_str) = parts.next() {
                if let Ok(ms) = parse_u32(ms_str) {
                    println(&format!("😴 Sleeping for {} ms...", format_u32(ms))).unwrap();
                    match sleep_ms(ms) {
                        Ok(_) => println("⏰ Wake up!").unwrap(),
                        Err(e) => println(&format!("❌ Sleep failed: {}", e)).unwrap(),
                    }
                } else {
                    println("❌ Invalid sleep duration").unwrap();
                }
            } else {
                println("❌ Missing sleep duration").unwrap();
            }
        }
        
        Some("mem") => {
            println("🧠 Testing memory allocation...").unwrap();
            match mtos_runtime::malloc(512) {
                Ok(ptr) => {
                    println("✅ Allocated 512 bytes").unwrap();
                    match mtos_runtime::free(ptr) {
                        Ok(_) => println("✅ Memory freed successfully").unwrap(),
                        Err(e) => println(&format!("⚠️ Failed to free memory: {}", e)).unwrap(),
                    }
                }
                Err(e) => println(&format!("❌ Allocation failed: {}", e)).unwrap(),
            }
        }
        
        Some("exit") => {
            println("👋 Goodbye!").unwrap();
        }
        
        Some(unknown) => {
            println(&format!("❓ Unknown command: {}", unknown)).unwrap();
            println("Type 'help' for available commands").unwrap();
        }
        
        None => {
            // Empty command, do nothing
        }
    }
}

fn print(s: &str) {
    mtos_runtime::print(s).ok();
}

fn parse_u32(s: &str) -> Result<u32, ()> {
    let mut result = 0u32;
    
    for ch in s.chars() {
        if let Some(digit) = ch.to_digit(10) {
            result = result.checked_mul(10).ok_or(())?;
            result = result.checked_add(digit).ok_or(())?;
        } else {
            return Err(());
        }
    }
    
    Ok(result)
}

mtos_main!(main);
