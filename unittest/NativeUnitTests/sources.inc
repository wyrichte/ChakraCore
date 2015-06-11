TARGETLIBS=     \
    $(SDK_LIB_PATH)\Uuid.lib \
    $(SDK_LIB_PATH)\muiload.lib \
    $(SDK_LIB_PATH)\oleaut32.lib \
    $(SDK_LIB_PATH)\ole32.lib \
    $(SDK_LIB_PATH)\advapi32.lib \
    $(SDK_LIB_PATH)\kernel32.lib \
    $(SDK_LIB_PATH)\shell32.lib \
    $(INETCORE_LIB_PATH)\ieuuid.lib \
    $(INETCORE_LIB_PATH)\ieuuidp.lib \
    $(SDK_LIB_PATH)\version.lib \
    $(SDK_LIB_PATH)\shlwapi.lib \
    $(OBJECT_JSCRIPT_DIR)\Dll\jscriptdiag\lib\$(O)\ChakraDiagLib.lib \
!if !$(FREEBUILD)
    $(SDK_LIB_PATH)\user32.lib \
!endif
