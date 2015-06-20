USE_NOTHROW_NEW=1 
UMENTRY = main
UMTYPE = console

TARGETLIBS=     \
    $(SHELL_LIB_PATH)\CommonUtilityLibrary.lib       \
    $(INETCORE_LIB_PATH)\iertutilp.lib \
    $(INETCORE_LIB_PATH)\IESettingsId.lib \
    $(INETCORE_LIB_PATH)\IESettingsApi.lib \
    $(SDK_LIB_PATH)\oleaut32.lib \
    $(SDK_LIB_PATH)\ole32.lib \
    $(SDK_LIB_PATH)\Uuid.lib \
    $(SDK_LIB_PATH)\advapi32.lib \
    $(SDK_LIB_PATH)\kernel32.lib \
    $(SDK_LIB_PATH)\ntdll.lib \
    $(OBJECT_JSCRIPT_DIR)\private\lib\HostCommon\$(O)\Chakra.Host.Common.lib \
!if !$(FREEBUILD)
    $(SDK_LIB_PATH)\dbghelp.lib \
    $(SDK_LIB_PATH)\user32.lib \
!endif
