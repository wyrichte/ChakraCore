TARGETNAME=JScript.Common.DataStructures
TARGETTYPE=LIBRARY

INCLUDES= $(COREINCLUDES);

SOURCES= \
    ..\BigInt.cpp \
    ..\BufferBuilder.cpp \
    ..\DictionaryStats.cpp \
    ..\FixedBitVector.cpp \
    ..\FixedBitVectorEnumerator.cpp \
    ..\growingarray.cpp \
    ..\ImmutableList.cpp \
    ..\InternalString.cpp \
    ..\Interval.cpp \
    ..\InternalStringNoCaseComparer.cpp \

PRECOMPILED_INCLUDE=..\..\stdafx.h
PRECOMPILED_SOURCEFILE=..\..\stdafx.cpp
PRECOMPILED_CXX=1
