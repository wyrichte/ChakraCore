TARGETNAME=JScript.Runtime.Telemetry
TARGETTYPE=LIBRARY

INCLUDES= $(COREINCLUDES);$(COM_INC_PATH);$(SDK_LIB_PATH)\clr45;..\..\tools\external\inc

SOURCES= \
    ..\IScriptContextTelemetryProvider.cpp \
    ..\OpcodeTelemetry.cpp \
    ..\ScriptContextTelemetry.cpp \
    ..\ScriptEngineTelemetry.cpp \


PRECOMPILED_INCLUDE=..\..\stdafx.h
PRECOMPILED_SOURCEFILE=..\..\stdafx.cpp
PRECOMPILED_CXX=1
