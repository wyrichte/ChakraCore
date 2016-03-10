;++
; Copyright (C) Microsoft Corporation
;
; Module Name:
;   UnknownImplHelper.asm
;
; Abstract:
;
;--
    TTL	Dll\JScript\arm\UnknownImplHelper.asm

    OPT	2	; disable listing
#include "ksarm.h"
    OPT	1	; reenable listing

#if defined(_CONTROL_FLOW_GUARD)
    IMPORT __guard_check_icall_fptr
#endif
    EXPORT IndirectMethodInvoker

    TEXTAREA

; Define the CUnknownImpl_MethodImpl macro.

        MACRO 
        CUnknownImpl_MethodImpl $MethodId

        LEAF_ENTRY __CUnknownImpl_Method$MethodId
        mov     r12, #$MethodId
        b       IndirectMethodInvoker
        LEAF_END __Interceptor_meth$Method

        MEND
    
; Linkage stubs __CUnknownImpl_Method0... -- actual entry points -- they all delegate to IndirectMethodInvoker
; What they are (VariableArgMethodHelper.h):
;   #define Delegate_Invoke 0
;   #define ArrayAsVector_IndexOf 1
;   #define ArrayAsVector_SetAt 2
;   #define ArrayAsVector_InsertAt 3
;   #define ArrayAsVector_Append 4

        CUnknownImpl_MethodImpl 0
        CUnknownImpl_MethodImpl 1
        CUnknownImpl_MethodImpl 2
        CUnknownImpl_MethodImpl 3
        CUnknownImpl_MethodImpl 4

;++
; HRESULT IndirectMethodInvoker(...)
;
; Routine description:
;   This function is jumped to from the corresponding linkage stub and calls the CUnknown's common rountine that invokes the ultimate function.
;   - converts arguments from ... representation into vararg representation (sort of opposite of arm_ProjectionCall).
;
; Arguments:
;   ...
;     - r0 == this + 4, i.e. address of m_pvtbl which is 1st field, see UnknownImpl.h:
;       vtable-address xxxx -- actual "this".
;                      xxxx -- m_pvtbl.
;
; Implicit Arguments:
;   MethodId (r12) - supplies the method number from the thunk code (see the macro above).
;
; Return Value:
;   The value as returned by the target function.
;
; Notes:
;   This calls into (ulong is 4 bytes): 
;     virtual HRESULT __stdcall CUnknownImpl::CallIndirect(ulong methodId, ApcsCallLayout* callLayout, ulong* pcbArgs)
;       - methodId is the id of the method to call (__CUnknownImpl_Method<id>, see above).
;       - pvArgs   is array of arguments.
;       - pcbArgs  is the out parameter, seems to be how many arguments to deallocate from stack (?). We don't use, anyhow.
;     ApcsCallLayout (same as StackVarArg):
;       byte* FloatRegisters;
;       byte* GeneralRegisters;
;       byte* Stack;
;       int StackSize;
;     Stack looks like this:
;       sp     +00  xxxx        -- space for 8-byte alignment.
;              +04  xxxx        -- pcbArgs, space used for outgoing call.
;              +08  xxxx        -- ApcsCallLayout.FloatRegisters. Note: callLayout will point here.
;              +0Ch xxxx        -- ApcsCallLayout.GeneralRegisters.
;              +10h xxxx        -- ApcsCallLayout.Stack.
;              +14h unused      -- ApcsCallLayout.StackSize.
;              +18h xxxx        callLayout->FloatRegisters[0]  -- Placeholder 
;              ...
;              +48h xxxx        callLayout->FloatRegisters[15] -- for s0-s15.
;       r11    +58h ebp-frame
;       lr     +5Ch xxxx
;              +60h callLayout->GeneralRegisters[0]   -- r0 (home)
;              +64h callLayout->GeneralRegisters[1]   -- r1 (home)
;              +68h callLayout->GeneralRegisters[2]   -- r2 (home)
;              +6Ch callLayout->GeneralRegisters[3]   -- r3 (home)
;       caller-sp +70h callLayout->Stack[0]              -- the other (stack) args -- prepared by the caller.
;                   ...
;       
; See also: 
;     com\ole32\com\txf\CallFrame\arm\stubless.asm
;--
        NESTED_ENTRY IndirectMethodInvoker

