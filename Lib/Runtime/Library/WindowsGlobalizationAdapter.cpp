//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "strsafe.h"

//Messy hack to get rid of the new/delete operators; as we still need implements.h
#define __PLACEMENT_NEW_INLINE
#include <wrl\implements.h>
#undef __PLACEMENT_NEW_INLINE

//#include <wrl\wrappers\corewrappers.h>


#ifdef ENABLE_INTL_OBJECT

using namespace Windows::Globalization;

#define IfFailThrowHr(op) \
    if (FAILED(hr=(op))) \
    { \
        JavascriptError::MapAndThrowError(scriptContext, hr); \
    } \

#define IfNullReturnError(EXPR, ERROR) do { if (!(EXPR)) { return (ERROR); } } while(FALSE)
#define IfFailedReturn(EXPR) do { hr = (EXPR); if (FAILED(hr)) { return hr; }} while(FALSE)
#define IfFailedGoLabel(expr, label) if (FAILED(expr)) { goto label; }
#define IfFailedGo(expr) IfFailedGoLabel(expr, LReturn)

//The "helper" methods below are to resolve external symbol references to our delay-loaded libraries.
HRESULT WindowsCreateString(__in_ecount_opt(length) const WCHAR * sourceString, UINT32 length, __out HSTRING * string)
{
    return ThreadContext::GetContextForCurrentThread()->GetWindowsGlobalizationLibrary()->WindowsCreateString(sourceString, length, string);
}

HRESULT WindowsCreateStringReference(__in_ecount_opt(length+1) const WCHAR * sourceString, UINT32 length, __out HSTRING_HEADER * header, __out HSTRING * string)
{
    return ThreadContext::GetContextForCurrentThread()->GetWindowsGlobalizationLibrary()->WindowsCreateStringReference(sourceString, length, header, string);
}

HRESULT WindowsDeleteString(HSTRING string)
{
    return ThreadContext::GetContextForCurrentThread()->GetWindowsGlobalizationLibrary()->WindowsDeleteString(string);
}

PCWSTR WindowsGetStringRawBuffer(HSTRING string, __out_opt UINT32 * length)
{
    return ThreadContext::GetContextForCurrentThread()->GetWindowsGlobalizationLibrary()->WindowsGetStringRawBuffer(string, length);
}

HRESULT WindowsCompareStringOrdinal(HSTRING string1, HSTRING string2, __out INT32 * result)
{
    return ThreadContext::GetContextForCurrentThread()->GetWindowsGlobalizationLibrary()->WindowsCompareStringOrdinal(string1, string2, result);
}

HRESULT WindowsDuplicateString(HSTRING original, __out HSTRING *newString)
{
    return ThreadContext::GetContextForCurrentThread()->GetWindowsGlobalizationLibrary()->WindowsDuplicateString(original, newString);
}

namespace Js
{


    class HSTRINGIterator : public Microsoft::WRL::RuntimeClass<Windows::Foundation::Collections::IIterator<HSTRING>>
    {

        HSTRING *items;
        uint32 length;
        boolean hasMore;
        uint32 currentPosition;

    public:
        HRESULT RuntimeClassInitialize(HSTRING *items, uint32 length)
        {
            this->items = items;
            this->currentPosition = 0;
            this->length = length;
            this->hasMore = currentPosition < this->length;

            return S_OK;
        }
        ~HSTRINGIterator()
        {
        }

        // IIterator
        IFACEMETHODIMP get_Current(_Out_ HSTRING *current)
        {
            if(hasMore && current != nullptr)
            {
                return WindowsDuplicateString(items[currentPosition], current);
            }
            return S_OK;
        }

        IFACEMETHODIMP get_HasCurrent(_Out_ boolean *hasCurrent)
        {
            if(hasCurrent != nullptr)
                *hasCurrent = hasMore;
            return S_OK;
        }

        IFACEMETHODIMP MoveNext(_Out_ boolean *hasCurrent) sealed
        {
            this->currentPosition++;

            this->hasMore = this->currentPosition < this->length;
            if(hasCurrent != nullptr) *hasCurrent = hasMore;

            return S_OK;
        }

