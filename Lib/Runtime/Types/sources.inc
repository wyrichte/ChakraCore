TARGETNAME=JScript.Runtime.Types
TARGETTYPE=LIBRARY

INCLUDES= $(COREINCLUDES)

#4702: unreachable code
MSC_WARNING_LEVEL=$(MSC_WARNING_LEVEL) /wd4702

SOURCES= \
    ..\ActivationObjectType.cpp \
    ..\ArrayObject.cpp \
    ..\CrossSite.cpp \
    ..\DomFastPath.cpp \
    ..\DynamicObject.cpp \
    ..\DynamicObjectEnumerator.cpp \
    ..\DynamicObjectSnapshotEnumerator.cpp \
    ..\DynamicObjectSnapshotEnumeratorWPCache.cpp \
    ..\DynamicType.cpp \
    ..\ExternalType.cpp \
    ..\TypeHandler.cpp \
    ..\NullTypeHandler.cpp \
    ..\DeferredTypeHandler.cpp \
    ..\SimpleTypeHandler.cpp \
    ..\PathTypeHandler.cpp \
    ..\SimpleDictionaryTypeHandler.cpp \
    ..\SimpleDictionaryUnorderedTypeHandler.cpp \
    ..\DictionaryTypeHandler.cpp \
    ..\MissingPropertyTypeHandler.cpp \
    ..\FunctionInfo.cpp \
    ..\FunctionBody.cpp \
    ..\JavascriptEnumerator.cpp \
    ..\JavascriptExceptionObject.cpp \
    ..\PerfHint.cpp \
    ..\PropertyDescriptor.cpp \
    ..\RecyclableObject.cpp \
    ..\WithScopeObject.cpp \
    ..\SpreadArgument.cpp \
    ..\StaticType.cpp \
    ..\Type.cpp \
    ..\TypePath.cpp \
    ..\TempArenaAllocatorObject.cpp \
    ..\TypePropertyCache.cpp

PRECOMPILED_INCLUDE=..\..\stdafx.h
PRECOMPILED_SOURCEFILE=..\..\stdafx.cpp
PRECOMPILED_CXX=1
