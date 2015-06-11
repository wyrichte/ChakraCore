;---------------------------------------------------------------------------
; Copyright (C) Microsoft. All rights reserved. 
;----------------------------------------------------------------------------

;Var arm_GET_CURRENT_FRAME()
;  
;   This method returns the current value of the frame pointer.
;
    OPT 2       ; disable listing

#include "ksarm.h"

    OPT 1       ; reenable listing

    TTL Lib\Common\arm\arm_GET_CURRENT_FRAME.asm

    
    EXPORT  arm_GET_CURRENT_FRAME

    TEXTAREA

    LEAF_ENTRY arm_GET_CURRENT_FRAME

    mov     r0,r11

    bx      lr

    LEAF_END arm_GET_CURRENT_FRAME

    END
