//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    TYPE_STATS(CommentTable, L"CommentTable")
 
    void CommentTable::Clear() 
    { 
        entries.Clear(); 
        prevCommentLine = 0;
    }

    void CommentTable::AddComment(OLECHAR firstChar, OLECHAR secondChar, bool containTypeDef, charcount_t min, charcount_t lim, bool adjacent, bool multiline, charcount_t startLine, charcount_t endLine)
    {
        /********************************************************************************************************
         * Note: For single line comments, the secondChar is always '\0' because it is not read (as it is unused)
         * If we reach the end of the buffer, secondChar will also be '\0';
         ********************************************************************************************************/

        // The entries (comment-table) is a sorted list (since comments scanning has happened in the lexical order).
        // But due to deferred parsing, it is possible that the same comments are added again in the list which will violate the sorting-assumption-of-the-entires.
        // There is no need to add that comment again.

        int lastIndex = entries.Count();
        if (lastIndex > 0 && entries.Item(lastIndex - 1).ichLim > min)
        {
#if DBG
            // Add some validation over here, that this comment was already included.
            int existingIndex = ListHelpers::BinarySearch<CommentTableEntry>(entries, [&](CommentTableEntry& entry) -> int
            {
                if (entry.ichMin < min)
                {
                    return -1;
                }
                if (entry.ichMin > lim)
                {
                    return 1;
                }
                return 0;
            });

            Assert(existingIndex >= 0 && entries.Item(existingIndex).ichLim == lim);
#endif
            return;
        }

        //
        // Determine the comment type ( commenttypeSingleLine | commenttypeBlock | commenttypeVSDoc ).
        //
        CommentType commentType = commenttypeSingleLine;
        if(multiline)
        {
            // commenttypeJSDoc is a special case of multi line comment when firstChar is '*'.
            if (firstChar == '*' && secondChar != '*')
            {
                commentType = commenttypeJSDoc;
            }
            else
            {
                commentType = commenttypeBlock;
            }
        }
        else
        {
            // commenttypeVSDoc is a special case of single line comment when firstChar is '/'.
            if(firstChar == '/')
            {
                commentType = commenttypeVSDoc;
            }
        }

        //
        //  Reset adjacent flag when the comment is not on the next line or commentType of the previous entry is not the same.
        //
        if(adjacent) 
        {
            //  Don't do this for VS Doc Comments for backward compatibility reasons.
            if(commentType != commenttypeVSDoc && startLine != prevCommentLine + 1)
            {
                // Not on the next line
                adjacent = false;
            } 
            else if(entries.Count() > 0)
            {
                auto prevEntry = entries.Item(entries.Count() - 1);
                if(!prevEntry.MatchesType(commentType)) 
                {
                    // Different comment type
                    adjacent = false;
                }
            }
        }

        if (containTypeDef)
        {
            int typeDefIndex = entries.Count();
            if (commentType == commenttypeJSDoc)
            {
                // we should never merge typedef JSDoc with other comments for parsing
                this->typeDefIndices.Add(typeDefIndex);
            }
            else if (commentType == commenttypeVSDoc)
            {
                AssertMsg(false, "This is not implemented yet.");
                // And when that comes, we need to make sure we go back to the first non adjacent comment 
                // and add that to the list
            }
        }

        CommentTableEntry entry(min, lim, commentType, adjacent);
        entries.Add(entry);
        prevCommentLine = endLine;
    }

    int CommentTable::AnyCommentInRange(charcount_t min, charcount_t lim)
    {
        // Assuming comment table is sorted.
        return ListHelpers::BinarySearch<CommentTableEntry>(entries, [&] (CommentTableEntry& entry) -> int
        {
            if(entry.ichMin < min)
                return -1;
            if(entry.ichMin > lim)
                return 1;
            // The comment is fully or partially in the min..max range
            return 0;
        });
    }

    // Finds a comment in a specified range using TFindSpecific algorithm. Ensures that the comment matches the specified type.
    template <typename TFindSpecific>
    int CommentTable::FindCommentInRange(charcount_t min, charcount_t lim, CommentType commentType, TFindSpecific FindSpecific)
    {
        int result      = -1;
        auto startIndex = AnyCommentInRange(min, lim);
        if(startIndex >= 0)
        {
            result = FindSpecific(startIndex);
            if(result >= 0 && commentType != commenttypeAny && !entries.Item(result).MatchesType(commentType))
            {
                // The entry found doesn't match the desired type
                result = -1;
            }
        }

        return result;
    }

    // Finds the very first comment in the specified range. 
    int CommentTable::FirstCommentInRange(charcount_t min, charcount_t lim, CommentType commentType)
    {
        return FindCommentInRange(min, lim, commentType, [&](int startIndex) -> int 
        {
            int result = -1;
            // Scan upward to find the very first comment in the range.
            CommentTableEntry entry;
            for(int i = startIndex; i >= 0; i--)
            {
                auto entry = entries.Item(i);
                // Check we're not out of the range
                if (entry.ichMin < min)
                    break;

                // Keep the entry index
                result = i;
            }
            return result;
        });
    }

    // Finds the very last comment in the specified range. 
    int CommentTable::LastCommentInRange(charcount_t min, charcount_t lim, CommentType commentType)
    {
        return FindCommentInRange(min, lim, commentType, [&](int startIndex) -> int 
        {
            int result = -1;
            // Scan downwards, take the last entry for which the conditions are still met.
            CommentTableEntry entry;
            for(int i = startIndex; i < Count(); i++)
            {
                entry = entries.Item(i);
                // Check we're not out of the range
                if (entry.ichLim > lim)
                    break;

                // Keep the entry index
                result = i;
            }
            return result;
        });
    }

    bool CommentTable::OffsetInComment(charcount_t offset)
    {
        // Assuming comment table is sorted.
        auto entryIndex = ListHelpers::BinarySearch<CommentTableEntry>(entries, [&] (CommentTableEntry& entry) -> int
        {
            if(entry.ichMin > offset)
                return 1;

            if(entry.ichLim < offset)
                return -1;

            return 0;
        });

        // make sure we found the correct element
        Assert(entryIndex < 0 ||  (entries.Item(entryIndex).ichMin <= offset && entries.Item(entryIndex).ichLim >= offset));

        if (entryIndex >= 0)
        {
            auto entry = entries.Item(entryIndex);
            // entry min & lim may be the same in case of a multiline comment which is missing a closing tag.
            if(entry.ichLim > entry.ichMin)
            {
                // For a valid comment, do not consider the min to be in a comment. 
                // Also do not consider lim for multiline commment to be in a comment.
                // lim is also not considered for jsdoc comments for backward compatibility
                if ((entry.ichMin == offset) || (entry.ichLim == offset && (entry.flags.commentType == commenttypeBlock || entry.flags.commentType == commenttypeJSDoc)))
                {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    HRESULT CommentTable::Add(void *data, OLECHAR firstChar, OLECHAR secondChar, bool containTypeDef, charcount_t min, charcount_t lim, bool adjacent, bool multiline, charcount_t startLine, charcount_t endLine)
    {
        METHOD_PREFIX;

        Assert(data != nullptr);
        reinterpret_cast<CommentTable *>(data)->AddComment(firstChar, secondChar, containTypeDef, min, lim, adjacent, multiline, startLine, endLine);

        METHOD_POSTFIX;
    }

    void CommentTable::Associate(int location, int commentTableEntryIndex)
    {
        int* temp;
        if (this->locationToMaxAssociatedCommentEntryMap.TryGetReference(location, &temp))
        {
            if (commentTableEntryIndex > *temp)
            {
                *temp = commentTableEntryIndex;
            }
        }
        else
        {
            this->locationToMaxAssociatedCommentEntryMap.Add(location, commentTableEntryIndex);
        }

        if (this->commentEntryToMinAssociatedLocationMap.TryGetReference(commentTableEntryIndex, &temp))
        {
            if (location < *temp)
            {
                *temp = location;
            }
        }
        else
        {
            this->commentEntryToMinAssociatedLocationMap.Add(commentTableEntryIndex, location);
        }
    }

    bool CommentTable::AssociateCommentBefore(charcount_t ichMin, charcount_t ichLim, CommentTableAssociator& handler)
    {
        bool result = false;
        lastRememberedComment = -1;
        // Find the last comment in range, the one that directly precedes the target node.
        // For example:
        // var o = { 
        //      /* undesired comment */
        //      /// desired comment
        //      /// desired comment <==== WE'RE LOOKING FOR THIS COMMENT  
        //      field: 0   
        //      ^==== TARGET NODE
        // };     
        int index = LastCommentInRange(ichMin, ichLim, commenttypeAny);

        if (index >= 0)
        {
            //
            // Now go up and associate all adjacent comments of the same type with the location
            //
            CommentType commentType = commenttypeAny;
            for (int current = index; current >= 0; current--)
            {
                CommentTableEntry entry = GetEntry(current);
                if (current == index)
                {
                    // The very last entry - keep the commentType. 
                    commentType = entry.flags.commentType;
                }
                else
                {
                    if (entry.flags.commentType != commentType || !entry.InRange(ichMin, ichLim))
                    {
                        // Stop if commentType doesn't match or out of range 
                        break;
                    }
                }

                handler.run(this, current);

                result = true;

                entries.Item(current, entry);

                if (!entry.flags.adjacent)
                {
                    // The very first entry in a series has adjacent = false.
                    // var o = { 
                    //      /// line 1 [adjacent=false] <=| STOP HERE  
                    //      /// line 2 [adjacent=true]    |
                    //      /// line 3 [adjacent=true]    |  
                    //      field: 0   
                    lastRememberedComment = current;
                    lastRememberedCommentType = commentType;
                    break;
                }
            }
        }

        return result;
    }

    CommentBuffer* CommentTable::GetLastAssociatedComment(ArenaAllocator* alloc, AuthoringFileText *text, CommentType commentType)
    {
        if (lastRememberedComment >= 0 && lastRememberedComment < Count())
        {
            Assert(lastRememberedCommentType);
            if(CommentTypeMatches(lastRememberedCommentType, commentType))
            {
                int dummyNextIndex;
                return GetAdjacentComments(alloc, text, lastRememberedComment, &dummyNextIndex);
            }
        }
        
        return CommentBuffer::New(alloc);
    }

    CommentBuffer* CommentTable::GetCommentsBefore(ArenaAllocator* alloc, AuthoringFileText *text, charcount_t ichLocation, CommentType commentType)
    {
        Assert(alloc);

        // If there is no text in the m_text, there are no comments either.
        if (text != nullptr && !text->Empty())
        {
            int maxAssociatedCommentTableIndex = -1;
            int minAssociatedLocation = -1;
            if (this->locationToMaxAssociatedCommentEntryMap.ContainsKey(ichLocation))
            {
                maxAssociatedCommentTableIndex = this->locationToMaxAssociatedCommentEntryMap.Item(ichLocation);
                Assert(this->commentEntryToMinAssociatedLocationMap.ContainsKey(maxAssociatedCommentTableIndex));
                minAssociatedLocation = commentEntryToMinAssociatedLocationMap.Item(maxAssociatedCommentTableIndex);
            }

            // Notice the fall through behavior - if the mapping is not found - maxAssociatedCommentTableIndex will keep the old value -1, 
            // and therefore, the loop and the conditional will be skipped and exit quickly.

            int index = -1;
            CommentType lastEntryType = commenttypeAny;

            // Scan upwards since we need the entries directly preceding the location.
            // Initially find the last entry and then continue to scan upwards to find the first one of the same comment type.
            for (int current = maxAssociatedCommentTableIndex; current >= 0; current--)
            {
                CommentTableEntry entry = GetEntry(current);
                {
                    if(index < 0)
                    {
                        // This is the very last entry associated with the location
                        if(!entry.MatchesType(commentType))
                        {
                            // Found the last entry associated with the location, but it doesn't match the commentType.
                            // For example, when commentType = commenttypeVSDoc:
                            // /* undesired comment */   <==== COMMENT TYPE IS NOT commenttypeVSDoc  
                            // var x; 
                            break;
                        }
                        if(!entry.MatchesType(commenttypeAnyDoc))
                        {
                            // Do not return simple comments if there is one or more empty lines after it.
                            auto newLinesCount = text->GetNewLineCountInRange(entry.ichLim, minAssociatedLocation);
                            if(newLinesCount > 1) break;
                        }

                        // Remember entry index & type. 
                        index         = current;
                        lastEntryType = entry.flags.commentType;
                    }

                    if(index >= 0)
                    {
                        if (entry.flags.commentType == CommentType::commenttypeJSDoc || entry.flags.commentType != lastEntryType)
                        {
                            // Same associated location, but different type. Stop.
                            // For example:
                            //      /* undesired comment */   <==== STOP
                            //      /// desired comment
                            //      var x; 
                            // Do NOT associate JSDoc comments
                            // For example:
                            //      /** undesired comment */   <==== STOP (even they have same comment type)
                            //      /** desired comment */
                            //      var x; 
                            break;
                        }

                        index = current;

                        if(!entry.flags.adjacent)
                        {
                            // The first entry. Stop.
                            break;
                        }
                    }
                }
            }

            if(index >= 0)
            {
                int dummyNextIndex;
                CommentBuffer* result = GetAdjacentComments(alloc, text, index, &dummyNextIndex);
                return result;
            }
        }

        return CommentBuffer::New(alloc);
    }

    CommentBuffer* CommentTable::GetFirstCommentBetween(ArenaAllocator* alloc, AuthoringFileText *text, charcount_t location, charcount_t lim, CommentType commentType)
    {
        Assert(alloc != nullptr);

        // If there is no text in the m_text, there are no comments either.
        if (text == nullptr || text->Empty())
        {
            return CommentBuffer::New(alloc);
        }

        int index = this->FirstCommentInRange(location, lim, commentType);
        int dummyNextIndex;

        return this->GetAdjacentComments(alloc, text, index, &dummyNextIndex);
    }

    CommentBuffer *CommentTable::GetAdjacentComments(ArenaAllocator* alloc, AuthoringFileText *text, int currentIndex, /* out */ int* pNextIndex) const
    {
        Assert(pNextIndex != nullptr);

        int min, lim;
        return this->GetAdjacentComments(alloc, text, currentIndex, GetAdjacentCommentsOperation::GetAdjacentCommentsOperation_GetBuffer, 
            /* vsDocCorrection = */true, /* typeDefOnly = */false, /* groupAdjacentComments = */true, /*lastIndex = */-1, &min, &lim, pNextIndex);
    }

    CommentBuffer *CommentTable::GetAdjacentComments(ArenaAllocator* alloc, AuthoringFileText *text, int currentIndex, CommentTable::GetAdjacentCommentsOperation op, 
        bool vsDocCorrection, bool typeDefOnly, bool groupAdjacentComments, int lastIndex, /* out */ int* pMin, /* out */ int* pLim, /* out */ int* pNextIndex) const
    {
        Assert(alloc != nullptr);
        Assert(pMin != nullptr);
        Assert(pLim != nullptr);
        Assert(pNextIndex != nullptr);

        CommentBuffer* buffer = nullptr;

        if (op == CommentTable::GetAdjacentCommentsOperation::GetAdjacentCommentsOperation_GetBuffer)
        {
            buffer = CommentBuffer::New(alloc);
            // If there is no text in the m_text, there are no comments either.
            if (currentIndex < 0 || text == nullptr || text->Empty())
            {
                return buffer;
            }
        }

        int typeDefIndex = currentIndex;
        if (typeDefOnly)
        {
            // For typeDefOnly case - the passed in currentIndex means the index to the typeDefIndices table.
            currentIndex = this->typeDefIndices.Item(typeDefIndex);
        }

        int current;
        for (current = currentIndex; current < Count(); current++)
        {
            CommentTableEntry entry = GetEntry(current);

            if (typeDefOnly)
            {
                groupAdjacentComments = (entry.flags.commentType == commenttypeVSDoc);
            }

            if (current == currentIndex)
            {
                *pMin = entry.ichMin;
            }

            if (lastIndex != -1)
            {
                if (current > lastIndex)
                {
                    break;
                }
            }
            else if (groupAdjacentComments)
            {
                if (current != currentIndex && !entry.flags.adjacent)
                {
                    break; // Stop if we run into a non-adjacent entry
                }
            }
            else
            {
                if (current != currentIndex)
                {
                    break;
                }
            }

            *pLim = entry.ichLim;

            if (op == CommentTable::GetAdjacentCommentsOperation::GetAdjacentCommentsOperation_GetBuffer)
            {
                buffer->SetCommentType(entry.flags.commentType);
                if (entry.MatchesType(commenttypeJSDoc))
                {
                    buffer->SetCommentType(commenttypeJSDoc);
                }
                if (entry.MatchesType(commenttypeVSDoc))
                {
                    buffer->SetCommentType(commenttypeVSDoc);
                }

                LPCUTF8 commentSrc = text->Buffer(entry.ichMin);
                charcount_t len = entry.ichLim - entry.ichMin;
                if (commentSrc[0] == '/' && (commentSrc[1] == '*' || commentSrc[1] == '/'))
                {
                    if (commentSrc[1] == '*') // Is this a /* comment?
                    {
                        AssertMsg(len >= 2, "We already know it is a // or /* comment, so len >= 2, the memory access will be within bounds.");
                        if (len > 3 && commentSrc[len - 2] == '*' && commentSrc[len - 1] == '/') /* Tricky note: We need to also make sure the comment is not slash star slash - that's why the length check */
                        {
                            len -= 2; // Ignore the trailing */ of the comment.
                        }
                    }
                    else if (vsDocCorrection && commentSrc[2] == '/')
                    {
                        // Ignore leading '/' in /// comments.
                        len--;
                        commentSrc++;
                    }

                    // Ignore the leading /* or //
                    len -= 2;
                    commentSrc += 2;
                }

                buffer->AddUtf8(commentSrc, len);
                buffer->Add(L"\r\n");
            }
        }

        if (typeDefOnly)
        {
            *pNextIndex = typeDefIndex + 1;
        }
        else
        {
            *pNextIndex = current;
        }

        return buffer;
    }

    CommentTableIterator* CommentTable::GetIterator(bool groupAdjacentComments)
    {
        return Anew(&this->alloc, CommentTableIterator, &this->alloc, this, groupAdjacentComments, /* typeDefOnly = */false);
    }

    CommentTableIterator* CommentTable::GetTypeDefIterator()
    {
        return Anew(&this->alloc, CommentTableIterator, &this->alloc, this, /* groupAdjacentComments = */false, /* typeDefOnly = */true);
    }

    CommentTableIterator::CommentTableIterator(ArenaAllocator* allocator, const CommentTable* commentTable, bool groupAdjacentComments, bool typeDefOnly) : m_allocator(allocator), m_commentTable(commentTable), m_groupAdjacentComments(groupAdjacentComments), m_typeDefOnly(typeDefOnly), m_index(0), m_nextIndex(-1)
    {
    }

    CommentBuffer* CommentTableIterator::GetCurrentComment(AuthoringFileText* text, /* out */ int* pMin, /* out */ int* pLim)
    {
        Assert(pMin != nullptr);
        Assert(pLim != nullptr);
        Assert(m_nextIndex == -1);
        return this->m_commentTable->GetAdjacentComments(this->m_allocator, text, this->m_index, CommentTable::GetAdjacentCommentsOperation::GetAdjacentCommentsOperation_GetBuffer, /* vsDocCorrection = */ false, /* typeDefOnly = */m_typeDefOnly, m_groupAdjacentComments, /* lastIndex = */-1, pMin, pLim, &this->m_nextIndex);
    }

    void CommentTableIterator::GetCurrentCommentSpan(/* out */ int* pMin, /* out */ int* pLim)
    {
        Assert(pMin != nullptr);
        Assert(pLim != nullptr);
        Assert(m_nextIndex == -1);
        this->m_commentTable->GetAdjacentComments(this->m_allocator, nullptr, this->m_index, CommentTable::GetAdjacentCommentsOperation::GetAdjacentCommentsOperation_GetSpan, /* vsDocCorrection = */ false, /* typeDefOnly = */m_typeDefOnly, m_groupAdjacentComments, /* lastIndex = */-1, pMin, pLim, &this->m_nextIndex);
    }
    
    bool CommentTableIterator::HasNext()
    {
        if (this->m_typeDefOnly)
        {
            return this->m_index < this->m_commentTable->typeDefIndices.Count();
        }
        else 
        {
            return this->m_index < this->m_commentTable->entries.Count();
        }
    }

    void CommentTableIterator::MoveNext()
    {
        Assert(m_nextIndex != -1);
        this->m_index = this->m_nextIndex;
        this->m_nextIndex = -1;
    }

#if ENABLE_DEBUG_CONFIG_OPTIONS

    void CommentTable::DumpCommentTable(AuthoringFileText* text)
    {   
        for (CommentTableIterator* i = this->GetIterator(/* groupAdjacentComments = */true); i->HasNext(); i->MoveNext())
        {
            int min, lim;
            CommentBuffer* comment = i->GetCurrentComment(text, &min, &lim);
            OUTPUT_TRACE(Js::CommentTablePhase, L"[offset : (%d, %d), commentType : %d] %s",
                            min,
                            lim,
                            (int)comment->GetCommentType(),
                            comment->Sz());
        }
    }
#endif
}