#include "stdafx.h"

namespace DevTests
{
  namespace ContractVersioned
  {
    class XyzServer:
        public Microsoft::WRL::RuntimeClass<DevTests::ContractVersioned::IXyz>
    {
        InspectableClass(L"DevTests.ContractVersioned.Xyz", BaseTrust);

      public:
        IFACEMETHOD(Method1)(__in int param1) override;
        IFACEMETHOD(get_Property1)(__out int* value) override {
            if (value == nullptr)
            {
                return E_INVALIDARG;
            }

            *value = _intVal;
            return S_OK;
        }

        IFACEMETHOD(put_Property1)(int value) override {
            _intVal = value;
            return S_OK;
        }

      private:
        int _intVal;
    };

    class XyzPlatformVersionedServer:
        public Microsoft::WRL::RuntimeClass<DevTests::ContractVersioned::IXyzPlatformVersioned>
    {
        InspectableClass(L"DevTests.ContractVersioned.XyzPlatformVersioned", BaseTrust);

      public:
        IFACEMETHOD(Method1)(__in int param1) override;
    };
  };
};