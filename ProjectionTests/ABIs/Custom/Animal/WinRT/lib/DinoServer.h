#include "stdafx.h"
using namespace Microsoft::WRL;

namespace Animals
{
    
    class DinoServer :
            public Microsoft::WRL::RuntimeClass<Animals::IDino, Animals::IExtinct>
    {
        InspectableClass(L"Animals.Dino", BaseTrust);

    public:
        DinoServer();

        IFACEMETHOD(CanRoar)(__out boolean* result) override;
        IFACEMETHOD(Roar)(int numtimes) override;
        IFACEMETHOD(get_Height)(int* high) override {*high = 5; return NOERROR; }
        IFACEMETHOD(hasTeeth)(__out boolean* res) override {*res = true; return NOERROR; }

        IFACEMETHOD(IsExtinct)(__out boolean* res) override;
        IFACEMETHOD(HasTeeth)(__out boolean* res) override {*res = false; return NOERROR; }

    private:
        boolean m_canRoar;

    };

    class DinoFactory :
        public Microsoft::WRL::ActivationFactory<IStaticDino>
    {
//		InspectableClass(L"Animals.DinoFactory", BaseTrust);

    private:
        boolean isScary;
        Microsoft::WRL::EventSource<IFossilsFoundHandler> _evtFossilsFound;

    public:
        STDMETHOD_(ULONG, Release)() override
        {
            return Microsoft::WRL::ActivationFactory<IStaticDino>::Release();
        }

        ~DinoFactory()
        {
        }

        // IActivationFactory
        IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable) {
            HRESULT hr = S_OK;
            ComPtr<IDino> spIDino;
            *ppInspectable = nullptr;

            ComPtr<DinoServer> spObj = Make<DinoServer>();

            hr = spObj.As(&spIDino);
            if (SUCCEEDED(hr))
            {
                *ppInspectable = spIDino.Detach();
            }

            return hr;
        };

        // IStaticDino
        IFACEMETHOD(LookForFossils)(__in int timeSpent, __out int* fossilsFound) {
            *fossilsFound = (timeSpent/4);
            _evtFossilsFound.InvokeAll(this, *fossilsFound);
            return S_OK;
        };
        IFACEMETHOD(InspectDino)(__in IDino* specimen, __out HSTRING* results);
        IFACEMETHOD(get_IsScary)(__out boolean* value) {
            *value = isScary;
            return S_OK;
        };
        IFACEMETHOD(put_IsScary)(__in boolean value) {
            isScary = value;
            return S_OK;
        };

        IFACEMETHOD(add_FossilsFoundEvent)( 
            __in Animals::IFossilsFoundHandler *clickHandler,
            __out EventRegistrationToken *pCookie);
                
        IFACEMETHOD(remove_FossilsFoundEvent)( 
            __in EventRegistrationToken iCookie);
    }
    ;
}
