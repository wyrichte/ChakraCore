TARGETNAME=JScript.Common.common
TARGETTYPE=LIBRARY

INCLUDES= $(INCLUDES);$(SDK_LIB_PATH);$(SDK_INC_PATH);$(COM_INC_PATH)\WinRT;..\..;$(OBJ_PATH)\..\..\..\..\manifests\$(O)

SOURCES= \
    ..\Api.cpp \
    ..\CFGLogger.cpp \
    ..\DaylightTimeHelper.cpp \
    ..\DateUtilities.cpp \
    ..\Event.cpp \
    ..\Int32Math.cpp \
    ..\Jobs.cpp \
    ..\MathUtil.cpp \
    ..\NumberUtilities.cpp \
    ..\NumberUtilities_strtod.cpp \
    ..\RejitReason.cpp \
    ..\Tick.cpp \
    ..\VTInfo.cpp \

AMD64_SOURCES=\
    ..\amd64\commona.asm

ARM_SOURCES= \
    ..\arm\arm_GET_CURRENT_FRAME.asm \

ARM64_SOURCES= \
    ..\arm64\arm64_GET_CURRENT_FRAME.asm \

PRECOMPILED_INCLUDE=..\..\stdafx.h
PRECOMPILED_SOURCEFILE=..\..\stdafx.cpp
PRECOMPILED_CXX=1
