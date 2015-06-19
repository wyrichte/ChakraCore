TARGETNAME=JScript.Authoring
TARGETTYPE=LIBRARY

INCLUDES=$(INCLUDES);$(COREINCLUDES);$(SDK_LIB_PATH);$(SDK_INC_PATH)\atl30;$(JSCRIPT_ROOT)\core\lib\common\codex;
C_DEFINES = $(C_DEFINES) -DLANGUAGE_SERVICE=1

#4102: unreferenced label
#4389: '==' : signed/unsigned mismatch
MSC_WARNING_LEVEL=$(MSC_WARNING_LEVEL) /wd4102 /wd4389 

SOURCES=\
       ..\authorstr.cpp \
       ..\AuthUtils.cpp \
       ..\ScriptDirectAuthor.idl \
       ..\TokenEnumerator.cpp \
       ..\Colorizer.cpp \
       ..\RegionSet.cpp \
       ..\TaskCommentSet.cpp \
       ..\IdentifierAddPolicy.cpp \
       ..\TreeValidator.cpp \
       ..\CleanTree.cpp \
       ..\RewriteTreeHelpers.cpp \
       ..\AddLoopGuards.cpp \
       ..\DirectExecution.cpp \
       ..\AddNestedCalls.cpp \
       ..\HoistSubsumedFunctions.cpp \
       ..\MessageSet.cpp \
       ..\AuthoringProbe.cpp \
       ..\CommentTable.cpp \
       ..\MemoryAuthoringFile.cpp \
       ..\Completions.cpp \
       ..\DocComments.cpp \
       ..\JsDocCommentsParser.cpp \
       ..\JsDocCommentsScanner.cpp \
       ..\AuthoringFileHandle.cpp \
       ..\AuthorExecUtils.cpp \
       ..\ScriptContextPath.cpp \
       ..\ScriptContextManager.cpp \
       ..\ParseNodeCursor.cpp \
       ..\FunctionHelpInfo.cpp \
       ..\FileAuthoring.cpp \
       ..\AuthoringServicesInfo.cpp \
       ..\AstSerializers.cpp \
       ..\AuthoringFactory.cpp \
       ..\SymbolHelp.cpp \
       ..\DocCommentsRewrite.cpp \
       ..\ParseNodeTree.cpp \
       ..\ASTBuilder.cpp \
       ..\JsValueDoc.cpp \
       ..\LangSvcExtensibility.cpp \
       ..\AuthorDiagnostics.cpp \
       ..\MissingValueHandler.cpp \
       ..\AuthoringFileText.cpp \
       ..\LevelLimiter.cpp \
       ..\AsyncRequests.cpp \
       ..\Structure.cpp \
       ..\ReferenceSet.cpp \
       ..\SimpleComObject.cpp \

PASS1_BINPLACE= \
       -:DEST jscript\References ..\references\dedicatedworker.js \
       -:DEST jscript\References ..\references\domWeb.js \
       -:DEST jscript\References ..\references\domWindows_8.1.js \
       -:DEST jscript\References ..\references\libhelp.js \
       -:DEST jscript\References ..\references\sitetypesWeb.js \
       -:DEST jscript\References ..\references\sitetypesWindows.js \
       -:DEST jscript\References ..\references\showPlainComments.js \
       -:DEST jscript\References ..\references\base.intellisense-setup.js \
       -:DEST jscript\References ..\references\base.intellisense.js \
       -:DEST jscript\References ..\references\extensibility.intellisense.js \
       -:DEST jscript\References ..\references\JSDocExtension.js \
       -:DEST jscript\ ..\ScriptDirectAuthor.idl \

PRECOMPILED_INCLUDE=..\stdafx.h
PRECOMPILED_SOURCEFILE=..\stdafx.cpp
PRECOMPILED_CXX=1
