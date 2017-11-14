//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include "SimpleStaticConflictServer.h"

// ------------------------------------------------------------------------------------------
// (common logic)
// ------------------------------------------------------------------------------------------
HSTRING hs(LPCWSTR sz)
{
    HSTRING hs;
    WindowsCreateString(sz, (UINT32)wcslen(sz), &hs);
    return hs;
}

// ------------------------------------------------------------------------------------------
// SimpleStaticConflict
// ------------------------------------------------------------------------------------------
IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflict::SimpleStaticConflictAccessFactory::ActivateInstance(__deref_out IInspectable **ppInspectable)
{
    *ppInspectable = nullptr;
    return E_NOTIMPL;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflict::SimpleStaticConflictAccessFactory::DoSomething(boolean a, __out HSTRING * result)
{
    (void)a;
    *result = hs(L"SimpleStaticConflict.IA.DoSomething(boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflict::SimpleStaticConflictAccessFactory::DoSomething2(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"SimpleStaticConflict.IB.DoSomething2(boolean, boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflict::SimpleStaticConflictAccessFactory::DoSomethingNotOverloaded(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"SimpleStaticConflict.IA.DoSomethingNotOverloaded(boolean, boolean) called");

    return S_OK;
}

// ------------------------------------------------------------------------------------------
// SimpleStaticConflictWithSameArity
// ------------------------------------------------------------------------------------------
IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictWithSameArity::SimpleStaticConflictWithSameArityAccessFactory::ActivateInstance(__deref_out IInspectable **ppInspectable)
{
    *ppInspectable = nullptr;
    return E_NOTIMPL;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictWithSameArity::SimpleStaticConflictWithSameArityAccessFactory::DoSomething(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"SimpleStaticConflictWithSameArity.IA.DoSomething(boolean, boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictWithSameArity::SimpleStaticConflictWithSameArityAccessFactory::DoSomething2(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"SimpleStaticConflictWithSameArity.IB.DoSomething2(boolean, boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictWithSameArity::SimpleStaticConflictWithSameArityAccessFactory::DoSomething3(HSTRING a, __out HSTRING * result)
{
    (void)a;
    *result = hs(L"SimpleStaticConflictWithSameArity.IB.DoSomething3(HSTRING) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictWithSameArity::SimpleStaticConflictWithSameArityAccessFactory::DoSomethingNotOverloaded(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"SimpleStaticConflictWithSameArity.IA.DoSomethingNotOverloaded(boolean, boolean) called");

    return S_OK;
}

// ------------------------------------------------------------------------------------------
// SimpleStaticConflictWithinInterface
// ------------------------------------------------------------------------------------------
IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictWithinInterface::SimpleStaticConflictWithinInterfaceAccessFactory::ActivateInstance(__deref_out IInspectable **ppInspectable)
{
    *ppInspectable = nullptr;
    return E_NOTIMPL;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictWithinInterface::SimpleStaticConflictWithinInterfaceAccessFactory::DoSomething(boolean a, __out HSTRING * result)
{
    (void)a;
    *result = hs(L"SimpleStaticConflictWithinInterface.IA.DoSomething(boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictWithinInterface::SimpleStaticConflictWithinInterfaceAccessFactory::DoSomething2(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"SimpleStaticConflictWithinInterface.IA.DoSomething2(boolean, boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictWithinInterface::SimpleStaticConflictWithinInterfaceAccessFactory::DoSomething3(HSTRING a, __out HSTRING * result)
{
    (void)a;
    *result = hs(L"SimpleStaticConflictWithinInterface.IA.DoSomething(HSTRING) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictWithinInterface::SimpleStaticConflictWithinInterfaceAccessFactory::DoSomethingNotOverloaded(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"SimpleStaticConflictWithinInterface.IA.DoSomethingNotOverloaded(boolean, boolean) called");

    return S_OK;
}

// ------------------------------------------------------------------------------------------
// SimpleStaticConflictWithDifferentArity
// ------------------------------------------------------------------------------------------
IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictWithDifferentArity::SimpleStaticConflictWithDifferentArityAccessFactory::ActivateInstance(__deref_out IInspectable **ppInspectable)
{
    *ppInspectable = nullptr;
    return E_NOTIMPL;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictWithDifferentArity::SimpleStaticConflictWithDifferentArityAccessFactory::DoSomething(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"SimpleStaticConflictWithDifferentArity.IA.DoSomething(boolean, boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictWithDifferentArity::SimpleStaticConflictWithDifferentArityAccessFactory::DoSomething2(boolean a, __out HSTRING * result)
{
    (void)a;
    *result = hs(L"SimpleStaticConflictWithDifferentArity.IB.DoSomething2(boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictWithDifferentArity::SimpleStaticConflictWithDifferentArityAccessFactory::DoAnotherThing(boolean a, __out HSTRING * result)
{
    (void)a;
    *result = hs(L"SimpleStaticConflictWithDifferentArity.IA.DoAnotherThing(boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictWithDifferentArity::SimpleStaticConflictWithDifferentArityAccessFactory::DoAnotherThing2(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"SimpleStaticConflictWithDifferentArity.IB.DoAnotherThing2(boolean, boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictWithDifferentArity::SimpleStaticConflictWithDifferentArityAccessFactory::DoSomethingNotOverloaded(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"SimpleStaticConflictWithDifferentArity.IA.DoSomethingNotOverloaded(boolean, boolean) called");

    return S_OK;
}

// ------------------------------------------------------------------------------------------
// SimpleStaticConflictDefaultOverloadLast
// ------------------------------------------------------------------------------------------
IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictDefaultOverloadLast::SimpleStaticConflictDefaultOverloadLastAccessFactory::ActivateInstance(__deref_out IInspectable **ppInspectable)
{
    *ppInspectable = nullptr;
    return E_NOTIMPL;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictDefaultOverloadLast::SimpleStaticConflictDefaultOverloadLastAccessFactory::DoSomething(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"SimpleStaticConflictDefaultOverloadLast.IA.DoSomething(boolean, boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictDefaultOverloadLast::SimpleStaticConflictDefaultOverloadLastAccessFactory::DoSomething2(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"SimpleStaticConflictDefaultOverloadLast.IB.DoSomething2(boolean, boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictDefaultOverloadLast::SimpleStaticConflictDefaultOverloadLastAccessFactory::DoSomethingNotOverloaded(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"SimpleStaticConflictDefaultOverloadLast.IA.DoSomethingNotOverloaded(boolean, boolean) called");

    return S_OK;
}

// ------------------------------------------------------------------------------------------
// SimpleStaticConflictVersioned
// ------------------------------------------------------------------------------------------
IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictVersioned::SimpleStaticConflictVersionedAccessFactory::ActivateInstance(__deref_out IInspectable **ppInspectable)
{
    *ppInspectable = nullptr;
    return E_NOTIMPL;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictVersioned::SimpleStaticConflictVersionedAccessFactory::DoSomething(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"SimpleStaticConflictVersioned.IA.DoSomething(boolean, boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictVersioned::SimpleStaticConflictVersionedAccessFactory::DoSomething2(boolean a, __out HSTRING * result)
{
    (void)a;
    *result = hs(L"SimpleStaticConflictVersioned.IB.DoSomething2(boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictVersioned::SimpleStaticConflictVersionedAccessFactory::DoSomething3(boolean a, boolean b, boolean c, __out HSTRING * result)
{
    (void)a;
    (void)b;
    (void)c;
    *result = hs(L"SimpleStaticConflictVersioned.IC.DoSomething3(boolean, boolean, boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictVersioned::SimpleStaticConflictVersionedAccessFactory::DoSomethingNotOverloaded(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"SimpleStaticConflictVersioned.IA.DoSomethingNotOverloaded(boolean, boolean) called");

    return S_OK;
}

// ------------------------------------------------------------------------------------------
// SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface
// ------------------------------------------------------------------------------------------
IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface::SimpleStaticConflictVersionedWithMultipleOverloadsPerInterfaceAccessFactory::ActivateInstance(__deref_out IInspectable **ppInspectable)
{
    *ppInspectable = nullptr;
    return E_NOTIMPL;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface::SimpleStaticConflictVersionedWithMultipleOverloadsPerInterfaceAccessFactory::DoSomething(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.IA.DoSomething(boolean, boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface::SimpleStaticConflictVersionedWithMultipleOverloadsPerInterfaceAccessFactory::DoSomething2(boolean a, __out HSTRING * result)
{
    (void)a;
    *result = hs(L"SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.IB.DoSomething2(boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface::SimpleStaticConflictVersionedWithMultipleOverloadsPerInterfaceAccessFactory::DoSomething3(boolean a, boolean b, boolean c, __out HSTRING * result)
{
    (void)a;
    (void)b;
    (void)c;
    *result = hs(L"SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.IB.DoSomething3(boolean, boolean, boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface::SimpleStaticConflictVersionedWithMultipleOverloadsPerInterfaceAccessFactory::DoSomethingNotOverloaded(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.IA.DoSomethingNotOverloaded(boolean, boolean) called");

    return S_OK;
}

// ------------------------------------------------------------------------------------------
// StaticConflictWithRequiresInterface
// ------------------------------------------------------------------------------------------
IFACEMETHODIMP
Winery::Overloading::StaticConflictWithRequiresInterface::StaticConflictWithRequiresInterfaceAccessFactory::ActivateInstance(__deref_out IInspectable **ppInspectable)
{
    *ppInspectable = nullptr;
    return E_NOTIMPL;
}

IFACEMETHODIMP
Winery::Overloading::StaticConflictWithRequiresInterface::StaticConflictWithRequiresInterfaceAccessFactory::DoSomething(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"StaticConflictWithRequiresInterface.IA.DoSomething(boolean, boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::StaticConflictWithRequiresInterface::StaticConflictWithRequiresInterfaceAccessFactory::DoSomething2(boolean a, __out HSTRING * result)
{
    (void)a;
    *result = hs(L"StaticConflictWithRequiresInterface.IB.DoSomething2(boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::StaticConflictWithRequiresInterface::StaticConflictWithRequiresInterfaceAccessFactory::DoSomething3(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"StaticConflictWithRequiresInterface.IB.DoSomething3(boolean, boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::StaticConflictWithRequiresInterface::StaticConflictWithRequiresInterfaceAccessFactory::DoSomething4(boolean a, boolean b, boolean c, __out HSTRING * result)
{
    (void)a;
    (void)b;
    (void)c;
    *result = hs(L"StaticConflictWithRequiresInterface.IB.DoSomething4(boolean, boolean, boolean) called");

    return S_OK;
}

IFACEMETHODIMP
Winery::Overloading::StaticConflictWithRequiresInterface::StaticConflictWithRequiresInterfaceAccessFactory::DoSomethingNotOverloaded(boolean a, boolean b, __out HSTRING * result)
{
    (void)a;
    (void)b;
    *result = hs(L"StaticConflictWithRequiresInterface.IA.DoSomethingNotOverloaded(boolean, boolean) called");

    return S_OK;
}
