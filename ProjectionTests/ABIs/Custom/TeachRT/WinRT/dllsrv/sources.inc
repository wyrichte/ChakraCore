TARGETNAME=FabrikamKitchenServer
TARGETTYPE=DYNLINK

!IF "$(_BUILDARCH)" == "arm"
NT_SIGNCODE=1
TEST_CODE=1
!ENDIF

DLLDEF=

_NT_TARGET_VERSION=$(_NT_TARGET_VERSION_WIN8)

USE_MSVCRT=1
USE_NOTHROW_NEW=1

C_DEFINES=-DUNICODE -D_UNICODE

PRECOMPILED_CXX=1
PRECOMPILED_INCLUDE=..\stdafx.h

MSC_WARNING_LEVEL=/W4

INCLUDES= \
    $(ONECORECOM_INC_PATH); \
    $(COM_INC_PATH);\
    $(COM_INC_PATH)\winrt;\
    ..;\
    ..\..\lib;\
    ..\..\inc;\
    $(INTERNAL_SDK_INC_PATH)\winrt;\

SOURCES=\
    ..\module.cpp \
    ..\guids.cpp \
    ..\FabrikamKitchenServer.rc \

DLOAD_ERROR_HANDLER= kernel32

TARGETLIBS=\
    $(OBJ_PATH)\..\..\lib\$(O)\fabrikamkitchenserver.lib \
    $(ONECORE_PRIV_SDK_LIB_PATH)\OneCore_Forwarder_user32.lib \
    $(MINWIN_SDK_LIB_PATH)\ntdllp.lib \
    $(MINCORE_SDK_LIB_VPATH)\api-ms-win-core-winrt-error-l1.lib \
    $(MINCORE_SDK_LIB_VPATH)\api-ms-win-core-winrt-l1.lib \
    $(MINCORE_SDK_LIB_VPATH)\api-ms-win-core-winrt-string-l1.lib \
    $(MINCORE_SDK_LIB_VPATH)\api-ms-win-core-com-l1.lib \
