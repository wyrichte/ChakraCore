TARGETNAME=Chakra.Runtime.ByteCode
TARGETTYPE=LIBRARY

INCLUDES= $(COREINCLUDES);..

SOURCES= \
        ..\ByteBlock.cpp   \
        ..\ByteCodeDumper.cpp      \
        ..\ByteCodeGenerator.cpp   \
        ..\ByteCodeReader.cpp      \
        ..\ByteCodeWriter.cpp      \
        ..\Opcodes.cpp     \
        ..\OpcodeUtil.cpp     \
        ..\ScopeInfo.cpp    \
        ..\StatementReader.cpp    \
        ..\AsmJSByteCodeDumper.cpp\
        ..\AsmJsByteCodeWriter.cpp\
        ..\OpCodeUtilAsmJs.cpp    \

PRECOMPILED_INCLUDE=..\..\stdafx.h
PRECOMPILED_SOURCEFILE=..\..\stdafx.cpp
PRECOMPILED_CXX=1
