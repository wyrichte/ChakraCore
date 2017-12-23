//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "ByteCodeCacheReleaseFileVersion.h"

struct AutoStr
{
    const char16* str;
    AutoStr(const char16* str = nullptr) : str(str) {}
    ~AutoStr()
    {
        if (str)
        {
            HeapFree(GetProcessHeap(), 0, (void*)str);
        }
    }
};

struct AutoStrVector
{
    std::vector<char16*>& vec;
    AutoStrVector(std::vector<char16*>& vec) : vec(vec) {}
    ~AutoStrVector()
    {
        for (size_t i = 0; i < vec.size(); ++i)
        {
            delete vec[i];
        }
    }
};

VerifyAllByteCodeReturnCode VerifyAllByteCode(LPWSTR verificationFile)
{
    std::vector<char16*> lines;
    AutoStrVector autoStrVector(lines);
    const auto appendLine = [&lines](const char16* format, ...)
    {
        const int bufSize = 256;
        char16* line = new char16[bufSize];
        va_list argptr;
        va_start(argptr, format);
        size_t size = _vsnwprintf_s(line, bufSize, _TRUNCATE, format, argptr);
        Assert(size != -1);
        va_end(argptr);
        line[size] = 0;
        lines.push_back(line);
        return size;
    };

    const size_t guidLength = appendLine(_u("Bytecode GUID = {%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
        byteCodeCacheReleaseFileVersion.Data1,
        byteCodeCacheReleaseFileVersion.Data2,
        byteCodeCacheReleaseFileVersion.Data3,
        byteCodeCacheReleaseFileVersion.Data4[0],
        byteCodeCacheReleaseFileVersion.Data4[1],
        byteCodeCacheReleaseFileVersion.Data4[2],
        byteCodeCacheReleaseFileVersion.Data4[3],
        byteCodeCacheReleaseFileVersion.Data4[4],
        byteCodeCacheReleaseFileVersion.Data4[5],
        byteCodeCacheReleaseFileVersion.Data4[6],
        byteCodeCacheReleaseFileVersion.Data4[7]
    );
    const char16* guid = lines[0];

    // Print OpCodes
    int opcode = 0;
    char16* tag = _u("OpCode");
#define DEF_OP(op, layout, ...) \
    appendLine(_u("%s: %s = %d. Layout = %s"), tag, _u(#op), opcode++, _u(#layout));

#include "OpCodeList.h"
    opcode = 256;
#include "ExtendedOpCodeList.h"
    // Print AsmJs OpCodes
    tag = _u("OpCodeAsmJs");
    opcode = 0;
#include "OpCodeListAsmJs.h"
    opcode = 256;
#include "ExtendedOpCodeListAsmJs.h"
#undef DEF_OP

    // Print Properties
#define APPEND_PROPERTY(tag, n) \
    appendLine(_u("%s: %s"), _u(#tag), _u(#n));
#define INTERNALPROPERTY(n) APPEND_PROPERTY(InternalProperty, n)
#include "InternalPropertyList.h"
#define ENTRY_INTERNAL_SYMBOL(n) APPEND_PROPERTY(Property, n)
#define ENTRY_SYMBOL(n, d) APPEND_PROPERTY(Property, n)
#define ENTRY(n) APPEND_PROPERTY(Property, n)
#define ENTRY2(n, s) APPEND_PROPERTY(Property, n)
#include "Base/JnDirectFields.h"
#undef APPEND_PROPERTY

    // Print Serializable fields
#define APPEND_SERIALIZABLE(type, name, ...) appendLine(_u("Serializable Field: %s %s;"), _u(#type), _u(#name));
#define DECLARE_SERIALIZABLE_FIELD APPEND_SERIALIZABLE
#define DECLARE_SERIALIZABLE_ACCESSOR_FIELD APPEND_SERIALIZABLE
#define DECLARE_SERIALIZABLE_ACCESSOR_FIELD_NO_CHECK APPEND_SERIALIZABLE
#define DEFINE_ALL_FIELDS
#include "SerializableFunctionFields.h"

    if (verificationFile != nullptr)
    {
        const char16 *fileContent = nullptr;
        if (FAILED(JsHostLoadScriptFromFile(verificationFile, fileContent)))
        {
            return VerifyAllByteCodeReturnCode::FileOpenError;
        }
        AutoStr autoContents(fileContent);

        std::vector<char16*> fileLines;
        char16* pwc = wcstok((char16*)fileContent, _u("\r\n"));
        while (pwc != nullptr)
        {
            fileLines.push_back(pwc);
            pwc = wcstok(nullptr, _u("\r\n"));
        }
        if (fileLines.size() == 0)
        {
            wprintf(_u("Empty verification file\n"));
            return VerifyAllByteCodeReturnCode::BadFileContent;
        }

        const char16* guidInFile = fileLines[0];
        const size_t guidInFileLength = wcslen(guidInFile);
        bool GUIDMatches = guidInFileLength == guidLength && wcsncmp(guidInFile, guid, guidLength) == 0;
        if (!GUIDMatches)
        {
            wprintf(_u("GUID do not match, baseline update required\n"));
            return VerifyAllByteCodeReturnCode::GUIDMismatch;
        }
        bool opcodesChanged = fileLines.size() != lines.size();
        for (size_t i = 0; !opcodesChanged && i < fileLines.size(); ++i)
        {
            const size_t aLen = wcslen(fileLines[i]);
            const size_t bLen = wcslen(lines[i]);
            opcodesChanged = aLen != bLen;
            if (!opcodesChanged)
            {
                opcodesChanged = wcsncmp(fileLines[i], lines[i], aLen) != 0;
            }
        }
        if (opcodesChanged)
        {
            // Printf in stderr so stdout contains only what we need to change in ByteCodeCacheReleaseFileVersion.h
            fwprintf(stderr, _u("Change affecting serialization without GUID update. Must update GUID in core/lib/Runtime/ByteCode/ByteCodeCacheReleaseFileVersion.h\n"));
            GUID pguid;
            CoCreateGuid(&pguid);
            fwprintf(stderr, _u("Recommended GUID update\n\n"));

            wprintf(_u("\
//-------------------------------------------------------------------------------------------------------\n\
// Copyright (C) Microsoft. All rights reserved.\n\
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.\n\
//-------------------------------------------------------------------------------------------------------\n\
// NOTE: If there is a merge conflict the correct fix is to make a new GUID.\n\n"));

            wprintf(_u("// {%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}\n"),
                pguid.Data1,
                pguid.Data2,
                pguid.Data3,
                pguid.Data4[0],
                pguid.Data4[1],
                pguid.Data4[2],
                pguid.Data4[3],
                pguid.Data4[4],
                pguid.Data4[5],
                pguid.Data4[6],
                pguid.Data4[7]
            );
            wprintf(_u("const GUID byteCodeCacheReleaseFileVersion =\n"));
            wprintf(_u("{ 0x%08X, 0x%04X, 0x%04X, { 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X } };\n"),
                pguid.Data1,
                pguid.Data2,
                pguid.Data3,
                pguid.Data4[0],
                pguid.Data4[1],
                pguid.Data4[2],
                pguid.Data4[3],
                pguid.Data4[4],
                pguid.Data4[5],
                pguid.Data4[6],
                pguid.Data4[7]
            );
            return VerifyAllByteCodeReturnCode::RequireGUIDUpdate;
        }
    }

    for (size_t i = 0; i < lines.size(); ++i)
    {
        wprintf(_u("%s\n"), lines[i]);
    }

    return VerifyAllByteCodeReturnCode::Success;
}
