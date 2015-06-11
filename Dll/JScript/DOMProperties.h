#include "StdAfx.h"

namespace Js
{
#if DOMEnabled
    class DOMBuiltInPropertyRecords
    {
    public:
#define ENTRYDOM(n, hash) const static BuiltInPropertyRecord<ARRAYSIZE(L#n)> DOM_##n;
#define ENTRYDOM_Existing(n, key) 
#include "JnDOMDirectFields.h"
    };

    BEGIN_ENUM_UINT(DOMPropertyIds)
        _lastJSProperty = PropertyIds::_countJSOnlyProperty - 1,
#define ENTRYDOM(n, hash) DOM_##n,
#include "JnDOMDirectFields.h"
        _count,
#define ENTRYDOM_Existing(n, key) DOM_##n = PropertyIds::##key,
#include "JnDOMDirectFields.h"
    END_ENUM_UINT()

    class DOMProperties
    {
    static const Js::PropertyRecord *builtInPropertyRecords[];

    public:
        static void InitializeDOMProperties(ThreadContext* threadContext);
    };
#endif
}