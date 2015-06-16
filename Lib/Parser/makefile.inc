
$(OBJ_PATH)\$(O)\jserr.rc2: ..\jserr.gen
        cl.exe /nologo /EP /E /I..\..\Common $(C_DEFINES) $** -I.. >$@
