//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    using Js::StringCopyInfo;

    class StringCopyInfoStack : public CAtlArray<StringCopyInfo>
    {
    public:
        void Push(const StringCopyInfo copyInfo);
        const StringCopyInfo Pop();
    };

    class IRemoteStringFactory
    {
    public:
        // Read remote "str" content to "buf". throw if conent can not fit in "bufLen". Return actual content length in "*actual" (<= bufLen).
        virtual void Read(InspectionContext* context, Js::Var str,
            _Out_writes_to_(bufLen, *actual) char16* buf, _In_ charcount_t bufLen, _Out_ charcount_t* actual) = 0;

        // Copy remote "str" content to "buf". throw if remote "str" length is not "bufLen".
        virtual void Copy(InspectionContext* context, Js::Var str,
            _Out_writes_(bufLen) char16* buf, _In_ charcount_t bufLen, StringCopyInfoStack &nestedStringTreeCopyInfos, const byte recursionDepth) = 0;

        virtual bool IsTree(InspectionContext* context, Js::Var str) const = 0;
    };

    template <class T, class DAC>
    class RemoteStringFactory:
        public IRemoteStringFactory
    {
    public:
        virtual void Read(InspectionContext* context, Js::Var str,
            _Out_writes_to_(bufLen, *actual) char16* buf, _In_ charcount_t bufLen, _Out_ charcount_t* actual)
        {
            DAC(context, static_cast<const T*>(str)).Read(buf, bufLen, actual);
        }

        virtual void Copy(InspectionContext* context, Js::Var str,
            _Out_writes_(bufLen) char16* buf, _In_ charcount_t bufLen, StringCopyInfoStack &nestedStringTreeCopyInfos, const byte recursionDepth)
        {
            DAC(context, static_cast<const T*>(str)).Copy(buf, bufLen, nestedStringTreeCopyInfos, recursionDepth);
        }

        virtual bool IsTree(InspectionContext* context, Js::Var str) const override sealed
        {
            return DAC(context, static_cast<const T*>(str)).IsTree();
        }

        static RemoteStringFactory s_instance;
    };

    template <class T, class DAC>
    RemoteStringFactory<T, DAC> RemoteStringFactory<T, DAC>::s_instance;

    //
    // Base class for reading remote string
    //
    template <class T, class Sub>
    class RemoteString:
        public RemoteData<T>
    {
    protected:
        InspectionContext *const m_context;

    public:
        RemoteString(InspectionContext* context, const TargetType* str)
            : RemoteData(context->GetReader(), str), m_context(context)
        {
        }

        void Read(_Out_writes_to_(bufLen, *actual) char16* buffer, _In_ charcount_t bufLen, _Out_ charcount_t* actual);
        void Copy(_Out_writes_(bufLen) char16* buffer, _In_ charcount_t bufLen, StringCopyInfoStack &nestedStringTreeCopyInfos, const byte recursionDepth);
        bool IsTree() const { return false; }

        charcount_t GetLength() const { return ToTargetPtr()->m_charLength; }

    protected:
        const Sub* pThis() const { return (const Sub*)this; }
        Sub* pThis() { return (Sub*)this; }
        const char16* GetString() const { return ToTargetPtr()->m_pszValue; }
        bool IsFinalized() const { return GetString() != NULL; }
    };

    //
    // Handles general remote JavascriptString
    //
    class RemoteJavascriptString:
        public RemoteString<JavascriptString, RemoteJavascriptString>
    {
    public:
        RemoteJavascriptString(InspectionContext* context, const TargetType* str): RemoteString(context, str) {}
    };

    typedef RemoteJavascriptString          RemoteLiteralString;
    typedef RemoteJavascriptString          RemoteArenaLiteralString;
    typedef RemoteJavascriptString          RemoteSingleCharString;
    typedef RemoteJavascriptString          RemotePropertyString;
    typedef RemoteJavascriptString          RemoteSubString;
    typedef RemoteJavascriptString          RemoteBufferStringBuilderWritableString;

    class RemoteJSONString : public RemoteString<JSONString, RemoteJSONString>
    {
        friend class JSONString;
    public:
        RemoteJSONString(InspectionContext* context, const TargetType* str) : RemoteString(context, str) {}

        void Copy(_Out_writes_(bufLen) char16* buffer, _In_ charcount_t bufLen, StringCopyInfoStack &nestedStringTreeCopyInfos, const byte recursionDepth);
        bool IsTree() const { return false; }

        JavascriptString * GetOriginalString() { return this->ToTargetPtr()->m_originalString; }
    };

    //
    // Handles remote CompoundString
    //
    class RemoteCompoundString : public RemoteString<CompoundString, RemoteCompoundString>
    {
    private:
        //
        // Handles remote CompoundString::Block (This is not a JavascriptString type)
        //
        class RemoteBlock : public RemoteData<CompoundString::Block>
        {
        private:
            AutoArrayPtr<BYTE> buffer;

        public:
            RemoteBlock(InspectionContext *const context, const TargetType *const addr, const bool loadBuffer = true);
            void Reinitialize(InspectionContext *const context, const TargetType *const addr);

        public:
            const void *Buffer() const;
            const CompoundString::Block *Previous() const;

        public:
            const char16 *Chars() const;
            CharCount CharLength() const;

        public:
            void *const *Pointers() const;
            CharCount PointerLength() const;
        };

    private:
        AutoArrayPtr<BYTE> lastBlockBuffer;

    public:
        RemoteCompoundString(InspectionContext *const context, const TargetType *const str);

    private:
        bool OwnsLastBlock() const;
        bool HasOnlyDirectChars() const;
        const void *LastBlockBuffer();

    private:
        const char16 *LastBlockChars();
        CharCount LastBlockCharLength() const;

    private:
        void *const *LastBlockPointers();
        CharCount LastBlockPointerLength() const;

    public:
        void Copy(_Out_writes_(bufLen) char16* buffer, _In_ charcount_t bufLen, StringCopyInfoStack &nestedStringTreeCopyInfos, const byte recursionDepth);
        bool IsTree() const;
    };

    //
    // Base class for reading remote ConcatString
    //
    template <class T, class Sub>
    class RemoteConcatStringBase:
        public RemoteString<T, Sub>
    {
    public:
        RemoteConcatStringBase(InspectionContext* context, const TargetType* str): RemoteString(context, str) {}

        void Copy(_Out_writes_(bufLen) char16* buffer, _In_ charcount_t bufLen, StringCopyInfoStack &nestedStringTreeCopyInfos, const byte recursionDepth);
        bool IsTree() const { return true; }

        // These below are used by Copy. Derived classes that don't support these operations need to override (hide) Copy.
        int GetItemCount() const { Assert(false); return 0; }
        JavascriptString *GetItem(const int index) const { Assert(false); return nullptr; }
    };

    //
    // Handles remote ConcatStringN
    //
    template <int N>
    class RemoteConcatStringN:
        public RemoteConcatStringBase<ConcatStringN<N>, RemoteConcatStringN<N>>
    {
    public:
        RemoteConcatStringN(InspectionContext* context, const TargetType* str): RemoteConcatStringBase(context, str) {}

        int GetItemCount() const { return N; }
        JavascriptString *GetItem(const int index) const { Assert(index >= 0 && index < N); return ToTargetPtr()->m_slots[index]; }
    };

    typedef RemoteConcatStringN<2> RemoteConcatString;
    typedef RemoteConcatStringN<2> RemoteConcatStringN2;   
    typedef RemoteConcatStringN<4> RemoteConcatStringN4;
    typedef RemoteConcatStringN<6> RemoteConcatStringN6;
    typedef RemoteConcatStringN<7> RemoteConcatStringN7;

    //
    // Handles remote ConcatStringBuilder
    //
    class RemoteConcatStringBuilder:
        public RemoteConcatStringBase<ConcatStringBuilder, RemoteConcatStringBuilder>
    {
    public:
        RemoteConcatStringBuilder(InspectionContext* context, const TargetType* str): RemoteConcatStringBase(context, str) {}

        void Copy(_Out_writes_(bufLen) char16* buffer, _In_ charcount_t bufLen, StringCopyInfoStack &nestedStringTreeCopyInfos, const byte recursionDepth);
    };

    //
    // Handles ConcatStringWrapping
    //
    template <char16 L, char16 R>
    class RemoteConcatStringWrapping:
        public RemoteConcatStringBase<ConcatStringWrapping<L, R>, RemoteConcatStringWrapping<L, R>>
    {
    public:
        RemoteConcatStringWrapping(InspectionContext* context, const TargetType* str): RemoteConcatStringBase(context, str) {}

        void Copy(_Out_writes_(bufLen) char16* buffer, _In_ charcount_t bufLen, StringCopyInfoStack &nestedStringTreeCopyInfos, const byte recursionDepth);
    };

    typedef RemoteConcatStringWrapping<_u('['), _u(']')> RemoteConcatStringWrappingSB;
    typedef RemoteConcatStringWrapping<_u('{'), _u('}')> RemoteConcatStringWrappingB;
    typedef RemoteConcatStringWrapping<_u('"'), _u('"')> RemoteConcatStringWrappingQ;

    class RemoteConcatStringMulti:
        public RemoteConcatStringBase<ConcatStringMulti, RemoteConcatStringMulti>
    {
    private:
        // ConcatStringMulti has m_slots[] allocated through RecyclerNewPlus. Its memory is not contained within ConcatStringMulti struct. Needs to read it separately.
        RemoteArray<JavascriptString*> m_items;

    public:
        RemoteConcatStringMulti(InspectionContext* context, const TargetType* str);

        int GetItemCount() const { return ToTargetPtr()->slotCount; }
        JavascriptString *GetItem(const int index) const { Assert(index >= 0 && index < GetItemCount()); return m_items[index]; }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Template member definitions

    template<class T, class DAC>
    void RemoteString<T, DAC>::Read(
        _Out_writes_to_(bufLen, *actual) char16* buffer,
        _In_ charcount_t bufLen,
        _Out_ charcount_t* actual)
    {
        Assert(buffer);
        Assert(actual);

        charcount_t len = pThis()->GetLength();
        DIAG_VERIFY_RUNTIME(len <= bufLen);
        *actual = len;

        if (IsFinalized())
        {
            InspectionContext::ReadStringLen(m_reader, pThis()->GetString(), buffer, len);
            return;
        }

        // Copy everything except nested string trees into the buffer. Collect nested string trees and the buffer locations
        // where they need to be copied, and copy them at the end. This is done to avoid excessive recursion during the copy.
        StringCopyInfoStack nestedStringTreeCopyInfos;
        pThis()->Copy(buffer, len, nestedStringTreeCopyInfos, 0);
        while(!nestedStringTreeCopyInfos.IsEmpty())
        {
            const StringCopyInfo copyInfo(nestedStringTreeCopyInfos.Pop());
            const CharCount sourceStringLength = RemoteJavascriptString(m_context, copyInfo.SourceString()).GetLength();
            Assert(sourceStringLength <= GetLength());
            Assert(copyInfo.DestinationBuffer() >= buffer);
            Assert(copyInfo.DestinationBuffer() <= buffer + (GetLength() - sourceStringLength));
            m_context->GetRemoteStringFactory(copyInfo.SourceString())->Copy(
                m_context,
                copyInfo.SourceString(),
                copyInfo.DestinationBuffer(),
                sourceStringLength,
                nestedStringTreeCopyInfos,
                0);
        }
    }

    template<class T, class DAC>
    void RemoteString<T, DAC>::Copy(
        _Out_writes_(bufLen) char16* buffer,
        _In_ charcount_t bufLen,
        StringCopyInfoStack &nestedStringTreeCopyInfos,
        const byte recursionDepth)
    {
        Assert(IsFinalized()); // derived classes should override (hide) this function to handle unfinalized strings
        DIAG_VERIFY_RUNTIME(pThis()->GetLength() == bufLen);
        InspectionContext::ReadStringLen(m_reader, pThis()->GetString(), buffer, bufLen);
    }

    template<class T, class Sub>
    void RemoteConcatStringBase<T, Sub>::Copy(
        _Out_writes_(bufLen) char16* buffer,
        _In_ charcount_t bufLen,
        StringCopyInfoStack &nestedStringTreeCopyInfos,
        const byte recursionDepth)
    {
        Assert(!IsFinalized());
        Assert(buffer);
        DIAG_VERIFY_RUNTIME(pThis()->GetLength() == bufLen);

        CharCount copiedCharLength = 0;
        const int n = pThis()->GetItemCount();
        for(int i = 0; i < n; ++i)
        {
            RemoteJavascriptString s(m_context, pThis()->GetItem(i));
            if(!s.GetRemoteAddr())
            {
                continue;
            }

            const CharCount copyCharLength = s.GetLength();
            DIAG_VERIFY_RUNTIME(copyCharLength <= bufLen - copiedCharLength);

            IRemoteStringFactory *const remoteStringFactory = m_context->GetRemoteStringFactory(s.GetRemoteAddr());
            if(recursionDepth == Js::JavascriptString::MaxCopyRecursionDepth && remoteStringFactory->IsTree(m_context, s.GetRemoteAddr()))
            {
                // Don't copy nested string trees yet, as that involves a recursive call, and the recursion can become
                // excessive. Just collect the nested string trees and the buffer location where they should be copied, and
                // the caller can deal with those after returning.

                // Suppress 26015 prefast warning: Potential overflow using expression '& buffer[copiedCharLength]' 
                // Buffer access is apparently unbounded by the buffer size.
                // The buffer length is checked in DIAG_VERIFY_RUNTIME
#pragma prefast(suppress:26015)    
                nestedStringTreeCopyInfos.Push(StringCopyInfo(s.GetRemoteAddr(), &buffer[copiedCharLength]));
            }
            else
            {
                Assert(recursionDepth <= Js::JavascriptString::MaxCopyRecursionDepth);
                remoteStringFactory->Copy(
                    m_context,
                    s.GetRemoteAddr(),
                    &buffer[copiedCharLength],
                    copyCharLength,
                    nestedStringTreeCopyInfos,
                    recursionDepth + 1);
            }
            copiedCharLength += copyCharLength;
        }

        DIAG_VERIFY_RUNTIME(copiedCharLength == bufLen);
    }

    class WritableStringBuffer
    {    
    public:
        WritableStringBuffer(_In_count_(length) char16* str, _In_ int length) : m_pszString(str), m_pszCurrentPtr(str), m_length(length) {}

        void Append(char16 c);
        void Append(const char16 * str, charcount_t countNeeded);
        void AppendLarge(const char16 * str, charcount_t countNeeded);
    private:
        char16* m_pszString;
        char16* m_pszCurrentPtr;
        charcount_t m_length;
#if DBG
        charcount_t GetCount() 
        { 
            Assert(m_pszCurrentPtr >= m_pszString);
            Assert(m_pszCurrentPtr - m_pszString <= MaxCharCount);            
            return static_cast<charcount_t>(m_pszCurrentPtr - m_pszString); 
        }
#endif
    };
} // namespace JsDiag.
