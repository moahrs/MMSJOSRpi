// To keep this in the first portion of the binary.
.section ".text.boot"

// Make _start global.
.globl _start

// Entry point for the kernel.
// r15 -> should begin execution at 0x8000.
// r0 -> 0x00000000
// r1 -> 0x00000C42
// r2 -> 0x00000100 - start of ATAGS
// preserve these registers as argument for mmsjos_main
_start:
;@"========================================================================="
@#    PI NSACR regsister setup for access to floating point unit
@#    Cortex A-7 => Section 4.3.34. Non-Secure Access Control Register
@#    Cortex A-53 => Section 4.5.32. Non-Secure Access Control Register
;@"========================================================================="
    mrc p15, 0, r0, c1, c1, 2                           // Read NSACR into R0
    cmp r0, #0x00000C00                                 // Access turned on or in AARCH32 mode and can not touch register or EL3 fault
    beq .free_to_enable_fpu1
    orr r0, r0, #0x3<<10                                // Set access to both secure and non secure modes
    mcr p15, 0, r0, c1, c1, 2                           // Write NSACR
;@"========================================================================="
@#                               Bring fpu online
;@"========================================================================="
.free_to_enable_fpu1:
    mrc p15, 0, r0, c1, c0, #2                          // R0 = Access Control Register
    orr r0, #(0x300000 + 0xC00000)                      // Enable Single & Double Precision
    mcr p15,0,r0,c1,c0, #2                              // Access Control Register = R0
    mov r0, #0x40000000                                 // R0 = Enable VFP
    vmsr fpexc, r0                                      // FPEXC = R0
;@"========================================================================="
@#                              Enable L1 cache
;@"========================================================================="
.equ SCTLR_ENABLE_DATA_CACHE,           0x4
.equ SCTLR_ENABLE_BRANCH_PREDICTION,    0x800
.equ SCTLR_ENABLE_INSTRUCTION_CACHE,    0x1000
    mrc p15,0,r0,c1,c0,0                                ;@ R0 = System Control Register

    /* Enable caches and branch prediction */
    orr r0, #SCTLR_ENABLE_BRANCH_PREDICTION             ;@ Branch prediction on
    orr r0, #SCTLR_ENABLE_DATA_CACHE                    ;@ Data cache on
    orr r0, #SCTLR_ENABLE_INSTRUCTION_CACHE             ;@ Instruction cache on

    mcr p15,0,r0,c1,c0,0                                ;@ System Control Register = R0
;@"========================================================================="
;@#                  ARM CPU ID CONSTANT DEFINITIONS
;@"========================================================================="
    mrc p15, 0, r6, c0, c0, 0                           ;@ Read CPU ID Register
    ldr r1, =RPI_ARM_CPU_ID                             ;@ Address to RPI_ARM_CPU_ID
    str r6, [r1]                                        ;@ Save RPI_ARM_CPU_ID for interface
;@"========================================================================="
@#        Finally that all done jump to the C compiler entry point
;@"========================================================================="    
    // Setup the stack.
    mov sp, #0x8000

    // Clear out bss.
    ldr r4, =__bss_start
    ldr r9, =__bss_end
    mov r5, #0
    mov r6, #0
    mov r7, #0
    mov r8, #0
    b       2f

1:
    // store multiple at r4.
    stmia r4!, {r5-r8}

    // If we are still below bss_end, loop.
2:
    cmp r4, r9
    blo 1b

    // Call mmsjos_main
	mov r2, #0x100
    ldr r3, =mmsjos_main
    blx r3
    b halt

// halt
halt:
    wfe
    b halt

.globl dummy
dummy:
    bx lr

.globl RPI_ARM_CPU_ID;                                ;@ Make sure RPI_ARM_CPU_ID label is global
RPI_ARM_CPU_ID : .4byte 0;                            ;@ Arm Cpu ID is 4 byte variable in 32bit mode
    
    