TARGETNAME=Chakra.Parser
TARGETTYPE=LIBRARY

INCLUDES= $(COREINCLUDES);

SOURCES=\
    ..\alloc.cpp \
    ..\cmperr.cpp \
    ..\errstr.cpp \
    ..\globals.cpp \
    ..\hash.cpp \
    ..\parse.cpp \
    ..\TextbookBoyerMoore.cpp \
    ..\scan.cpp \
    ..\screrror.cpp \
    ..\scrutil.cpp \
    ..\DebugWriter.cpp \
    ..\RegexStats.cpp \
    ..\OctoquadIdentifier.cpp \
    ..\CaseInsensitive.cpp \
    ..\BitCounts.cpp \
    ..\CharSet.cpp \
    ..\CharTrie.cpp \
    ..\StandardChars.cpp \
    ..\RegexCompileTime.cpp \
    ..\RegexParser.cpp \
    ..\TextbookBoyerMoore.cpp \
    ..\RegexRunTime.cpp \
    ..\RegexPattern.cpp \
    ..\RtError.cpp \
    ..\CharClassifier.cpp \
    ..\BackgroundParser.cpp \

PRECOMPILED_INCLUDE=..\stdafx.h
PRECOMPILED_SOURCEFILE=..\stdafx.cpp
PRECOMPILED_CXX=1


NTTARGETFILE0= \
	$(OBJ_PATH)\$(O)\jserr.rc2 \
