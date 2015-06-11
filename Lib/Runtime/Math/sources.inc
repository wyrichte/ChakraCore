TARGETNAME=JScript.Runtime.Math
TARGETTYPE=LIBRARY

INCLUDES= $(COREINCLUDES)

!ifdef X86
MSC_OPTIMIZATION=$(MSC_OPTIMIZATION) /arch:SSE2
!endif

SOURCES= \
    ..\JavascriptSSE2MathOperators.cpp\
    ..\AsmJsMath.cpp

PRECOMPILED_INCLUDE=..\..\stdafx.h
PRECOMPILED_SOURCEFILE=..\..\stdafx.cpp 
PRECOMPILED_CXX=1
