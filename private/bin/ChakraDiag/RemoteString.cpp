//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

namespace JsDiag
{
    void StringCopyInfoStack::Push(const StringCopyInfo copyInfo)
    {
        Add(copyInfo);
    }

    const StringCopyInfo StringCopyInfoStack::Pop()
    {
        Assert(!IsEmpty());

        const size_t lastIndex = GetCount() - 1;
        const Js::StringCopyInfo copyInfo((*this)[lastIndex]);
        RemoveAt(lastIndex);
        return copyInfo;
    }

    RemoteCompoundString::RemoteBlock::RemoteBlock(
        InspectionContext *const context,
        const TargetType *const addr,
        const bool loadBuffer)
        : RemoteData(context->GetReader(), addr),
        buffer(
            addr && loadBuffer && ToTargetPtr()->charLength != 0
                ? VirtualReader::ReadBuffer(
                    context->GetReader(),
                    ToTargetPtr()->bufferOwner + 1,
                    ToTargetPtr()->charLength * sizeof(char16))
                : nullptr)
    {
    }

    void RemoteCompoundString::RemoteBlock::Reinitialize(InspectionContext *const context, const TargetType *const addr)
    {
        this->~RemoteBlock();
        new(this) RemoteBlock(context, addr);
    }

    const void *RemoteCompoundString::RemoteBlock::Buffer() const
    {
        return const_cast<const void *>(static_cast<void *>(buffer));
    }

    const CompoundString::Block *RemoteCompoundString::RemoteBlock::Previous() const
    {
        return ToTargetPtr()->previous;
    }

    const char16 *RemoteCompoundString::RemoteBlock::Chars() const
    {
        return CompoundString::Block::Chars(Buffer());
    }

    CharCount RemoteCompoundString::RemoteBlock::CharLength() const
    {
        return ToTargetPtr()->charLength;
    }

    void *const *RemoteCompoundString::RemoteBlock::Pointers() const
    {
        return CompoundString::Block::Pointers(Buffer());
    }

    CharCount RemoteCompoundString::RemoteBlock::PointerLength() const
    {
        return CompoundString::Block::PointerLengthFromCharLength(CharLength());
    }

    RemoteCompoundString::RemoteCompoundString(InspectionContext *const context, const TargetType *const str)
        : RemoteString(context, str)
    {
    }

    bool RemoteCompoundString::OwnsLastBlock() const
    {
        return ToTargetPtr()->ownsLastBlock;
    }

    bool RemoteCompoundString::HasOnlyDirectChars() const
    {
        return ToTargetPtr()->directCharLength == static_cast<CharCount>(-1);
    }

    const void *RemoteCompoundString::LastBlockBuffer()
    {
        if(!lastBlockBuffer && ToTargetPtr()->lastBlockInfo.charLength != 0)
        {
            lastBlockBuffer =
                VirtualReader::ReadBuffer(
                    m_context->GetReader(),
                    ToTargetPtr()->lastBlockInfo.buffer,
                    ToTargetPtr()->lastBlockInfo.charLength * sizeof(char16));
        }
        return const_cast<const void *>(static_cast<void *>(lastBlockBuffer));
    }

    const char16 *RemoteCompoundString::LastBlockChars()
    {
        return CompoundString::Block::Chars(LastBlockBuffer());
    }

    CharCount RemoteCompoundString::LastBlockCharLength() const
    {
        return ToTargetPtr()->lastBlockInfo.charLength;
    }

    void *const *RemoteCompoundString::LastBlockPointers()
    {
        return CompoundString::Block::Pointers(LastBlockBuffer());
    }

    CharCount RemoteCompoundString::LastBlockPointerLength() const
    {
        return CompoundString::Block::PointerLengthFromCharLength(LastBlockCharLength());
    }

