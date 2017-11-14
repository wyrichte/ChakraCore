# This sources file contains 2 parts: first, it includes a few 
# lines that build an IDL file or winrt file that contains
# Windows Runtime API definitions. Note that .winrt support
# is now deprecated. For new projects, use idl files.
# Second, this file contains additional instructions to build a standalone
# COM proxy/stub DLL. This is needed when using the out-of-process version
# of the sample.

##########################################################
# 1) build runtime API definitions.

# rexpp sources lines
#SOURCES=Kitchen.winrt

# midl sources lines
IDL_TYPE=winrt
SOURCES=..\Fabrikam.idl \
        ..\PrivateInterfaces.idl

PASS1_BINPLACE= \
    $(OBJ_PATH)\$(O)\Output\Fabrikam.Kitchen.winmd

BINPLACE_EXCLUDE_FILE=..\winmds.txt

NTTARGETFILE1 = $(OBJ_PATH)\$O\ComposedMetadata

# All metadata for WinRT types should be published as shown below. 
# publishing is disabled for this sample because this doesn't represent 
# real product APIs.
#
# PASS0_PUBLISH= \
#    {$(OBJ_PATH)\$(O)\$(TARGETNAME).winmd=$(PROJECT_INTERIM_METADATA_PATH)\$(TARGETNAME).winmd}

##########################################################
# 2) build proxy / stub DLL for Exe version of the sample.

TARGETNAME=   FabrikamKitchenServer.ProxyStub
TARGETTYPE=   DYNLINK
DLLENTRY  =   DllMain
DLLDEF    =   ..\FabrikamKitchenServer.ProxyStub.def

!IF "$(_BUILDARCH)" == "arm"
NT_SIGNCODE=1
TEST_CODE=1
!ENDIF

_NT_TARGET_VERSION=$(_NT_TARGET_VERSION_WIN8)

USE_MSVCRT= 1

INCLUDES = \
    ..;\
    $(OBJ_PATH)\$(O);\
    $(COM_INC_PATH)\winrt;\
    $(MINWIN_PRIV_SDK_INC_PATH)

SOURCES=\
    $(SOURCES) \
    ..\guids.c \
    $(OBJ_PATH)\$O\Fabrikam_p.c \
    $(OBJ_PATH)\$O\Fabrikam_i.c \
    $(OBJ_PATH)\$O\privateinterfaces_p.c \
    $(OBJ_PATH)\$O\privateinterfaces_i.c \
    $(OBJ_PATH)\$O\dlldata.c \

TARGETLIBS = \
    $(SDK_LIB_PATH)\rpcrt4.lib \
    $(MINCORE_SDK_LIB_VPATH)\api-ms-win-core-winrt-string-l1.lib \
    $(ONECORECOM_LIB_PATH)\combasep.lib \

C_DEFINES= $(C_DEFINES) -DUNICODE=1 -D_UNICODE=1 -DREGISTER_PROXY_DLL -DPROXY_CLSID_IS={0xF6EFEABD,0x3370,0x4AA0,{0xA4,0xFD,0x88,0xBA,0xDC,0x3B,0x1A,0x39}}

