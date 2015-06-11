TARGETNAME=Jscript.MemProtectHeap
TARGETTYPE=LIBRARY

INCLUDES= $(COREINCLUDES);$(OBJ_PATH)\..\..\..\..\manifests\inbox\$(O);$(OBJ_PATH)\..\..\..\..\manifests\$(O)

SOURCES= \
    ..\MemProtectHeap.cpp \

PRECOMPILED_INCLUDE=..\stdafx.h
PRECOMPILED_SOURCEFILE=..\stdafx.cpp
PRECOMPILED_CXX=1
