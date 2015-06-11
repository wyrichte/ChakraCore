//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "backend.h"
#include "PEWriter.h"

inline static size_t roundUp(size_t len, size_t align) 
{
    return ((len + align-1) & ~(align-1));
}

PEWriter::PEWriter(ArenaAllocator *allocator)
    : allocator(allocator)
    , currentTextRVA(headerImageSize)
    , currentBase(0)
    , currentNativeThrowMap(nullptr)
    , currentNativeCode()
#if DBG_DUMP
    , currentNativeMap(nullptr)
#endif
#if _M_X64 || _M_ARM
    , currentUnwindInfo()
#endif
#if _M_ARM
    , currentPdataTable()
#endif
{
    codeBlocks = JsUtil::List<CodeBlock, ArenaAllocator>::New(allocator);
    relocations = JsUtil::List<size_t, ArenaAllocator>::New(allocator);
}

size_t PEWriter::RecordNativeCodeSize(ptrdiff_t codeSize, ushort pdataCount, ushort xdataSize)
{
    Assert(currentBase == 0);
    currentBase = currentTextRVA + baseAddress;
    currentTextRVA += roundUp(codeSize, 2);

    size_t newCodeSize = roundUp(codeSize, 2);
    BYTE *code = AnewArray(allocator, BYTE, newCodeSize);
    Assert(currentNativeCode.data == nullptr);
    Assert(currentNativeCode.size == 0);
    currentNativeCode.data = code;
    currentNativeCode.size = newCodeSize;

#if _M_IX86
    Assert(pdataCount == 0 && xdataSize == 0);
#elif _M_X64
    Assert(pdataCount == 1);
#else
    Assert(pdataCount > 0);
#endif

#if _M_X64 || _M_ARM
    DWORD newXdataSize = (DWORD)roundUp(xdataSize, 4);
    BYTE *newUnwindInfo = AnewArray(allocator, BYTE, newXdataSize);
    Assert(currentUnwindInfo.data == nullptr);
    Assert(currentUnwindInfo.size == 0);
    currentUnwindInfo.data = newUnwindInfo;
    currentUnwindInfo.size = newXdataSize;
#endif

#if _M_ARM
    BYTE *newPdataTable = (BYTE *)AnewArray(allocator, RUNTIME_FUNCTION, pdataCount);
    Assert(currentPdataTable.data == nullptr);
    Assert(currentPdataTable.size == 0);
    currentPdataTable.data = newPdataTable;
    currentPdataTable.size = pdataCount;
#endif

    return currentBase;
}

BYTE *PEWriter::RecordNativeCode(size_t bytes, const BYTE* sourceBuffer)
{
    Assert(currentNativeCode.size == roundUp(bytes, 2));
    memcpy_s(currentNativeCode.data, currentNativeCode.size, sourceBuffer, bytes);
    return currentNativeCode.data;
}

#if _M_X64 || _M_ARM
size_t PEWriter::RecordUnwindInfo(size_t offset, BYTE *unwindInfo, size_t size)
{
#if _M_X64
    Assert(offset == 0);
#endif
    memcpy_s(currentUnwindInfo.data + offset, currentUnwindInfo.size - offset, unwindInfo, size);
    return (size_t)offset;
}
#endif

#if _M_ARM
void PEWriter::RecordPdataEntry(int index, DWORD beginAddress, DWORD unwindData)
{
    RUNTIME_FUNCTION *pdata = ((RUNTIME_FUNCTION *)currentPdataTable.data) + index;
    pdata->BeginAddress = beginAddress - baseAddress;
    pdata->UnwindData = unwindData;
}
#endif

void PEWriter::RecordNativeRelocation(size_t offset)
{
    relocations->Add(offset - baseAddress - headerImageSize);
}

#if DBG_DUMP | defined(VTUNE_PROFILING)
void PEWriter::RecordNativeMap(uint32 nativeOffset, uint32 statementIndex)
{
    if (currentNativeMap == nullptr)
    {
        currentNativeMap = JsUtil::List<NativeMapEntry, ArenaAllocator>::New(allocator);
    }

    NativeMapEntry nativeMapEntry = { nativeOffset, statementIndex };
    currentNativeMap->Add(nativeMapEntry);
}
#endif

void PEWriter::FinalizeNativeCode()
{
    CodeBlock block(
        currentBase,
        currentNativeCode.data,
        currentNativeCode.size,
#if _M_X64 || _M_ARM
        currentUnwindInfo,
#endif
#if _M_ARM
        currentPdataTable,
#endif
#if DBG_DUMP
        currentNativeMap,
#endif
        currentNativeThrowMap);
    currentBase = 0;
#if _M_X64 || _M_ARM
    currentUnwindInfo = NativeData();
#endif
#if _M_ARM
    currentPdataTable = NativeData();
#endif
    currentNativeCode = NativeData();
#if DBG_DUMP
    currentNativeMap = nullptr;
#endif
    currentNativeThrowMap = nullptr;
    codeBlocks->Add(block);
}

void PEWriter::RecordEmptyFunction()
{
    Assert(currentBase == 0);
    codeBlocks->Add(CodeBlock());
}