    void RemoteCompoundString::Copy(
        _Out_writes_(bufLen) char16* buffer,
        _In_ charcount_t bufLen,
        StringCopyInfoStack &nestedStringTreeCopyInfos,
        const byte recursionDepth)
    {
        Assert(!IsFinalized());
        Assert(buffer);

        DIAG_VERIFY_RUNTIME(GetLength() == bufLen);
        const CharCount totalCharLength = bufLen;
        switch(totalCharLength)
        {
            case 0:
                return;

            case 1:
                Assert(HasOnlyDirectChars());
                Assert(LastBlockCharLength() == 1);

                buffer[0] = LastBlockChars()[0];
                return;
        }

        // Copy buffers from string pointers
        const bool hasOnlyDirectChars = HasOnlyDirectChars();
        const CharCount directCharLength = hasOnlyDirectChars ? totalCharLength : ToTargetPtr()->directCharLength;
        CharCount remainingCharLengthToCopy = totalCharLength;
        RemoteBlock block(m_context, ToTargetPtr()->lastBlock, false);
        void *const *blockPointers = LastBlockPointers();
        CharCount pointerIndex = LastBlockPointerLength();
        while(remainingCharLengthToCopy > directCharLength)
        {
            while(pointerIndex == 0)
            {
                Assert(block.GetRemoteAddr());
                block.Reinitialize(m_context, block.Previous());
                Assert(block.GetRemoteAddr());
                blockPointers = block.Pointers();
                pointerIndex = block.PointerLength();
            }

            void *const pointer = blockPointers[--pointerIndex];
            if(CompoundString::IsPackedInfo(pointer))
            {
                Assert(pointerIndex != 0);
                void *pointer2 = blockPointers[--pointerIndex];
                JavascriptString *remoteStringAddress;
    #ifdef _M_X64
                Assert(!CompoundString::IsPackedInfo(pointer2));
    #else
                if(CompoundString::IsPackedInfo(pointer2))
                {
                    Assert(pointerIndex != 0);
                    remoteStringAddress = static_cast<JavascriptString *>(blockPointers[--pointerIndex]);
                }
                else
    #endif
                {
                    remoteStringAddress = static_cast<JavascriptString *>(pointer2);
                    pointer2 = nullptr;
                }

                CharCount startIndex, copyCharLength;
                CompoundString::UnpackSubstringInfo(pointer, pointer2, &startIndex, &copyCharLength);

                RemoteJavascriptString s(m_context, remoteStringAddress);
                DIAG_VERIFY_RUNTIME(startIndex <= s.GetLength());
                DIAG_VERIFY_RUNTIME(copyCharLength <= s.GetLength() - startIndex);

                DIAG_VERIFY_RUNTIME(remainingCharLengthToCopy >= copyCharLength);
                remainingCharLengthToCopy -= copyCharLength;
                CString copyBuffer = m_context->ReadString(s.GetRemoteAddr());
                Assert(static_cast<CharCount>(copyBuffer.GetLength()) == s.GetLength());
                js_wmemcpy_s(
                    &buffer[remainingCharLengthToCopy],
                    copyCharLength,
                    &static_cast<LPCWSTR>(copyBuffer)[startIndex],
                    copyCharLength);
            }
            else
            {
                RemoteJavascriptString s(m_context, static_cast<JavascriptString *>(pointer));
                const CharCount copyCharLength = s.GetLength();

                DIAG_VERIFY_RUNTIME(remainingCharLengthToCopy >= copyCharLength);
                remainingCharLengthToCopy -= copyCharLength;
                IRemoteStringFactory *const remoteStringFactory = m_context->GetRemoteStringFactory(s.GetRemoteAddr());
                if(recursionDepth == Js::JavascriptString::MaxCopyRecursionDepth && remoteStringFactory->IsTree(m_context, s.GetRemoteAddr()))
                {
                    // Don't copy nested string trees yet, as that involves a recursive call, and the recursion can become
                    // excessive. Just collect the nested string trees and the buffer location where they should be copied, and
                    // the caller can deal with those after returning.
                    nestedStringTreeCopyInfos.Push(StringCopyInfo(s.GetRemoteAddr(), &buffer[remainingCharLengthToCopy]));
                }
                else
                {
                    Assert(recursionDepth <= Js::JavascriptString::MaxCopyRecursionDepth);
                    remoteStringFactory->Copy(
                        m_context,
                        s.GetRemoteAddr(),
                        &buffer[remainingCharLengthToCopy],
                        copyCharLength,
                        nestedStringTreeCopyInfos,
                        recursionDepth + 1);
                }
            }
        }

        Assert(remainingCharLengthToCopy == directCharLength);
        if(remainingCharLengthToCopy != 0)
        {
            // Determine the number of direct chars in the current block
            CharCount blockCharLength;
            if(pointerIndex == 0)
            {
                // The string switched to pointer mode at the beginning of the current block, or the string never switched to
                // pointer mode and the last block is empty. In either case, direct chars span to the end of the previous block.
                Assert(block.GetRemoteAddr());
                block.Reinitialize(m_context, block.Previous());
                Assert(block.GetRemoteAddr());
                blockCharLength = block.CharLength();
            }
            else if(hasOnlyDirectChars)
            {
                // The string never switched to pointer mode, so the current block's char length is where direct chars end
                blockCharLength =
                    block.GetRemoteAddr() == ToTargetPtr()->lastBlock ? LastBlockCharLength() : block.CharLength();
            }
            else
            {
                // The string switched to pointer mode somewhere in the middle of the current block. To determine where direct
                // chars end in this block, all previous blocks are scanned and their char lengths discounted.
                blockCharLength = remainingCharLengthToCopy;
                if(block.GetRemoteAddr())
                {
                    for(RemoteBlock previousBlock(m_context, block.Previous());
                        previousBlock.GetRemoteAddr();
                        previousBlock.Reinitialize(m_context, previousBlock.Previous()))
                    {
                        Assert(blockCharLength >= previousBlock.CharLength());
                        blockCharLength -= previousBlock.CharLength();
                    }
                }
                Assert(CompoundString::Block::PointerLengthFromCharLength(blockCharLength) == pointerIndex);
            }

            // Copy direct chars
            const char16 *blockChars = block.GetRemoteAddr() == ToTargetPtr()->lastBlock ? LastBlockChars() : block.Chars();
            while(true)
            {
                if(blockCharLength != 0)
                {
                    DIAG_VERIFY_RUNTIME(remainingCharLengthToCopy >= blockCharLength);
                    remainingCharLengthToCopy -= blockCharLength;
                    js_wmemcpy_s(&buffer[remainingCharLengthToCopy], blockCharLength, blockChars, blockCharLength);
                    if(remainingCharLengthToCopy == 0)
                        break;
                }

                Assert(block.GetRemoteAddr());
                block.Reinitialize(m_context, block.Previous());
                Assert(block.GetRemoteAddr());
                blockChars = block.Chars();
                blockCharLength = block.CharLength();
            }
        }

#if DBG
        // Verify that all nonempty blocks have been visited
        if(block.GetRemoteAddr())
        {
            while(true)
            {
                block.Reinitialize(m_context, block.Previous());
                if(!block.GetRemoteAddr())
                    break;
                Assert(block.CharLength() == 0);
            }
        }
#endif

        Assert(remainingCharLengthToCopy == 0);
    }

