;---------------------------------------------------------------------------
; Copyright (C) Microsoft. All rights reserved. 
;----------------------------------------------------------------------------

;Var arm64_GET_CURRENT_FRAME()
;  
;   This method returns the current value of the frame pointer.
;
    OPT 2       ; disable listing

#include "ksarm64.h"

    OPT 1       ; reenable listing

    TTL Lib\Common\arm64\arm64_GET_CURRENT_FRAME.asm

    
    EXPORT  arm64_GET_CURRENT_FRAME

    TEXTAREA

    LEAF_ENTRY arm64_GET_CURRENT_FRAME

    mov     x0,x29

    br      lr

    LEAF_END arm64_GET_CURRENT_FRAME

    END
