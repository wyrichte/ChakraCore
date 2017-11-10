TEST_CODE=1

!IF "$(_BUILDARCH)" == "arm"
NT_SIGNCODE=1
!ENDIF

TARGETNAME=Animals
TARGETTYPE=DYNLINK
DLLDEF=
C_DEFINES=-DUNICODE -D_UNICODE
USER_C_FLAGS=$(USER_C_FLAGS) /Zc:wchar_t 
NO_WCHAR_T=1
USE_MSVCRT=1
USE_NOTHROW_NEW=1

MSC_WARNING_LEVEL=/W4

_NT_TARGET_VERSION=$(_NT_TARGET_VERSION_WIN8)

PRECOMPILED_CXX=1
PRECOMPILED_INCLUDE=..\stdafx.h

INCLUDES= \
    $(ONECORECOM_INC_PATH); \
    $(COM_INC_PATH);\
    $(ONECORECOM_INC_PATH)\winrt; \
    $(COM_INC_PATH)\winrt;\
    $(JSCRIPT_ROOT)\ProjectionTests\ABIs\Custom\TeachRT\WinRT\idl; \
    $(OBJ_PATH)\..\..\lib\$(O);\
    ..\..\lib;\
    $(OBJ_PATH)\..\..\idl\$(O);\
	
SOURCES=\
    ..\module.cpp \
    ..\AnimalServer.rc \

TARGETLIBS=\
    $(OBJ_PATH)\..\..\lib\$(O)\Animals.lib \
    $(MINCORE_SDK_LIB_VPATH)\api-ms-win-core-winrt-error-l1.lib \
    $(MINCORE_SDK_LIB_VPATH)\api-ms-win-core-winrt-l1.lib \
    $(MINCORE_SDK_LIB_VPATH)\api-ms-win-core-winrt-string-l1.lib \
    $(MINCORE_SDK_LIB_VPATH)\api-ms-win-core-com-l1.lib \
