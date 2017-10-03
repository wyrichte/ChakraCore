
       title   "Projection method call"
;++
;
; Copyright (C) Microsoft Corporation
;
; Module Name:
;
;   projectioncall.asm
;
; Abstract:
;
;   This module contains interpreter support routines for the AMD64 platform.
;
;--

include ksamd64.inc

        extern  __chkstk:proc
ifdef _CONTROL_FLOW_GUARD
        extrn __guard_check_icall_fptr:QWORD
        extrn amd64_CheckICall:proc

endif


        subttl  "Invoke Function with Parameter List"
;++
;
; HRESULT
; amd64_ProjectionCall (
;     void* methodAddress,
;	  IUnknown* thisPointer,
;     REGISTER_TYPE *ArgumentList,
;     ULONG Arguments
;     )
;
; Routine description:
;
;   This function builds an appropriate argument list and calls the specified
;   function.
;
; Arguments:
;
;   Function (rcx) - Supplies a pointer to the target function.
;
;   thisPointer (rdx) - Supplies "this" pointer for the method call
;
;   ArgumentList (r8) - Supplies a pointer to the argument list, not including "this" pointer
;
;   Arguments (r9d) - Supplies the number of arguments.
;
; Return Value:
;
;   The value as returned by the target function.
;
;--

InFrame struct
        P1Home  dq ?                    ; argument home addresses
        P2Home  dq ?                    ;
        P3Home  dq ?                    ;
        P4Home  dq ?                    ;
        SavedRbp dq ?                   ; saved nonvolatile registers
        SavedRsi dq ?                   ;
        SavedRdi dq ?                   ;
Inframe ends

        NESTED_ENTRY amd64_ProjectionCall, _TEXT$00

        alloc_stack (sizeof InFrame)    ; allocate stack frame
        save_reg rbp, InFrame.SavedRbp  ; save nonvolatile registers
        save_reg rsi, InFrame.SavedRsi  ;
        save_reg rdi, InFrame.SavedRdi  ;
        set_frame rbp, 0                ; set frame pointer

        END_PROLOGUE

        mov     eax, r9d                ; round to even argument count
        inc     eax                     ; add one for "this"
        inc     eax                     ;
        and     eax, -2                 ;
        shl     eax, 3                  ; compute number of bytes
        call    __chkstk                ; check stack allocation
        sub     rsp, rax                ; allocate argument list
        mov     r10, rcx                ; save address of function
        mov     rsi, r8                 ; set source argument list address       
        mov     rdi, rsp                ; set destination argument list address
        add     rdi, 8
        mov     ecx, r9d                ; set number of arguments
    rep movsq                           ; copy arguments to the stack

        mov     rdi, rdx                ; save "this" pointer argument
        mov     rsi, r10                ; save call target

;
; Verify that the call target is valid
;
ifdef _CONTROL_FLOW_GUARD        
        mov     rcx, r10
        call    amd64_CheckICall        ; verify that the call target is valid
endif
        
        mov     r10, rsi                ; restore call target
        mov     rdx, rdi                ; restore "this" pointer

;
; N.B. All four argument registers are loaded regardless of the actual number
;      of arguments.
;

        mov     rcx, rdx                ; "this" pointer is in rdx
        mov     rdx, 8[rsp]             ;
        movq    xmm1, QWORD PTR 8[rsp]  ;
        mov     r8, 16[rsp]             ;
        movq    xmm2, QWORD PTR 16[rsp] ;
        mov     r9, 24[rsp]             ;
        movq    xmm3, QWORD PTR 24[rsp] ;
        call    r10                     ; call target function
        mov     rsi, InFrame.SavedRsi[rbp] ; restore nonvolatile registers
        mov     rdi, InFrame.SavedRdi[rbp] ;
        mov     rsp, rbp                ; deallocate argument list
        mov     rbp, InFrame.SavedRbp[rbp] ; restore nonvolatile register
        add     rsp, (sizeof InFrame)   ; deallocate stack frame
        ret                             ;

        NESTED_END amd64_ProjectionCall , _TEXT$00

        end