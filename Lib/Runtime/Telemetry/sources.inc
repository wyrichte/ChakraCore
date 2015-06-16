TARGETNAME=JScript.Runtime.Telemetry
TARGETTYPE=LIBRARY

INCLUDES= $(COREINCLUDES)

SOURCES= \
    ..\IScriptContextTelemetryProvider.cpp \
    ..\OpcodeTelemetry.cpp \
    ..\ScriptContextTelemetry.cpp \
    ..\ScriptEngineTelemetry.cpp \


PRECOMPILED_INCLUDE=..\..\stdafx.h
PRECOMPILED_SOURCEFILE=..\..\stdafx.cpp
PRECOMPILED_CXX=1
