TEST_CODE=1

!IF "$(_BUILDARCH)" == "arm"
NT_SIGNCODE=1
!ENDIF

TARGETNAME=Winery
TARGETTYPE=DYNLINK
DLLDEF=

USE_MSVCRT=1
USE_NOTHROW_NEW=1

_NT_TARGET_VERSION=$(_NT_TARGET_VERSION_WIN8)
C_DEFINES=-DUNICODE -D_UNICODE

PRECOMPILED_CXX=1
PRECOMPILED_INCLUDE=..\stdafx.h

MSC_WARNING_LEVEL=/W4

INCLUDES= \
    $(ONECORECOM_INC_PATH); \
    $(COM_INC_PATH);\
    $(COM_INC_PATH)\winrt;\
    $(OBJ_PATH)\..\..\idl\$(O);\
    $(OBJ_PATH)\..\..\lib\$(O);\
    ..\..\lib;\
    $(PROJECT_ROOT)\winrt\async\inc;\

SOURCES=\
    ..\module.cpp \
    ..\NapaWineryServer.rc \

TARGETLIBS=\
    $(OBJ_PATH)\..\..\lib\$(O)\wineryserver.lib \
    $(MINCORE_SDK_LIB_VPATH)\api-ms-win-core-winrt-error-l1.lib \
    $(MINCORE_SDK_LIB_VPATH)\api-ms-win-core-winrt-string-l1.lib \
    $(MINCORE_SDK_LIB_VPATH)\api-ms-win-core-com-l1.lib \
    $(MINWIN_SDK_LIB_VPATH)\api-ms-win-security-sddl-l1.lib \
    $(MINCORE_PRIV_SDK_LIB_PATH)\api-ms-win-core-winrt-errorprivate-l1-1-1.lib \
    $(MINWIN_SDK_LIB_VPATH)\api-ms-win-security-base-l1.lib \

