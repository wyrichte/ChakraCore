
$(OBJ_PATH)\$(O)\jserr.rc2: ..\jserr.gen
        cl.exe /nologo /EP /E $(C_DEFINES) $** -I.. >$@
