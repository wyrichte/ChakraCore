#include <stdafx.h>
#include <wrl\dllexports.h>
#include "winery.h"

#include "WineryServer.h"
#include "RestrictedErrorAccessServer.h"
#include "SimpleStaticConflictServer.h"

namespace Winery
{
    namespace Servers
    {
        ActivatableClassWithFactory(WineryServer, WineryFactory)
    }

    namespace WinRTErrorTests
    {
        ActivatableClassWithFactory(RestrictedErrorAccessServer, RestrictedErrorAccessFactory)
        ActivatableClassWithFactory(RestrictedErrorAccessInstanceServer, RestrictedErrorAccessInstanceFactory)
    }

    namespace Overloading
    {
        namespace SimpleStaticConflict
        {
            ActivatableClassWithFactory(SimpleStaticConflictAccessServer, SimpleStaticConflictAccessFactory)
        }

        namespace SimpleStaticConflictWithSameArity
        {
            ActivatableClassWithFactory(SimpleStaticConflictWithSameArityAccessServer, SimpleStaticConflictWithSameArityAccessFactory)
        }

        namespace SimpleStaticConflictWithinInterface
        {
            ActivatableClassWithFactory(SimpleStaticConflictWithinInterfaceAccessServer, SimpleStaticConflictWithinInterfaceAccessFactory)
        }

        namespace SimpleStaticConflictWithDifferentArity
        {
            ActivatableClassWithFactory(SimpleStaticConflictWithDifferentArityAccessServer, SimpleStaticConflictWithDifferentArityAccessFactory)
        }

        namespace SimpleStaticConflictDefaultOverloadLast
        {
            ActivatableClassWithFactory(SimpleStaticConflictDefaultOverloadLastAccessServer, SimpleStaticConflictDefaultOverloadLastAccessFactory)
        }

        namespace SimpleStaticConflictVersioned
        {
            ActivatableClassWithFactory(SimpleStaticConflictVersionedAccessServer, SimpleStaticConflictVersionedAccessFactory)
        }

        namespace SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface
        {
            ActivatableClassWithFactory(SimpleStaticConflictVersionedWithMultipleOverloadsPerInterfaceAccessServer, SimpleStaticConflictVersionedWithMultipleOverloadsPerInterfaceAccessFactory)
        }

        namespace StaticConflictWithRequiresInterface
        {
            ActivatableClassWithFactory(StaticConflictWithRequiresInterfaceAccessServer, StaticConflictWithRequiresInterfaceAccessFactory)
        }
    }
}

BOOL
WINAPI
QuirkIsEnabled(__in ULONG QuirkId)
{
    UNREFERENCED_PARAMETER(QuirkId);
    return FALSE;
}
