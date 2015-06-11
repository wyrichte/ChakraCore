//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    // Implementation of IAuthorReferenceSet
    class ReferenceSet : public SimpleComObjectWithAlloc<IAuthorReferenceSet>
    {
    private:
        CComBSTR identifier;
        ReferenceList references;

    public:
        ReferenceSet(PageAllocator* pageAlloc) : SimpleComObjectWithAlloc<IAuthorReferenceSet>(pageAlloc, L"ls:ReferenceSet"), references(Alloc(), 17) { }

        // IAuthorReferenceSet
        STDMETHOD(get_Identifier)(__RPC__deref_out_opt BSTR *result);
        STDMETHOD(get_Count)(int *result);
        STDMETHOD(GetItems)(int startIndex, int count, AuthorSymbolReference *references);

        ReferenceList *GetReferenceList() { return &references; }
        void SetIdentifer(IdentPtr ident) { identifier = ident->Psz(); }
    };

}
