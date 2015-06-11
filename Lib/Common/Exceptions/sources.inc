TARGETNAME=JScript.Common.exceptions
TARGETTYPE=LIBRARY

INCLUDES= $(COREINCLUDES);

SOURCES= \
    ..\ExceptionBase.cpp       \
    ..\ExceptionCheck.cpp \
    ..\ReportError.cpp \
    ..\Throw.cpp       \

PRECOMPILED_INCLUDE=..\..\stdafx.h
PRECOMPILED_SOURCEFILE=..\..\stdafx.cpp
PRECOMPILED_CXX=1
