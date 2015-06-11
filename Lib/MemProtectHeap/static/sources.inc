!include ..\..\..\..\jslib.inc

TARGETNAME=JScript.MemProtectHeap.static
TARGETTYPE=LIBRARY

INCLUDES= $(COREINCLUDES);$(OBJ_PATH)\..\..\..\..\..\manifests\inbox\$(O);$(OBJ_PATH)\..\..\..\..\..\manifests\$(O)

!if "$(_BUILDARCH)" == "AMD64"
INCLUDES=$(INCLUDES);amd64
!else
INCLUDES=$(INCLUDES);arm
!endif

PRECOMPILED_INCLUDE=..\..\stdafx.h
PRECOMPILED_SOURCEFILE=..\..\stdafx.cpp
PRECOMPILED_CXX=1

SOURCES=     \
    ..\StubExternalAPI.cpp \
    ..\InitializeAPI.cpp \

OBJLIBFILES= \
    $(OBJECT_JSCRIPT_DIR)\lib\common\memory\$(JS_LIBTYPE)\$(O)\jscript.common.memory.lib \
    $(OBJECT_JSCRIPT_DIR)\lib\MemProtectHeap\$(JS_LIBTYPE)\$(O)\jscript.memprotectheap.lib \
    $(OBJECT_JSCRIPT_DIR)\lib\common\core\$(JS_LIBTYPE)\$(O)\jscript.common.core.lib \
    $(OBJECT_JSCRIPT_DIR)\lib\common\exceptions\$(JS_LIBTYPE)\$(O)\jscript.common.exceptions.lib \
    $(OBJECT_JSCRIPT_DIR)\lib\common\datastructures\$(JS_LIBTYPE)\$(O)\jscript.common.datastructures.lib \

