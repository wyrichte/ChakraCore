//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

namespace Js {

    inline JavascriptMap* JavascriptMap::New(ScriptContext* scriptContext)
    {
        JavascriptMap* map = scriptContext->GetLibrary()->CreateMap();
        map->map = RecyclerNew(scriptContext->GetRecycler(), MapDataMap, scriptContext->GetRecycler());

        return map;
    }

    inline bool JavascriptMap::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_Map;
    }

    inline JavascriptMap* JavascriptMap::FromVar(Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptMap'");

        return static_cast<JavascriptMap *>(RecyclableObject::FromVar(aValue));
    }

    inline JavascriptMap::MapDataList::Iterator JavascriptMap::GetIterator()
    {
        return list.GetIterator();
    }

} // namespace Js
