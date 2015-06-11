TARGETNAME=JScript.backend
TARGETTYPE=LIBRARY

!if !$(FREEBUILD)
MSC_OPTIMIZATION=$(MSC_OPTIMIZATION) /Ob1
!endif

SOURCES= \
    ..\BackEndOpcodeAttr.cpp \
    ..\BackwardPass.cpp \
    ..\BailOut.cpp \
    ..\CaseNode.cpp \
    ..\CodeGenWorkItem.cpp \
    ..\CodeGenAllocators.cpp \
    ..\CodeGenNumberAllocator.cpp \
    ..\Debug.cpp \
    ..\DbCheckPostLower.cpp \
    ..\EmitBuffer.cpp \
    ..\InterpreterThunkEmitter.cpp \
    ..\Encoder.cpp \
    ..\FlowGraph.cpp \
    ..\Func.cpp \
    ..\GlobOpt.cpp \
    ..\GlobOptBailOut.cpp \
    ..\GlobOptExpr.cpp \
    ..\GlobOptFields.cpp \
    ..\GlobOptIntBounds.cpp \
    ..\Inline.cpp \
    ..\InliningHeuristics.cpp \
    ..\InliningDecider.cpp \
    ..\InlineeFrameInfo.cpp \
    ..\ValueRelativeOffset.cpp \
    ..\IntBounds.cpp \
    ..\InductionVariable.cpp \
    ..\IR.cpp \
    ..\IRBuilder.cpp \
    ..\IRBuilderAsmJs.cpp \
    ..\IRType.cpp \
    ..\JnHelperMethod.cpp \
    ..\LinearScan.cpp \
    ..\Lower.cpp \
    ..\NativeCodeGenerator.cpp \
    ..\NativeCodeData.cpp \
    ..\Opnd.cpp \
    ..\PDataManager.cpp \
    ..\Peeps.cpp \
    ..\PEWriter.cpp \
    ..\PreLowerPeeps.cpp \
    ..\QueuedFullJitWorkItem.cpp \
    ..\Region.cpp \
    ..\SCCLiveness.cpp \
    ..\Security.cpp \
    ..\SimpleJitProfilingHelpers.cpp \
    ..\SimpleLayout.cpp \
    ..\Sym.cpp \
    ..\SymTable.cpp \
    ..\TempTracker.cpp \
    ..\IRViewer.cpp \
    ..\BackEndOpcodeAttrAsmJs.cpp\

PRECOMPILED_INCLUDE=..\backend.h
PRECOMPILED_SOURCEFILE=..\backend.cpp
PRECOMPILED_CXX=1
