TARGETNAME=Chakra.WinRT
TARGETTYPE=LIBRARY
USE_ATL=1
ATL_VER=70

INCLUDES= $(COREINCLUDES);$(COM_INC_PATH)\winrt;$(COM_INC_PATH)\midlrt;
C_DEFINES =$(C_DEFINES) -DWINNT -DUNICODE -D_UNICODE -D__PLACEMENT_NEW_INLINE

MSC_WARNING_LEVEL=$(MSC_WARNING_LEVEL)

SOURCES=\
    ..\GUIDParser.cpp \
    ..\Metadata.cpp \
    ..\ProjectionModel.cpp \
    ..\SortProjection.cpp \
    ..\ModelHelper.cpp \
    ..\Visitor.cpp \
    ..\MetadataLocator.cpp \
    ..\AutoHSTRING.cpp

PRECOMPILED_INCLUDE=..\stdafx.h
PRECOMPILED_SOURCEFILE=..\stdafx.cpp
PRECOMPILED_CXX=1
