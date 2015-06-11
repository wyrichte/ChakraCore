
#include "runtime.h"

using namespace Js;

class DynamicProfileMutatorImpl : public DynamicProfileMutator
{
public:
    virtual void Mutate(DynamicProfileInfo * info) override;
    virtual void Delete() { delete this; }
    virtual void Initialize(const wchar_t * options) override;
    DynamicProfileMutatorImpl(): m_elem(-1), m_return(-1), m_param(-1), m_loopImplicitFlag(-1), m_implicitFlag(-1), m_random(false) {}
private:
    static const size_t MaxProfiledValueTypes = 256;
    static ValueType s_profiledValueTypes[MaxProfiledValueTypes];
    static size_t s_profiledValueTypeCount;

    static void EnsureProfiledValueTypes();
    static bool IsProfiledValueType(const ValueType valueType);
    static ValueType RandomValueType(ScriptContext *const scriptContext);
    static int FromValueType(const ValueType valueType);
    static ValueType ToValueType(const int i);
    void ConvertStrToValueType(wchar_t * enumStr, int *param);

    void ConvertStrToImplicitCallFlags(wchar_t * enumStr, int *param);
    // Can't make this as enum because we need to know if no option is passed, Constructor sets these to -1
    int m_elem;
    int m_return;
    int m_param;
    int m_loopImplicitFlag;
    int m_implicitFlag;
    bool m_random;
};

ValueType DynamicProfileMutatorImpl::s_profiledValueTypes[MaxProfiledValueTypes];
size_t DynamicProfileMutatorImpl::s_profiledValueTypeCount = 0;

void DynamicProfileMutatorImpl::EnsureProfiledValueTypes()
{
    if(s_profiledValueTypeCount)
    {
        return;
    }

    ValueType::MapInitialIndefiniteValueTypesUntil([](const ValueType valueType, const size_t i) -> bool
    {
        Assert(s_profiledValueTypeCount == i);
        if(s_profiledValueTypeCount >= MaxProfiledValueTypes)
        {
            Assert(false); // need to increase the array size
            Js::Throw::FatalInternalError();
        }

        s_profiledValueTypes[s_profiledValueTypeCount++] = valueType;
        return false;
    });
}

bool DynamicProfileMutatorImpl::IsProfiledValueType(const ValueType valueType)
{
    EnsureProfiledValueTypes();
    for(size_t i = 0; i < s_profiledValueTypeCount; ++i)
    {
        if(s_profiledValueTypes[i] == valueType)
        {
            return true;
        }
    }
    return false;
}

ValueType DynamicProfileMutatorImpl::RandomValueType(ScriptContext *const scriptContext)
{
    EnsureProfiledValueTypes();
    return s_profiledValueTypes[static_cast<size_t>(JavascriptMath::Random(scriptContext) * s_profiledValueTypeCount)];
}

int DynamicProfileMutatorImpl::FromValueType(const ValueType valueType)
{
    CompileAssert(sizeof(ValueType) == sizeof(uint16));
    return static_cast<int>(reinterpret_cast<const uint16 &>(valueType));
}

ValueType DynamicProfileMutatorImpl::ToValueType(const int i)
{
    CompileAssert(sizeof(ValueType) == sizeof(uint16));
    Assert(i != -1);
    const uint16 ui16 = static_cast<uint16>(i);
    return reinterpret_cast<const ValueType &>(ui16);
}

void DynamicProfileMutatorImpl::Mutate(DynamicProfileInfo * info)
{
    Js::FunctionBody * functionBody = info->functionBody;

    if (true == m_random) {
        ScriptContext *scriptContext = functionBody->GetScriptContext();
        for (uint i = 0; i < functionBody->GetProfiledElemCount(); i++)
        {
            info->elemInfo[i] = RandomValueType(scriptContext);
        }
        for (uint i = 0; i < functionBody->GetProfiledReturnTypeCount(); i++)
        {
            info->returnTypeInfo[i] = RandomValueType(scriptContext);
        }
        for (uint i = 0; i < functionBody->GetProfiledInParamsCount(); i++)
        {
            info->parameterInfo[i] = RandomValueType(scriptContext);
        }
        for (uint i = 0; i < functionBody->GetLoopCount(); i++)
        {
            double randomNum = JavascriptMath::Random(scriptContext);
            info->loopImplicitCallFlags[i] = (ImplicitCallFlags)(BYTE)(randomNum * (ImplicitCall_AsyncHostOperation + 1));
        }
        double randomNum = JavascriptMath::Random(scriptContext);
        info->implicitCallFlags = (ImplicitCallFlags)(BYTE)(randomNum * (ImplicitCall_AsyncHostOperation + 1));
    }
    else {
        // Elements
        if(-1 != m_elem) {
            for (uint i = 0; i < functionBody->GetProfiledElemCount(); i++)
            {
                info->elemInfo[i] = ToValueType(m_elem);
            }   
        }

        // Return
        if(-1 != m_return) {
            for (uint i = 0; i < functionBody->GetProfiledReturnTypeCount(); i++)
            {
                info->returnTypeInfo[i] = ToValueType(m_return);
            }   
        }

        // Params
        if(-1 != m_param) {
            for (uint i = 0; i < functionBody->GetProfiledInParamsCount(); i++)
            {
                info->parameterInfo[i] = ToValueType(m_param);
            }   
        }

        // Loop Implicit Flags
        if(-1 != m_loopImplicitFlag) {
            for (uint i = 0; i < functionBody->GetLoopCount(); i++)
            {
                info->loopImplicitCallFlags[i] = (ImplicitCallFlags)m_loopImplicitFlag;
            }   
        }

        // Implicit Flags
        if(-1 != m_implicitFlag) {
            info->implicitCallFlags = (ImplicitCallFlags)m_implicitFlag;
        }
    }
}


