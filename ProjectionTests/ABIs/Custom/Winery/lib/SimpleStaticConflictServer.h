//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include "winery.h"

namespace Winery
{
    namespace Overloading
    {
        namespace SimpleStaticConflict
        {
            class SimpleStaticConflictAccessServer :
                public Microsoft::WRL::RuntimeClass<IEmpty>
            {
                InspectableClass(L"Winery.Overloading.SimpleStaticConflict.Access", BaseTrust);
            };

            class SimpleStaticConflictAccessFactory :
                public Microsoft::WRL::ActivationFactory<IA,IB>
            {
            public:
                // IActivationFactory
                IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable);
                    
                IFACEMETHOD(DoSomething)(boolean a, __out HSTRING * result);

                IFACEMETHOD(DoSomething2)(boolean a, boolean b, __out HSTRING * result);

                IFACEMETHOD(DoSomethingNotOverloaded)(boolean a, boolean b, __out HSTRING * result);
            };
        }

        namespace SimpleStaticConflictWithSameArity
        {
            class SimpleStaticConflictWithSameArityAccessServer :
                public Microsoft::WRL::RuntimeClass<IEmpty>
            {
                InspectableClass(L"Winery.Overloading.SimpleStaticConflictWithSameArity.Access", BaseTrust);
            };

            class SimpleStaticConflictWithSameArityAccessFactory :
                public Microsoft::WRL::ActivationFactory<IA,IB>
            {
            public:
                // IActivationFactory
                IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable);
                    
                IFACEMETHOD(DoSomething)(boolean a, boolean b, __out HSTRING * result);

                IFACEMETHOD(DoSomething2)(boolean a, boolean b, __out HSTRING * result);

                IFACEMETHOD(DoSomething3)(HSTRING a, __out HSTRING * result);

