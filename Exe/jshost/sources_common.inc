!include $(PROJECT_ROOT)\platform.inc

TARGETNAME=JsHost

_NT_TARGET_VERSION = $(_NT_TARGET_VERSION_WIN7)

!IF "$(_BUILDARCH)" == "arm" || "$(_BUILDARCH)" == "arm64"
NT_SIGNCODE=1
TEST_CODE=1
!ENDIF

!IF "$(_BUILDARCH)" == "x86"
C_DEFINES=$(C_DEFINES) -DLANGUAGE_SERVICE_TEST
!endif

# Turns on fretest for jshost
!include $(JSCRIPT_ROOT)\jslibtest.inc

INCLUDES=	$(COREINCLUDES);				\
		$(COM_INC_PATH);				\
		$(DS_INC_PATH);					\
		$(SDK_INC_PATH)\crt\stl70;			\
		..\..\common;					\
		$(JSCRIPT_ROOT)\dll\jscript;			\
		$(JSCRIPT_ROOT)\dll\jscriptdiag;		\
		$(OBJ_PATH)\..\..\..\dll\jscript\$(O);		\
		$(OBJ_PATH)\..\..\..\dll\jscript\test\$(O);	\
		$(OBJ_PATH)\$(O);				\
		$(OBJ_PATH)\..\..\..\lib\author\test\$(O);	\
		$(SDK_INC_PATH)\atlmfc				

C_DEFINES =$(C_DEFINES) -DWINNT -DUNICODE -D_UNICODE -DWIN32_DCOM


# TODO: ARM/Evanesco: 2M stack is temp workaround to avoid overflow on controlflow tests --
#       each InterpreterStackFrame::Process allocates 3400 bytes on the stack (vs 1688 bytes on x86). FRE is OK (184 vs 180).
#       Note: nevertheless, IE on Evanesco uses 2M.

!IF "$(_BUILDARCH)" == "amd64" || "$(_BUILDARCH)" == "arm" || "$(_BUILDARCH)" == "arm64"
LINKER_STACKSIZE=/STACK:0x200000,0x2000
!ELSE
LINKER_STACKSIZE=/STACK:0x100000,0x2000
!ENDIF

USE_IDLBASED_DLLDATA=1

DLLDEF=..\JsHost.def

USE_STL=1
STL_VER=70
USE_STATIC_ATL=1
ATL_VER=70

!INCLUDE sources.inc

# Note: EXCLUDE_BACKEND comes from inetcore\jscript\project.mk

SOURCES= \
    ..\DebuggerCore.cpp \
    ..\DelayLoadLibrary.cpp \
    ..\DelegateWrapper.cpp \
    ..\DiagnosticsHelper.cpp \
    ..\Helpers.cpp \
    ..\HtmlApp.cpp \
    ..\HtmlAppWindow.cpp \
    ..\HtmlHeapEnum.cpp \
    ..\HtmlHost.cpp \
    ..\HtmlWinSink.cpp \
    ..\JsHost.cpp \
    ..\JsHost.rc \
    ..\JsHostScriptSite.idl \
    ..\LanguageService.cpp \
    ..\LanguageServiceTestDriver.cpp \
    ..\MockImageData.cpp \
    ..\MockScaContext.cpp \
    ..\MockTypeOperations.cpp \
    ..\PhScriptsite.cpp \
    ..\ProxyStub.c \
    ..\Sca.cpp \
    ..\ScriptDebugNodeSource.cpp \
    ..\ScriptDirectHelper.cpp \
    ..\ScriptProfiler.cpp \
    ..\TestUtilities.cpp \
    ..\TypeResolution.cpp \
    ..\WScriptFastDom.cpp \
    ..\WScriptJsrt.cpp \

PRECOMPILED_INCLUDE=..\stdafx.h
PRECOMPILED_SOURCEFILE=..\stdafx.cpp
PRECOMPILED_CXX=1

MUI=1
