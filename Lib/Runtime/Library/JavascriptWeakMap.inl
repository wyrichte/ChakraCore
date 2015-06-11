//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

namespace Js {

    inline bool JavascriptWeakMap::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_WeakMap;
    }

    inline JavascriptWeakMap* JavascriptWeakMap::FromVar(Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptWeakMap'");
        
        return static_cast<JavascriptWeakMap *>(RecyclableObject::FromVar(aValue));
    }

} // namespace Js