    bool RemoteCompoundString::IsTree() const
    {
        Assert(!IsFinalized());

        return !HasOnlyDirectChars();
    }

    void RemoteConcatStringBuilder::Copy(
        _Out_writes_(bufLen) char16* buffer,
        _In_ charcount_t bufLen,
        StringCopyInfoStack &nestedStringTreeCopyInfos,
        const byte recursionDepth)
    {
        Assert(!this->IsFinalized());
        Assert(buffer);

        DIAG_VERIFY_RUNTIME(GetLength() == bufLen);
        CharCount remainingCharLengthToCopy = bufLen;
        const ConcatStringBuilder *remoteCurrent = GetRemoteAddr();
        while(remoteCurrent)
        {
            RemoteData<ConcatStringBuilder> current(m_reader, remoteCurrent);
            RemoteArray<JavascriptString *> slots(m_reader, current->m_slots);
            for(int i = current->m_count - 1; i >= 0; --i)
            {
                RemoteJavascriptString s(m_context, slots[i]);
                if(!s.GetRemoteAddr())
                {
                    continue;
                }

                const CharCount copyCharLength = s.GetLength();
                DIAG_VERIFY_RUNTIME(remainingCharLengthToCopy >= copyCharLength);
                remainingCharLengthToCopy -= copyCharLength;
                IRemoteStringFactory *const remoteStringFactory = m_context->GetRemoteStringFactory(s.GetRemoteAddr());
                if(recursionDepth == Js::JavascriptString::MaxCopyRecursionDepth && remoteStringFactory->IsTree(m_context, s.GetRemoteAddr()))
                {
                    // Don't copy nested string trees yet, as that involves a recursive call, and the recursion can become
                    // excessive. Just collect the nested string trees and the buffer location where they should be copied, and
                    // the caller can deal with those after returning.
                    nestedStringTreeCopyInfos.Push(StringCopyInfo(s.GetRemoteAddr(), &buffer[remainingCharLengthToCopy]));
                }
                else
                {
                    Assert(recursionDepth <= Js::JavascriptString::MaxCopyRecursionDepth);
                    remoteStringFactory->Copy(
                        m_context,
                        s.GetRemoteAddr(),
                        &buffer[remainingCharLengthToCopy],
                        copyCharLength,
                        nestedStringTreeCopyInfos,
                        recursionDepth + 1);
                }
            }

            remoteCurrent = current->m_prevChunk;
        }
    }

