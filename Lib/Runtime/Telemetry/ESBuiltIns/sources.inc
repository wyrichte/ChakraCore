TARGETNAME=JScript.Runtime.Telemetry.ESBuiltIns
TARGETTYPE=LIBRARY

INCLUDES= $(COREINCLUDES)

SOURCES= \
    ..\ESBuiltInsDatabase.cpp \
	..\ESBuiltInsOpcodeTelemetry.cpp \
    ..\ESBuiltInsTelemetryProvider.cpp \


PRECOMPILED_INCLUDE=..\..\..\stdafx.h
PRECOMPILED_SOURCEFILE=..\..\..\stdafx.cpp
PRECOMPILED_CXX=1