#if defined(_CONTROL_FLOW_GUARD)
        PROLOG_PUSH {r0-r5}             ; extra register pair r4/r5 used to save info across icall check
#else
        PROLOG_PUSH {r0-r3}             ; home r0-r3 -- right above the [potential] stack args to make nice continuos array of regs + stack args.
#endif
        PROLOG_PUSH r11, lr
        PROLOG_STACK_SAVE r11           ; mov r11, sp -- stack frame needed for ETW.
        PROLOG_STACK_ALLOC 0x40         ; space for memory pointed to by callLayout->FloatRegisters.
        PROLOG_STACK_ALLOC 0x10         ; space for the callLayout struct.
        PROLOG_STACK_ALLOC 8            ; space for params we pass over to the target (out ulong* pcbArgs) + 4 bytes for 8-byte alignment.

        ; Copy VFP registers that can potentially be used by the call (ARM uses up to 16 float regs for the call).
        add     r1, sp, #0x18           ; r1 = callLayout + 18h == placeholder for float registers.
        vstm    r1, {s0-s15}            ; store s0-s15 into the placeholder memory.

        ; Zero-out the pcbAgrs.
        eor     r1, r1, r1              ; r1 = 0;
        str     r1, [sp, #4]

        ; Fill the ApcsCallLayout struct (callLayout parameter).
        str     r1, [sp, #0x14]         ; callLayout.StackSize = 0 -- unused, Note that we don't know the size of the stack at this point.
        add     r1, sp,  #0x18
        str     r1, [sp, #0x08]         ; callLayout.FloatRegisters.
        add     r1, sp,  #0x60
        str     r1, [sp, #0x0C]         ; callLayout.GeneralRegisters.
        add     r1, sp,  #0x70
        str     r1, [sp, #0x10]         ; callLayout.Stack.

#if defined(_CONTROL_FLOW_GUARD)
        ; Get address of target function and verify that the call target is valid
        sub     r2, r0, #4              ; r2 = this, (initially r0 = "this" + 4, as it's address of m_pvtbl, see note abobe).
        mov     r4, r2                  ; save r4 = this
        ldr     r2, [r2]                ; Now r2 = address of vtable.
        ldr     r0, [r2]                ; Now r2 = 1st vtable entry, which is the CallIndirect() method and the call target to verify.
        mov     r5, r12                 ; save r12

        mov32   r3, __guard_check_icall_fptr 
        ldr     r3, [r3]
        blx     r3
        mov     r12, r5                 ; restore r12
        mov     r0, r4                  ; restore r0 = this
#else
        sub     r0, r0, #4              ; r0 = this, (initially r0 = "this" + 4, as it's address of m_pvtbl, see note abobe).
#endif 
        ; Fill the parameters that we pass over to the target call.
        mov     r1, r12                 ; r1 = methodId.
        add     r2, sp, #8              ; r2 = callLayout.
        add     r3, sp, #4              ; r3 = pcbArgs (out parameter).

        ; Get the address of the target function.
        ldr     r12, [r0]               ; Now r12 = address of vtable.
        ldr     r12, [r12]              ; Now r12 = 1st vtable entry, which is the CallIndirect() method.

        blx     r12                     ; Call into CUnknownImpl::CallIndirect(ulong methodId, StackArgs* pvArgs, ulong* pcbArgs).

        ; Note: pcbArgs is unused.

        EPILOG_STACK_RESTORE r11        ; mov sp, r11
        EPILOG_POP r11, lr
#if defined(_CONTROL_FLOW_GUARD)
        EPILOG_POP {r0-r5}
#else
        EPILOG_STACK_FREE 16
#endif
        EPILOG_RETURN

        NESTED_END IndirectMethodInvoker

    END