    template<char16 L, char16 R>
    void RemoteConcatStringWrapping<L, R>::Copy(
        _Out_writes_(bufLen) char16* buffer,
        _In_ charcount_t bufLen,
        StringCopyInfoStack &nestedStringTreeCopyInfos,
        const byte recursionDepth)
    {
        Assert(!this->IsFinalized());
        Assert(buffer);

        DIAG_VERIFY_RUNTIME(GetLength() == bufLen);
        buffer[0] = L;
        buffer[bufLen - 1] = R;

        RemoteJavascriptString s(m_context, ToTargetPtr()->m_inner);
        if(!s.GetRemoteAddr())
        {
            return;
        }

        const CharCount copyCharLength = s.GetLength();
        DIAG_VERIFY_RUNTIME(copyCharLength == bufLen - 2);
        IRemoteStringFactory *const remoteStringFactory = m_context->GetRemoteStringFactory(s.GetRemoteAddr());
        if(recursionDepth == Js::JavascriptString::MaxCopyRecursionDepth && remoteStringFactory->IsTree(m_context, s.GetRemoteAddr()))
        {
            // Don't copy nested string trees yet, as that involves a recursive call, and the recursion can become
            // excessive. Just collect the nested string trees and the buffer location where they should be copied, and
            // the caller can deal with those after returning.
            nestedStringTreeCopyInfos.Push(StringCopyInfo(s.GetRemoteAddr(), &buffer[1]));
        }
        else
        {
            Assert(recursionDepth <= Js::JavascriptString::MaxCopyRecursionDepth);
            remoteStringFactory->Copy(
                m_context,
                s.GetRemoteAddr(),
                &buffer[1],
                copyCharLength,
                nestedStringTreeCopyInfos,
                recursionDepth + 1);
        }

        Assert(buffer[GetLength() - 1] == R);
    }