#if DBG_DUMP
void PEWriter::DumpNativeOffsetMaps()
{
    const CodeBlock codeBlock = codeBlocks->Item(codeBlocks->Count() - 1);

    if (codeBlock.nativeMap && codeBlock.nativeMap->Count())
    {
        Output::Print(L"Native Map: baseAddr: 0x%IX, size: 0x%Ix\nstatementId, offset range, address range\n",
            codeBlock.base,
            codeBlock.size);

        int count = codeBlock.nativeMap->Count();
        for(int i = 0; i < count; i++)
        {
            NativeMapEntry map = codeBlock.nativeMap->Item(i);
            Output::Print(L"S%4d, 0x%Ix\n", map.statementIndex, map.nativeOffset + codeBlock.base);
        }
    }
}

void PEWriter::DumpNativeThrowSpanSequence()
{
    const CodeBlock codeBlock = codeBlocks->Item(codeBlocks->Count() - 1);

    if (codeBlock.nativeThrowMap && codeBlock.nativeThrowMap->Count())
    {
        Output::Print(L"Native Throw Map: baseAddr: 0x%Ix, size: 0x%Ix\nstatementId, offset range, address range\n",
            codeBlock.base,
            codeBlock.size);

        int count = codeBlock.nativeThrowMap->Count();
        Js::SmallSpanSequenceIter iter;
        for (int i = 0; i < count; i++)
        {
            NativeMapEntry map = codeBlock.nativeThrowMap->Item(i);
            Output::Print(L"S%4d, 0x%Ix\n", map.statementIndex, map.nativeOffset + codeBlock.base);
        }
    }
}
#endif

void PEWriter::RecordNativeThrowMap(uint32 nativeOffset, uint32 statementIndex)
{
    if (currentNativeThrowMap == nullptr)
    {
        currentNativeThrowMap = JsUtil::List<NativeMapEntry, ArenaAllocator>::New(allocator);
    }

    NativeMapEntry nativeThrowMapEntry = { nativeOffset, statementIndex };
    currentNativeThrowMap->Add(nativeThrowMapEntry);
}

size_t RelocationsSize(JsUtil::List<size_t, ArenaAllocator> *relocations)
{
    size_t currentBlock = (size_t)-1;
    size_t size = 0;
    size_t count = 0;

    relocations->Map([&] (int index, size_t relocation)
    {
        size_t block = relocation / 0x1000;

        if (block != currentBlock)
        {
            if (count % 2 != 0)
            {
                count++;
            }

            if (currentBlock != (size_t)-1)
            {
                size += (count * 2) + sizeof(IMAGE_BASE_RELOCATION);
                Assert(size % 4 == 0);
            }

            currentBlock = block;
            count = 0;
        }

        count++;
    });

    if (count % 2 != 0)
    {
        count++;
    }

    if (currentBlock != (size_t)-1)
    {
        size += (count * 2) + sizeof(IMAGE_BASE_RELOCATION);
        Assert(size % 4 == 0);
    }

    return size;
}

size_t NativeMapCount(JsUtil::List<CodeBlock, ArenaAllocator> *codeBlocks)
{
#if DBG_DUMP
    size_t total = 0;

    codeBlocks->Map([&] (int index, CodeBlock codeBlock)
    {
        total += codeBlock.nativeMap ? codeBlock.nativeMap->Count() : 0;
    });

    return total;
#else
    return 0;
#endif
}

size_t NativeThrowMapCount(JsUtil::List<CodeBlock, ArenaAllocator> *codeBlocks)
{
    size_t total = 0;

    codeBlocks->Map([&] (int index, CodeBlock codeBlock)
    {
        total += codeBlock.nativeThrowMap ? codeBlock.nativeThrowMap->Count() : 0;
    });

    return total;
}

#if _M_X64 || _M_ARM
uint PdataCount(JsUtil::List<CodeBlock, ArenaAllocator> *codeBlocks)
{
    uint total = 0;

    codeBlocks->Map([&] (int index, CodeBlock codeBlock)
    {
        if (codeBlock.base != 0)
        {
#if _M_X64
            total++;
#else
            total += codeBlock.pdataTable.size;
#endif
        }
    });

    return total;
}

size_t UnwindInfosSize(JsUtil::List<CodeBlock, ArenaAllocator> *codeBlocks)
{
    size_t total = 0;

    codeBlocks->Map([&] (int index, CodeBlock codeBlock)
    {
        if (codeBlock.base != 0)
        {
            total += codeBlock.unwindInfo.size;
        }
    });

    return total;
}
#endif

