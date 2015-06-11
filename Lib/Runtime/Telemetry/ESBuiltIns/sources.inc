TARGETNAME=JScript.Runtime.Telemetry.ESBuiltIns
TARGETTYPE=LIBRARY

INCLUDES= $(COREINCLUDES);$(COM_INC_PATH);$(SDK_LIB_PATH)\clr45;..\..\..\tools\external\inc

SOURCES= \
    ..\ESBuiltInsDatabase.cpp \
	..\ESBuiltInsOpcodeTelemetry.cpp \
    ..\ESBuiltInsTelemetryProvider.cpp \


PRECOMPILED_INCLUDE=..\..\..\stdafx.h
PRECOMPILED_SOURCEFILE=..\..\..\stdafx.cpp
PRECOMPILED_CXX=1
