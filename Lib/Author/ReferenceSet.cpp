//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    TYPE_STATS(ReferenceSet, L"ReferenceSet")

    STDMETHODIMP ReferenceSet::get_Identifier(__RPC__deref_out_opt BSTR *result)
    {
        STDMETHOD_PREFIX;

        *result = this->identifier.Copy();

        STDMETHOD_POSTFIX;
    }

    STDMETHODIMP ReferenceSet::get_Count(int *result)
    {
        STDMETHOD_PREFIX;

        *result = this->references.Count();

        STDMETHOD_POSTFIX;
    }

    STDMETHODIMP ReferenceSet::GetItems(int startIndex, int count, AuthorSymbolReference *references)
    {
        STDMETHOD_PREFIX;

        ValidateArg(count == 0 || (startIndex >= 0 && startIndex < this->references.Count()));
        ValidateArg(count == 0 || (count <= this->references.Count() - startIndex));

        for (int i = 0; i < count; i++)
        {
            references[i] = this->references.Item(startIndex + i);
        }

        STDMETHOD_POSTFIX;
    }

}
