//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

namespace Js {

    inline JavascriptSet* JavascriptSet::New(ScriptContext* scriptContext)
    {
        JavascriptSet* set = scriptContext->GetLibrary()->CreateSet();
        set->set = RecyclerNew(scriptContext->GetRecycler(), SetDataSet, scriptContext->GetRecycler());

        return set;
    }

    inline bool JavascriptSet::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_Set;
    }

    inline JavascriptSet* JavascriptSet::FromVar(Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptSet'");
        
        return static_cast<JavascriptSet *>(RecyclableObject::FromVar(aValue));
    }

    inline JavascriptSet::SetDataList::Iterator JavascriptSet::GetIterator()
    {
        return list.GetIterator();
    }

} // namespace Js