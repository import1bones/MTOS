# MTOS Multi-Platform Build System
# Comprehensive build system supporting multiple platforms and deployment targets

CMAKE_MINIMUM_REQUIRED(VERSION 3.16)
PROJECT(MTOS VERSION 1.0.0 LANGUAGES C ASM)

# ============================================================================
# PLATFORM DETECTION AND CONFIGURATION
# ============================================================================

# Detect host platform
if(WIN32)
    set(HOST_PLATFORM "windows")
    set(HOST_ARCH "x86_64")
elseif(APPLE)
    set(HOST_PLATFORM "macos")
    execute_process(COMMAND uname -m OUTPUT_VARIABLE HOST_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
elseif(UNIX)
    set(HOST_PLATFORM "linux")
    execute_process(COMMAND uname -m OUTPUT_VARIABLE HOST_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
    message(FATAL_ERROR "Unsupported host platform")
endif()

message(STATUS "Host platform: ${HOST_PLATFORM} (${HOST_ARCH})")

# Target platform configuration (can be overridden)
set(TARGET_PLATFORM "${HOST_PLATFORM}" CACHE STRING "Target platform: windows, linux, macos, embedded")
set(TARGET_ARCH "i386" CACHE STRING "Target architecture: i386, x86_64, arm, aarch64")

# Build configuration
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type: Debug, Release, MinSizeRel")
set(MTOS_ENABLE_TESTS ON CACHE BOOL "Enable test framework")
set(MTOS_ENABLE_DOCS ON CACHE BOOL "Enable documentation generation")
set(MTOS_ENABLE_CONTAINER OFF CACHE BOOL "Enable container builds")

# ============================================================================
# TOOLCHAIN SELECTION
# ============================================================================

# Cross-compilation support
if(NOT "${TARGET_PLATFORM}" STREQUAL "${HOST_PLATFORM}" OR NOT "${TARGET_ARCH}" STREQUAL "${HOST_ARCH}")
    set(CMAKE_SYSTEM_NAME Generic)
    set(CMAKE_SYSTEM_PROCESSOR ${TARGET_ARCH})
    
    # Set cross-compilation toolchain
    if(TARGET_ARCH STREQUAL "i386")
        set(CMAKE_C_COMPILER "gcc")
        set(CMAKE_ASM_COMPILER "gcc")
        set(CMAKE_LINKER "ld")
        set(CMAKE_C_FLAGS "-m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector")
        set(CMAKE_ASM_FLAGS "--32")
        set(CMAKE_EXE_LINKER_FLAGS "-m elf_i386")
    elseif(TARGET_ARCH STREQUAL "x86_64")
        set(CMAKE_C_COMPILER "gcc")
        set(CMAKE_ASM_COMPILER "gcc")
        set(CMAKE_C_FLAGS "-m64 -nostdlib -nostdinc -fno-builtin -fno-stack-protector")
        set(CMAKE_ASM_FLAGS "--64")
        set(CMAKE_EXE_LINKER_FLAGS "-m elf_x86_64")
    elseif(TARGET_ARCH STREQUAL "arm")
        set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
        set(CMAKE_ASM_COMPILER "arm-none-eabi-gcc")
        set(CMAKE_LINKER "arm-none-eabi-ld")
        set(CMAKE_C_FLAGS "-mcpu=cortex-a7 -nostdlib -nostdinc -fno-builtin")
    endif()
endif()

# Platform-specific compiler settings
if(TARGET_PLATFORM STREQUAL "windows")
    # Windows-specific settings
    if(MSVC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
    endif()
elseif(TARGET_PLATFORM STREQUAL "linux")
    # Linux-specific settings
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -std=c99")
elseif(TARGET_PLATFORM STREQUAL "macos")
    # macOS-specific settings
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -std=c99")
endif()

# ============================================================================
# DIRECTORY STRUCTURE
# ============================================================================

set(MTOS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(MTOS_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(MTOS_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/install)

# Source directories
set(BOOT_DIR ${MTOS_SOURCE_DIR}/boot)
set(KERNEL_DIR ${MTOS_SOURCE_DIR}/kernel)
set(INCLUDE_DIR ${MTOS_SOURCE_DIR}/include)
set(TESTS_DIR ${MTOS_SOURCE_DIR}/tests)
set(DOCS_DIR ${MTOS_SOURCE_DIR}/docs)
set(TOOLS_DIR ${MTOS_SOURCE_DIR}/tools)
set(USERSPACE_DIR ${MTOS_SOURCE_DIR}/userspace)

# Build directories
set(BUILD_BOOT_DIR ${MTOS_BINARY_DIR}/boot)
set(BUILD_KERNEL_DIR ${MTOS_BINARY_DIR}/kernel)
set(BUILD_TESTS_DIR ${MTOS_BINARY_DIR}/tests)
set(BUILD_TOOLS_DIR ${MTOS_BINARY_DIR}/tools)

# ============================================================================
# BUILD CONFIGURATION
# ============================================================================

# Component selection (can be overridden via command line)
set(MEMORY_ALLOCATOR "bitmap" CACHE STRING "Memory allocator: bitmap, buddy")
set(SCHEDULER "round_robin" CACHE STRING "Scheduler: round_robin, priority")
set(IPC_MECHANISM "message_queue" CACHE STRING "IPC mechanism: message_queue, shared_memory")

# Validate component selections
set(VALID_ALLOCATORS bitmap buddy)
set(VALID_SCHEDULERS round_robin priority)
set(VALID_IPC message_queue shared_memory)

if(NOT MEMORY_ALLOCATOR IN_LIST VALID_ALLOCATORS)
    message(FATAL_ERROR "Invalid MEMORY_ALLOCATOR: ${MEMORY_ALLOCATOR}. Valid options: ${VALID_ALLOCATORS}")
endif()

if(NOT SCHEDULER IN_LIST VALID_SCHEDULERS)
    message(FATAL_ERROR "Invalid SCHEDULER: ${SCHEDULER}. Valid options: ${VALID_SCHEDULERS}")
endif()

if(NOT IPC_MECHANISM IN_LIST VALID_IPC)
    message(FATAL_ERROR "Invalid IPC_MECHANISM: ${IPC_MECHANISM}. Valid options: ${VALID_IPC}")
endif()

message(STATUS "Configuration: ${MEMORY_ALLOCATOR} + ${SCHEDULER} + ${IPC_MECHANISM}")

# ============================================================================
# COMPILER FLAGS AND DEFINITIONS
# ============================================================================

# Base flags for kernel
set(KERNEL_C_FLAGS 
    -nostdlib 
    -nostdinc 
    -fno-builtin 
    -fno-stack-protector 
    -nostartfiles 
    -nodefaultlibs 
    -Wall 
    -Wextra 
    -I${INCLUDE_DIR}
    -I${KERNEL_DIR}
)

# Target-specific flags
if(TARGET_ARCH STREQUAL "i386")
    list(APPEND KERNEL_C_FLAGS -m32)
    set(KERNEL_ASM_FLAGS --32)
    set(KERNEL_LD_FLAGS -m elf_i386)
elseif(TARGET_ARCH STREQUAL "x86_64")
    list(APPEND KERNEL_C_FLAGS -m64 -mcmodel=kernel)
    set(KERNEL_ASM_FLAGS --64)
    set(KERNEL_LD_FLAGS -m elf_x86_64)
endif()

# Build type specific flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND KERNEL_C_FLAGS -g -O0 -DDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND KERNEL_C_FLAGS -O2 -DNDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    list(APPEND KERNEL_C_FLAGS -Os -DNDEBUG)
endif()

# Component-specific definitions
add_compile_definitions(
    MTOS_MEMORY_ALLOCATOR_${MEMORY_ALLOCATOR}
    MTOS_SCHEDULER_${SCHEDULER}
    MTOS_IPC_${IPC_MECHANISM}
)

# ============================================================================
# SOURCE FILES
# ============================================================================

# Boot sources
set(BOOT_SOURCES
    ${BOOT_DIR}/boot.S
    ${BOOT_DIR}/main.c
)

# Kernel interface sources
set(KERNEL_INTERFACE_SOURCES
    ${KERNEL_DIR}/interfaces/kernel_interfaces.c
)

# Memory allocator sources
set(MEMORY_SOURCES
    ${KERNEL_DIR}/memory/bitmap_allocator.c
    ${KERNEL_DIR}/memory/buddy_allocator.c
)

# Scheduler sources
set(SCHEDULER_SOURCES
    ${KERNEL_DIR}/scheduler/round_robin_scheduler.c
    ${KERNEL_DIR}/scheduler/priority_scheduler.c
)

# IPC sources
set(IPC_SOURCES
    ${KERNEL_DIR}/ipc/message_queue_ipc.c
    ${KERNEL_DIR}/ipc/shared_memory_ipc.c
)

# All kernel sources
set(KERNEL_SOURCES
    ${KERNEL_INTERFACE_SOURCES}
    ${MEMORY_SOURCES}
    ${SCHEDULER_SOURCES}
    ${IPC_SOURCES}
)

# Header files
file(GLOB_RECURSE HEADER_FILES 
    ${INCLUDE_DIR}/*.h
    ${KERNEL_DIR}/**/*.h
)

# ============================================================================
# BUILD TARGETS
# ============================================================================

# Create output directories
file(MAKE_DIRECTORY ${BUILD_BOOT_DIR})
file(MAKE_DIRECTORY ${BUILD_KERNEL_DIR})
file(MAKE_DIRECTORY ${MTOS_INSTALL_DIR})

# Boot object files
add_library(boot_objects OBJECT ${BOOT_SOURCES})
target_compile_options(boot_objects PRIVATE ${KERNEL_C_FLAGS})
set_target_properties(boot_objects PROPERTIES
    COMPILE_FLAGS "${KERNEL_C_FLAGS}"
)

# Kernel object files
add_library(kernel_objects OBJECT ${KERNEL_SOURCES})
target_compile_options(kernel_objects PRIVATE ${KERNEL_C_FLAGS})
target_include_directories(kernel_objects PRIVATE ${INCLUDE_DIR} ${KERNEL_DIR})

# Bootloader binary
add_custom_target(bootloader
    COMMAND ${CMAKE_LINKER} ${KERNEL_LD_FLAGS} 
            -T ${BOOT_DIR}/boot.ld 
            -o ${BUILD_BOOT_DIR}/bootloader.elf
            $<TARGET_OBJECTS:boot_objects>
    COMMAND ${CMAKE_OBJCOPY} -O binary 
            ${BUILD_BOOT_DIR}/bootloader.elf 
            ${BUILD_BOOT_DIR}/bootloader.bin
    DEPENDS boot_objects
    COMMENT "Building bootloader"
)

# Kernel binary
add_custom_target(kernel
    COMMAND ${CMAKE_LINKER} ${KERNEL_LD_FLAGS}
            -T ${KERNEL_DIR}/kernel.ld
            -o ${BUILD_KERNEL_DIR}/kernel.elf
            $<TARGET_OBJECTS:kernel_objects>
    COMMAND ${CMAKE_OBJCOPY} -O binary
            ${BUILD_KERNEL_DIR}/kernel.elf
            ${BUILD_KERNEL_DIR}/kernel.bin
    DEPENDS kernel_objects
    COMMENT "Building kernel with ${MEMORY_ALLOCATOR}+${SCHEDULER}+${IPC_MECHANISM}"
)

# OS image
add_custom_target(os_image
    COMMAND cat ${BUILD_BOOT_DIR}/bootloader.bin ${BUILD_KERNEL_DIR}/kernel.bin > ${MTOS_BINARY_DIR}/mtos.img
    DEPENDS bootloader kernel
    COMMENT "Creating OS image"
)

# Main build target
add_custom_target(mtos ALL
    DEPENDS os_image
)

# ============================================================================
# TESTING FRAMEWORK
# ============================================================================

if(MTOS_ENABLE_TESTS)
    enable_testing()
    
    # Find Python for test framework
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    
    # Test dependencies
    find_program(QEMU_SYSTEM_I386 qemu-system-i386)
    if(NOT QEMU_SYSTEM_I386)
        message(WARNING "QEMU not found. Tests will be disabled.")
        set(MTOS_ENABLE_TESTS OFF)
    endif()
    
    if(MTOS_ENABLE_TESTS)
        # Boot tests
        add_test(NAME test_boot
            COMMAND ${Python3_EXECUTABLE} ${TESTS_DIR}/test_boot.py
            WORKING_DIRECTORY ${MTOS_SOURCE_DIR}
        )
        
        # Memory tests
        add_test(NAME test_memory
            COMMAND ${Python3_EXECUTABLE} ${TESTS_DIR}/test_memory.py
            WORKING_DIRECTORY ${MTOS_SOURCE_DIR}
        )
        
        # Integration tests
        add_test(NAME test_integration
            COMMAND ${Python3_EXECUTABLE} ${TESTS_DIR}/test_integration.py
            WORKING_DIRECTORY ${MTOS_SOURCE_DIR}
        )
        
        # Component comparison tests
        add_test(NAME test_allocator_comparison
            COMMAND ${Python3_EXECUTABLE} ${TESTS_DIR}/compare_components.py allocators
            WORKING_DIRECTORY ${MTOS_SOURCE_DIR}
        )
        
        add_test(NAME test_scheduler_comparison
            COMMAND ${Python3_EXECUTABLE} ${TESTS_DIR}/compare_components.py schedulers
            WORKING_DIRECTORY ${MTOS_SOURCE_DIR}
        )
    endif()
endif()

# ============================================================================
# DOCUMENTATION
# ============================================================================

if(MTOS_ENABLE_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        # Configure Doxygen
        set(DOXYGEN_IN ${DOCS_DIR}/Doxyfile.in)
        set(DOXYGEN_OUT ${MTOS_BINARY_DIR}/Doxyfile)
        
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        
        add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${MTOS_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    else()
        message(WARNING "Doxygen not found. Documentation will not be generated.")
    endif()
endif()

# ============================================================================
# INSTALLATION AND PACKAGING
# ============================================================================

# Install targets
install(FILES ${MTOS_BINARY_DIR}/mtos.img 
        DESTINATION bin
        COMPONENT runtime)

install(FILES ${BUILD_BOOT_DIR}/bootloader.bin
              ${BUILD_KERNEL_DIR}/kernel.bin
        DESTINATION bin/components
        COMPONENT development)

install(DIRECTORY ${INCLUDE_DIR}/
        DESTINATION include
        COMPONENT development
        FILES_MATCHING PATTERN "*.h")

install(DIRECTORY ${TESTS_DIR}/
        DESTINATION share/mtos/tests
        COMPONENT testing
        FILES_MATCHING PATTERN "*.py")

install(FILES README.md STUDENT_GUIDE.md README_TEST_FRAMEWORK.md
        DESTINATION share/mtos/docs
        COMPONENT documentation)

# Package configuration
set(CPACK_PACKAGE_NAME "MTOS")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Modular Teaching Operating System")
set(CPACK_PACKAGE_VENDOR "MTOS Project")
set(CPACK_PACKAGE_CONTACT "mtos@example.com")

# Platform-specific packaging
if(TARGET_PLATFORM STREQUAL "windows")
    set(CPACK_GENERATOR "ZIP;NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "MTOS ${PROJECT_VERSION}")
    set(CPACK_NSIS_PACKAGE_NAME "MTOS")
elseif(TARGET_PLATFORM STREQUAL "linux")
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "MTOS Project")
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
elseif(TARGET_PLATFORM STREQUAL "macos")
    set(CPACK_GENERATOR "TGZ;Bundle")
    set(CPACK_BUNDLE_NAME "MTOS")
endif()

include(CPack)

# ============================================================================
# CUSTOM TARGETS FOR EDUCATION
# ============================================================================

# Component-specific builds
foreach(allocator ${VALID_ALLOCATORS})
    add_custom_target(build-${allocator}
        COMMAND ${CMAKE_COMMAND} -DMEMORY_ALLOCATOR=${allocator} ${MTOS_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} --build ${MTOS_BINARY_DIR}
        COMMENT "Building with ${allocator} allocator"
    )
endforeach()

foreach(scheduler ${VALID_SCHEDULERS})
    add_custom_target(build-${scheduler}
        COMMAND ${CMAKE_COMMAND} -DSCHEDULER=${scheduler} ${MTOS_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} --build ${MTOS_BINARY_DIR}
        COMMENT "Building with ${scheduler} scheduler"
    )
endforeach()

foreach(ipc ${VALID_IPC})
    add_custom_target(build-${ipc}
        COMMAND ${CMAKE_COMMAND} -DIPC_MECHANISM=${ipc} ${MTOS_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} --build ${MTOS_BINARY_DIR}
        COMMENT "Building with ${ipc} IPC"
    )
endforeach()

# Demo targets
add_custom_target(demo-all
    COMMAND ${CMAKE_COMMAND} -E echo "Building all component combinations..."
    DEPENDS build-bitmap build-buddy build-round_robin build-priority build-message_queue build-shared_memory
    COMMENT "Building all available implementations"
)

# ============================================================================
# EMULATION AND TESTING TARGETS
# ============================================================================

if(QEMU_SYSTEM_I386)
    add_custom_target(run
        COMMAND ${QEMU_SYSTEM_I386} -drive format=raw,file=${MTOS_BINARY_DIR}/mtos.img -no-reboot -no-shutdown
        DEPENDS mtos
        COMMENT "Running MTOS in QEMU"
    )
    
    add_custom_target(debug
        COMMAND ${QEMU_SYSTEM_I386} -drive format=raw,file=${MTOS_BINARY_DIR}/mtos.img -no-reboot -no-shutdown -s -S
        DEPENDS mtos
        COMMENT "Running MTOS in QEMU with GDB support"
    )
endif()

# ============================================================================
# SUMMARY
# ============================================================================

message(STATUS "")
message(STATUS "MTOS Build Configuration Summary:")
message(STATUS "==================================")
message(STATUS "Host Platform: ${HOST_PLATFORM} (${HOST_ARCH})")
message(STATUS "Target Platform: ${TARGET_PLATFORM} (${TARGET_ARCH})")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Memory Allocator: ${MEMORY_ALLOCATOR}")
message(STATUS "Scheduler: ${SCHEDULER}")
message(STATUS "IPC Mechanism: ${IPC_MECHANISM}")
message(STATUS "Tests Enabled: ${MTOS_ENABLE_TESTS}")
message(STATUS "Docs Enabled: ${MTOS_ENABLE_DOCS}")
message(STATUS "")
message(STATUS "Build Commands:")
message(STATUS "  cmake --build . --target mtos     # Build OS")
message(STATUS "  cmake --build . --target run      # Run in QEMU")
message(STATUS "  ctest                              # Run tests")
message(STATUS "  cmake --build . --target docs     # Generate docs")
message(STATUS "  cmake --build . --target package  # Create package")
message(STATUS "")