    RemoteConcatStringMulti::RemoteConcatStringMulti(InspectionContext* context, const TargetType* str) :
        RemoteConcatStringBase(context, str),
        m_items(context->GetReader(), str->m_slots)
    {
        // Make sure that m_slots is declared as 0-size array in the end of the type (which doesn't add to the sizeof of the type).
        CompileAssert(offsetof(ConcatStringMulti, m_slots) == sizeof(ConcatStringMulti));
    }

    void RemoteJSONString::Copy(_Out_writes_(bufLen) char16* buffer, _In_ charcount_t bufLen, StringCopyInfoStack &nestedStringTreeCopyInfos, const byte recursionDepth)
    {
        Assert(!this->IsFinalized());
        RemoteJavascriptString originalString(this->m_context, this->GetOriginalString());
        CString originalStringBuffer = m_context->ReadString(originalString.GetRemoteAddr());
       
        charcount_t start = this->ToTargetPtr()->m_start;

        WritableStringBuffer stringBuffer(buffer, bufLen);
        const wchar* sz = originalStringBuffer.GetBuffer();
        const wchar* endSz = sz + originalStringBuffer.GetLength();
        const wchar* startSz =  sz + start;
        const wchar* lastFlushSz = startSz;
        
        stringBuffer.Append(L'\"');
        if(start != 0)
        {
            stringBuffer.AppendLarge(sz, start);
        }
        for (const wchar* current = startSz; current < endSz; current++ )
        {
            WCHAR wch = *current;
            WCHAR specialChar;
            if(wch < _countof(Js::JSONString::escapeMap))
            {
                specialChar = Js::JSONString::escapeMap[(char)wch];
            }
            else
            {
                specialChar = '\0';
            }

            if (specialChar != '\0')
            {
               
                stringBuffer.AppendLarge(lastFlushSz, (charcount_t)(current - lastFlushSz));
                lastFlushSz = current + 1;
                stringBuffer.Append(L'\\');
                stringBuffer.Append(specialChar);
                if(specialChar == _u('u'))
                {
                    char16 bf[5];
                    _ltow_s(wch, bf, _countof(bf), 16);
                    size_t count = wcslen(bf);
                    if (count < 4)
                    {
                        if(count == 1)
                        {
                            stringBuffer.Append(_u("000"), 3);
                        }
                        else if(count == 2)
                        {
                            stringBuffer.Append(_u("00"), 2);
                        }
                        else
                        {
                            stringBuffer.Append(_u("0"), 1);
                        }
                    }
                    stringBuffer.Append(bf, count);
                }
            }   
        }
        if(lastFlushSz < endSz)
        {
            stringBuffer.AppendLarge(lastFlushSz, (charcount_t)(endSz - lastFlushSz));
        }
        stringBuffer.Append(L'\"');
    }

    template class RemoteConcatStringWrapping<_u('['), _u(']')>;
    template class RemoteConcatStringWrapping<_u('{'), _u('}')>;
    template class RemoteConcatStringWrapping<_u('"'), _u('"')>;

    void WritableStringBuffer::Append(const char16 * str, charcount_t countNeeded)
    {
        this->AppendLarge(str, countNeeded);
    }

    void WritableStringBuffer::Append(char16 c)
    {   
        *m_pszCurrentPtr = c;
        this->m_pszCurrentPtr++;
        Assert(this->GetCount() <= m_length);
    }
    void WritableStringBuffer::AppendLarge(const char16 * str, charcount_t countNeeded)
    {
        js_memcpy_s(m_pszCurrentPtr, sizeof(WCHAR) * countNeeded, str, sizeof(WCHAR) * countNeeded);
        this->m_pszCurrentPtr += countNeeded;
        Assert(this->GetCount() <= m_length);
    }

} // namespace JsDiag.
#define IsJsDiag
// Include the defintion of the escapeMap array
#include "JSONString.cpp"
#undef IsJsDiag

