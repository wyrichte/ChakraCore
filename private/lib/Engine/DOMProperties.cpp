#include "EnginePch.h"

#if DOMEnabled
const Js::PropertyRecord * Js::DOMProperties::builtInPropertyRecords[] =
{
#define ENTRYDOM(n, hash) Js::DOMBuiltInPropertyRecords::DOM_##n,
//Don't add existing properties ehre as they will be covered by builtInPropertyRecords above
#define ENTRYDOM_Existing(n, key)  
#include "JnDOMDirectFields.h"
};


#define ENTRYDOM(n, hash) const Js::BuiltInPropertyRecord<ARRAYSIZE(L#n)> Js::DOMBuiltInPropertyRecords::DOM_##n = { Js::PropertyRecord(DOMPropertyIds::DOM_##n, hash, false, (ARRAYSIZE(L#n) - 1) * sizeof(wchar_t)), L#n };
#define ENTRYDOM_Existing(n, key) 
#include "JnDOMDirectFields.h"

void Js::DOMProperties::InitializeDOMProperties(ThreadContext* threadContext)
{
    for (int i = 0; i < _countof(builtInPropertyRecords); i++)
    {
        // The hash for these Properties was pre-calculated during the generation of the fastdom_propertyidtable.inl. The code for generating the hash can be found at:
        // \inetcore\mshtml\types\fastDOMCompiler.pl : getHash()
        Assert(JsUtil::CharacterBuffer<WCHAR>::StaticGetHashCode(builtInPropertyRecords[i]->GetBuffer(), builtInPropertyRecords[i]->GetLength()) == builtInPropertyRecords[i]->GetHashCode());
        threadContext->AddBuiltInPropertyRecord(builtInPropertyRecords[i]);
    }
}
#endif