void PEWriter::Write(Js::FunctionBody *rootFunction, BYTE *sourceCode, DWORD dwSourceCodeUnalignedSize, BYTE *byteCode, DWORD dwByteCodeSize, BYTE ** nativeCode, DWORD * pdwNativeCodeSize)
{
    const char *unknown = "<unknown>";

    // If we end up with a zero sized text section due to partial serialization, we have to round up
    // to at least create the section.
    size_t textRawUnalignedSize = (currentTextRVA - headerImageSize) == 0 ? 1 : (currentTextRVA - headerImageSize);
    size_t textRawSize = roundUp(textRawUnalignedSize, fileAlignment);
    size_t textImageSize = roundUp(textRawUnalignedSize, sectionAlignment);

    size_t relocRawUnalignedSize = RelocationsSize(relocations);
    size_t relocRawSize = roundUp(relocRawUnalignedSize, fileAlignment);
    size_t relocImageSize = roundUp(relocRawUnalignedSize, sectionAlignment);

    size_t nativeMapSize = (codeBlocks->Count() * sizeof(NativeMapFunction)) + (NativeMapCount(codeBlocks) * sizeof(NativeMapEntry));
    size_t nativeThrowMapSize = (codeBlocks->Count() * sizeof(NativeMapFunction)) + (NativeThrowMapCount(codeBlocks) * sizeof(NativeMapEntry));
    size_t dwSourceCodeSize = roundUp(dwSourceCodeUnalignedSize, sizeof(uint32));
    size_t rsrcRawUnalignedSize = 
        (sizeof(IMAGE_RESOURCE_DIRECTORY) * 5) + 
        (sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY) * 7) + 
        (sizeof(IMAGE_RESOURCE_DATA_ENTRY) * 3) +
        ((dwSourceCodeSize > 0) ? 
            (dwSourceCodeSize + 
             sizeof(IMAGE_RESOURCE_DIRECTORY) +
             (sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY) * 2) +
             sizeof(IMAGE_RESOURCE_DATA_ENTRY)) :
            0) +
        dwByteCodeSize +
        nativeMapSize +
        nativeThrowMapSize;
    size_t rsrcRawSize = roundUp(rsrcRawUnalignedSize, fileAlignment);
    size_t rsrcImageSize = roundUp(rsrcRawUnalignedSize, sectionAlignment);

    size_t edataRawUnalignedSize =
        sizeof(IMAGE_EXPORT_DIRECTORY) +
        ((codeBlocks->Count() + 1) * 4) +
        strlen(unknown) + 1;
    size_t edataRawSize = roundUp(edataRawUnalignedSize, fileAlignment);
    size_t edataImageSize = roundUp(edataRawUnalignedSize, sectionAlignment);

#if _M_X64 || _M_ARM
    size_t pdataRawUnalignedSize = PdataCount(codeBlocks) * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);
    size_t pdataRawSize = roundUp(pdataRawUnalignedSize, fileAlignment);
    size_t pdataImageSize = roundUp(pdataRawUnalignedSize, sectionAlignment);

    size_t xdataRawUnalignedSize = UnwindInfosSize(codeBlocks);
    size_t xdataRawSize = roundUp(xdataRawUnalignedSize, fileAlignment);
    size_t xdataImageSize = roundUp(xdataRawUnalignedSize, sectionAlignment);
#else
    size_t pdataRawSize = 0;
    size_t pdataImageSize = 0;

    size_t xdataRawSize = 0;
    size_t xdataImageSize = 0;
#endif

    size_t rawSize = headerRawSize + textRawSize + relocRawSize + rsrcRawSize + edataRawSize + pdataRawSize + xdataRawSize;
    size_t imageSize = headerImageSize + textImageSize + relocImageSize + rsrcImageSize + edataImageSize + pdataImageSize + xdataImageSize;

    BYTE *memory;
    memory = (BYTE *)CoTaskMemAlloc(rawSize);
    BYTE *current = memory;
    if (memory == nullptr)
    {
        Js::Throw::OutOfMemory();
    }
    memset(memory, 0, rawSize);

    //
    // DOS header
    //

    IMAGE_DOS_HEADER *dosHeader = (IMAGE_DOS_HEADER *)memory;

    time_t now;
    time(&now);

    dosHeader->e_magic = IMAGE_DOS_SIGNATURE;
    dosHeader->e_lfanew = sizeof(IMAGE_DOS_HEADER);

#if _M_X64
    current += sizeof(IMAGE_DOS_HEADER);
    IMAGE_NT_HEADERS64 *ntHeaders = (IMAGE_NT_HEADERS64 *)current;
#else
    current += sizeof(IMAGE_DOS_HEADER);
    IMAGE_NT_HEADERS32 *ntHeaders = (IMAGE_NT_HEADERS32 *)current;
#endif

    //
    // NT header
    //

    ntHeaders->Signature = IMAGE_NT_SIGNATURE;

#if _M_IX86
    ntHeaders->FileHeader.Machine = IMAGE_FILE_MACHINE_I386;
#elif _M_X64
    ntHeaders->FileHeader.Machine = IMAGE_FILE_MACHINE_AMD64;
#elif _M_ARM
    ntHeaders->FileHeader.Machine = IMAGE_FILE_MACHINE_ARMNT;
#endif
#if _M_X64 || _M_ARM
    ntHeaders->FileHeader.NumberOfSections = (relocations->Count() > 0) ? 6 : 5;
#else
    ntHeaders->FileHeader.NumberOfSections = (relocations->Count() > 0) ? 4 : 3;
#endif
    ntHeaders->FileHeader.TimeDateStamp = (ULONG) now;
#if _M_X64
    ntHeaders->FileHeader.SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER64);
#else
    ntHeaders->FileHeader.SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER32);
#endif
    ntHeaders->FileHeader.Characteristics = 
        IMAGE_FILE_EXECUTABLE_IMAGE |
#if _M_ARM
        IMAGE_FILE_32BIT_MACHINE |
        IMAGE_FILE_LARGE_ADDRESS_AWARE |
#endif
        IMAGE_FILE_DLL;

#if _M_X64
    ntHeaders->OptionalHeader.Magic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
#else
    ntHeaders->OptionalHeader.Magic = IMAGE_NT_OPTIONAL_HDR32_MAGIC;
