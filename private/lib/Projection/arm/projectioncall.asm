;++
; Copyright (C) Microsoft Corporation
;
; Module Name:
;   projectioncall.asm
;
; Abstract:
;   This module contains support routines to marshal parameters for a projection call for the ARM platform.
;
; Reasons why we need this:
;   1) internally the arguments are kept in an array, but for WinRT call, the arguments must be passed as param1, param2, etc,
;      thus we need to convert from internal representation to WinRT.
;   2) WinRT API is usually defined as IFACEMETHODIMP (HRESULT __stdcall) and thus the caller must clean up the stack,
;      but stack cleanup can't be done in C++ code so we still need to do that in ASM.
;   3) Also note that we can't shortcut for 0,1,2,3 args and just call from C++ code as in arm_CallFunction -- because of #2.
; 
; This function must be called as:
;   HRESULT arm_ProjectionCall(IUnknown* thisPointer, ApcsCallLayout* callLayout, void* entryPoint);
;
; Routine description:
;   This function builds an appropriate argument list and calls the specified function.
;
; Arguments:
;   function      (r0) - Supplies a pointer to the target function.
;   callLayout    (r1) - Supplies a pointer to the ApcsCallLayout which contains the layout for outgoing call.
;
; Return Value:   (r0)
;   The value as returned by the target function.
;
; Notes:
;   This funciton makes the following call into WinRT:
;     return entryPoint(thisPointer, arguments[0], arguments[1], .., arguments[argumentCount - 1]);
;   What we do is just copy all general and float registers and stack from callLayout and make the call. Very simple.
;     callLayout:
;       [+0]  FloatRegisters;       // NULL if the call does not use parameters in VFP regs, otherwise pointer to memory allocated for float regs.
;       [+4]  GeneralRegisters;     // Pointer to memory allocated for generanl regs (should never be NULL as every call has implicit "this" parameter).
;       [+8]  Stack;                // NULL if the call does not use parameters stack, otherwise pointer to memory allocated for data on the stack.
;       [+12] StackSize;            // Actual size on the stack needed for parameters.
;   For outgoing call to the target:
;     The values are placed into r0-r3, d0-d7 and stack, according to the ARM Calling Convention.
;--


    TTL	Dll\JScript\arm\projectioncall.asm

    OPT	2	                        ; disable listing
#include "ksarm.h"
    OPT	1	                        ; reenable listing

    EXPORT arm_ProjectionCall
    IMPORT  __chkstk                ; See \\cpvsbuild\drops\dev11\Main\raw\current\sources\vctools\crt\crtw32\startup\arm\chkstk.asm.
#if defined(_CONTROL_FLOW_GUARD)
        IMPORT __guard_check_icall_fptr
#endif

    TEXTAREA

    NESTED_ENTRY arm_ProjectionCall

    ; IMPORTANT: the number of pushed registers (even vs odd) must be consistent with 8-bytes align check below.

    PROLOG_PUSH {r4-r6}, r11, lr   ; extra register pair to save info across icall/MarkerForExternalDebugStep check
    PROLOG_STACK_SAVE r11           ; mov r11, sp -- stack frame needed for ETW.

    #define callLayout          r1
    #define offset              r2
    #define spaceForStackArgs   r3
    #define stackSlotCount      r4
    #define inputArgs           r4

    ; Adjust sp to meet ABI requirement: stack must be 8-bytes aligned at any function boundary.
    ; Note: each reg is 4 bytes, to be aligned, the # of regs we push must be even. We push 3 (odd) # of regs in prolog. 
    ; If # of stack slots is even, alignment is broken, odd is OK.
    ; // TODO: Evanesco: it seems that keeping old values on stack can cause false-positive jcroot's. We should see if that's the case. 
    ldr     spaceForStackArgs, [callLayout, #12]        ; spaceForStackArgs = *(callLayout + 12) == stack size in bytes.
    mov     stackSlotCount, spaceForStackArgs, lsr #2   ; stackSlotCount = spaceForStackArgs / 4;
    asrs    r12, stackSlotCount, #1                     ; r-shift stackSlotCount into carry.
    addcc   stackSlotCount, stackSlotCount, #1          ; if carry is clear (even), add one more slot for alignment.

    ;Probe stack. This is required when we need more than 1 page of stack space.
    ; __chkstk will mark required number of pages as committed / throw stack overflow exception if can't.
    ;   - input:  r4 = the number of WORDS (word = 4 bytes) to allocate, 
    ;   - output: r4 = total number of BYTES probed/allocated.
    blx     __chkstk            ; after blx, r4 = the space to allocate, r5 = actual space needed for values on stack, r4 >= r5.

    ;offset stack by the amount of space we'll need.
    sub     sp, sp, r4

    ; Copy data that goes onto stack (if any).
    cbz     spaceForStackArgs, stackArgsDone    ; if (spaceForStackArgs == 0) goto stackArgsDone.
    ldr     inputArgs, [callLayout, #8]         ; inputArgs = *(callLayout + 8) == callLayout.Stack.
    mov     offset, #0
stackArgsLoop
    cmp     offset, spaceForStackArgs
    beq     stackArgsDone

    ldr     r12, [inputArgs, offset]
    str     r12, [sp, offset]
    
    add     offset, offset, #4      ; next iteration.
    b       stackArgsLoop
stackArgsDone

    mov     r5, r0          ; save call target (r0)
    mov     r6, callLayout  ; save callLayout  (r1)

    ; Verify that the call target is valid, and process last two arguments
#if defined(_CONTROL_FLOW_GUARD)
    mov32   r12, __guard_check_icall_fptr 
    ldr     r12, [r12]
    blx     r12
#endif

    mov     r0, r5          ; restore r0
    mov     callLayout, r6  ; restore callLayout (r1)

    ; Copy floating point registers (if any).
    ldr     inputArgs, [callLayout, #0]     ; inputArgs = *(callLayout + 0) == callLayout.FloatRegisters.
    cbz     inputArgs, floatArgsDone        ; if (inputArgs == 0) goto floatArgsDone.
    vldm    inputArgs, {s0-s15}
floatArgsDone

    mov     r12, r0                  ; Save entry point into r12.

    ; Copy general registers (these will always present because there will be at least "this" pointer).
    ldr     inputArgs, [callLayout, #4]
    ldm     inputArgs, {r0-r3}

    blx     r12                      ; Call the entryPoint.
    ; Return value will auto-be in r0, so we don't have to take special care about it.

    EPILOG_STACK_RESTORE r11        ; mov sp, r11

    EPILOG_POP {r4-r6}, r11, pc     ; Note that last one is the Pc, that's our "return" from this function.

    NESTED_END arm_ProjectionCall

    END
