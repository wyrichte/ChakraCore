!if "$(_BUILDARCH)" == "arm" || "$(_BUILDARCH)" == "arm64"
NT_SIGNCODE=1
TEST_CODE=1
!endif

DLLENTRY=_DllMainCRTStartup

!if "$(DBGSDK_INC_PATH)" != ""
INCLUDES = $(DBGSDK_INC_PATH);$(INCLUDES)
!else
INCLUDES = $(ONECORESDKTOOLS_INC_PATH);$(ONECOREUAPSDKTOOLS_INC_PATH);$(MINWIN_PRIV_SDK_INC_PATH)\dbg;$(SDKTOOLS_INC_PATH);$(INCLUDES)
!endif
!if "$(DBGSDK_LIB_PATH)" != ""
DBGSDK_LIB_PATH = $(DBGSDK_LIB_PATH)\$(TARGET_DIRECTORY)
!else
DBGSDK_LIB_PATH = $(SDK_LIB_PATH)
!endif
!if "$(DBGLIB_LIB_PATH)" != ""
DBGLIB_LIB_PATH = $(DBGLIB_LIB_PATH)\$(TARGET_DIRECTORY)
!else
DBGLIB_LIB_PATH = $(MINWIN_PRIV_SDK_LIB_PATH)
!endif

# Link to atlsd.lib without requiring DEBUG_CRTS
ATL_INCLUDES = $(ATL_INC_ROOT)\atlmfc
C_DEFINES = $(C_DEFINES) /D_ATL_STATIC_REGISTRY
ATL_LIBS = $(ATL_LIB_PATH)\atlthunk.lib
!if !$(FREEBUILD)
ATL_LIBS = $(ATL_LIB_PATH)\atlsd.lib $(ATL_LIBS)
!else
ATL_LIBS = $(ATL_LIB_PATH)\atls.lib $(ATL_LIBS)
!endif

INCLUDES = \
    $(INCLUDES); \
    $(JSCRIPT_ROOT)\core\lib\common; \
    $(JSCRIPT_ROOT)\core\lib\common\core; \
    $(JSCRIPT_ROOT)\core\lib\common\memory; \
    $(JSCRIPT_ROOT)\private\bin\jshost; \
    $(JSCRIPT_ROOT)\private\bin\jdtest; \
    $(JSCRIPT_ROOT)\private\bin\ChakraDiag \

LINKLIBS = $(DBGLIB_LIB_PATH)\engextcpp.lib

TARGETLIBS = \
    $(ATL_LIBS) \
    $(SDK_LIB_PATH)\onecore_downlevel.lib \
    $(DBGSDK_LIB_PATH)\dbghelp.lib \
    $(OBJECT_JSCRIPT_DIR)\private\bin\ChakraDiag\lib\$(O)\ChakraDiagLib.lib \

USE_MSVCRT = 1
USE_STL = 1
STL_VER = 70
UMTYPE = windows

#4290: C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
MSC_WARNING_LEVEL = /W4 /WX /wd4290


DLLDEF_OBJECTS=\
    $(DBGLIB_LIB_PATH)\engextcpp.lib


PRECOMPILED_INCLUDE=..\stdafx.h
PRECOMPILED_SOURCEFILE=..\stdafx.cpp
PRECOMPILED_CXX=1
