//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include "winery.h"
#include <stdio.h>
//#include <windowscollectionsp.h>

 #include "IEnumerableServer.h"

namespace Winery
{
    class CustomAsyncInfoImpl :
        public Microsoft::WRL::RuntimeClass<
            Windows::Foundation::IAsyncInfo,
            ICustomAsync
        >
    {
        ICustomAsyncCompleted * completed;
        ICustomAsyncProgress * progress;
        IInspectable * result;
        AsyncStatus status;
        bool incorrectReturnValues;
        bool invalidStatusArg;
        bool invalidSenderArg;
        bool allowProgressWhenComplete;
    public:
        ~CustomAsyncInfoImpl()
        {
            if (completed)
            {
                completed->Release();
            }
            if (progress)
            {
                progress->Release();
            }
            if (this->result)
            {
                this->result->Release();
            }
        }

        void Initialize()
        {
            completed = nullptr;
            progress = nullptr;
            result = nullptr;
            status = Windows::Foundation::AsyncStatus::Started;
            incorrectReturnValues = false;
            invalidStatusArg = false;
            invalidSenderArg = false;
            allowProgressWhenComplete = false;
        }

        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Completed(ICustomAsyncCompleted ** ppcompleted) 
        { 
            *ppcompleted = completed;
            if (completed)
            {
                completed->AddRef();
            }
            return S_OK; 
        }

        virtual HRESULT STDMETHODCALLTYPE put_Completed(ICustomAsyncCompleted * pcompleted) 
        { 
            if (completed)
            {
                completed->Release();
            }

            completed = pcompleted;
            if(completed)
            {
                completed->AddRef();
            }
            return S_OK; 
        }

        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Progress(ICustomAsyncProgress ** ppprogress) 
        { 
            *ppprogress = progress;
            if (progress)
            {
                progress->AddRef();
            }
            return S_OK; 
        }

        virtual HRESULT STDMETHODCALLTYPE put_Progress(ICustomAsyncProgress * pprogress) 
        { 
            if (progress)
            {
                progress->Release();
            }

            progress = pprogress;
            if (progress)
            {
                progress->AddRef();
            }
            return S_OK; 
        }

        virtual HRESULT STDMETHODCALLTYPE GetResults(IInspectable ** result)
        { 
            *result = this->result;
            if (*result)
            {
                (*result)->AddRef();
            }

            if (status == Windows::Foundation::AsyncStatus::Error)
            {
                if (incorrectReturnValues)
                {
                    return S_OK;
                }
                return E_OUTOFMEMORY;
            }
            else
            {
                if (incorrectReturnValues)
                {
                    return E_OUTOFMEMORY;
                }
                return S_OK; 
            }
        }

        virtual HRESULT STDMETHODCALLTYPE MoveToCompleted(IInspectable * result) 
        { 
            if (this->result)
            {
                this->result->Release();
            }
            this->result = result;
            if (this->result)
            {
                this->result->AddRef();
            }

            AsyncStatus status;
            ICustomAsync * sender;
            if(this->status == Windows::Foundation::AsyncStatus::Started)
            {
                this->status = Windows::Foundation::AsyncStatus::Completed;

                status = this->status;
                if (invalidStatusArg)
                {
                    status = (AsyncStatus)42;
                }
                sender = this;
                if (invalidSenderArg)
                {
                    sender = nullptr;
                }

                completed->Invoke(sender, status);
                return S_OK;
            }
            return E_FAIL; 
        }

        virtual HRESULT STDMETHODCALLTYPE MoveToError(void) 
        { 
            AsyncStatus status;
            ICustomAsync * sender;
            if(this->status == Windows::Foundation::AsyncStatus::Started)
            {
                this->status = Windows::Foundation::AsyncStatus::Error;

                status= this->status;
                if (invalidStatusArg)
                {
                    status = (AsyncStatus)42;
                }
                sender = this;
                if (invalidSenderArg)
                {
                    sender = nullptr;
                }

                if (completed)
                {
                    completed->Invoke(sender, status);
                }
                return S_OK;
            }
            return E_FAIL;
        }

        virtual HRESULT STDMETHODCALLTYPE TriggerProgress(int percent)
        {
            if (this->status != Windows::Foundation::AsyncStatus::Started && !allowProgressWhenComplete)
            {
                return E_FAIL;
            }
            
            ICustomAsync* sender = this;
            if (invalidSenderArg)
            {
                sender = nullptr;
            }

            if (progress)
            {
                progress->Invoke(sender, percent);
            }

            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE AllowProgressCalledAfterCompletion()
        {
            allowProgressWhenComplete = true;
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE DisallowProgressCalledAfterCompletion()
        {
            allowProgressWhenComplete = false;
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE TriggerIncorrectReturnValues(void) 
        {
            incorrectReturnValues = true;
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE RestoreCorrectReturnValues(void) 
        {
            incorrectReturnValues = false;
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE TriggerInvalidStatusArg(void) 
        {
            invalidStatusArg = true;
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE RestoreStatusArg(void) 
        {
            invalidStatusArg = false;
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE TriggerInvalidSenderArg(void) 
        {
            invalidSenderArg = true;
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE RestoreSenderArg(void) 
        {
            invalidSenderArg = false;
            return S_OK;
        }

        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Id( 
            /* [retval][out] */ __RPC__out unsigned __int32 *id) 
        { 
            *id = 0; 
            return S_OK; 
        }
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Status( 
            /* [retval][out] */ __RPC__out AsyncStatus *pstatus) 
        { 
            *pstatus = status; 
            return S_OK; 
        }
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_ErrorCode( 
            /* [retval][out] */ __RPC__out HRESULT *errorCode) 
        { 
            if (status == Windows::Foundation::AsyncStatus::Error && !incorrectReturnValues)
            {
                *errorCode = E_FAIL; 
            }
            return S_OK; 
        }
        
        virtual HRESULT STDMETHODCALLTYPE Start( void) 
        { 
            return S_OK; 
        }
        
        virtual HRESULT STDMETHODCALLTYPE Cancel( void) 
        { 
            if(status == Windows::Foundation::AsyncStatus::Started && !incorrectReturnValues)
            {
                status = Windows::Foundation::AsyncStatus::Canceled;
                completed->Invoke(this, status);
                return S_OK;
            }
            return E_FAIL; 
        }
        
        virtual HRESULT STDMETHODCALLTYPE Close( void) 
        { 
            return S_OK; 
        }
    };

    [uuid("98A755AF-A55F-4c63-8A52-73427083A1e8")] 
    class CustomAsyncInfo :
        public CustomAsyncInfoImpl
    {
        InspectableClass(L"Winery.CustomAsyncInfo", BaseTrust);
    };

    class AllowForWebCustomAsyncInfo :
        public CustomAsyncInfoImpl
    {
        InspectableClass(L"Winery.AllowForWebCustomAsyncInfo", BaseTrust);
    };

    inline HSTRING hs(LPCWSTR sz)
    {
        HSTRING hs;
        WindowsCreateString(sz, (UINT32)wcslen(sz), &hs);
        return hs;
    }

    [uuid("02A755AF-A52F-4c63-8A52-7A5270a9A1D8")] 
    struct SimpleConflict :
        public Microsoft::WRL::RuntimeClass<
            Winery::Overloading::SimpleConflict::IA, 
            Winery::Overloading::SimpleConflict::IB>
    {
        InspectableClass(L"Winery.Overloading.SimpleConflict.C", BaseTrust);
        HRESULT STDMETHODCALLTYPE Draw(int,HSTRING * result) { *result = hs(L"IA.Draw(int) Called"); return S_OK; }
        HRESULT STDMETHODCALLTYPE Draw(HSTRING,HSTRING * result) { *result = hs(L"IB.Draw(HSTRING) Called"); return S_OK; }
    };


    [uuid("02A755AF-A52F-4c63-1952-7A5270a9A1D8")] 
    struct SimpleOverloadSet :
        public Microsoft::WRL::RuntimeClass<
            Winery::Overloading::SimpleOverloadSet::IA>
    {
        InspectableClass(L"Winery.Overloading.SimpleOverloadSet.C", BaseTrust);
        HRESULT STDMETHODCALLTYPE Draw1(int,HSTRING * result) { *result = hs(L"IA.Draw(int) Called"); return S_OK; }
        HRESULT STDMETHODCALLTYPE Draw2(HSTRING,HSTRING,HSTRING * result) { *result = hs(L"IA.Draw(HSTRING,HSTRING) Called"); return S_OK; }
    };

    [uuid("B2A756AF-A52F-4c63-1952-9A5270a9A1D8")] 
    struct InheritedConflict :
        public Microsoft::WRL::RuntimeClass<
            Winery::Overloading::InheritedConflict::IA,
            Winery::Overloading::InheritedConflict::IB>
    {
        InspectableClass(L"Winery.Overloading.InheritedConflict.C", BaseTrust);
        HRESULT STDMETHODCALLTYPE Draw(int,HSTRING * result) { *result = hs(L"IA.Draw(int) Called"); return S_OK; }
        HRESULT STDMETHODCALLTYPE Draw(HSTRING,HSTRING,HSTRING * result) {  *result = hs(L"IB.Draw(HSTRING,HSTRING) Called"); return S_OK; }
    };



    [uuid("B2A756AF-A52F-4c63-6952-9A5270a9A1D8")] 
    struct NameConflictingWithOverloadSet :
        public Microsoft::WRL::RuntimeClass<
            Winery::Overloading::NameConflictingWithOverloadSet::IA,
            Winery::Overloading::NameConflictingWithOverloadSet::IB,
            Winery::Overloading::NameConflictingWithOverloadSet::IC>
    {
        InspectableClass(L"Winery.Overloading.NameConflictingWithOverloadSet.C", BaseTrust);
        HRESULT STDMETHODCALLTYPE Draw(int,HSTRING * result) { *result = hs(L"IA.Draw(int) Called"); return S_OK; }
        HRESULT STDMETHODCALLTYPE Draw(HSTRING,HSTRING,HSTRING * result) { *result = hs(L"IB.Draw(HSTRING,HSTRING) Called");  return S_OK; }
        HRESULT STDMETHODCALLTYPE Draw1(int,int,int,HSTRING * result) { *result = hs(L"IC.Draw(int,int,int) Called");  return S_OK; }
        HRESULT STDMETHODCALLTYPE Draw2(HSTRING,HSTRING,HSTRING,HSTRING,HSTRING * result) { *result = hs(L"IC.Draw(HSTRING,HSTRING,HSTRING,HSTRING) Called");  return S_OK; }
    };

    [uuid("B23756AF-A52F-4c63-6952-9A5270a9A2D8")] 
    struct Diamond :
        public Microsoft::WRL::RuntimeClass<
            Winery::Overloading::Diamond::ILeft,
            Winery::Overloading::Diamond::IRight,
            Winery::Overloading::Diamond::IRoot>
    {
        InspectableClass(L"Winery.Overloading.Diamond.C", BaseTrust);
        HRESULT STDMETHODCALLTYPE Draw(int,HSTRING * result) { *result = hs(L"IRoot.Draw(int) Called");  return S_OK; }
    };

    [uuid("02A755Ad-A50F-4c63-1952-7A5270a9A1D8")] 
    struct SimpleDefaultOverloadSet :
        public Microsoft::WRL::RuntimeClass<
            Winery::Overloading::SimpleDefaultOverloadSet::IA>
    {
        InspectableClass(L"Winery.Overloading.SimpleDefaultOverloadSet.C", BaseTrust);
        HRESULT STDMETHODCALLTYPE Draw1(int,HSTRING * result) { *result = hs(L"IA.Draw(int) Called"); return S_OK; }
        HRESULT STDMETHODCALLTYPE Draw2(HSTRING,HSTRING * result) { *result = hs(L"IA.Draw(HSTRING) Called"); return S_OK; }
    };
    
    [uuid("ace1598b-c74b-437e-8b17-0a80ad3fade9")] 
    class DeprecatedAttributes :
        public Microsoft::WRL::RuntimeClass<
            Winery::Overloading::DeprecatedAttributes::IAmazingInterface,
            Winery::Overloading::DeprecatedAttributes::IExceptionalInterface>
    {
    public:
        InspectableClass(L"Winery.Overloading.DeprecatedAttributes.FantasticClass", BaseTrust);
        HRESULT STDMETHODCALLTYPE AmazingMethod( int ) { return S_OK; }
        HRESULT STDMETHODCALLTYPE ExceptionalMethod(HSTRING ) {  return S_OK; }
        HRESULT STDMETHODCALLTYPE put_ExceptionalProp(HSTRING ) {return S_OK; }
        HRESULT STDMETHODCALLTYPE get_ExceptionalProp(HSTRING *exceptionallyMeaningless) { *exceptionallyMeaningless = hs(L"ExceptionalProp Called");  return S_OK; }
    };

    [uuid("FB4E7945-7E56-4FBD-AF10-5C3C09024877")]
    class AsyncMethodStatics :
        public Microsoft::WRL::Implements<
            Winery::IAsyncMethodStatics
        >
    {
    public:
        AsyncMethodStatics() {}
        ~AsyncMethodStatics() {}

        // IAsyncMethodStatics::AsyncOperationOutStatic
        HRESULT STDMETHODCALLTYPE AsyncOperationOutStatic(ICustomAsync ** asyncOp)
        { 
            auto instance = Microsoft::WRL::Make<CustomAsyncInfo>();
            instance->Initialize();
            *asyncOp =  instance.Get();
            (*asyncOp)->AddRef();
            return S_OK; 
        }

        // IAsyncMethodStatics::AsyncOperationOutStaticNotFastPath
        HRESULT STDMETHODCALLTYPE AsyncOperationOutStaticNotFastPath(double num1, double num2, double num3, ICustomAsync ** asyncOp)
        {
            // Ignore input params, these are just here to make sure we don't fastpath the method.
            num1++; num2++; num3++;
            auto instance = Microsoft::WRL::Make<CustomAsyncInfo>();
            instance->Initialize();
            *asyncOp =  instance.Get();
            (*asyncOp)->AddRef();
            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE GetHiddenHandler(__out Windows::Foundation::ITypedEventHandler<Winery::ISimpleHiddenObject*, Winery::ISimpleHiddenObject*> **handler)
        {
            UNREFERENCED_PARAMETER(handler);
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE GetVisibleHandler(__out Windows::Foundation::ITypedEventHandler<IInspectable*, IInspectable*> **handler)
        {
            UNREFERENCED_PARAMETER(handler);
            return S_OK;
        }
    };

    [uuid("02A755AF-A55F-4c63-8A52-7A427083A1D8")] 
    class WineryServer :
        public Microsoft::WRL::RuntimeClass<
            Winery::IProductionLine, 
            Winery::IWarehouse,
            Winery::IWineRetail,
            Winery::IRetail,
            Winery::IGeneralShop,
            Winery::INameTopologies,
            Winery::IAsyncMethods,
            Winery::IAllowForWebAsyncMethods
        >
    {
        InspectableClass(L"Winery.RWinery", BaseTrust);

    public:
        WineryServer();
        ~WineryServer();

        // DevX & UEX Coding standards require Hungarian notation. WinRT
        // API guidelines prohibit it. This is because the hungarian type
        // prefixes may be misleading when mapped into different languages. 
        // The way to resolve this is to specify non-prefixed names in the 
        // ReXML or IDL files, but use hungarian parameter names in your 
        // server implementation files.

        // IGeneralShop::get_ShopArea
        IFACEMETHOD(get_ShopArea)(__out Area *pArea) override;
                
        // IGeneralShop::put_ShopArea
        IFACEMETHOD(put_ShopArea)(Area area) override;

        // IGeneralShop::get_ShopArea
        IFACEMETHOD(get_ShopDimension)(__out Dimension *pDim) override;
                
        // IGeneralShop::put_ShopArea
        IFACEMETHOD(put_ShopDimension)(Dimension dim) override;

        // IGeneralShop::CompareAreaOfNeighbors
        //IFACEMETHOD(CompareAreaOfNeighbors)(
        //	Windows::Foundation::Collections::IVector<Winery::IGeneralShop*> *neighbors,
        //	__out double *largest) override;

        // IGeneralShop::ShopName
        IFACEMETHOD(get_ShopName)(__out HSTRING *msg) override;

        // IGeneralShop::ShopName
        IFACEMETHOD(put_ShopName)(HSTRING msg) override;

        // IGeneralShop::NeighborShopName
        //IFACEMETHOD(NeighborShopName)(
        //	Windows::Foundation::Collections::IVector<Winery::IGeneralShop*> *neighbors,
        //	__out Windows::Foundation::Collections::IMap<HSTRING, Winery::IGeneralShop*> **namesMappings) override;

        // IProductionLine::add_AgeCompleteEvent
        IFACEMETHOD(add_AgeEmptyEvent)( 
            Winery::IAgeCompleteHandler *handler,
            __out EventRegistrationToken *pCookie) override;

        // IProductionLine::remove_AgeCompleteEvent
        IFACEMETHOD(remove_AgeEmptyEvent)(EventRegistrationToken iCookie) override;

        // IProductionLine::add_AgeCompleteEvent
        IFACEMETHOD(add_AgeCompleteEvent)( 
            Winery::IAgeCompleteHandler *handler,
            __out EventRegistrationToken *pCookie) override;

        // IProductionLine::remove_AgeCompleteEvent
        IFACEMETHOD(remove_AgeCompleteEvent)(EventRegistrationToken iCookie) override;

        // IProductionLine::SendToWarehouse
        IFACEMETHOD(SendToWarehouse)(Winery::IWarehouse *pWarehouse) override;

        //IProductionLine::Produce
        IFACEMETHOD(Produce)() override;

        //[overload("ThrowVinegar")]
        // IWarehouse::ThrowSelectedVinegar
        IFACEMETHOD(ThrowSelectedVinegar)(
            __in Winery::IRandomIntGenerator *randomGenerator,
            __in Winery::Reds wineType,
            __out int *thrown) override;

        // [overload("ThrowVinegar")]
        // IWarehouse::ThrowZinfandelVinegar
        IFACEMETHOD(ThrowZinfandelVinegar)(
            __in Winery::IRandomIntGenerator *randomGenerator,
            __out int *thrown) override;

        // IWarehouse::ClearWarehouse
        IFACEMETHOD (ClearWarehouse)() override;

        // IWarehouse::StoreAgedWine
        IFACEMETHOD(StoreAgedWine)() override;

        // IWarehouse::WineInStorage
        IFACEMETHOD(get_WineInStorage)(__out int *val) override;

        // IRetail::WelcomeMessage
        IFACEMETHOD(put_WelcomeMessage)(HSTRING msg) override;

        // IRetail::WelcomeMessage
        IFACEMETHOD(get_WelcomeMessage)(__out HSTRING *msg) override;

        // IWineRetail::InitDatabase
        IFACEMETHOD(InitDatabase)() override;

        // IWineRetail::SellReds
        IFACEMETHOD(SellReds)(
            __in Winery::Reds wineType,
            __in int amount) override;

        // IWineRetail::SellWhites
        IFACEMETHOD(SellWhites)(
            __in Winery::Whites wineType,
            __in int amount) override;

        // IWineRetail::SellSweets
        IFACEMETHOD(SellSweets)(
            __in Winery::Sweets wineType,
            __in int amount) override;

        // IWineRetail::GetBestSellingRed()
        IFACEMETHOD(GetBestSellingRed)(__out Winery::Reds *pVal) override;

        // IWineRetail::GetBestSellingWhite()
        IFACEMETHOD(GetBestSellingWhite)(__out Winery::Whites *pVal) override;

        // IWineRetail::GetBestSellingSweet()
        IFACEMETHOD(GetBestSellingSweet)(__out Winery::Sweets *pVal) override;


        IFACEMETHOD(MarshalIRetail)(__in IRetail * _in, __out IRetail ** _out) override;
        IFACEMETHOD(MarshalIGeneralShop)(__in IGeneralShop * _in, __out IGeneralShop ** _out) override;
        IFACEMETHOD(MarshalIProductionLine)(__in IProductionLine * _in, __out IProductionLine ** _out) override;
        IFACEMETHOD(MarshalIWineRetail)(__in IWineRetail * _in, __out IWineRetail ** _out) override;
        IFACEMETHOD(MarshalIWarehouse)(__in IWarehouse * _in, __out IWarehouse ** _out) override;


        HRESULT STDMETHODCALLTYPE GetSimpleConflict(Winery::Overloading::SimpleConflict::IB ** c) 
        { 
            auto instance = Microsoft::WRL::Make<SimpleConflict>();
            *c =  instance.Get();
            (*c)->AddRef();
            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE GetSimpleOverloadSet(Winery::Overloading::SimpleOverloadSet::IA ** c)
        { 
            auto instance = Microsoft::WRL::Make<SimpleOverloadSet>();
            *c =  instance.Get();
            (*c)->AddRef();
            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE GetInheritedConflict(Winery::Overloading::InheritedConflict::IB ** c)
        { 
            auto instance = Microsoft::WRL::Make<InheritedConflict>();
            *c =  instance.Get();
            (*c)->AddRef();
            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE GetNameConflictingWithOverloadSet(Winery::Overloading::NameConflictingWithOverloadSet::IC ** c)
        { 
            auto instance = Microsoft::WRL::Make<NameConflictingWithOverloadSet>();
            *c =  instance.Get();
            (*c)->AddRef();
            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE GetDiamond(Winery::Overloading::Diamond::ILeft ** c)
        { 
            auto instance = Microsoft::WRL::Make<Diamond>();
            *c =  instance.Get();
            (*c)->AddRef();
            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE GetEnumerableOfDefaultInterface(Winery::IEnumerable::EnumerableOfDefaultInterface::IMethodColl ** c)
        { 
            auto instance = Microsoft::WRL::Make<Winery::IEnumerable::EnumerableOfDefaultInterface::EnumerableOfDefaultInterfaceAccessServer>();
            *c =  instance.Get();
            (*c)->AddRef();
            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE GetEnumerableOfDefaultInterfaceWithMultipleSameName(Winery::IEnumerable::EnumerableOfDefaultInterfaceWithMultipleSameName::IMethodColl ** c)
        { 
            auto instance = Microsoft::WRL::Make<Winery::IEnumerable::EnumerableOfDefaultInterfaceWithMultipleSameName::EnumerableOfDefaultInterfaceAccessServer>();
            *c =  instance.Get();
            (*c)->AddRef();
            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE GetEnumerableOfItself(Winery::IEnumerable::EnumerableOfItself::IMethod ** c)
        { 
            auto instance = Microsoft::WRL::Make<Winery::IEnumerable::EnumerableOfItself::EnumerableOfDefaultInterfaceAccessServer>();
            *c =  instance.Get();
            (*c)->AddRef();
            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE GetEnumerableOfItselfAsRTC(Winery::IEnumerable::EnumerableOfItselfAsRTC::IMethod ** c)
        { 
            auto instance = Microsoft::WRL::Make<Winery::IEnumerable::EnumerableOfItselfAsRTC::EnumerableOfDefaultInterfaceAccessServer>();
            *c =  instance.Get();
            (*c)->AddRef();
            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE GetSimpleDefaultOverloadSet(Winery::Overloading::SimpleDefaultOverloadSet::IA ** c)
        { 
            auto instance = Microsoft::WRL::Make<SimpleDefaultOverloadSet>();
            *c =  instance.Get();
            (*c)->AddRef();
            return S_OK; 
        }

// ----
        HRESULT STDMETHODCALLTYPE GetSimpleConflictAsInterface(Winery::Overloading::SimpleConflict::IB ** c) 
        { 
            auto instance = Microsoft::WRL::Make<SimpleConflict>();
            *c =  instance.Get();
            (*c)->AddRef();
            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE GetSimpleOverloadSetAsInterface(Winery::Overloading::SimpleOverloadSet::IA ** c)
        { 
            auto instance = Microsoft::WRL::Make<SimpleOverloadSet>();
            *c =  instance.Get();
            (*c)->AddRef();
            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE GetInheritedConflictAsInterface(Winery::Overloading::InheritedConflict::IB ** c)
        { 
            auto instance = Microsoft::WRL::Make<InheritedConflict>();
            *c =  instance.Get();
            (*c)->AddRef();
            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE GetNameConflictingWithOverloadSetAsInterface(Winery::Overloading::NameConflictingWithOverloadSet::IB ** c)
        { 
            auto instance = Microsoft::WRL::Make<NameConflictingWithOverloadSet>();
            *c =  instance.Get();
            (*c)->AddRef();
            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE GetDiamondAsInterface(Winery::Overloading::Diamond::IRoot ** c)
        { 
            auto instance = Microsoft::WRL::Make<Diamond>();
            *c =  instance.Get();
            (*c)->AddRef();
            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE GetSimpleDefaultOverloadSetAsInterface(Winery::Overloading::SimpleDefaultOverloadSet::IA ** c)
        { 
            auto instance = Microsoft::WRL::Make<SimpleDefaultOverloadSet>();
            *c =  instance.Get();
            (*c)->AddRef();
            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE AsyncOperationOut(ICustomAsync ** asyncOp)
        { 
            auto instance = Microsoft::WRL::Make<CustomAsyncInfo>();
            instance->Initialize();
            *asyncOp =  instance.Get();
            (*asyncOp)->AddRef();
            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE GetDeprecatedAttributes(Winery::Overloading::DeprecatedAttributes::IExceptionalInterface** c)
        {
            auto instance = Microsoft::WRL::Make<DeprecatedAttributes>();
            *c = instance.Get();
            (*c)->AddRef();
            return S_OK;
        }


        HRESULT STDMETHODCALLTYPE AsyncOperationOutAfterExecuteDelegate(ISimpleDelegate * simpleDelegate, ICustomAsync ** asyncOp)
        {
            simpleDelegate->Invoke(1);

            auto instance = Microsoft::WRL::Make<CustomAsyncInfo>();
            instance->Initialize();
            *asyncOp =  instance.Get();
            (*asyncOp)->AddRef();
            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE AsyncOperationViaDelegate(ISimpleDelegateWithAsyncInParameter * simpleDelegateWithAsyncInParameter)
        {
            auto instance = Microsoft::WRL::Make<CustomAsyncInfo>();
            instance->Initialize();
            ICustomAsync * asyncOp = instance.Get();
            asyncOp->AddRef();

            simpleDelegateWithAsyncInParameter->Invoke(asyncOp);

            asyncOp->Release();
            
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE AsyncOperationOutAfterExecuteDelegateWithAsyncInParameter(ISimpleDelegateWithAsyncInParameter * simpleDelegateWithAsyncInParameter, ICustomAsync ** asyncOp)
        {
            auto instance = Microsoft::WRL::Make<CustomAsyncInfo>();
            instance->Initialize();
            *asyncOp = instance.Get();
            (*asyncOp)->AddRef();

            instance = Microsoft::WRL::Make<CustomAsyncInfo>();
            instance->Initialize();
            ICustomAsync * asyncOp2 = instance.Get();
            asyncOp2->AddRef();

            simpleDelegateWithAsyncInParameter->Invoke(asyncOp2);

            asyncOp2->Release();

            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE AsyncOperationOutAfterExecuteDelegateWithAsyncInParameterUseSameAsyncObject(ISimpleDelegateWithAsyncInParameter * simpleDelegateWithAsyncInParameter, ICustomAsync ** asyncOp)
        {
            auto instance = Microsoft::WRL::Make<CustomAsyncInfo>();
            instance->Initialize();
            *asyncOp = instance.Get();
            (*asyncOp)->AddRef();

            simpleDelegateWithAsyncInParameter->Invoke(*asyncOp);

            return S_OK; 
        }

        HRESULT STDMETHODCALLTYPE AsyncOperationWithMultipleAsyncOutParameters(ICustomAsync ** asyncOp1, ICustomAsync ** asyncOp2)
        {
            auto instance = Microsoft::WRL::Make<CustomAsyncInfo>();
            instance->Initialize();
            *asyncOp1 = instance.Get();
            (*asyncOp1)->AddRef();

            instance = Microsoft::WRL::Make<CustomAsyncInfo>();
            instance->Initialize();
            *asyncOp2 = instance.Get();
            (*asyncOp2)->AddRef();

            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE AsyncOperationWithMultipleOutParameters(int * num, ICustomAsync ** asyncOp)
        {
            auto instance = Microsoft::WRL::Make<CustomAsyncInfo>();
            instance->Initialize();
            *asyncOp = instance.Get();
            (*asyncOp)->AddRef();

            *num = 0;

            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE AllowForWebAsyncOperationOut(ICustomAsync ** asyncOp)
        { 
            auto instance = Microsoft::WRL::Make<AllowForWebCustomAsyncInfo>();
            instance->Initialize();
            *asyncOp =  instance.Get();
            (*asyncOp)->AddRef();
            return S_OK;
        }

    private:
        int _numAgedWine;
            
        int _redSold[4];
        int _whiteSold[3];
        int _sweetSold[3];
            
        Winery::Reds _topRed;
        Winery::Whites _topWhite;
        Winery::Sweets _topSweet;

        HSTRING _shopMsg;
        HSTRING _shopName;

        Area _shopArea;
        Dimension _shopDim;

        Microsoft::WRL::EventSource<IAgeCompleteHandler> _evtAgeComplete;
    };

    // The activatable class macro must be defined *inside* the same namespace
    // as the class that it's describing, and the server name must be specified
    // as the unqualified name. This is a limitation of the macro mechanics used
    // to make the server activatable.

    [uuid("63109BE8-1A17-4abc-9F19-AF4A3AA7AC1B")]
    class WineryFactory :
        public Microsoft::WRL::ActivationFactory<IWineryFactory, Microsoft::WRL::Implements<Winery::AsyncMethodStatics> >
    {
    public:
        // IWineryFactory::CreateWinery
        IFACEMETHOD(CreateWinery)(__in int val, __deref_out IGeneralShop **ppWinery);

        // IWineryFactory::ActivateInstance
        IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable);
    };



}