DynamicProfileMutator * CREATE_MUTATOR_PROC_NAME()
{
    return new DynamicProfileMutatorImpl;
}


void DynamicProfileMutatorImpl::Initialize(const wchar_t * options) {
    if(NULL == options) {
        Output::Print(L"ERROR: Options must be specified with -dynamicprofilemutator switch\n");
        Js::Throw::FatalInternalError();
    }
    // Need to copy the const string to a buffer to split
    wchar_t optionsBuffer[2048];
    wcscpy_s(optionsBuffer, wcslen(options) + 1, options);
    wchar_t *p = optionsBuffer;
    wchar_t *nextOption = NULL;
    p = wcstok_s(optionsBuffer, L";",&nextOption);

    while(p != NULL) {
        wchar_t *optionType = p;
        wchar_t *optionValue = NULL;

        optionType = wcstok_s(optionType, L"=",&optionValue);
        if(0 == wcscmp(optionType, L"random")) {
            m_random = true;
            break;
        }
        if(0 == wcscmp(optionType, L"elem")) {
            ConvertStrToValueType(optionValue, &m_elem);
        } else if(0 == wcscmp(optionType, L"return")) {
            ConvertStrToValueType(optionValue, &m_return);
        } else if(0 == wcscmp(optionType, L"param")) {
            ConvertStrToValueType(optionValue, &m_param);
        } else if(0 == wcscmp(optionType, L"loopimplicitflag")) {
            ConvertStrToImplicitCallFlags(optionValue, &m_loopImplicitFlag);
        } else if(0 == wcscmp(optionType, L"implicitflag")) {
            ConvertStrToImplicitCallFlags(optionValue, &m_implicitFlag);
        }
        p = wcstok_s(NULL, L";",&nextOption);
    }
    if((-1 == m_elem) && (-1 == m_return) && (-1 == m_param) && (-1 == m_loopImplicitFlag) && (-1 == m_implicitFlag) && (false == m_random)) {
        Output::Print(L"ERROR: Invalid value passed for dynamicprofilemutator:%s\n", options);
        Js::Throw::FatalInternalError();
    }
}

void DynamicProfileMutatorImpl::ConvertStrToValueType(wchar_t * enumStr, int *param) {
    ValueType valueType;
    if(ValueType::FromString(enumStr, &valueType) && IsProfiledValueType(valueType))
    {
        *param = FromValueType(valueType);
    }
    else
    {
        Output::Print(L"ERROR: Invalid enum type %s\n", enumStr);
        Js::Throw::FatalInternalError();
    }
}

void DynamicProfileMutatorImpl::ConvertStrToImplicitCallFlags(wchar_t * enumStr, int *param) {
    wchar_t * implicitCallFlagsEnumStrs[] = { L"ImplicitCall_HasNoInfo", L"ImplicitCall_None", L"ImplicitCall_ToPrimitive",L"ImplicitCall_Accessor", L"ImplicitCall_External", L"ImplicitCall_Exception", L"ImplicitCall_NoOpSetProperty", L"ImplicitCall_All", L"ImplicitCall_AsyncHostOperation"};
    uint impCallFlagsEnumLen = sizeof(implicitCallFlagsEnumStrs)/sizeof(wchar_t *);

    for(uint index = 0; index < impCallFlagsEnumLen; ++index) {
        if(0 == wcscmp(enumStr, implicitCallFlagsEnumStrs[index])) {
            *param = index;
            break;
        }
    }
    if (-1 == *param)
    {
        Output::Print(L"ERROR: Invalid enum type %s\n", enumStr);
        Js::Throw::FatalInternalError();
    }
}