#endif

    // A convenient way for us to find the data section.
    ntHeaders->OptionalHeader.BaseOfCode = (DWORD)(headerImageSize + textImageSize);

    ntHeaders->OptionalHeader.MajorLinkerVersion = PE_MAJOR_VERSION;
    ntHeaders->OptionalHeader.MinorLinkerVersion = PE_MINOR_VERSION;

    ntHeaders->OptionalHeader.ImageBase = baseAddress;
    ntHeaders->OptionalHeader.SectionAlignment = sectionAlignment;
    ntHeaders->OptionalHeader.FileAlignment = fileAlignment;
    ntHeaders->OptionalHeader.MajorOperatingSystemVersion = 6;
    ntHeaders->OptionalHeader.MinorOperatingSystemVersion = 0;
    ntHeaders->OptionalHeader.MajorImageVersion = 0;
    ntHeaders->OptionalHeader.MinorImageVersion = 0;
    ntHeaders->OptionalHeader.MajorSubsystemVersion = 6;
    ntHeaders->OptionalHeader.MinorSubsystemVersion = 0;
    ntHeaders->OptionalHeader.Win32VersionValue = 0;
    ntHeaders->OptionalHeader.SizeOfImage = (DWORD)imageSize;
    ntHeaders->OptionalHeader.SizeOfHeaders = headerRawSize;
    ntHeaders->OptionalHeader.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI;
    ntHeaders->OptionalHeader.DllCharacteristics = 
        IMAGE_DLLCHARACTERISTICS_NO_SEH |
        IMAGE_DLLCHARACTERISTICS_NX_COMPAT |
        IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE;
#if _M_X64
    ntHeaders->OptionalHeader.SizeOfStackReserve = 0x400000;
    ntHeaders->OptionalHeader.SizeOfStackCommit = 0x4000;
    ntHeaders->OptionalHeader.SizeOfHeapReserve = 0x100000;
    ntHeaders->OptionalHeader.SizeOfHeapCommit = 0x2000;
#else
    ntHeaders->OptionalHeader.SizeOfStackReserve = 0x100000;
    ntHeaders->OptionalHeader.SizeOfStackCommit = 0x1000;
    ntHeaders->OptionalHeader.SizeOfHeapReserve = 0x100000;
    ntHeaders->OptionalHeader.SizeOfHeapCommit = 0x1000;
#endif
    ntHeaders->OptionalHeader.NumberOfRvaAndSizes = 16;

    if (relocations->Count() > 0)
    {
        ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = (DWORD)(headerImageSize + textImageSize);
        ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = (DWORD)relocRawUnalignedSize;
    }

    ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress = (DWORD)(headerImageSize + textImageSize + relocImageSize);
    ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size = (DWORD)rsrcRawUnalignedSize;

    ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress = (DWORD)(headerImageSize + textImageSize + relocImageSize + rsrcImageSize);
    ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size = (DWORD)edataRawUnalignedSize;

#if _M_X64 || _M_ARM
    ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress = (DWORD)(headerImageSize + textImageSize + relocImageSize + rsrcImageSize + edataImageSize);
    ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size = (DWORD)pdataRawUnalignedSize;
#endif

    //
    // Section headers
    //

#if _M_X64
    current += sizeof(IMAGE_NT_HEADERS64);
#else
    current += sizeof(IMAGE_NT_HEADERS32);
#endif
    IMAGE_SECTION_HEADER *textSectionHeader = (IMAGE_SECTION_HEADER *)current;

    textSectionHeader->Name[0] = '.';
    textSectionHeader->Name[1] = 't';
    textSectionHeader->Name[2] = 'e';
    textSectionHeader->Name[3] = 'x';
    textSectionHeader->Name[4] = 't';
    textSectionHeader->Misc.VirtualSize = (DWORD)textRawUnalignedSize;
    textSectionHeader->VirtualAddress = headerImageSize;
    textSectionHeader->SizeOfRawData = (DWORD)textRawSize;
    textSectionHeader->PointerToRawData = headerRawSize;
    textSectionHeader->Characteristics =
        IMAGE_SCN_CNT_CODE |
        IMAGE_SCN_MEM_READ |
        IMAGE_SCN_MEM_EXECUTE;

    if (relocations->Count() > 0)
    {
        current += sizeof(IMAGE_SECTION_HEADER);
        IMAGE_SECTION_HEADER *relocSectionHeader = (IMAGE_SECTION_HEADER *)current;

        relocSectionHeader->Name[0] = '.';
        relocSectionHeader->Name[1] = 'r';
        relocSectionHeader->Name[2] = 'e';
        relocSectionHeader->Name[3] = 'l';
        relocSectionHeader->Name[4] = 'o';
        relocSectionHeader->Name[5] = 'c';
        relocSectionHeader->Misc.VirtualSize = (DWORD)relocRawUnalignedSize;
        relocSectionHeader->VirtualAddress = (DWORD)(headerImageSize + textImageSize);
        relocSectionHeader->SizeOfRawData = (DWORD)relocRawSize;
        relocSectionHeader->PointerToRawData = (DWORD)(headerRawSize + textRawSize);
        relocSectionHeader->Characteristics =
            IMAGE_SCN_CNT_INITIALIZED_DATA |
            IMAGE_SCN_MEM_DISCARDABLE |
            IMAGE_SCN_MEM_READ;
    }

    current += sizeof(IMAGE_SECTION_HEADER);
    IMAGE_SECTION_HEADER *rsrcSectionHeader = (IMAGE_SECTION_HEADER *)current;

    rsrcSectionHeader->Name[0] = '.';
    rsrcSectionHeader->Name[1] = 'r';
    rsrcSectionHeader->Name[2] = 's';
    rsrcSectionHeader->Name[3] = 'r';
    rsrcSectionHeader->Name[4] = 'c';
    rsrcSectionHeader->Misc.VirtualSize = (DWORD)rsrcRawUnalignedSize;
    rsrcSectionHeader->VirtualAddress = (DWORD)(headerImageSize + textImageSize + relocImageSize);
    rsrcSectionHeader->SizeOfRawData = (DWORD)rsrcRawSize;
    rsrcSectionHeader->PointerToRawData = (DWORD)(headerRawSize + textRawSize + relocRawSize);
    rsrcSectionHeader->Characteristics =
        IMAGE_SCN_CNT_INITIALIZED_DATA |
        IMAGE_SCN_MEM_READ;

    current += sizeof(IMAGE_SECTION_HEADER);
    IMAGE_SECTION_HEADER *edataSectionHeader = (IMAGE_SECTION_HEADER *)current;

    edataSectionHeader->Name[0] = '.';
    edataSectionHeader->Name[1] = 'e';
    edataSectionHeader->Name[2] = 'd';
    edataSectionHeader->Name[3] = 'a';
    edataSectionHeader->Name[4] = 't';
    edataSectionHeader->Name[5] = 'a';
    edataSectionHeader->Misc.VirtualSize = (DWORD)edataRawUnalignedSize;
    edataSectionHeader->VirtualAddress = (DWORD)(headerImageSize + textImageSize + relocImageSize + rsrcImageSize);
    edataSectionHeader->SizeOfRawData = (DWORD)edataRawSize;
    edataSectionHeader->PointerToRawData = (DWORD)(headerRawSize + textRawSize + relocRawSize + rsrcRawSize);
    edataSectionHeader->Characteristics =
        IMAGE_SCN_CNT_INITIALIZED_DATA |
        IMAGE_SCN_MEM_READ;

