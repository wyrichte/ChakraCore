//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace IR
{
enum JnHelperMethod;
}

struct NativeMapFunction
{
    uint32 entryOffset;
    uint32 numberOfEntries;
};

struct NativeMapEntry
{
    uint32 nativeOffset;
    uint32 statementIndex;
};

struct NativeData
{
    BYTE *data;
    size_t size;
};

struct CodeBlock
{
    CodeBlock()
        : base(0)
        , code(0)
        , size(0)
#if DBG_DUMP
        , nativeMap(nullptr)
#endif
        , nativeThrowMap(nullptr)
#if _M_X64 || _M_ARM
        , unwindInfo()
#endif
#if _M_ARM
        , pdataTable()
#endif
    {
    }

    CodeBlock(
        size_t base,
        const BYTE *code, 
        ptrdiff_t size,
#if _M_X64 || _M_ARM
        NativeData unwindInfo,
#endif
#if _M_ARM
        NativeData pdataTable,
#endif
#if DBG_DUMP
        JsUtil::List<NativeMapEntry, ArenaAllocator> *nativeMap,
#endif
        JsUtil::List<NativeMapEntry, ArenaAllocator> *nativeThrowMap)
        : base(base)
        , code(code)
        , size(size)
#if _M_X64 || _M_ARM
        , unwindInfo(unwindInfo)
#endif
#if _M_ARM
        , pdataTable(pdataTable)
#endif
#if DBG_DUMP
        , nativeMap(nativeMap)
#endif
        , nativeThrowMap(nativeThrowMap)
    {
    }

    size_t base;
    const BYTE *code;
    ptrdiff_t size;
#if _M_X64 || _M_ARM
    NativeData unwindInfo;
#endif
#if _M_ARM
    NativeData pdataTable;
#endif
    JsUtil::List<NativeMapEntry, ArenaAllocator> *nativeMap;
    JsUtil::List<NativeMapEntry, ArenaAllocator> *nativeThrowMap;
};

class PEWriter
{
public:
    PEWriter(ArenaAllocator *allocator);
    void Write(Js::FunctionBody *rootFunction, BYTE *sourceCode, DWORD dwSourceCodeSize, BYTE *byteCode, DWORD dwByteCodeSize, BYTE ** nativeCode, DWORD * pdwNativeCodeSize);
    size_t RecordNativeCodeSize(ptrdiff_t codeSize, ushort pdataCount, ushort xdataSize);
    BYTE *RecordNativeCode(size_t bytes, const BYTE* sourceBuffer);
    void RecordNativeRelocation(size_t offset);
#if DBG_DUMP | defined(VTUNE_PROFILING)
    void RecordNativeMap(uint32 nativeOffset, uint32 statementIndex);
#endif
#if DBG_DUMP
    void DumpNativeOffsetMaps();
    void DumpNativeThrowSpanSequence();
#endif
    void RecordNativeThrowMap(uint32 nativeOffset, uint32 statementIndex);
#if _M_X64 || _M_ARM
    size_t RecordUnwindInfo(size_t offset, BYTE *unwindInfo, size_t size);
#endif
#if _M_ARM
    void RecordPdataEntry(int index, DWORD beginAddress, DWORD unwindData);
#endif
    void FinalizeNativeCode();
    void RecordEmptyFunction();

private:
#if _M_X64
    static const unsigned __int64 baseAddress = 0x0000000180000000;
#else
    static const unsigned baseAddress = 0x10000000;
#endif
    static const unsigned fileAlignment = 0x0200;
    static const unsigned sectionAlignment = 0x1000;

    static const unsigned headerRawSize = 0x0400;
    static const unsigned headerImageSize = 0x1000;

    ArenaAllocator *allocator;
    size_t currentTextRVA;
    size_t currentBase;
    NativeData currentNativeCode;

#if DBG_DUMP | defined(VTUNE_PROFILING)
    JsUtil::List<NativeMapEntry, ArenaAllocator> *currentNativeMap;
#endif

    JsUtil::List<NativeMapEntry, ArenaAllocator> *currentNativeThrowMap;

#if _M_X64 || _M_ARM
    NativeData currentUnwindInfo;
#endif
#if _M_ARM
    NativeData currentPdataTable;
#endif

    JsUtil::List<CodeBlock, ArenaAllocator> *codeBlocks;
    JsUtil::List<size_t, ArenaAllocator> *relocations;
};
