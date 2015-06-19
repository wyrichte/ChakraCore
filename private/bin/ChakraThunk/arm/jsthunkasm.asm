;++
;
;Module Name:
;
;    jsthunkasm.asm
;
;Abstract:
;
;    JScript dynamic interpreter thunks without writable executable memory, based on ATL.
;
;Author:
;
;    Jay Krell (jaykrell) 11-May-2015
;
;--

#include "ksarm.h"

    export get_guard_check_icall_fptr
    export GetJsThunkData
    export GetImageBase
    import __guard_check_icall_fptr
    import JsThunkData

    TEXTAREA

; This code is position independent, in case relocs do not get applied.

    LEAF_ENTRY GetImageBase                         ; return __ImageBase, in support of position independent code
    ldr r0, _GetImageBase_data
    neg r0, r0
    add r0, r0, pc
_GetImageBase_adjust and r0, r0, ~1                 ; convert from odd Thumb code address to even data address (ImageBase is 64K-aligned)
    bx lr
_GetImageBase_data dcdu 0
    reloc 2, _GetImageBase_adjust                   ; winnt.h #define IMAGE_REL_ARM_ADDR32NB  0x0002 // 32 bit address w/o image base
    LEAF_END GetImageBase


    NESTED_ENTRY get_guard_check_icall_fptr         ; return the address of __guard_check_icall_fptr
    PROLOG_PUSH {r11, lr}                           ; lr suffices but this is more conventional.
    bl GetImageBase
    ldr r12, imagerel_guard_check_icall_fptr
    add r0, r12, r0
    EPILOG_POP  {r11, pc}                           ; pc suffices but this is more conventional.
imagerel_guard_check_icall_fptr dcdu 0
    reloc 2, __guard_check_icall_fptr               ; winnt.h IMAGE_REL_ARM_ADDR32NB 2
    NESTED_END get_guard_check_icall_fptr


    NESTED_ENTRY GetJsThunkData                     ; return the address of JsThunkData, with an offset so the thunks are the same size
    PROLOG_PUSH {r11, lr}                           ; lr suffices but this is more conventional.
    bl GetImageBase
    ldr r12, imagerel_JsThunkData
    add r0, r12, r0
    EPILOG_POP  {r11, pc}                           ; pc suffices but this is more conventional.
imagerel_JsThunkData dcdu 0
    reloc 2, JsThunkData - 128                      ; winnt.h IMAGE_REL_ARM_ADDR32NB 2
    NESTED_END GetJsThunkData


    end
