#include <Windows.Foundation.h>
#include <WindowsDateTimeP.h>

namespace Animals
{
    class CFastSigInterface : public Microsoft::WRL::Implements<Animals::IFastSigInterface>
    {
    public:
        CFastSigInterface() { }
        ~CFastSigInterface() { }

        IFACEMETHOD(GetOneVector)(__out Windows::Foundation::Collections::IVector<int> **outVal) override;
        IFACEMETHOD(GetNullAsVector)(__out Windows::Foundation::Collections::IVector<int> **outVal) override;
        IFACEMETHOD(GetOneObservableVector)(__out Windows::Foundation::Collections::IObservableVector<int> **outVal) override;
        IFACEMETHOD(GetNullAsObservableVector)(__out Windows::Foundation::Collections::IObservableVector<int> **outVal) override;
        IFACEMETHOD(GetOneAnimal)(__out IAnimal **outVal) override;
        IFACEMETHOD(GetNullAsAnimal)(__out IAnimal **outVal) override;
        IFACEMETHOD(GetOneMap)(__out Windows::Foundation::Collections::IMap<HSTRING, int> **outVal) override;
        IFACEMETHOD(GetNullAsMap)(__out Windows::Foundation::Collections::IMap<HSTRING, int> **outVal) override;
        IFACEMETHOD(GetOnePropertyValue)(__out Windows::Foundation::IPropertyValue **outVal) override;
        IFACEMETHOD(GetNullAsPropertyValue)(__out Windows::Foundation::IPropertyValue **outVal) override;
        IFACEMETHOD(GetOneEmptyGRCNInterface)(__out IEmptyGRCN **outValue) override;
        IFACEMETHOD(GetOneEmptyGRCNNull)(__out IEmptyGRCN **outValue) override;
        IFACEMETHOD(GetOneEmptyGRCNFail)(__out IEmptyGRCN **outValue) override;
    };


    class FishServer :
        public Microsoft::WRL::RuntimeClass<
            Animals::IFish,
            Animals::ILikeToSwim,
            CFastSigInterface>
    {
        InspectableClass(L"Animals.Fish", BaseTrust);

    public:
        FishServer();
        ~FishServer();
    
        IFACEMETHOD(GetNumFins)(__out int* numberOfFins) override;
        IFACEMETHOD(SetNumFins)(int numberOfFins) override;
        IFACEMETHOD(MarshalIFish)(__in IFish * _in, __out IFish ** _out) override;
        IFACEMETHOD(MarshalILikeToSwim)(__in ILikeToSwim * _in, __out ILikeToSwim ** _out) override;
        IFACEMETHOD(MarshalIFishToFish)(__in IFish * _in, __out IFish ** _out) override;
        IFACEMETHOD(MarshalILikeToSwimToFish)(__in ILikeToSwim * _in, __out ILikeToSwim ** _out) override;
        IFACEMETHOD(SingTheSwimmingSong)(__out HSTRING*) override;
        IFACEMETHOD(get_Name)(__out HSTRING *value) override;
        IFACEMETHOD(put_Name)(__in HSTRING value) override;
    private:
        int m_NumFins;
        HSTRING m_name;
    };


    class TurkeyServer :
        public Microsoft::WRL::RuntimeClass<
            Animals::ITurkey,
            Fabrikam::Kitchen::IBurgerMaster
            >
    {
        InspectableClass(L"Animals.Turkey", BaseTrust);

    public:
        TurkeyServer() : m_NumFeathers(100)
        {
        }
    
        IFACEMETHOD(ToSandwich1)(BOOL * hasMayo) override 
        {
            *hasMayo = TRUE;
            return S_OK;
        }

        IFACEMETHOD(ToSandwich2)(int baconSlices, BOOL * hasMayo) override 
        {
            *hasMayo = FALSE;
            if(baconSlices>0)
            {
                // Bacon without mayo? No way.
                *hasMayo = TRUE;
            }
            return S_OK;
        }

        IFACEMETHOD(GetNumFeathers)(int * feathers)
        {
            if(*feathers != NULL)
            {
                return E_INVALIDARG;
            }

            *feathers = m_NumFeathers;
            return S_OK;
        }

        IFACEMETHOD(MakeBurger)(DWORD baconSlices, DWORD cheeseSlices, int * mayoFactor) override
        {
            *mayoFactor = 50;

            // You're already having bacon and cheese, no mayo for you!
            if (baconSlices>0 && cheeseSlices>0)
            {
                *mayoFactor = 0;
            }

            return S_OK;
        }

        IFACEMETHOD(MakeBurgerAlwaysPuttingMayoInTheBun)(int * mayoFactor) override 
        {
            *mayoFactor = 100;
            return S_OK;
        }

    private:
        int m_NumFeathers;
    };

    class ElephantServer :
            public Microsoft::WRL::RuntimeClass<IAgeable, IInspectable>
        {
            InspectableClass(RuntimeClass_Animals_Elephant, BaseTrust);

        public:
            ElephantServer()
            {
                m_age.UniversalTime = 0;
                m_timeToGetToSixtyMPH.Duration = 2000; // Note that when we marshal this timespan to JS, it will be 0 because we divide 2000 by 10000
            }

            // IAgeable methods
            IFACEMETHOD(StartLifeNow)() override
            {
                SYSTEMTIME now;
                GetSystemTime( &now);
                RoSystemTimeToDateTime(now, &this->m_age);

                return S_OK;
            }

            IFACEMETHOD(GetAgeTicks)(__out t_Int64* pTicks) override
            {
                if (pTicks == NULL)
                {
                    return E_INVALIDARG;
                }

                (*pTicks) = m_age.UniversalTime;
                return S_OK;
            }

            IFACEMETHOD(SetAgeTicks)(t_Int64 ticks) override
            {
                m_age.UniversalTime = ticks;
                return S_OK;
            }

            IFACEMETHOD(SetAge)(Windows::Foundation::DateTime age) override
            {
                this->m_age = age;
                return S_OK;
            }

            IFACEMETHOD(GetAge)(__out Windows::Foundation::DateTime* age) override
            {
                if (age == NULL)
                {
                    return E_INVALIDARG;
                }

                (*age) = this->m_age;
                return S_OK;
            }

            IFACEMETHOD(SetTimeToGetToSixtyMPH)(Windows::Foundation::TimeSpan timespan) override
            {
                this->m_timeToGetToSixtyMPH = timespan;
                return S_OK;
            }

            IFACEMETHOD(GetTimeToGetToSixtyMPH)(__out Windows::Foundation::TimeSpan* timespan) override
            {
                if (timespan == NULL)
                {
                    return E_INVALIDARG;
                }

                (*timespan) = this->m_timeToGetToSixtyMPH;
                return S_OK;
            }

        private:
            Windows::Foundation::DateTime m_age;
            Windows::Foundation::TimeSpan m_timeToGetToSixtyMPH;
        };

}
