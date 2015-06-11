//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#ifdef ENABLE_INTL_OBJECT

#include "Windows.Globalization.h"

#define IfFailedReturn(EXPR) do { hr = (EXPR); if (FAILED(hr)) { return hr; }} while(FALSE)

namespace Js
{
    class WindowsGlobalizationAdapter
    {
    private:
        bool initialized;
        bool failedToInitialize;
        AutoCOMPtr<Windows::Globalization::ILanguageFactory> languageFactory;
        AutoCOMPtr<Windows::Globalization::ILanguageStatics> languageStatics;
        AutoCOMPtr<Windows::Globalization::NumberFormatting::ICurrencyFormatterFactory> currencyFormatterFactory;
        AutoCOMPtr<Windows::Globalization::NumberFormatting::IDecimalFormatterFactory> decimalFormatterFactory;
        AutoCOMPtr<Windows::Globalization::NumberFormatting::IPercentFormatterFactory> percentFormatterFactory;
        AutoCOMPtr<Windows::Globalization::DateTimeFormatting::IDateTimeFormatterFactory> dateTimeFormatterFactory;
        AutoCOMPtr<IActivationFactory> incrementNumberRounderActivationFactory;
        AutoCOMPtr<IActivationFactory> significantDigitsRounderActivationFactory;
        AutoCOMPtr<Windows::Data::Text::IUnicodeCharactersStatics> unicodeStatics;

        DelayLoadWindowsGlobalization* GetWindowsGlobalizationLibrary(_In_ ScriptContext* scriptContext);
        DelayLoadWindowsGlobalization* GetWindowsGlobalizationLibrary(_In_ ThreadContext* threadContext);
        
        template <typename T>
        HRESULT GetActivationFactory(ScriptContext *scriptContext, LPCWSTR factoryName, T** instance);
        
        template <typename T>
        HRESULT GetActivationFactory(ThreadContext *threadContext, LPCWSTR factoryName, T** instance);

        template <typename T>
        HRESULT GetActivationFactory(DelayLoadWindowsGlobalization *library, LPCWSTR factoryName, T** instance);

        
    public:
        WindowsGlobalizationAdapter()
            : initialized(false),
            failedToInitialize(false),
            languageFactory(nullptr),
            languageStatics(nullptr),
            currencyFormatterFactory(nullptr),
            decimalFormatterFactory(nullptr),
            percentFormatterFactory(nullptr),
            dateTimeFormatterFactory(nullptr),
            incrementNumberRounderActivationFactory(nullptr),
            significantDigitsRounderActivationFactory(nullptr),
            unicodeStatics(nullptr)
        { }

        HRESULT EnsureInitialized(ScriptContext *scriptContext);
        HRESULT EnsureInitialized(ThreadContext *threadContext, bool isES6Mode);
        HRESULT EnsureInitialized(DelayLoadWindowsGlobalization *library, bool isES6Mode);
        HRESULT CreateLanguage(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR languageTag, Windows::Globalization::ILanguage** language);
        boolean IsWellFormedLanguageTag(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR languageTag);
        HRESULT NormalizeLanguageTag(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR languageTag, HSTRING *result);
        HRESULT CreateCurrencyFormatterCode(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR currencyCode, Windows::Globalization::NumberFormatting::ICurrencyFormatter** currencyFormatter);
        HRESULT CreateCurrencyFormatter(_In_ ScriptContext* scriptContext, PCWSTR* localeStrings, uint32 numLocaleStrings, _In_z_ PCWSTR currencyCode, Windows::Globalization::NumberFormatting::ICurrencyFormatter** currencyFormatter);
        HRESULT CreateNumberFormatter(_In_ ScriptContext* scriptContext, PCWSTR* localeStrings, uint32 numLocaleStrings, Windows::Globalization::NumberFormatting::INumberFormatter** numberFormatter);
        HRESULT CreatePercentFormatter(_In_ ScriptContext* scriptContext, PCWSTR* localeStrings, uint32 numLocaleStrings, Windows::Globalization::NumberFormatting::INumberFormatter** numberFormatter);
        HRESULT CreateDateTimeFormatter(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR formatString, _In_z_ PCWSTR* localeStrings, uint32 numLocaleStrings, 
            _In_z_ PCWSTR calendar, _In_z_ PCWSTR clock, __out Windows::Globalization::DateTimeFormatting::IDateTimeFormatter** formatter);
        HRESULT CreateIncrementNumberRounder(_In_ ScriptContext* scriptContext, Windows::Globalization::NumberFormatting::INumberRounder** numberRounder);
        HRESULT CreateSignificantDigitsRounder(_In_ ScriptContext* scriptContext, Windows::Globalization::NumberFormatting::INumberRounder** numberRounder);

        Windows::Data::Text::IUnicodeCharactersStatics* GetUnicodeStatics()
        {
            return unicodeStatics;
        }
    };
}
#endif