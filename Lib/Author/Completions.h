//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    class FileAuthoring;
    class AuthoringFileText;
    class ScriptContextPath;

    class Completions : public SimpleComObjectWithAlloc<IAuthorCompletionSet>
    {
    public:
        struct InternalCompletion
        {
            AuthorCompletionKind    kind;
            AuthorCompletionFlags   group;
            Js::PropertyId          id;
            StringSpan         name;
            StringSpan         insertionText;
            StringSpan         displayText;
            HintInfo*               hintInfo;
            Js::RecyclableObject*   parentObject;
            Js::Var                 value;
            StringSpan         glyph;

            HRESULT FetchPropertyValue(FileAuthoring* fileAuthoring);
        };

        typedef JsUtil::List<InternalCompletion *, ArenaAllocator> CompletionList;
    private:
        BVSparse<ArenaAllocator> ids;
        AuthorFileRegion extent;
        CompletionList *results;
        AuthorCompletionSetKind kind;
        AuthorCompletionObjectKind objectKind;
        AuthorCompletionDiagnosticFlags diagFlags;

        Js::PropertyId nonRemovablePropertyId;

        RefCountedPtr<FileAuthoring> _fileAuthoring;

        Completions::InternalCompletion* AddCompletion(Js::RecyclableObject* parentObject, Js::Var value, AuthorCompletionKind kind, AuthorCompletionFlags group, Js::PropertyId id, wchar_t const *name, 
            charcount_t len, LPCWSTR glyph, HintInfo* hintInfo);
        Completions::InternalCompletion* AddUniqueCompletion(Js::RecyclableObject* parentObject, Js::Var value, AuthorCompletionKind kind, AuthorCompletionFlags group, Js::PropertyId id, wchar_t const *name, 
            charcount_t len, LPCWSTR glyph, HintInfo* hintInfo);

        Completions(FileAuthoring *fileAuthoring);
    public:
        static Completions* New(FileAuthoring *fileAuthoring);

        void ExtendLifetime();
        void EngineClosing();

        BVSparse<ArenaAllocator>* Ids() { return &ids; }
        CompletionList* Items() { return results; }
        FileAuthoring* GetFileAuthoring() { return _fileAuthoring; }

        void Clear(bool preserveNonRemovable = false);
        
        void AddUnique(Js::RecyclableObject* parentObject, Js::Var value, AuthorCompletionKind kind, AuthorCompletionFlags group, Js::PropertyId id, LPCWSTR glyph, HintInfo* hintInfo);
        void AddUnique(AuthorCompletionKind kind, AuthorCompletionFlags group, LPCWSTR name, charcount_t len, LPCWSTR glyph, HintInfo* hintInfo);
        void AddUnique(AuthorCompletionKind kind, AuthorCompletionFlags group, IdentPtr pid, LPCWSTR glyph, HintInfo* hintInfo);
        void UpdateFlags(Js::PropertyId id, AuthorCompletionFlags group);
        void SetExtent(long offset, long length);
        void SetKind(AuthorCompletionSetKind kind);
        void SetObjectKind(AuthorCompletionObjectKind objectKind);
        void SetDiagnosticFlags(AuthorCompletionDiagnosticFlags flags);
        InternalCompletion* Find(Js::InternalString* name);
        Js::PropertyId ToId(LPCWSTR name);

        STDMETHOD(get_Count)(int *result);
        STDMETHOD(GetItems)(int startIndex, int count, AuthorCompletion *completions);
        STDMETHOD(GetExtent)(AuthorFileRegion *extent);
        STDMETHOD(GetHintFor)(int index, IAuthorSymbolHelp **result);
        STDMETHOD(get_Kind)(AuthorCompletionSetKind *result);
        STDMETHOD(get_ObjectKind)(AuthorCompletionObjectKind *result);
        STDMETHOD(get_DiagnosticFlags)(AuthorCompletionDiagnosticFlags *result);

    protected:
        virtual void OnDelete() override;
    private:
        void PreventJsObjectsRecycling();
        bool IsNonRemovable(InternalCompletion* item);
    };

}