#if _M_X64 || _M_ARM
    current += sizeof(IMAGE_SECTION_HEADER);
    IMAGE_SECTION_HEADER *pdataSectionHeader = (IMAGE_SECTION_HEADER *)current;

    pdataSectionHeader->Name[0] = '.';
    pdataSectionHeader->Name[1] = 'p';
    pdataSectionHeader->Name[2] = 'd';
    pdataSectionHeader->Name[3] = 'a';
    pdataSectionHeader->Name[4] = 't';
    pdataSectionHeader->Name[5] = 'a';
    pdataSectionHeader->Misc.VirtualSize = (DWORD)pdataRawUnalignedSize;
    pdataSectionHeader->VirtualAddress = (DWORD)(headerImageSize + textImageSize + relocImageSize + rsrcImageSize + edataImageSize);
    pdataSectionHeader->SizeOfRawData = (DWORD)pdataRawSize;
    pdataSectionHeader->PointerToRawData = (DWORD)(headerRawSize + textRawSize + relocRawSize + rsrcRawSize + edataRawSize);
    pdataSectionHeader->Characteristics =
        IMAGE_SCN_CNT_INITIALIZED_DATA |
        IMAGE_SCN_MEM_READ;

    current += sizeof(IMAGE_SECTION_HEADER);
    IMAGE_SECTION_HEADER *xdataSectionHeader = (IMAGE_SECTION_HEADER *)current;

    xdataSectionHeader->Name[0] = '.';
    xdataSectionHeader->Name[1] = 'x';
    xdataSectionHeader->Name[2] = 'd';
    xdataSectionHeader->Name[3] = 'a';
    xdataSectionHeader->Name[4] = 't';
    xdataSectionHeader->Name[5] = 'a';
    xdataSectionHeader->Misc.VirtualSize = (DWORD)xdataRawUnalignedSize;
    xdataSectionHeader->VirtualAddress = (DWORD)(headerImageSize + textImageSize + relocImageSize + rsrcImageSize + edataImageSize + pdataImageSize);
    xdataSectionHeader->SizeOfRawData = (DWORD)xdataRawSize;
    xdataSectionHeader->PointerToRawData = (DWORD)(headerRawSize + textRawSize + relocRawSize + rsrcRawSize + edataRawSize + pdataRawSize);
    xdataSectionHeader->Characteristics =
        IMAGE_SCN_CNT_INITIALIZED_DATA |
        IMAGE_SCN_MEM_READ;
