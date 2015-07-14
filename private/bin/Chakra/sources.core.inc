!include $(PROJECT_ROOT)\platform.inc
!include $(JSCRIPT_ROOT)\jslib.inc

MIDL_TARGET=NT60
USE_NOTHROW_NEW=1
BUILD_FOR_CORESYSTEM=1
USE_DEFAULT_WIN32_LIBS=0
USE_UMBRELLA_LIBS=1
USE_64BIT_LINKER=1
LINK_TIME_CODE_GENERATION=0

!if !$(ARM64)
LINKER_FLAGS=$(LINKER_FLAGS) -d2:-pspeed1
!endif

USER_INCLUDES=$(JS_LIBS_PARSER_INC);$(OBJ_PATH)\..\..\..\dll\retail\$(O)

C_DEFINES =$(C_DEFINES) -DWINNT -DUNICODE -D_UNICODE
USER_C_FLAGS =$(USER_C_FLAGS) /Zm150

USE_IDLBASED_DLLDATA=1

#4510: default constructor could not be generated
#4610: can never be instantiated - user defined constructor required
MSC_WARNING_LEVEL=$(MSC_WARNING_LEVEL) /wd4510 /wd4610

# generates SAL annotations for our interface
MIDL_OPTIMIZATION=$(MIDL_OPTIMIZATION) -sal_local

PRECOMPILED_INCLUDE=stdafx.h
PRECOMPILED_SOURCEFILE=stdafx.cpp
PRECOMPILED_CXX=1

DLLDEF_OBJECTS=$(DLLDEF_OBJECTS) \
    $(JS_LIBS_MEMPROTECTHEAP) \
    $(JS_LIBS_JSRT) \

MUI_DRV=1

INCLUDES= \
    $(INCLUDES) \
    $(COREINCLUDES); \
    $(SDK_LIB_PATH); \
    $(SDK_INC_PATH); \
    $(SDK_INC_PATH)\atl30; \
    $(COM_INC_PATH); \
    $(COM_INC_PATH)\winrt; \
    $(PROJECT_ROOT)\lib\inc; \
    $(PROJECT_OBJ_ROOT)\manifests\inbox\$(O); \
    $(COM_INC_PATH)\midlrt; \
    $(PROJECT_ROOT)\jscript\core\lib\jsrt; \
    ..\; \
!if "$(_BUILDARCH)" == "arm"
    ..\arm\; \
!endif
!if "$(_BUILDARCH)" == "arm64"
    ..\arm64\; \
!endif

# Dependent Libraries
#   iedevtools_host_iel1.lib:
#       LoadLatestPDM
#       ShouldUseLocalPDM
#
#   codex_iel1.lib:
#       utf8 support
#
#   CommonUtilityLibrary.lib:
#       _DoNotUseDirectly_CULprintf
#
#   PathCchApis.lib: (to avoid loading api-ms-win-core-path-l1-1-0.dll)
#       PathCchCombine  (used in iedevtools_host_iel1.lib)
#       PatthCchAppend  (used in iedevtools_host_iel1.lib)


LINKLIBS= \
    $(SHELL_LIB_PATH)\CommonUtilityLibrary.lib \
    $(OBJECT_INETCORE_DIR)\jscript\core\lib\common\codex\$(O)\Chakra.Common.Codex.lib \
    $(OBJECT_INETCORE_DIR)\lib\devtb\dtbhost\$(O)\iedevtools_host_iel1.lib \
    $(MINCORE_PRIV_SDK_LIB_PATH)\pathcchapis.lib    \
    $(JS_LINK_LIBS) \
    $(JS_LIBS_WINRT) \

TARGETLIBS= \
    $(BASE_LIB_PATH)\AppModel\Common\AppModel.Common.lib \
    $(MINCORE_PRIV_SDK_LIB_VPATH)\mincore_private.lib \
    $(MINCORE_SDK_LIB_PATH)\mincore_fw.lib    \
    $(ONECORE_SDK_LIB_VPATH)\onecore.lib \
    $(SDK_LIB_PATH)\muiload.lib \
    $(SDK_LIB_PATH)\Uuid.lib \
!if (!$(FREEBUILD))
    $(MINCORE_PRIV_SDK_LIB_VPATH)\ext-ms-win-ntuser-dialogbox-l1.lib \
    $(SDK_LIB_PATH)\dbghelp.lib \
!endif
!IF (!$(FREEBUILD) || defined(FRETEST) || defined(NTTESTENV)) && "$(TARGET_DIRECTORY)" != "arm" && "$(TARGET_DIRECTORY)" != "arm64"
    ..\..\..\..\tools\external\lib\$(TARGET_DIRECTORY)\jitprofiling.lib \
!ENDIF

SOURCES= \
    ..\jscript9.rc \
    ..\JsrtContext.cpp \
    ..\JsrtContextCore.cpp \
    ..\ConfigParserExternals.cpp \
    ..\ChakraCoreDLLFunc.cpp
