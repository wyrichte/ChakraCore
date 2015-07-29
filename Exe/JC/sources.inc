!include $(PROJECT_ROOT)\platform.inc

UMENTRY = wmain
UMTYPE = console
LARGE_ADDRESS_AWARE=1

BUILD_FOR_CORESYSTEM=1
USE_DEFAULT_WIN32_LIBS=0
USE_UMBRELLA_LIBS=1

# Make sure that common.memory.lib is passed to linker prior to msvcrt so that our new/delete 
# do not cause 'already defined in msvcrt.lib error'.
CRTLIBS= \
    $(OBJECT_JSCRIPT_DIR)\lib\common\memory\$(JS_LIBTYPE)\$(O)\jscript.common.memory.lib

TARGETLIBS=     \
    $(ONECORE_SDK_LIB_VPATH)\onecore.lib \
    $(MINCORE_SDK_LIB_PATH)\mincore_fw.lib    \
    $(MINCORE_PRIV_SDK_LIB_VPATH)\mincore_private.lib \
    $(SHELL_LIB_PATH)\CommonUtilityLibrary.lib       \
    $(OBJECT_INETCORE_DIR)\lib\codex\iel1\$(O)\codex_iel1.lib \
    $(INETCORE_LIB_PATH)\iertutilp.lib \
    $(INETCORE_LIB_PATH)\IESettingsId.lib \
    $(INETCORE_LIB_PATH)\IESettingsApi.lib \
    $(OBJECT_INETCORE_DIR)\lib\common\iel1_sp\$(O)\ieshare_common_iel1.lib \
    $(JS_LINK_LIBS) \
    $(OBJECT_JSCRIPT_DIR)\exe\common\$(O)\jscript.exe.common.lib \
    $(SDK_LIB_PATH)\muiload.lib \
    $(COM_LIB_PATH)\combasep.lib \
    $(SDK_LIB_PATH)\Uuid.lib \
!IF (!$(FREEBUILD) || defined(FRETEST) || defined(NTTESTENV)) && "$(TARGET_DIRECTORY)" != "arm" && "$(TARGET_DIRECTORY)" != "arm64"
    ..\..\..\tools\external\lib\$(TARGET_DIRECTORY)\jitprofiling.lib \
!ENDIF
!if !$(FREEBUILD)
    $(SDK_LIB_PATH)\DbgHelp.lib \
    $(MINCORE_PRIV_SDK_LIB_VPATH)\ext-ms-win-ntuser-dialogbox-l1.lib \
!endif
    $(JSCRIPT_THUNK_LIB) \

DELAYLOAD = $(DELAYLOAD) \
!IF (!$(FREEBUILD))
    ext-ms-win-ntuser-dialogbox-l1-1-0.dll; \
!ENDIF

ALLOW_NT_STYLE_DLOAD_ERROR_HANDLER=1
DLOAD_ERROR_HANDLER=kernelbase.dll
