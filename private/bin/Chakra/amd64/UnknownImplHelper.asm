
       title   "CUnknownImpl helper"
;++
;
; Copyright (C) Microsoft Corporation
;
; Module Name:
;
;   UnknownImplHelper.asm
;
; Abstract:
;
;   Since the amd64 has fastcall symantics wherein it passes first four arguments if possible in registers (xmm0-3 for floats or rcx, rdx, r8-9 for integers)
;   we need to make sure we pass in these values as it is to the UnknownImpl method so that it can make sure of this data. But sadly amd64 doesnt have inline asm support
;   which means we end up generating all the thunks. 
;
;--

include ksamd64.inc

ifdef _CONTROL_FLOW_GUARD
        extrn __guard_check_icall_fptr:QWORD
        extrn amd64_CheckICall:proc
endif

;++ 
;
; Step 1: 
; ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
;   Write macro that can generate the static method thunks that would be used in C++ code to actually pass as method ptr.
;   We create a __CUnknownImpl_Method#Id thunk that passes the methodId in the r10 to the IndirectMethodInvoker - the method that do the register saving magic and fwd call 
;
;--

;
; Define __CUnknownImpl_Method# client macro.
;

CUnknownImpl_MethodImpl macro MethodId

        LEAF_ENTRY __CUnknownImpl_Method&MethodId, _TEXT$00

        mov     r10, MethodId               ; set method number
        jmp     IndirectMethodInvoker       ; finish in common code

        LEAF_END __CUnknownImpl_Method&MethodId, _TEXT$00

endm

;++
;
; Step 2:
;
; ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
;   Now is the time to use the above macro and create the __CUnknownImpl_Method# thunks for all the methods that we want to implement
;   These Ids should match with what we use in C++ code.
;
;   (VariableArgMethodHelper.h) ------------------------
;   #define Delegate_Invoke 0
;   #define ArrayAsVector_IndexOf 1
;   #define ArrayAsVector_SetAt 2
;   #define ArrayAsVector_InsertAt 3
;   #define ArrayAsVector_Append 4
;
;--

index = 0
rept    5
        
        CUnknownImpl_MethodImpl %index

        index = index + 1

endm

;++
;
; Step 3:
; ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
;   Routine that actually saves the register and sends them to the CUnknownImpl::IndirectCall
;   Amd64 has stack managed by callee - so we dont need to be worrying about deallocating stack that our caller allocated.
;   We need to allocate and deallocate stack for CUnknownImpl::IndirectCall though.
;
;--

        subttl  "Common routine that saves registers and stack and passes them to the CUnknownImpl::IndirectCall"
;++
;
; long IndirectMethodInvoker(...)
;
; Routine description:
;   This function is jumped to from the corresponding linkage stub and calls
;   the CUnknown's common rountine that invokes the ultimate function.
;
; Arguments:
;   ...
;
; Implicit Arguments:
;   Method (r10) - Supplies the method number from the thunk code.
;
; Return Value:
;   The value as returned by the target function.
;
;--

FrameForCallIndirect struct                                         ; Frame for the CallIndirect
        P1Home	    dq ?		                                ; argument home addresses - there are 5 parameters so we need to have place for 5 of them
        P2Home	    dq ?		                                ; Note that even though we would pass first four parameters in registers we need to allocate
        P3Home	    dq ?		                                ; space for these arguments
        P4Home	    dq ?		                                ;
            SavedXmm0   dq ?                                        ; saved floating argument registers
            SavedXmm1   dq ?                                        ;
            SavedXmm2   dq ?                                        ;
            SavedXmm3   dq ?                                        ;
        pRegs	    dq ?		                                ; pointer to the stack where the floating regs are saved
        pArgs       dq ?                                        ; pointer to the stack where integer regs are saved
            cbArgs      dd ?                                        ; sizeof args - address of which we would pass in P5Home
            Fill1	    dd ?		                                ; fill to qword boundry
FrameForCallIndirect ends

;
; Prolog for IndirectMethodInvoker routine
;

        NESTED_ENTRY IndirectMethodInvoker, _TEXT$00

        mov	8[rsp], rcx                                     ; save integer argument registers
        mov	16[rsp], rdx                                    ;
        mov	24[rsp], r8                                     ;
        mov	32[rsp], r9                                     ;

        alloc_stack (sizeof FrameForCallIndirect)               ; allocate stack frame

        END_PROLOGUE

;
; Core - do the actual work
;

        sub     rcx, 8d                                         ; set the *this* pointer Rcx

ifdef _CONTROL_FLOW_GUARD
        mov     FrameForCallIndirect.P1Home[rsp], rcx           ; save the *this* pointer
        mov     rcx, [rcx]                                      ; set the vtable pointer
        mov     rcx, [rcx]                                      ; set the call target
        call    amd64_CheckICall                                ; verify call target is valid
        mov     rcx, FrameForCallIndirect.P1Home[rsp]           ; restore *this* pointer Rcx
endif

        xor     eax, eax                                        ; init memory that we are passing ptr to
        mov     FrameForCallIndirect.cbArgs[rsp], eax

        movq    FrameForCallIndirect.SavedXmm0[rsp], xmm0       ; save floating argument registers
        movq    FrameForCallIndirect.SavedXmm1[rsp], xmm1       ;
        movq    FrameForCallIndirect.SavedXmm2[rsp], xmm2       ;
        movq    FrameForCallIndirect.SavedXmm3[rsp], xmm3       ;
    
        lea     rax, FrameForCallIndirect.SavedXmm0[rsp]        ; set float regs address
        mov     FrameForCallIndirect.pRegs[rsp], rax

        lea	    rax, sizeof FrameForCallIndirect + 8[rsp]       ; set integer arguments address
        mov	    FrameForCallIndirect.pArgs[rsp], rax

        lea     r9, FrameForCallIndirect.cbArgs[rsp]            ; load pointer to cbArgs in R9

        lea     r8,  FrameForCallIndirect.pRegs[rsp]            ; set argument struct address in R8

        mov	    rdx, r10			                            ; set method number Rdx

        mov     rax, [rcx]                                      ; set the vtable pointer

        call    qword ptr [rax]                                 ; make the call to Interceptor::CallIndirect(methodId, pvArgs, pcbArgs)

        add     rsp, (sizeof FrameForCallIndirect)              ; deallocate stack frame
        ret                                                     ; return

        NESTED_END IndirectMethodInvoker, _TEXT$00

end

