TEST_CODE=1

!IF "$(_BUILDARCH)" == "arm"
NT_SIGNCODE=1
!ENDIF

TARGETNAME=DevTests.Repros
TARGETTYPE=DYNLINK
DLLDEF=
C_DEFINES=-DUNICODE -D_UNICODE

USE_MSVCRT=1
USE_NOTHROW_NEW=1

MSC_WARNING_LEVEL=/W4

_NT_TARGET_VERSION=$(_NT_TARGET_VERSION_WIN8)

PRECOMPILED_CXX=1
PRECOMPILED_INCLUDE=..\stdafx.h

INCLUDES= \
    $(ONECORECOM_INC_PATH); \
    $(COM_INC_PATH);\
    $(COM_INC_PATH)\winrt;\
    $(OBJ_PATH)\..\..\lib\$(O);\
    ..\..\lib;\
    $(OBJ_PATH)\..\..\idl\$(O);\
	
SOURCES=\
    ..\module.cpp \
    ..\ReprosServer.rc \

TARGETLIBS=\
    $(MINCORE_SDK_LIB_VPATH)\api-ms-win-core-winrt-error-l1.lib \
    $(MINCORE_SDK_LIB_VPATH)\api-ms-win-core-winrt-string-l1.lib \
    $(MINCORE_SDK_LIB_VPATH)\api-ms-win-core-com-l1.lib \


