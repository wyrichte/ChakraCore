include ksamd64.inc

        _TEXT SEGMENT

amd64_GET_CURRENT_FRAME PROC
        mov rax, rbp
        ret
amd64_GET_CURRENT_FRAME ENDP
        
        _TEXT ENDS
        end
