TARGETNAME=JC
TARGETTYPE=PROGRAM

!IF "$(_BUILDARCH)" == "arm" || "$(_BUILDARCH)" == "arm64"
NT_SIGNCODE=1
TEST_CODE=1
!ENDIF

# Turns on fretest for JC
!include $(JSCRIPT_ROOT)\jslibtest.inc

INCLUDES=$(COREINCLUDES);..\..\common;
USER_INCLUDES=$(JS_LIBS_PARSER_INC)


C_DEFINES =$(C_DEFINES) -DWINNT -DUNICODE -D_UNICODE


# TODO: ARM/Evanesco: 2M stack is temp workaround to avoid overflow on controlflow tests -- 
#       each InterpreterStackFrame::Process allocates 3400 bytes on the stack (vs 1688 bytes on x86). FRE is OK (184 vs 180).
#       Note: nevertheless, IE on Evanesco uses 2M.

!IF "$(_BUILDARCH)" == "amd64" || "$(_BUILDARCH)" == "arm" || "$(_BUILDARCH)" == "arm64"
LINKER_STACKSIZE=/STACK:0x200000,0x2000
!ELSE
LINKER_STACKSIZE=/STACK:0x100000,0x2000 
!ENDIF

# Note: EXCLUDE_BACKEND comes from inetcore\jscript\project.mk

!INCLUDE sources.inc

SOURCES= \
        ..\JnMain.cpp \
        ..\jc.rc \
        ..\WScript.cpp \


PRECOMPILED_INCLUDE=..\stdafx.h
PRECOMPILED_SOURCEFILE=..\stdafx.cpp
PRECOMPILED_CXX=1

MUI_DRV=1
