//
//    Copyright (C) Microsoft.  All rights reserved.
//
namespace Authoring
{
    class TaskCommentSet : public SimpleComObjectWithAlloc<IAuthorTaskCommentSet>
    {
    private:
        typedef JsUtil::List<AuthorFileTaskComment, ArenaAllocator> TaskComments;

        TaskComments m_taskComments;

    public:
        TaskCommentSet(PageAllocator* pageAlloc) : SimpleComObjectWithAlloc<IAuthorTaskCommentSet>(pageAlloc, L"ls: TaskCommentSet"), m_taskComments(Alloc())  { }

        void Add(long offset, long length, AuthorTaskCommentPriority priority);
        STDMETHOD(get_Count)(int *result);
        STDMETHOD(GetItems)(int startIndex, int count, AuthorFileTaskComment *taskComments);
    };

}
