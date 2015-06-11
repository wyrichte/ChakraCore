;++
; Copyright (C) Microsoft Corporation
;
; Module Name:
;   projectioncall.asm
;
; Abstract:
;   This module contains support routines to marshal parameters for a projection call for the ARM64 platform.
;
; Reasons why we need this:
;   1) internally the arguments are kept in an array, but for WinRT call, the arguments must be passed as param1, param2, etc,
;      thus we need to convert from internal representation to WinRT.
;   2) WinRT API is usually defined as IFACEMETHODIMP (HRESULT __stdcall) and thus the caller must clean up the stack,
;      but stack cleanup can't be done in C++ code so we still need to do that in ASM.
;   3) Also note that we can't shortcut for 0,1,2,3 args and just call from C++ code as in arm_CallFunction -- because of #2.
; 
; This function must be called as:
;   HRESULT arm64_ProjectionCall(IUnknown* thisPointer, ApcsCallLayout* callLayout, void* entryPoint);
;
; Routine description:
;   This function builds an appropriate argument list and calls the specified function.
;
; Arguments:
;   function      (x0) - Supplies a pointer to the target function.
;   callLayout    (x1) - Supplies a pointer to the ApcsCallLayout which contains the layout for outgoing call.
;
; Return Value:   (x0)
;   The value as returned by the target function.
;
; Notes:
;   This funciton makes the following call into WinRT:
;     return entryPoint(thisPointer, arguments[0], arguments[1], .., arguments[argumentCount - 1]);
;   What we do is just copy all general and float registers and stack from callLayout and make the call. Very simple.
;     callLayout:
;       [+0]  FloatRegisters;       // NULL if the call does not use parameters in VFP regs, otherwise pointer to memory allocated for float regs.
;       [+8]  GeneralRegisters;     // Pointer to memory allocated for generanl regs (should never be NULL as every call has implicit "this" parameter).
;       [+16] Stack;                // NULL if the call does not use parameters stack, otherwise pointer to memory allocated for data on the stack.
;       [+24] StackSize;            // Actual size on the stack needed for parameters.
;   For outgoing call to the target:
;     The values are placed into r0-r3, d0-d7 and stack, according to the ARM Calling Convention.
;--


    TTL	Dll\JScript\arm\projectioncall.asm

    OPT	2	                        ; disable listing
#include "ksarm64.h"
    OPT	1	                        ; reenable listing

    EXPORT arm64_ProjectionCall
    IMPORT  __chkstk                ; See \\cpvsbuild\drops\dev11\Main\raw\current\sources\vctools\crt\crtw32\startup\arm\chkstk.asm.

    TEXTAREA

    NESTED_ENTRY arm64_ProjectionCall

    ; IMPORTANT: the number of pushed registers (even vs odd) must be consistent with 8-bytes align check below.
    PROLOG_SAVE_REG_PAIR fp, lr, #-16!

    #define callLayout          x1
    #define offset              x9
    #define spaceForStackArgs   x10
    #define stackSlotCount      x11
    #define inputArgs           x12

    ; Adjust sp to meet ABI requirement: stack must be 16-bytes aligned at any function boundary.
    ; Note: each reg is 8 bytes, to be aligned, the # of regs we push must be even.
    ; If # of stack slots is even, alignment is broken, odd is OK.
    ; // TODO: Evanesco: it seems that keeping old values on stack can cause false-positive jcroot's. We should see if that's the case. 
    ldr     spaceForStackArgs, [callLayout, #24]        ; spaceForStackArgs = *(callLayout + 24) == stack size in bytes.
    lsr     stackSlotCount, spaceForStackArgs, #3       ; stackSlotCount = spaceForStackArgs / 8;
    and     x16, stackSlotCount, #1                     ; get low bit of slot count in x16.
    add     stackSlotCount, stackSlotCount, x16         ; add one more slot for alignment if necessary

    ;Probe stack. This is required when we need more than 1 page of stack space.
    ; __chkstk will mark required number of pages as committed / throw stack overflow exception if can't.
    ;   - input:  x15 = size of allocation / 16
    lsr     x15, stackSlotCount, #1                      ; get stack allocation / 16
    bl      __chkstk

    ;offset stack by the amount of space we'll need.
    sub     sp, sp, x15, lsl #4

    ; Copy data that goes onto stack (if any).
    cbz     spaceForStackArgs, stackArgsDone    ; if (spaceForStackArgs == 0) goto stackArgsDone.
    ldr     inputArgs, [callLayout, #16]        ; inputArgs = *(callLayout + 16) == callLayout.Stack.
    mov     offset, #0
stackArgsLoop
    cmp     offset, spaceForStackArgs
    beq     stackArgsDone

    ldr     x16, [inputArgs, offset]
    str     x16, [sp, offset]
    
    add     offset, offset, #8      ; next iteration.
    b       stackArgsLoop
stackArgsDone

    ; Copy floating point registers (if any).
    ldr     inputArgs, [callLayout, #0]     ; inputArgs = *(callLayout + 0) == callLayout.FloatRegisters.
    cbz     inputArgs, floatArgsDone        ; if (inputArgs == 0) goto floatArgsDone.
    ldp     q0, q1, [inputArgs, #0]
    ldp     q2, q3, [inputArgs, #32]
    ldp     q4, q5, [inputArgs, #64]
    ldp     q6, q7, [inputArgs, #96]
floatArgsDone

    mov     x16, x0                  ; Save entry point into r12.

    ; Copy general registers (these will always present because there will be at least "this" pointer).
    ldr     inputArgs, [callLayout, #8]
    ldp     x0, x1, [inputArgs, #0]
    ldp     x2, x3, [inputArgs, #16]
    ldp     x4, x5, [inputArgs, #32]
    ldp     x6, x7, [inputArgs, #48]

    blr     x16                      ; Call the entryPoint.
    ; Return value will auto-be in r0, so we don't have to take special care about it.

    EPILOG_RESTORE_REG_PAIR fp, lr, #16!
    EPILOG_RETURN

    NESTED_END arm64_ProjectionCall

    END
