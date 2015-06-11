USE_NOTHROW_NEW=1 
UMENTRY = wmain
UMTYPE = console

TARGETLIBS=     \
    $(ONECORE_SDK_LIB_VPATH)\onecore.lib \
    $(MINCORE_SDK_LIB_PATH)\mincore_fw.lib    \
    $(MINCORE_PRIV_SDK_LIB_VPATH)\mincore_private.lib \
    $(SDK_LIB_PATH)\muiload.lib \
    $(COM_LIB_PATH)\combasep.lib \
    $(SDK_LIB_PATH)\Uuid.lib \
    $(SDK_LIB_PATH)\user32.lib \
    $(SDK_LIB_PATH)\shlwapi.lib \
    $(SHELL_LIB_PATH)\CommonUtilityLibrary.lib       \
    $(INETCORE_LIB_PATH)\iertutilp.lib \
    $(INETCORE_LIB_PATH)\IESettingsId.lib \
    $(INETCORE_LIB_PATH)\IESettingsApi.lib \
    $(OBJECT_INETCORE_DIR)\lib\common\iel1_sp\$(O)\ieshare_common_iel1.lib \