        IFACEMETHODIMP GetMany(_In_ unsigned capacity,
                               _Out_writes_to_(capacity,*actual) HSTRING *value,
                               _Out_ unsigned *actual)
        {
            uint count = 0;
            while (this->hasMore)
            {
                if (count == capacity)
                {
                    break;
                }
                if(value != nullptr) get_Current(value + count);

                count ++;
                this->MoveNext(nullptr);
            }
            if(actual != nullptr) *actual = count;

            return S_OK;
        }
        IFACEMETHOD(GetRuntimeClassName)(_Out_ HSTRING* runtimeName) sealed
        {
            *runtimeName = nullptr;
            HRESULT hr = S_OK;
            const wchar_t *name = L"Js.HSTRINGIterator";

            hr = WindowsCreateString(name, static_cast<UINT32>(wcslen(name)), runtimeName);
            return hr;
        }
        IFACEMETHOD(GetTrustLevel)(_Out_ TrustLevel* trustLvl)
        {
            *trustLvl = BaseTrust;
            return S_OK;
        }
        IFACEMETHOD(GetIids)(_Out_ ULONG *iidCount, _Outptr_result_buffer_(*iidCount) IID **)
        {
            iidCount;
            return E_NOTIMPL;
        }
    };

    class HSTRINGIterable : public Microsoft::WRL::RuntimeClass<Windows::Foundation::Collections::IIterable<HSTRING>>
    {

        HSTRING *items;
        uint32 length;

    public:
        HRESULT RuntimeClassInitialize(HSTRING *string, uint32 length)
        {
            this->items = new HSTRING[length];

            if (this->items == nullptr)
            {
                return E_OUTOFMEMORY;
            }

            for(uint32 i = 0; i < length; i++)
            {
                this->items[i] = string[i];
            }
            this->length = length;

            return S_OK;
        }

        ~HSTRINGIterable()
        {
            if(this->items != nullptr)
            {
                delete [] items;
            }
        }

        IFACEMETHODIMP First(_Outptr_result_maybenull_ Windows::Foundation::Collections::IIterator<HSTRING> **first)
        {
            return Microsoft::WRL::MakeAndInitialize<HSTRINGIterator>(first, this->items, this->length);
        }

        IFACEMETHOD(GetRuntimeClassName)(_Out_ HSTRING* runtimeName) sealed
        {
            *runtimeName = nullptr;
            HRESULT hr = S_OK;
            // Return type that does not exist in metadata
            const wchar_t *name = L"Js.HSTRINGIterable";
            hr = WindowsCreateString(name, static_cast<UINT32>(wcslen(name)), runtimeName);
            return hr;
        }
        IFACEMETHOD(GetTrustLevel)(_Out_ TrustLevel* trustLvl)
        {
            *trustLvl = BaseTrust;
            return S_OK;
        }
        IFACEMETHOD(GetIids)(_Out_ ULONG *iidCount, _Outptr_result_buffer_(*iidCount) IID **)
        {
            iidCount;
            return E_NOTIMPL;
        }
    };

    __inline DelayLoadWindowsGlobalization* WindowsGlobalizationAdapter::GetWindowsGlobalizationLibrary(_In_ ScriptContext* scriptContext)
    {
        return this->GetWindowsGlobalizationLibrary(scriptContext->GetThreadContext());
    }

    __inline DelayLoadWindowsGlobalization* WindowsGlobalizationAdapter::GetWindowsGlobalizationLibrary(_In_ ThreadContext* threadContext)
    {
        return threadContext->GetWindowsGlobalizationLibrary();
    }

    template<typename T>
    HRESULT WindowsGlobalizationAdapter::GetActivationFactory(ScriptContext *scriptContext, LPCWSTR factoryName, T** instance)
    {
        return this->GetActivationFactory<T>(this->GetWindowsGlobalizationLibrary(scriptContext), factoryName, instance);
    }

    template<typename T>
    HRESULT WindowsGlobalizationAdapter::GetActivationFactory(ThreadContext *threadContext, LPCWSTR factoryName, T** instance)
    {
        return this->GetActivationFactory<T>(this->GetWindowsGlobalizationLibrary(threadContext), factoryName, instance);
    }

