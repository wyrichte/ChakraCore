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

.model flat
.686

extern ___guard_check_icall_fptr:dword
extern _JsThunkData:qword

.code

; This code is position independent, in case relocs do not get applied.

public GetImageBase
GetImageBase proc                               ; return __ImageBase, in support of position independent code
    call @F                                     ; call is position independent
@@:
    pop eax
    sub eax, imagerel $ - 1                     ; -1 because pop eax is one byte
    ret
GetImageBase endp

public _get_guard_check_icall_fptr              ; return the address of __guard_check_icall_fptr
_get_guard_check_icall_fptr proc
    call GetImageBase
    add eax, imagerel ___guard_check_icall_fptr
    ret
_get_guard_check_icall_fptr endp

public _GetJsThunkData                          ; return the address of JsThunkData, with an offset so the thunks are the same size
_GetJsThunkData proc
    call GetImageBase
    add eax, imagerel _JsThunkData - 128
    ret
_GetJsThunkData endp

end
