# This assembly is for diagnostics (WER, debugging) purposes.
# It is separate assemlby from jscript9.dll in order to keep VS/WER functionaly separate, mainly for maintenance reasons.
#
!include sources_common.inc

RESOURCE_FILE=..\jscript9diag.rc

USE_NOTHROW_NEW=1

USE_DEFAULT_WIN32_LIBS=0
BUILD_FOR_CORESYSTEM=1
ATL_VER=80
USE_STATIC_ATL=1

INCLUDES = $(OBJECT_ROOT)\inetcore\jscript\publish\$(O);$(INCLUDES);

LINKLIBS= \
    $(SDK_LIB_PATH)\ntdll.lib \



TARGETLIBS=     \
    $(INETCORE_LIB_PATH)\iertutilp.lib \
    $(INETCORE_LIB_PATH)\IESettingsId.lib \
    $(INETCORE_LIB_PATH)\IESettingsApi.lib \
    $(OBJECT_INETCORE_DIR)\lib\common\iel1_sp\$(O)\ieshare_common_iel1.lib \
    $(SDK_LIB_PATH)\mincore.lib \
    $(MINCORE_SDK_LIB_PATH)\mincore_fw.lib    \
    $(MINCORE_SDK_LIB_PATH)\mincore_obsolete.lib \
    $(MINCORE_PRIV_SDK_LIB_VPATH)\mincore_private.lib \
    $(SHELL_LIB_VPATH)\api-ms-win-shcore-stream-l1.lib \
    $(MINCORE_PRIV_SDK_LIB_VPATH)\ext-ms-win-ntuser-windowclass-l1.lib \
    $(MINCORE_PRIV_SDK_LIB_VPATH)\ext-ms-win-ntuser-message-l1.lib \
    $(MINCORE_PRIV_SDK_LIB_VPATH)\ext-ms-win-ntuser-synch-l1.lib \
    $(COM_LIB_PATH)\combasep.lib \
    $(SDK_LIB_PATH)\uuid.lib \
    $(SHELL_LIB_PATH)\CommonUtilityLibrary.lib \
    $(OBJECT_INETCORE_DIR)\lib\codex\iel1\$(O)\codex_iel1.lib \
    $(OBJECT_JSCRIPT_DIR)\lib\common\core\$(JS_LIBTYPE)\$(O)\chakra.common.core.lib \
    $(OBJECT_JSCRIPT_DIR)\lib\common\common\$(JS_LIBTYPE)\$(O)\chakra.common.common.lib \
    $(OBJECT_JSCRIPT_DIR)\lib\common\datastructures\$(JS_LIBTYPE)\$(O)\chakra.common.datastructures.lib \
    $(OBJECT_JSCRIPT_DIR)\lib\runtime\language\$(JS_LIBTYPE)\$(O)\Chakra.runtime.language.lib \

DELAYLOAD = \
    api-ms-win-shcore-stream-l1.dll; \
    ext-ms-win-ntuser-windowclass-l1.dll; \
    ext-ms-win-ntuser-message-l1.dll; \
    ext-ms-win-ntuser-synch-l1.dll; \
    
    

ALLOW_NT_STYLE_DLOAD_ERROR_HANDLER=1
DLOAD_ERROR_HANDLER=kernelbase.dll


SOURCES= \
    $(RESOURCE_FILE) \
    ..\DAC.cpp \
    ..\DebugClient.cpp \
    ..\DiagProvider.cpp \
    ..\dllfunc.cpp \
    ..\expression.cpp \
    ..\guids.c \
    ..\InternalStackFrame.cpp \
    ..\RemoteDiagFrame.cpp \
    ..\RemoteStackFrame.cpp \
    ..\RemoteStackFrameEnumerator.cpp \
    ..\RemoteStackFrameLocals.cpp \
    ..\RemoteStackWalker.cpp \
    ..\RemoteString.cpp \
    ..\RemoteTypeHandler.cpp \
    ..\RemoteInlineFrameWalker.cpp \
    ..\Serializer.cpp \
    ..\ScriptDump.cpp \
    ..\ScriptState.cpp \
    ..\thrownew.cpp \
    ..\VirtualReader.cpp \
    ..\inspection.cpp \
    ..\jsdebug.cpp \
    ..\jsdebugProcess.cpp \
    ..\jsdebugStackWalker.cpp \
    ..\jsdebugBreakPoint.cpp \
    ..\RemoteAllocator.cpp \

DLLDEF=$(OBJ_PATH)\$(O)\JScript9diag.def

PRECOMPILED_INCLUDE=..\stdafx.h
PRECOMPILED_SOURCEFILE=..\stdafx.cpp
PRECOMPILED_CXX=1