    template<typename T>
    HRESULT WindowsGlobalizationAdapter::GetActivationFactory(DelayLoadWindowsGlobalization *delayLoadLibrary, LPCWSTR factoryName, T** instance)
    {
        *instance = nullptr;

        AutoCOMPtr<IActivationFactory> factory;
        HSTRING hString;
        HSTRING_HEADER hStringHdr;
        HRESULT hr;

        IfFailedReturn(delayLoadLibrary->WindowsCreateStringReference(factoryName, wcslen(factoryName), &hStringHdr, &hString));
        IfFailedReturn(delayLoadLibrary->DllGetActivationFactory(hString, &factory));

        return factory->QueryInterface(__uuidof(T), reinterpret_cast<void**>(instance));
    }

    HRESULT WindowsGlobalizationAdapter::EnsureInitialized(ScriptContext *scriptContext)
    {
        return this->EnsureInitialized(this->GetWindowsGlobalizationLibrary(scriptContext), scriptContext->GetConfig()->IsES6UnicodeExtensionsEnabled());
    }

    HRESULT WindowsGlobalizationAdapter::EnsureInitialized(ThreadContext *threadContext, bool isES6Mode)
    {
        return this->EnsureInitialized(this->GetWindowsGlobalizationLibrary(threadContext), isES6Mode);
    }
    
    HRESULT WindowsGlobalizationAdapter::EnsureInitialized(DelayLoadWindowsGlobalization *library, bool isES6Mode)
    {
        HRESULT hr = S_OK;

        if (initialized)
        {
            return hr;
        }
        else if (failedToInitialize)
        {
            return S_FALSE;
        }

        failedToInitialize = true;

        IfFailedReturn(GetActivationFactory(library, RuntimeClass_Windows_Globalization_Language, &languageFactory));
        IfFailedReturn(GetActivationFactory(library, RuntimeClass_Windows_Globalization_Language, &languageStatics));
        IfFailedReturn(GetActivationFactory(library, RuntimeClass_Windows_Globalization_NumberFormatting_CurrencyFormatter, &currencyFormatterFactory));
        IfFailedReturn(GetActivationFactory(library, RuntimeClass_Windows_Globalization_NumberFormatting_DecimalFormatter, &decimalFormatterFactory));
        IfFailedReturn(GetActivationFactory(library, RuntimeClass_Windows_Globalization_NumberFormatting_PercentFormatter, &percentFormatterFactory));
        IfFailedReturn(GetActivationFactory(library, RuntimeClass_Windows_Globalization_DateTimeFormatting_DateTimeFormatter, &dateTimeFormatterFactory));
        IfFailedReturn(GetActivationFactory(library, RuntimeClass_Windows_Globalization_NumberFormatting_SignificantDigitsNumberRounder, &significantDigitsRounderActivationFactory));
        IfFailedReturn(GetActivationFactory(library, RuntimeClass_Windows_Globalization_NumberFormatting_IncrementNumberRounder, &incrementNumberRounderActivationFactory));
        if(isES6Mode)
        {
            IfFailedReturn(GetActivationFactory(library, RuntimeClass_Windows_Data_Text_UnicodeCharacters, &unicodeStatics));
        }

        failedToInitialize = false;
        initialized = true;

        return hr;
    }


