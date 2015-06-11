TARGETTYPE=PROGLIB

!IF "$(_BUILDARCH)" == "arm"
NT_SIGNCODE=1
TEST_CODE=1
!ENDIF

UMENTRY = wmain
UMTYPE = console
LARGE_ADDRESS_AWARE=1

BUILD_FOR_CORESYSTEM=1
USE_DEFAULT_WIN32_LIBS=0
USE_UMBRELLA_LIBS=1

# Account for building ATL with _DEBUG defined
ATL_LIBS = $(SDK_LIB_PATH)\atl.lib $(SDK_LIB_PATH)\atls.lib

TARGETLIBS=     \
    $(ATL_LIBS)	\
    $(ONECORE_SDK_LIB_VPATH)\onecore.lib \
    $(MINCORE_SDK_LIB_PATH)\mincore_fw.lib    \
    $(MINCORE_PRIV_SDK_LIB_VPATH)\mincore_private.lib \
	$(MINCORE_PRIV_SDK_LIB_PATH)\api-ms-win-ntuser-rectangle-l1-1-0.lib \
	$(MINCORE_PRIV_SDK_LIB_PATH)\ext-ms-win-ntuser-synch-l1-1-0.lib \
	$(MINCORE_PRIV_SDK_LIB_PATH)\ext-ms-win-ntuser-message-l1-1-1.lib \
	$(MINCORE_PRIV_SDK_LIB_PATH)\ext-ms-win-ntuser-misc-l1-1-0.lib \
	$(MINCORE_PRIV_SDK_LIB_PATH)\ext-ms-win-ntuser-window-l1-1-0.lib \
	$(MINCORE_PRIV_SDK_LIB_PATH)\ext-ms-win-ntuser-gui-l1-1-0.lib \
	$(MINCORE_PRIV_SDK_LIB_PATH)\ext-ms-win-ntuser-windowclass-l1-1-0.lib \
	$(MINCORE_PRIV_SDK_LIB_PATH)\ext-ms-win-shell-shell32-l1-2-1.lib \
	$(MINCORE_PRIV_SDK_LIB_PATH)\ext-ms-win-ntuser-draw-l1-1-2.lib \
	$(SHELL_LIB_PATH)\api-ms-win-shcore-stream-l1-1-0.lib \
	$(SDK_LIB_PATH)\wininet.lib \
	$(SDK_LIB_PATH)\urlmon.lib \
    $(SDK_LIB_PATH)\Uuid.lib \
    $(SDK_LIB_PATH)\muiload.lib \
    $(SDK_LIB_PATH)\ole32.lib \
    $(SHELL_LIB_PATH)\CommonUtilityLibrary.lib       \
    $(SHELL_LIB_PATH)\ieguidp.lib \
    $(OBJECT_INETCORE_LIB_DIR)\indexeddb\scalookup\lib\$(O)\SCALookup_iel3.lib \
    $(OBJ_PATH)\..\..\..\exe\common\$(O)\jscript.exe.common.lib \
    $(INETCORE_LIB_PATH)\iertutilp.lib \
    $(INETCORE_LIB_PATH)\IESettingsId.lib \
    $(INETCORE_LIB_PATH)\IESettingsApi.lib \
    $(OBJECT_INETCORE_DIR)\lib\common\iel1_sp\$(O)\ieshare_common_iel1.lib \


DELAYLOAD = \
    ext-ms-win-ntuser-synch-l1-1-0.dll; \
	ext-ms-win-ntuser-message-l1-1-1.dll; \
	ext-ms-win-ntuser-misc-l1-1-0.dll; \
	ext-ms-win-ntuser-window-l1-1-0.dll; \
	ext-ms-win-ntuser-gui-l1-1-0.dll; \
	ext-ms-win-ntuser-windowclass-l1-1-0.dll; \
	ext-ms-win-shell-shell32-l1-2-0.dll; \
	ext-ms-win-ntuser-draw-l1-1-1.dll; \
	ole32.dll; \


ALLOW_NT_STYLE_DLOAD_ERROR_HANDLER=1
DLOAD_ERROR_HANDLER=kernelbase.dll

