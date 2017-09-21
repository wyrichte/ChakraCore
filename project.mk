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
JSCRIPT_ROOT=$(BASEDIR)\onecoreuap\inetcore\jscript
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

BUILDBRANCH_LOWERCASE = $(_BUILDBRANCH)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:A=a)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:B=b)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:C=c)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:D=d)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:E=e)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:F=f)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:G=g)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:H=h)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:I=i)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:J=j)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:K=k)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:L=l)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:M=m)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:N=n)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:O=o)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:P=p)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:Q=q)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:R=r)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:S=s)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:T=t)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:U=u)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:V=v)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:W=w)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:X=x)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:Y=y)
BUILDBRANCH_LOWERCASE = $(BUILDBRANCH_LOWERCASE:Z=z)

#
# N.B. For this to work depends on buildcache to
# ignore these directories. See here:
#
# Change 2878367 by REDMOND\ngoel@NGMAIN-150316095318 on 2015/04/23 11:26:00
#   BUILD: WINTCR:BLA86 Adding inetcore\jscript and inetcore\jscriptLegacy
# Affected files ...
# ... //depot/fbl_build_cb/root/tools/buildcache.ini#24 edit
#
# The branches listed here are:
#  - green signed; this will never be all branches
#  - don't presently allow JITing; this will change eventually
#
# TODO: Durango/ATL-style thunks to replace this.
#
!if "$(BUILDBRANCH_LOWERCASE)" == "fbl_xbox_rel_1504" \
    || "$(BUILDBRANCH_LOWERCASE)" == "fbl_xbox_rel_1505" \
    || "$(BUILDBRANCH_LOWERCASE)" == "fbl_xbox_rel_opt" \

# 0: no thunks -- no good stacks and possible bit-rot and malfunction
# 1: first version, using VirtualAlloc(read/write/execute)
# 2: second version, using repeated mappings of jscript9thunk.dll / chakrathunk.dll
# see inetcore changes 1301592 1302676 root change 2200574

DYNAMIC_INTERPRETER_THUNK=2
JSCRIPT_THUNK_LIB=$(_PROJECT_MK_OBJ_PATH)\dll\thunk\$O\chakrathunk.lib

!else

DYNAMIC_INTERPRETER_THUNK=1
JSCRIPT_THUNK_LIB=

!endif

!IF "$(_BUILDARCH)" == "amd64"
USER_C_FLAGS=$(USER_C_FLAGS) /homeparams
!ENDIF

ROOT_OBJ=$(OBJECT_INETCORE_DIR)\jscript

TREE_SYNC=1

!if $(FREEBUILD)
LINK_TIME_CODE_GENERATION=1
!endif


COREINCLUDES=\
    $(JSCRIPT_ROOT)\core\lib\common; \
    $(JSCRIPT_ROOT)\core\lib\parser; \
    $(JSCRIPT_ROOT)\core\lib\runtime; \
    $(JSCRIPT_ROOT)\core\lib\backend; \
    $(JSCRIPT_ROOT)\core\lib\JITClient; \
    $(JSCRIPT_ROOT)\core\lib\JITIDL; \
    $(JSCRIPT_ROOT)\core\lib\JITServer; \
    $(JSCRIPT_ROOT)\private\lib\memprotectheap; \
    $(JSCRIPT_ROOT)\publish; \
    $(ONECORESHELL_INC_PATH); \
    $(ONECOREUAPSHELL_INC_PATH); \
    $(SHELL_INC_PATH); \
    $(BROWSER_INETCORE_INC_PATH); \
    $(SDK_INC_PATH);\
    $(MINWIN_PRIV_SDK_INC_PATH); \
    $(OBJECT_INETCORE_LIB_NAV_FCK_O_DIR); \
    $(OBJECT_JSCRIPT_DIR)\private\manifests\$(O); \
    $(OBJECT_JSCRIPT_DIR)\private\core_razzle_build\manifests\$(O); \

SITEINCLUDES=\
    $(JSCRIPT_ROOT)\core\lib\common; \
    $(JSCRIPT_ROOT)\core\lib\parser; \
    $(JSCRIPT_ROOT)\core\lib\runtime; \
    $(JSCRIPT_ROOT)\core\lib\backend; \
    $(JSCRIPT_ROOT)\core\lib\JITClient; \
    $(JSCRIPT_ROOT)\core\lib\JITIDL; \
    $(JSCRIPT_ROOT)\core\lib\JITServer; \
    $(ONECORESHELL_INC_PATH); \
    $(ONECOREUAPSHELL_INC_PATH); \
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

_NT_TARGET_VERSION=$(_NT_TARGET_VERSION_WINBLUE)

!if $(FREEBUILD)
_NT_TARGET_VERSION=$(_NT_TARGET_VERSION_WIN10_RS2)
!endif
