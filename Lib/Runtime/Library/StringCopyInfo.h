// Copyright (C) Microsoft. All rights reserved. 

#pragma once

namespace Js
{
    #pragma region StringCopyInfo
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class StringCopyInfo
    {
    private:
        JavascriptString *sourceString;
        wchar_t *destinationBuffer;
    #if DBG
        bool isInitialized;
    #endif

    public:
        StringCopyInfo();
        StringCopyInfo(JavascriptString *const sourceString, __out_xcount(sourceString->m_charLength) wchar_t *const destinationBuffer);

    public:
        JavascriptString *SourceString() const;
        wchar_t *DestinationBuffer() const;

    private:
        static void InstantiateForceInlinedMembers();
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    #pragma endregion

    #pragma region StringCopyInfoStack
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class StringCopyInfoStack
    {
    private:
        ScriptContext *const scriptContext;
        TempArenaAllocatorObject *allocator;
        LargeStack<StringCopyInfo> *stack;

    public:
        StringCopyInfoStack(ScriptContext *const scriptContext);
        ~StringCopyInfoStack();

    public:
        bool IsEmpty();
        void Push(const StringCopyInfo copyInfo);
        const StringCopyInfo Pop();

    private:
        void CreateStack();

        PREVENT_COPY(StringCopyInfoStack);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    #pragma endregion
}
