//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    TYPE_STATS(TaskCommentSet, L"TaskCommentSet")

    void TaskCommentSet::Add(long offset, long length, AuthorTaskCommentPriority priority)
    {
        AuthorFileTaskComment taskComment;
        taskComment.offset = offset;
        taskComment.length = length;
        taskComment.priority = priority;
        m_taskComments.Add(taskComment);
    }

    STDMETHODIMP TaskCommentSet::get_Count(int *result)
    {
        if (result) *result = m_taskComments.Count();
        return S_OK;
    }

    STDMETHODIMP TaskCommentSet::GetItems(int startIndex, int count, AuthorFileTaskComment *taskComments)
    {
        STDMETHOD_PREFIX;

        ValidateArg(startIndex + count <= m_taskComments.Count());

        for (int i = 0; i < count; i++)
        {
            int sourceIndex = startIndex + i;
            taskComments[i] = m_taskComments.Item(sourceIndex);
        }

        STDMETHOD_POSTFIX;
    }
}