#endif

    //
    // .text
    //

    current = memory + headerRawSize;
    codeBlocks->Map([&] (int index, CodeBlock block)
    {
        if (block.size != 0)
        {
            memcpy_s(current, block.size, block.code, block.size);
            current += block.size;
        }
    });

    //
    // .reloc
    //

    if (relocations->Count() > 0)
    {
        current = memory + headerRawSize + textRawSize;

        size_t currentBlock = (size_t)-1;
        DWORD *currentBlockSize = nullptr;
        size_t count = 0;

        relocations->Map([&] (int index, size_t relocation)
        {
            size_t block = relocation / 0x1000;

            Assert(relocation < textRawSize);

            if (block != currentBlock)
            {
                if (count % 2 != 0)
                {
                    // Pad to 4 byte boundary
                    *(WORD *)current = (IMAGE_REL_BASED_ABSOLUTE << 12);
                    current += sizeof(WORD);
                    count++;
                }

                if (currentBlockSize != nullptr)
                {
                    *currentBlockSize = (DWORD)((count * 2) + sizeof(IMAGE_BASE_RELOCATION));
                    Assert((*currentBlockSize) % 4 == 0);
                }

                IMAGE_BASE_RELOCATION *blockHeader = (IMAGE_BASE_RELOCATION *)current;
                blockHeader->VirtualAddress = (DWORD)(textSectionHeader->VirtualAddress + (0x1000 * block));
                currentBlockSize = &(blockHeader->SizeOfBlock);
                current += sizeof(IMAGE_BASE_RELOCATION);
                currentBlock = block;
                count = 0;
            }

#if _M_X64
            *(WORD *)current = (IMAGE_REL_BASED_DIR64 << 12) + (relocation & 0xFFF);
#elif _M_ARM
            *(WORD *)current = (IMAGE_REL_BASED_THUMB_MOV32 << 12) + (relocation & 0xFFF);
#else
            *(WORD *)current = (IMAGE_REL_BASED_HIGHLOW << 12) + (relocation & 0xFFF);
#endif
            current += sizeof(WORD);
            count++;
        });

        if (count % 2 != 0)
        {
            // Pad to 4 byte boundary
            *(WORD *)current = (IMAGE_REL_BASED_ABSOLUTE << 12);
            current += sizeof(WORD);
            count++;
        }

        if (currentBlockSize != nullptr)
        {
            *currentBlockSize = (DWORD)((count * 2) + sizeof(IMAGE_BASE_RELOCATION));
            Assert((*currentBlockSize) % 4 == 0);
        }
    }

    //
    // .rsrc
    //

    current = memory + headerRawSize + textRawSize + relocRawSize;
    BYTE *rsrcStart = current;

    IMAGE_RESOURCE_DIRECTORY *typeDirectory = (IMAGE_RESOURCE_DIRECTORY *)current;

    typeDirectory->NumberOfIdEntries = 1;

    current += sizeof(IMAGE_RESOURCE_DIRECTORY);
    IMAGE_RESOURCE_DIRECTORY_ENTRY *typeDirectoryEntry = (IMAGE_RESOURCE_DIRECTORY_ENTRY *)current;

    typeDirectoryEntry->Id = 0xC0DE;
    typeDirectoryEntry->DataIsDirectory = 1;

    current += sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);
    typeDirectoryEntry->OffsetToDirectory = current - rsrcStart;
    IMAGE_RESOURCE_DIRECTORY *nameDirectory = (IMAGE_RESOURCE_DIRECTORY *)current;

    nameDirectory->NumberOfIdEntries = (dwSourceCodeSize > 0) ? 4 : 3;

    current += sizeof(IMAGE_RESOURCE_DIRECTORY);
    IMAGE_RESOURCE_DIRECTORY_ENTRY *sourceCodeNameDirectoryEntry = nullptr;
    if (dwSourceCodeSize > 0)
    {
        sourceCodeNameDirectoryEntry = (IMAGE_RESOURCE_DIRECTORY_ENTRY *)current;

        sourceCodeNameDirectoryEntry->Id = NativeResourceIds::SourceCodeResourceNameId;
        sourceCodeNameDirectoryEntry->DataIsDirectory = 1;

        current += sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);
    }
    IMAGE_RESOURCE_DIRECTORY_ENTRY *byteCodeNameDirectoryEntry = (IMAGE_RESOURCE_DIRECTORY_ENTRY *)current;

    byteCodeNameDirectoryEntry->Id = NativeResourceIds::ByteCodeResourceNameId;
    byteCodeNameDirectoryEntry->DataIsDirectory = 1;

    current += sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);
    IMAGE_RESOURCE_DIRECTORY_ENTRY *nativeMapNameDirectoryEntry = (IMAGE_RESOURCE_DIRECTORY_ENTRY *)current;

    nativeMapNameDirectoryEntry->Id = NativeResourceIds::NativeMapResourceNameId;
    nativeMapNameDirectoryEntry->DataIsDirectory = 1;

    current += sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);
    IMAGE_RESOURCE_DIRECTORY_ENTRY *nativeThrowMapNameDirectoryEntry = (IMAGE_RESOURCE_DIRECTORY_ENTRY *)current;

    nativeThrowMapNameDirectoryEntry->Id = NativeResourceIds::NativeThrowMapResourceNameId;
    nativeThrowMapNameDirectoryEntry->DataIsDirectory = 1;

    current += sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);
    IMAGE_RESOURCE_DIRECTORY *sourceCodeLanguageDirectory = nullptr;
    IMAGE_RESOURCE_DIRECTORY_ENTRY *sourceCodeLanguageDirectoryEntry = nullptr;
    if (dwSourceCodeSize > 0)
    {
        sourceCodeNameDirectoryEntry->OffsetToDirectory = current - rsrcStart;
        sourceCodeLanguageDirectory = (IMAGE_RESOURCE_DIRECTORY *)current;

        sourceCodeLanguageDirectory->NumberOfIdEntries = 1;

        current += sizeof(IMAGE_RESOURCE_DIRECTORY);
        sourceCodeLanguageDirectoryEntry = (IMAGE_RESOURCE_DIRECTORY_ENTRY *)current;

        sourceCodeLanguageDirectoryEntry->Id = 0x0409;

        current += sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);
    }
    byteCodeNameDirectoryEntry->OffsetToDirectory = current - rsrcStart;
    IMAGE_RESOURCE_DIRECTORY *byteCodeLanguageDirectory = (IMAGE_RESOURCE_DIRECTORY *)current;

    byteCodeLanguageDirectory->NumberOfIdEntries = 1;

    current += sizeof(IMAGE_RESOURCE_DIRECTORY);
    IMAGE_RESOURCE_DIRECTORY_ENTRY *byteCodeLanguageDirectoryEntry = (IMAGE_RESOURCE_DIRECTORY_ENTRY *)current;

    byteCodeLanguageDirectoryEntry->Id = 0x0409;

    current += sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);
    nativeMapNameDirectoryEntry->OffsetToDirectory = current - rsrcStart;
    IMAGE_RESOURCE_DIRECTORY *nativeMapLanguageDirectory = (IMAGE_RESOURCE_DIRECTORY *)current;

    nativeMapLanguageDirectory->NumberOfIdEntries = 1;

    current += sizeof(IMAGE_RESOURCE_DIRECTORY);
    IMAGE_RESOURCE_DIRECTORY_ENTRY *nativeMapLanguageDirectoryEntry = (IMAGE_RESOURCE_DIRECTORY_ENTRY *)current;

    nativeMapLanguageDirectoryEntry->Id = 0x0409;

    current += sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);
    nativeThrowMapNameDirectoryEntry->OffsetToDirectory = current - rsrcStart;
    IMAGE_RESOURCE_DIRECTORY *nativeThrowMapLanguageDirectory = (IMAGE_RESOURCE_DIRECTORY *)current;

    nativeThrowMapLanguageDirectory->NumberOfIdEntries = 1;

    current += sizeof(IMAGE_RESOURCE_DIRECTORY);
    IMAGE_RESOURCE_DIRECTORY_ENTRY *nativeThrowMapLanguageDirectoryEntry = (IMAGE_RESOURCE_DIRECTORY_ENTRY *)current;

    nativeThrowMapLanguageDirectoryEntry->Id = 0x0409;

    current += sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);
    IMAGE_RESOURCE_DATA_ENTRY *sourceCodeDataEntry = nullptr;
    if (dwSourceCodeSize > 0)
    {
        sourceCodeLanguageDirectoryEntry->OffsetToData = (DWORD)(current - rsrcStart);
        sourceCodeDataEntry = (IMAGE_RESOURCE_DATA_ENTRY *)current;

        sourceCodeDataEntry->Size = (DWORD)dwSourceCodeSize;

        current += sizeof(IMAGE_RESOURCE_DATA_ENTRY);
    }

    byteCodeLanguageDirectoryEntry->OffsetToData = (DWORD)(current - rsrcStart);
    IMAGE_RESOURCE_DATA_ENTRY *byteCodeDataEntry = (IMAGE_RESOURCE_DATA_ENTRY *)current;

    byteCodeDataEntry->Size = dwByteCodeSize;

    current += sizeof(IMAGE_RESOURCE_DATA_ENTRY);
    nativeMapLanguageDirectoryEntry->OffsetToData = (DWORD)(current - rsrcStart);
    IMAGE_RESOURCE_DATA_ENTRY *nativeMapDataEntry = (IMAGE_RESOURCE_DATA_ENTRY *)current;

    nativeMapDataEntry->Size = (DWORD)nativeMapSize;

    current += sizeof(IMAGE_RESOURCE_DATA_ENTRY);
    nativeThrowMapLanguageDirectoryEntry->OffsetToData = (DWORD)(current - rsrcStart);
    IMAGE_RESOURCE_DATA_ENTRY *nativeThrowMapDataEntry = (IMAGE_RESOURCE_DATA_ENTRY *)current;

    nativeThrowMapDataEntry->Size = (DWORD)nativeThrowMapSize;

    current += sizeof(IMAGE_RESOURCE_DATA_ENTRY);
    if (dwSourceCodeSize > 0)
    {
        sourceCodeDataEntry->OffsetToData = (DWORD)(headerImageSize + textImageSize + relocImageSize + (current - rsrcStart));
        BYTE *sourceCodeData = current;
    
        memcpy_s(sourceCodeData, dwSourceCodeUnalignedSize, sourceCode, dwSourceCodeUnalignedSize);

        current += dwSourceCodeSize;
    }

    byteCodeDataEntry->OffsetToData = (DWORD)(headerImageSize + textImageSize + relocImageSize + (current - rsrcStart));
    BYTE *byteCodeData = current;
    
    memcpy_s(byteCodeData, dwByteCodeSize, byteCode, dwByteCodeSize);

    current += dwByteCodeSize;
    nativeMapDataEntry->OffsetToData = (DWORD)(headerImageSize + textImageSize + relocImageSize + (current - rsrcStart));

    BYTE *nativeMapStart = current;
    BYTE *currentNativeMap = current + (codeBlocks->Count() * sizeof(NativeMapFunction));
    codeBlocks->Map([&] (int index, CodeBlock block)
    {
#if DBG_DUMP
        if (block.nativeMap && block.nativeMap->Count() > 0)
        {
            Assert(currentNativeMap - nativeMapStart <= UINT_MAX);
            NativeMapFunction func = { (uint32)(currentNativeMap - nativeMapStart), block.nativeMap->Count() };
            *(NativeMapFunction *)current = func;
            current += sizeof(NativeMapFunction);

            block.nativeMap->Map([&] (int index, NativeMapEntry entry)
            {
                *(NativeMapEntry *)currentNativeMap = entry;
                currentNativeMap += sizeof(NativeMapEntry);
            });
        }
        else
        {
            NativeMapFunction func = { 0, 0 };
            *(NativeMapFunction *)current = func;
            current += sizeof(NativeMapFunction);
        }
#else
        NativeMapFunction func = { 0, 0 };
        *(NativeMapFunction *)current = func;
        current += sizeof(NativeMapFunction);
#endif
    });

    current = currentNativeMap;
    nativeThrowMapDataEntry->OffsetToData = (DWORD)(headerImageSize + textImageSize + relocImageSize + (current - rsrcStart));
    BYTE *nativeThrowMapStart = current;
    BYTE *currentNativeThrowMap = current + (codeBlocks->Count() * sizeof(NativeMapFunction));
    codeBlocks->Map([&] (int index, CodeBlock block)
    {
        if (block.nativeThrowMap && block.nativeThrowMap->Count() > 0)
        {
            NativeMapFunction func = { (uint32)(currentNativeThrowMap - nativeThrowMapStart), block.nativeThrowMap->Count() };
            *(NativeMapFunction *)current = func;
            current += sizeof(NativeMapFunction);
            block.nativeThrowMap->Map([&] (int index, NativeMapEntry entry)
            {
                *(NativeMapEntry *)currentNativeThrowMap = entry;
                currentNativeThrowMap += sizeof(NativeMapEntry);
            });
        }
        else
        {
            NativeMapFunction func = { 0, 0 };
            *(NativeMapFunction *)current = func;
            current += sizeof(NativeMapFunction);
        }
    });

    //
    // .edata (exports)
    //

    current = memory + headerRawSize + textRawSize + relocRawSize + rsrcRawSize;
    Assert (current >= currentNativeThrowMap);
    IMAGE_EXPORT_DIRECTORY *exportDirectory = (IMAGE_EXPORT_DIRECTORY *)current;

    exportDirectory->TimeDateStamp = (DWORD)now;
    exportDirectory->Name = edataSectionHeader->VirtualAddress + sizeof(IMAGE_EXPORT_DIRECTORY) + ((codeBlocks->Count() + 1) * 4);
    exportDirectory->Base = 1;
    exportDirectory->NumberOfFunctions = codeBlocks->Count() + 1;
    exportDirectory->AddressOfFunctions = edataSectionHeader->VirtualAddress + sizeof(IMAGE_EXPORT_DIRECTORY);

    current += sizeof(IMAGE_EXPORT_DIRECTORY);
    DWORD *currentExport = (DWORD *)current;
    size_t currentRVA = textSectionHeader->VirtualAddress;

    codeBlocks->Map([&] (int index, CodeBlock block)
    {
        *currentExport = (DWORD) currentRVA;
        currentRVA += block.size;
        currentExport++;
    });

    // We emit one extra import so that we can calculate the size of each method's code by
    // subtracting from the next import.
    *currentExport = (DWORD)currentRVA;
    currentExport++;

    strcpy_s((char *)currentExport, strlen(unknown) + 1, unknown);

