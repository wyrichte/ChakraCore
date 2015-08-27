;++
; Copyright (C) Microsoft Corporation
;
; Module Name:
;   UnknownImplHelper.asm
;
; Abstract:
;
;--
    TTL	Dll\JScript\arm64\UnknownImplHelper.asm

    OPT	2	; disable listing
#include "ksarm64.h"
    OPT	1	; reenable listing

    EXPORT IndirectMethodInvoker

    TEXTAREA

; Define the CUnknownImpl_MethodImpl macro.

        MACRO 
        CUnknownImpl_MethodImpl $MethodId

        LEAF_ENTRY __CUnknownImpl_Method$MethodId
        mov     x16, #$MethodId
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
;   - converts arguments from ... representation into vararg representation (sort of opposite of arm64_ProjectionCall).
;
; Arguments:
;   ...
;     - x0 == this + 8, i.e. address of m_pvtbl which is 1st field, see UnknownImpl.h:
;       vtable-address xxxx -- actual "this".
;                      xxxx -- m_pvtbl.
;
; Implicit Arguments:
;   MethodId (x16) - supplies the method number from the thunk code (see the macro above).
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
;       sp     +00  xxxx        -- fp
;              +08  xxxx        -- lr
;              +10h xxxx        -- pcbArgs
;              +20h xxxx        -- ApcsCallLayout.FloatRegisters. Note: callLayout will point here.
;              +28h xxxx        -- ApcsCallLayout.GeneralRegisters.
;              +30h xxxx        -- ApcsCallLayout.Stack.
;              +38h unused      -- ApcsCallLayout.StackSize.
;              +40h xxxx        callLayout->FloatRegisters[0]  -- Placeholder 
;              ...
;              +B0h xxxx        callLayout->FloatRegisters[7] -- for q0-q7.
;              +C0h callLayout->GeneralRegisters[0]   -- x0 (home)
;              ...
;              +F8h callLayout->GeneralRegisters[7]   -- x7 (home)
;       caller-sp +100h callLayout->Stack[0]              -- the other (stack) args -- prepared by the caller.
;                   ...
;       
; See also: 
;     com\ole32\com\txf\CallFrame\arm64\stubless.asm
;--
        NESTED_ENTRY IndirectMethodInvoker
        
        PROLOG_SAVE_REG_PAIR fp, lr, #-0x100!
        
        stp     x0, x1, [sp, #0xC0]
        stp     x2, x3, [sp, #0xD0]
        stp     x4, x5, [sp, #0xE0]
        stp     x6, x7, [sp, #0xF0]

        ; Copy VFP registers that can potentially be used by the call (ARM64 uses up to 8 float regs for the call).
        stp     q0, q1, [sp, #0x40]
        stp     q2, q3, [sp, #0x60]
        stp     q4, q5, [sp, #0x80]
        stp     q6, q7, [sp, #0xA0]

        ; Fill the ApcsCallLayout struct (callLayout parameter).
        add     x8, sp, #0x40
        add     x9, sp, #0xC0
        add     x10, sp, #0x100
        str     x8, [sp, #0x20]         ; callLayout.FloatRegisters.
        str     x9, [sp, #0x28]         ; callLayout.GeneralRegisters.
        str     x10, [sp, #0x30]        ; callLayout.Stack.
        str     xzr, [sp, #0x38]        ; callLayout.StackSize = 0 -- unused, Note that we don't know the size of the stack at this point.

        ; Fill the parameters that we pass over to the target call.
        sub     x0, x0, #8              ; x0 = this, (initially x0 = "this" + 8, as it's address of m_pvtbl, see note abobe).
        mov     x1, x16                 ; x1 = methodId.
        add     x2, sp, #0x20           ; x2 = callLayout.
        add     x3, sp, #0x10           ; x3 = pcbArgs (out parameter).

        ; Get the address of the target function.
        ldr     x16, [x0]               ; Now x16 = address of vtable.
        ldr     x16, [x16]              ; Now x16 = 1st vtable entry, which is the CallIndirect() method.

        blr     x16                     ; Call into CUnknownImpl::CallIndirect(ulong methodId, StackArgs* pvArgs, ulong* pcbArgs).

        ; Note: pcbArgs is unused.
        
        EPILOG_RESTORE_REG_PAIR fp, lr, #0x100!
        EPILOG_RETURN

        NESTED_END IndirectMethodInvoker

    END
