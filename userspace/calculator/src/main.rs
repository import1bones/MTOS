#![no_std]
#![no_main]

//! Calculator userspace application for MTOS
//! 
//! Demonstrates more complex userspace functionality including
//! mathematical operations and memory management.

use mtos_runtime::{println, getpid, mtos_main, format_u32};
use heapless::String;

fn main() -> i32 {
    println("🧮 MTOS Calculator Application").unwrap();
    println("==============================").unwrap();
    println("").unwrap();
    
    let pid = getpid();
    println(&format!("Running as PID: {}", format_u32(pid))).unwrap();
    println("").unwrap();
    
    // Demonstrate basic arithmetic
    println("📊 Basic Arithmetic Operations:").unwrap();
    
    let a = 42;
    let b = 17;
    
    println(&format!("Numbers: {} and {}", format_u32(a), format_u32(b))).unwrap();
    
    // Addition
    let sum = a + b;
    println(&format!("Addition: {} + {} = {}", format_u32(a), format_u32(b), format_u32(sum))).unwrap();
    
    // Subtraction
    let diff = a - b;
    println(&format!("Subtraction: {} - {} = {}", format_u32(a), format_u32(b), format_u32(diff))).unwrap();
    
    // Multiplication
    let product = a * b;
    println(&format!("Multiplication: {} × {} = {}", format_u32(a), format_u32(b), format_u32(product))).unwrap();
    
    // Division
    let quotient = a / b;
    let remainder = a % b;
    println(&format!("Division: {} ÷ {} = {} remainder {}", 
                   format_u32(a), format_u32(b), format_u32(quotient), format_u32(remainder))).unwrap();
    
    println("").unwrap();
    
    // Demonstrate more complex operations
    println("🔬 Advanced Operations:").unwrap();
    
    // Square calculation
    let square = a * a;
    println(&format!("Square of {}: {}", format_u32(a), format_u32(square))).unwrap();
    
    // Simple power calculation (a^3)
    let cube = a * a * a;
    println(&format!("Cube of {}: {}", format_u32(a), format_u32(cube))).unwrap();
    
    // Factorial calculation (for small numbers)
    let factorial_num = 5;
    let factorial = calculate_factorial(factorial_num);
    println(&format!("Factorial of {}: {}", format_u32(factorial_num), format_u32(factorial))).unwrap();
    
    // Fibonacci sequence
    println("").unwrap();
    println("🌀 Fibonacci Sequence (first 10 numbers):").unwrap();
    for i in 0..10 {
        let fib = fibonacci(i);
        println(&format!("F({}) = {}", format_u32(i), format_u32(fib))).unwrap();
    }
    
    // Prime number check
    println("").unwrap();
    println("🔍 Prime Number Analysis:").unwrap();
    for num in 2..20 {
        if is_prime(num) {
            println(&format!("{} is prime", format_u32(num))).unwrap();
        }
    }
    
    // Memory usage demonstration
    println("").unwrap();
    println("🧠 Memory Operations:").unwrap();
    
    // Allocate some memory for calculations
    match mtos_runtime::malloc(256) {
        Ok(ptr) => {
            println("✅ Allocated 256 bytes for calculations").unwrap();
            
            // Simulate some work with the memory
            // (In a real implementation, we'd use this memory)
            
            match mtos_runtime::free(ptr) {
                Ok(_) => println("✅ Memory freed successfully").unwrap(),
                Err(e) => println(&format!("⚠️ Failed to free memory: {}", e)).unwrap(),
            }
        }
        Err(e) => {
            println(&format!("❌ Memory allocation failed: {}", e)).unwrap();
        }
    }
    
    println("").unwrap();
    println("🎉 Calculator operations completed successfully!").unwrap();
    println("📝 Educational Notes:").unwrap();
    println("  • All calculations performed in userspace").unwrap();
    println("  • Memory management handled by kernel allocator").unwrap();
    println("  • System calls used for I/O operations").unwrap();
    println("  • Demonstrates Rust's no_std capabilities").unwrap();
    
    0
}

/// Calculate factorial of a number (simple iterative approach)
fn calculate_factorial(n: u32) -> u32 {
    if n <= 1 {
        return 1;
    }
    
    let mut result = 1;
    for i in 2..=n {
        result *= i;
    }
    result
}

/// Calculate nth Fibonacci number (simple recursive approach)
fn fibonacci(n: u32) -> u32 {
    match n {
        0 => 0,
        1 => 1,
        _ => {
            let mut a = 0;
            let mut b = 1;
            for _ in 2..=n {
                let temp = a + b;
                a = b;
                b = temp;
            }
            b
        }
    }
}

/// Check if a number is prime
fn is_prime(n: u32) -> bool {
    if n < 2 {
        return false;
    }
    if n == 2 {
        return true;
    }
    if n % 2 == 0 {
        return false;
    }
    
    let mut i = 3;
    while i * i <= n {
        if n % i == 0 {
            return false;
        }
        i += 2;
    }
    true
}

// Helper function to format strings
fn format(template: &str, value: u32) -> String<64> {
    let mut result = String::new();
    let value_str = format_u32(value);
    
    // Simple string substitution (replace first {} with value)
    let mut found_placeholder = false;
    for ch in template.chars() {
        if ch == '{' && !found_placeholder {
            // Start of placeholder - skip until '}'
            found_placeholder = true;
        } else if ch == '}' && found_placeholder {
            // End of placeholder - insert value
            for value_ch in value_str.chars() {
                result.push(value_ch).ok();
            }
            found_placeholder = false;
        } else if !found_placeholder {
            result.push(ch).ok();
        }
    }
    
    result
}

mtos_main!(main);
