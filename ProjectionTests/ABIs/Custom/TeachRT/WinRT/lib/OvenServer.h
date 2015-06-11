//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include "Fabrikam.h"

using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

namespace Fabrikam
{
    namespace Kitchen
    {
        typedef Windows::Foundation::IAsyncOperationWithProgress<IVectorView<Fabrikam::Kitchen::ICookie *> *, int> IBakeOperation;
        typedef Windows::Foundation::IAsyncOperationProgressHandler<IVectorView<Fabrikam::Kitchen::ICookie *> *, int> IBakingProgressHandler;
        typedef Windows::Foundation::IAsyncOperationWithProgressCompletedHandler<IVectorView<Fabrikam::Kitchen::ICookie *> *, int> IBakingCompletedHandler;

        typedef Windows::Foundation::IAsyncOperation<bool> ITimerOperation;
        typedef Windows::Foundation::IAsyncOperationCompletedHandler<bool> ITimerCompletedHandler;

        // DO NOT give your server class the same namespace qualified name
        // or non-qualified name as the runtime class that it's intended
        // to implement. This will mitigate against possible naming conflicts
        // with the future C++ consumption model.
        class OvenServer :
            public Microsoft::WRL::RuntimeClass<
                Fabrikam::Kitchen::IOven, 
                Fabrikam::Kitchen::IAppliance>
        {
            InspectableClass(L"Fabrikam.Kitchen.Oven", BaseTrust);

        public:

            // DevX & UEX Coding standards require Hungarian notation. WinRT
            // API guidelines prohibit it. This is because the hungarian type
            // prefixes may be misleading when mapped into different languages. 
            // The way to resolve this is to specify non-prefixed names in the 
            // ReXML or IDL files, but use hungarian parameter names in your 
            // server implementation files.

            // The id for async worker objects must start at a number larger than 0.
            OvenServer() :
                _id(17) {}

            // IAppliance::get_Size
            IFACEMETHOD(get_Size)(__out Dimensions *pdims) override;
                
            // IAppliance::put_Size
            IFACEMETHOD(put_Size)(Dimensions dims) override;

            // IAppliance::get_ElectricityReporter
            IFACEMETHOD(get_ElectricityReporter)(
                __out IApplianceElectricityConsumptionReporter **electricityReporter) override;

            // IOven::BakeAsync
            IFACEMETHOD(BakeAsync)(
                __in        int iNumCookies,
                __deref_out Fabrikam::Kitchen::IBakeOperation **ppBakeAsync) override;

            // IOven::TimerAsync
            IFACEMETHOD(TimerAsync)(
                __in        int iDuration, 
                __deref_out Fabrikam::Kitchen::ITimerOperation **ppBakeAsync) override;

        private:

            Dimensions   _dims;
            unsigned int _id;
        };
    }
}