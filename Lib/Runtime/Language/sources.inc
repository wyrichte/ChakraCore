TARGETNAME=JScript.Runtime.Language
TARGETTYPE=LIBRARY

INCLUDES= $(COREINCLUDES)

!if $(FREEBUILD)
INCLUDES= \
    $(INCLUDES); \
    $(PUBLIC_ROOT)\internal\base\inc;
!endif

#4702: unreachable code
MSC_WARNING_LEVEL=$(MSC_WARNING_LEVEL) /wd4702 


SOURCES= \
    ..\ByteCodeSerializer.cpp \
    ..\CacheOperators.cpp \
    ..\CallInfo.cpp \
    ..\CodeGenRecyclableData.cpp \
    ..\diagProbe.cpp \
    ..\ProbeContainer.cpp \
    ..\diagObjectModel.cpp \
    ..\DiagStackFrame.cpp \
    ..\SourceContextInfo.cpp \
    ..\DiagHelperMethodWrapper.cpp \
    ..\DynamicProfileInfo.cpp \
    ..\DynamicProfileStorage.cpp \
    ..\DynamicProfileMutator.cpp \
    ..\ExecutionMode.cpp \
    ..\FunctionCodeGenJitTimeData.cpp \
    ..\FunctionCodeGenRuntimeData.cpp \
    ..\InlineCache.cpp \
    ..\InterpreterStackFrame.cpp \
    ..\JavascriptConversion.cpp \
    ..\JavascriptExceptionOperators.cpp \
    ..\JavascriptMathOperators.cpp \
    ..\JavascriptOperators.cpp \
    ..\JavascriptStackWalker.cpp \
    ..\MutationBreakpoint.cpp \
    ..\ProfilingHelpers.cpp \
    ..\PropertyRecord.cpp \
    ..\ReadOnlyDynamicProfileInfo.cpp \
    ..\ScriptContext.cpp \
    ..\ScriptContextOptimizationOverrideInfo.cpp \
    ..\ScriptContextProfiler.cpp \
    ..\SourceDynamicProfileManager.cpp \
    ..\SourceMutator.cpp \
    ..\StackTraceArguments.cpp \
    ..\ValueType.cpp \
    ..\Utf8SourceInfo.cpp \
    ..\SourceHolder.cpp \
    ..\AsmJs.cpp \
    ..\AsmJsByteCodeGenerator.cpp \
    ..\AsmJsTypes.cpp \
    ..\AsmJsUtils.cpp \
    ..\AsmJsLink.cpp \
    ..\AsmJsModule.cpp \

I386_SOURCES= \
    ..\i386\stackframe.cpp\
    ..\i386\AsmJSJitTemplate.cpp\
    ..\AsmJSCodeGenerator.cpp\
    ..\AsmJsEncoder.cpp

AMD64_SOURCES= \
    ..\amd64\JavascriptOperatorsA.asm \
    ..\amd64\amd64_Thunks.asm \
    ..\amd64\stackframe.cpp \
    ..\amd64\AsmJSJitTemplate.cpp \
    ..\AsmJSCodeGenerator.cpp \
    ..\AsmJsEncoder.cpp


ARM_SOURCES= \
    ..\arm\arm_CallEhFrame.asm \
    ..\arm\arm_Thunks.asm \
    ..\arm\stackframe.cpp

ARM64_SOURCES= \
    ..\arm64\arm64_CallEhFrame.asm \
    ..\arm64\arm64_Thunks.asm \
    ..\arm64\stackframe.cpp

PRECOMPILED_INCLUDE=..\..\stdafx.h
PRECOMPILED_SOURCEFILE=..\..\stdafx.cpp
PRECOMPILED_CXX=1
