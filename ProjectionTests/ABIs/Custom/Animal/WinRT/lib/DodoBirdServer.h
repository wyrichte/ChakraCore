#include "stdafx.h"
namespace Animals
{
    
    class DodoBirdServer :
            public Microsoft::WRL::RuntimeClass<Animals::IExtinct>
    {
        InspectableClass(L"Animals.DodoBird", BaseTrust);

    public:
        DodoBirdServer();

        IFACEMETHOD(IsExtinct)(__out boolean* res) override;

    };
    ;
}
