!include $(_PROJECT_MK_PATH)\..\project.mk
!include $(_PROJECT_MK_PATH)\..\inetcore.pathdefinitions.sources.inc

# don't use /d1warningLKG171 to disable warnings
NO_LKG171_FLAGS=1

# don't include implicit disable warnings
DISABLE_IMPLICIT_COMPILER_WARNING_SUPPRESSION=1

# inetcore\project.mk and inetcore.pathdefinitions.sources.inc disabled some warning that we still want. 
# Override with our own list.  
# All of these are for the benefit of test code that doesn't include lib\common\warnings.

# 4100: unreferenced formal parameter
# 4127: conditional expression is constant
# 4481: nonstandard extension used: override specifier 'sealed'
MSC_WARNING_LEVEL=/W4 /WX /wd4100 /wd4127 /wd4481

# TODO: clean up warning 4267
MSC_WARNING_LEVEL=$(MSC_WARNING_LEVEL) /wd4267

!ifndef JSCRIPT_ROOT
JSCRIPT_ROOT=$(INETCORE_DIR)\jscript
!endif
!ifndef OBJECT_JSCRIPT_DIR
OBJECT_JSCRIPT_DIR=$(OBJECT_INETCORE_DIR)\jscript
!endif

TARGET_DESTINATION=jscript

C_DEFINES =$(C_DEFINES) -DNTBUILD -DNOMINMAX -DNO_SHLWAPI_ISOS -DUSE_EDGEMODE_JSRT

# Some of our STDMETHOD can throw
# TODO: Code review STDMETHOD and separate out API that can throw and those that can't
C_DEFINES =$(C_DEFINES) -DCOM_STDMETHOD_CAN_THROW

!if "$(_BUILDBRANCH)" == "fbl_ie_dev1_chakraserver"
C_DEFINES =$(C_DEFINES) -DJSRT_BRANCH
!endif

USER_C_FLAGS=$(USER_C_FLAGS) /Zm150

!IF "$(_BUILDARCH)" == "amd64"
USER_C_FLAGS=$(USER_C_FLAGS) /homeparams
!ENDIF

ROOT_OBJ=$(OBJECT_INETCORE_DIR)\jscript

TREE_SYNC=1

!if $(FREEBUILD)
LINK_TIME_CODE_GENERATION=1
!endif


COREINCLUDES=\
    $(JSCRIPT_ROOT)\lib\common; \
    $(JSCRIPT_ROOT)\lib\parser; \
    $(JSCRIPT_ROOT)\lib\runtime; \
    $(JSCRIPT_ROOT)\lib\backend; \
    $(JSCRIPT_ROOT)\lib\memprotectheap; \
    $(JSCRIPT_ROOT)\publish; \
    $(SHELL_INC_PATH); \
    $(BROWSER_INETCORE_INC_PATH); \
    $(SDK_INC_PATH);\
    $(MINWIN_PRIV_SDK_INC_PATH); \
    $(OBJECT_INETCORE_LIB_NAV_FCK_O_DIR); \
    $(OBJECT_JSCRIPT_DIR)\manifests\$(O); \


SITEINCLUDES=\
    $(JSCRIPT_ROOT)\lib\common; \
    $(JSCRIPT_ROOT)\lib\parser; \
    $(JSCRIPT_ROOT)\lib\runtime; \
    $(JSCRIPT_ROOT)\lib\backend; \
    $(SHELL_INC_PATH); \
    $(OBJECT_INETCORE_LIB_NAV_FCK_O_DIR); \

!if !defined(WARNING_LEVEL)
WARNING_LEVEL=W4
!endif

!if !$(FREEBUILD)
C_DEFINES = $(C_DEFINES) -D_DEBUG -DDBG -DDBG_DUMP
LINKER_NOREF=1
!endif

!if !defined(NOT_UNICODE)
C_DEFINES       = $(C_DEFINES) -DUNICODE -D_UNICODE
!endif

USE_NATIVE_EH=CTHROW

!if !$(FREEBUILD)
FORCENATIVEOBJECT=1
USE_DEBUGLIB=1
!if defined(TESTBUILD)
MSC_OPTIMIZATION=$(MSC_OPTIMIZATION) /O2
!else
MSC_OPTIMIZATION=$(MSC_OPTIMIZATION) /Od
!endif
!else  # FREEBUILD
!if "$(_BUILDOPT)" != "no opt"
MSC_OPTIMIZATION=$(MSC_OPTIMIZATION) /O2
!endif
!endif

!ifdef X86
# MSC_OPTIMIZATION=$(MSC_OPTIMIZATION) /arch:SSE2
NOFTOL3=1
!endif

USE_MSVCRT=1

# By default, don't generate a .bsc file
NO_BROWSER_FILE=1

!if defined(USE_ICECAP) || defined(USING_ICECAP4_ICEPICK)
C_DEFINES = $(C_DEFINES) /DPRODUCT_PROF
!if defined(USING_ICECAP4_ICEPICK)
C_DEFINES = $(C_DEFINES) /DICECAP4
PERFLIBS=$(ROOT)\external\lib\icecap.lib
!endif
!endif

USE_PDB_TO_COMPILE=1

# On ARM we depend on API that was added in Win8 timeframe, specifically GetCurrentThreadLimits.
# Note that for ARM we don't need to support running on Win7, so it's fine to require Win8 as minimum,
# but for x86/amd64 we may need to ship IE10 for Win7, thus it's not OK in general to require Win8 as minimum.
!if "$(_BUILDARCH)" == "arm" || "$(_BUILDARCH)" == "arm64"
_NT_TARGET_VERSION=$(_NT_TARGET_VERSION_WIN8)
!else
_NT_TARGET_VERSION=$(_NT_TARGET_VERSION_WIN7)
!endif