                IFACEMETHOD(DoSomethingNotOverloaded)(boolean a, boolean b, __out HSTRING * result);
            };
        }

        namespace SimpleStaticConflictWithinInterface
        {
            class SimpleStaticConflictWithinInterfaceAccessServer :
                public Microsoft::WRL::RuntimeClass<IEmpty>
            {
                InspectableClass(L"Winery.Overloading.SimpleStaticConflictWithinInterface.Access", BaseTrust);
            };

            class SimpleStaticConflictWithinInterfaceAccessFactory :
                public Microsoft::WRL::ActivationFactory<IA>
            {
            public:
                // IActivationFactory
                IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable);
                    
                IFACEMETHOD(DoSomething)(boolean a, __out HSTRING * result);

                IFACEMETHOD(DoSomething2)(boolean a, boolean b, __out HSTRING * result);

                IFACEMETHOD(DoSomething3)(HSTRING a, __out HSTRING * result);

                IFACEMETHOD(DoSomethingNotOverloaded)(boolean a, boolean b, __out HSTRING * result);
            };
        }

        namespace SimpleStaticConflictWithDifferentArity
        {
            class SimpleStaticConflictWithDifferentArityAccessServer :
                public Microsoft::WRL::RuntimeClass<IEmpty>
            {
                InspectableClass(L"Winery.Overloading.SimpleStaticConflictWithDifferentArity.Access", BaseTrust);
            };

            class SimpleStaticConflictWithDifferentArityAccessFactory :
                public Microsoft::WRL::ActivationFactory<IA,IB>
            {
            public:
                // IActivationFactory
                IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable);
                    
                IFACEMETHOD(DoSomething)(boolean a, boolean b, __out HSTRING * result);

                IFACEMETHOD(DoSomething2)(boolean a, __out HSTRING * result);

                IFACEMETHOD(DoAnotherThing)(boolean a, __out HSTRING * result);

                IFACEMETHOD(DoAnotherThing2)(boolean a, boolean b, __out HSTRING * result);

                IFACEMETHOD(DoSomethingNotOverloaded)(boolean a, boolean b, __out HSTRING * result);
            };
        }

        namespace SimpleStaticConflictDefaultOverloadLast
        {
            class SimpleStaticConflictDefaultOverloadLastAccessServer :
                public Microsoft::WRL::RuntimeClass<IEmpty>
            {
                InspectableClass(L"Winery.Overloading.SimpleStaticConflictDefaultOverloadLast.Access", BaseTrust);
            };

            class SimpleStaticConflictDefaultOverloadLastAccessFactory :
                public Microsoft::WRL::ActivationFactory<IA,IB>
            {
            public:
                // IActivationFactory
                IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable);
                    
                IFACEMETHOD(DoSomething)(boolean a, boolean b, __out HSTRING * result);

                IFACEMETHOD(DoSomething2)(boolean a, boolean b, __out HSTRING * result);

                IFACEMETHOD(DoSomethingNotOverloaded)(boolean a, boolean b, __out HSTRING * result);
            };
        }

        namespace SimpleStaticConflictVersioned
        {
            class SimpleStaticConflictVersionedAccessServer :
                public Microsoft::WRL::RuntimeClass<IEmpty>
            {
                InspectableClass(L"Winery.Overloading.SimpleStaticConflictVersioned.Access", BaseTrust);
            };

            class SimpleStaticConflictVersionedAccessFactory :
                public Microsoft::WRL::ActivationFactory<IA,IB,IC>
            {
            public:
                // IActivationFactory
                IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable);
                    
                IFACEMETHOD(DoSomething)(boolean a, boolean b, __out HSTRING * result);

                IFACEMETHOD(DoSomething2)(boolean a, __out HSTRING * result);

                IFACEMETHOD(DoSomething3)(boolean a, boolean b, boolean c, __out HSTRING * result);

                IFACEMETHOD(DoSomethingNotOverloaded)(boolean a, boolean b, __out HSTRING * result);
            };
        }

        namespace SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface
        {
            class SimpleStaticConflictVersionedWithMultipleOverloadsPerInterfaceAccessServer :
                public Microsoft::WRL::RuntimeClass<IEmpty>
            {
                InspectableClass(L"Winery.Overloading.SimpleStaticConflictVersionedWithMultipleOverloadsPerInterface.Access", BaseTrust);
            };

            class SimpleStaticConflictVersionedWithMultipleOverloadsPerInterfaceAccessFactory :
                public Microsoft::WRL::ActivationFactory<IA,IB>
            {
            public:
                // IActivationFactory
                IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable);
                    
                IFACEMETHOD(DoSomething)(boolean a, boolean b, __out HSTRING * result);

                IFACEMETHOD(DoSomething2)(boolean a, __out HSTRING * result);

                IFACEMETHOD(DoSomething3)(boolean a, boolean b, boolean c, __out HSTRING * result);

                IFACEMETHOD(DoSomethingNotOverloaded)(boolean a, boolean b, __out HSTRING * result);
            };
        }

        namespace StaticConflictWithRequiresInterface
        {
            class StaticConflictWithRequiresInterfaceAccessServer :
                public Microsoft::WRL::RuntimeClass<IEmpty>
            {
                InspectableClass(L"Winery.Overloading.StaticConflictWithRequiresInterface.Access", BaseTrust);
            };

            class StaticConflictWithRequiresInterfaceAccessFactory :
                public Microsoft::WRL::ActivationFactory<IA,IB>
            {
            public:
                // IActivationFactory
                IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable);
                    
                IFACEMETHOD(DoSomething)(boolean a, boolean b, __out HSTRING * result);

                IFACEMETHOD(DoSomething2)(boolean a, __out HSTRING * result);

                IFACEMETHOD(DoSomething3)(boolean a, boolean b, __out HSTRING * result);

                IFACEMETHOD(DoSomething4)(boolean a, boolean b, boolean c, __out HSTRING * result);

                IFACEMETHOD(DoSomethingNotOverloaded)(boolean a, boolean b, __out HSTRING * result);
            };
        }
    }
}