#if _M_X64 || _M_ARM
    //
    // .pdata
    //

    current = memory + headerRawSize + textRawSize + relocRawSize + rsrcRawSize + edataRawSize;
    size_t currentFunctionRVA = textSectionHeader->VirtualAddress;
    size_t currentUnwindInfoRVA = xdataSectionHeader->VirtualAddress;

    codeBlocks->Map([&] (int index, CodeBlock block)
    {
        if (block.base != 0)
        {
#if _M_X64
            IMAGE_RUNTIME_FUNCTION_ENTRY *functionEntry = (IMAGE_RUNTIME_FUNCTION_ENTRY *)current;
            functionEntry->BeginAddress = (DWORD)currentFunctionRVA;
            functionEntry->EndAddress = (DWORD)(currentFunctionRVA + block.size);
            functionEntry->UnwindInfoAddress = (DWORD)currentUnwindInfoRVA;
            current += sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);
            currentFunctionRVA += block.size;
            currentUnwindInfoRVA += block.unwindInfo.size;
#elif _M_ARM
            RUNTIME_FUNCTION *pdataTable = (RUNTIME_FUNCTION *)block.pdataTable.data;
            for (uint index = 0; index < block.pdataTable.size; index++)
            {
                IMAGE_RUNTIME_FUNCTION_ENTRY *functionEntry = (IMAGE_RUNTIME_FUNCTION_ENTRY *)current;
                functionEntry->BeginAddress = pdataTable[index].BeginAddress;
                functionEntry->UnwindData = pdataTable[index].UnwindData;
                if (!pdataTable[index].Flag)
                {
                    Assert(functionEntry->UnwindData < block.unwindInfo.size);
                    functionEntry->UnwindData = currentUnwindInfoRVA + functionEntry->UnwindData;
                }
                current += sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);
            }
            currentFunctionRVA += block.size;
            currentUnwindInfoRVA += block.unwindInfo.size;
#endif
        }
    });

    //
    // .xdata
    //

    current = memory + headerRawSize + textRawSize + relocRawSize + rsrcRawSize + edataRawSize + pdataRawSize;
    codeBlocks->Map([&] (int index, CodeBlock block)
    {
        if (block.base != 0 && block.unwindInfo.size > 0)
        {
            memcpy_s(current, block.unwindInfo.size, block.unwindInfo.data, block.unwindInfo.size);
            current += block.unwindInfo.size;
        }
    });
#endif

    //
    // Done!
    //

    *nativeCode = memory;
    *pdwNativeCodeSize = (DWORD)rawSize;
}