    HRESULT WindowsGlobalizationAdapter::CreateLanguage(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR languageTag, ILanguage** language)
    {
        HRESULT hr = S_OK;
        HSTRING hString;
        HSTRING_HEADER hStringHdr;
        IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(languageTag, wcslen(languageTag), &hStringHdr, &hString));
        IfFailedReturn(this->languageFactory->CreateLanguage(hString, language));
        return hr;
    }

    boolean WindowsGlobalizationAdapter::IsWellFormedLanguageTag(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR languageTag)
    {
        boolean retVal;
        HRESULT hr;
        HSTRING hString;
        HSTRING_HEADER hStringHdr;
        IfFailThrowHr(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(languageTag, wcslen(languageTag), &hStringHdr, &hString));
        IfFailThrowHr(this->languageStatics->IsWellFormed(hString, &retVal));
        return retVal;
    }

    HRESULT WindowsGlobalizationAdapter::NormalizeLanguageTag(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR languageTag, HSTRING *result)
    {
        HRESULT hr;

        AutoCOMPtr<ILanguage> language;
        IfFailedReturn(CreateLanguage(scriptContext, languageTag, &language));

        IfFailedReturn(language->get_LanguageTag(result));
        return hr;
    }

    HRESULT WindowsGlobalizationAdapter::CreateCurrencyFormatterCode(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR currencyCode, NumberFormatting::ICurrencyFormatter** currencyFormatter)
    {
        HRESULT hr;
        HSTRING hString;
        HSTRING_HEADER hStringHdr;
        IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(currencyCode, wcslen(currencyCode), &hStringHdr, &hString));
        IfFailedReturn(this->currencyFormatterFactory->CreateCurrencyFormatterCode(hString, currencyFormatter));
        return hr;
    }

    HRESULT WindowsGlobalizationAdapter::CreateCurrencyFormatter(_In_ ScriptContext* scriptContext, PCWSTR* localeStrings, uint32 numLocaleStrings, _In_z_ PCWSTR currencyCode, NumberFormatting::ICurrencyFormatter** currencyFormatter)
    {
        HRESULT hr;
        HSTRING hString;
        HSTRING_HEADER hStringHdr;

        AutoArrayPtr<HSTRING> arr(HeapNewArray(HSTRING, numLocaleStrings), numLocaleStrings);
        AutoArrayPtr<HSTRING_HEADER> headers(HeapNewArray(HSTRING_HEADER, numLocaleStrings), numLocaleStrings);
        for(uint32 i = 0; i< numLocaleStrings; i++)
        {
            IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(localeStrings[i],  wcslen(localeStrings[i]), (headers + i), (arr + i)));
        }

        Microsoft::WRL::ComPtr<Windows::Foundation::Collections::IIterable<HSTRING>> languages(nullptr);
        IfFailedReturn(Microsoft::WRL::MakeAndInitialize<HSTRINGIterable>(&languages, arr, numLocaleStrings));

        HSTRING geoString;
        HSTRING_HEADER geoStringHeader;
        IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(L"ZZ", 2, &geoStringHeader, &geoString));

        IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(currencyCode, wcslen(currencyCode), &hStringHdr, &hString));
        IfFailedReturn(this->currencyFormatterFactory->CreateCurrencyFormatterCodeContext(hString, languages.Get(), geoString, currencyFormatter));
        return hr;
    }

    HRESULT WindowsGlobalizationAdapter::CreateNumberFormatter(_In_ ScriptContext* scriptContext, PCWSTR* localeStrings, uint32 numLocaleStrings, NumberFormatting::INumberFormatter** numberFormatter)
    {
        HRESULT hr = S_OK;

        AutoArrayPtr<HSTRING> arr(HeapNewArray(HSTRING, numLocaleStrings), numLocaleStrings);
        AutoArrayPtr<HSTRING_HEADER> headers(HeapNewArray(HSTRING_HEADER, numLocaleStrings), numLocaleStrings);
        for(uint32 i = 0; i< numLocaleStrings; i++)
        {
            IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(localeStrings[i],  wcslen(localeStrings[i]), (headers + i), (arr + i)));
        }

        Microsoft::WRL::ComPtr<Windows::Foundation::Collections::IIterable<HSTRING>> languages(nullptr);
        IfFailedReturn(Microsoft::WRL::MakeAndInitialize<HSTRINGIterable>(&languages, arr, numLocaleStrings));

        HSTRING geoString;
        HSTRING_HEADER geoStringHeader;
        IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(L"ZZ", 2, &geoStringHeader, &geoString));

        IfFailedReturn(this->decimalFormatterFactory->CreateDecimalFormatter(languages.Get(), geoString, numberFormatter));
        return hr;
    }

    HRESULT WindowsGlobalizationAdapter::CreatePercentFormatter(_In_ ScriptContext* scriptContext, PCWSTR* localeStrings, uint32 numLocaleStrings, NumberFormatting::INumberFormatter** numberFormatter)
    {
        HRESULT hr = S_OK;

        AutoArrayPtr<HSTRING> arr(HeapNewArray(HSTRING, numLocaleStrings), numLocaleStrings);
        AutoArrayPtr<HSTRING_HEADER> headers(HeapNewArray(HSTRING_HEADER, numLocaleStrings), numLocaleStrings);
        for(uint32 i = 0; i< numLocaleStrings; i++)
        {
            IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(localeStrings[i],  wcslen(localeStrings[i]), (headers + i), (arr + i)));
        }

        Microsoft::WRL::ComPtr<Windows::Foundation::Collections::IIterable<HSTRING>> languages(nullptr);
        IfFailedReturn(Microsoft::WRL::MakeAndInitialize<HSTRINGIterable>(&languages, arr, numLocaleStrings));

        HSTRING geoString;
        HSTRING_HEADER geoStringHeader;
        IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(L"ZZ", 2, &geoStringHeader, &geoString));

        IfFailedReturn(this->percentFormatterFactory->CreatePercentFormatter(languages.Get(), geoString, numberFormatter));

        return hr;
    }

    HRESULT WindowsGlobalizationAdapter::CreateDateTimeFormatter(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR formatString, _In_z_ PCWSTR* localeStrings, uint32 numLocaleStrings,
        _In_z_ PCWSTR calendar, _In_z_ PCWSTR clock, __out DateTimeFormatting::IDateTimeFormatter** result)
    {
        HRESULT hr = S_OK;

        if(numLocaleStrings == 0) return E_INVALIDARG;

        Assert((calendar == nullptr && clock == nullptr) || (calendar != nullptr && clock != nullptr));

        HSTRING fsHString;
        HSTRING_HEADER fsHStringHdr;

        IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(formatString, wcslen(formatString), &fsHStringHdr, &fsHString));

        AutoArrayPtr<HSTRING> arr(HeapNewArray(HSTRING, numLocaleStrings), numLocaleStrings);
        AutoArrayPtr<HSTRING_HEADER> headers(HeapNewArray(HSTRING_HEADER, numLocaleStrings), numLocaleStrings);
        for(uint32 i = 0; i< numLocaleStrings; i++)
        {
            IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(localeStrings[i],  wcslen(localeStrings[i]), (headers + i), (arr + i)));
        }

        Microsoft::WRL::ComPtr<Windows::Foundation::Collections::IIterable<HSTRING>> languages(nullptr);
        IfFailedReturn(Microsoft::WRL::MakeAndInitialize<HSTRINGIterable>(&languages, arr, numLocaleStrings));

        if(clock == nullptr)
        {
            IfFailedReturn(this->dateTimeFormatterFactory->CreateDateTimeFormatterLanguages(fsHString, languages.Get(), result));
        }
        else
        {
            HSTRING geoString;
            HSTRING_HEADER geoStringHeader;
            HSTRING calString;
            HSTRING_HEADER calStringHeader;
            HSTRING clockString;
            HSTRING_HEADER clockStringHeader;

            IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(L"ZZ", 2, &geoStringHeader, &geoString));
            IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(calendar, wcslen(calendar), &calStringHeader, &calString));
            IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(clock, wcslen(clock), &clockStringHeader, &clockString));
            IfFailedReturn(this->dateTimeFormatterFactory->CreateDateTimeFormatterContext(fsHString, languages.Get(), geoString, calString, clockString, result));
        }
        return hr;
    }

    HRESULT WindowsGlobalizationAdapter::CreateIncrementNumberRounder(_In_ ScriptContext* scriptContext, NumberFormatting::INumberRounder** numberRounder)
    {
        return incrementNumberRounderActivationFactory->ActivateInstance(reinterpret_cast<IInspectable**>(numberRounder));
    }

    HRESULT WindowsGlobalizationAdapter::CreateSignificantDigitsRounder(_In_ ScriptContext* scriptContext, NumberFormatting::INumberRounder** numberRounder)
    {
        return significantDigitsRounderActivationFactory->ActivateInstance(reinterpret_cast<IInspectable**>(numberRounder));
    }

}
#endif

