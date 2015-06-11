//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

namespace Js {

    inline bool JavascriptWeakSet::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_WeakSet;
    }

    inline JavascriptWeakSet* JavascriptWeakSet::FromVar(Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptWeakSet'");
        
        return static_cast<JavascriptWeakSet *>(RecyclableObject::FromVar(aValue));
    }

} // namespace Js