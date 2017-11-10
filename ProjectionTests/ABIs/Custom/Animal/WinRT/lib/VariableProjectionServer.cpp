#include "stdafx.h"
#include "VariableProjectionServer.h"

namespace Animals
{
    namespace VariableProjection
    {
        /*
         * DelegateWithMissingAndPartialInterfaceClassInParameter
         */
        DELEGATE_IMPL(DelegateWithMissingAndPartialInterfaceClassInParameter, MissingConstructs::IMissingInterface* value)
        {
            if (value != nullptr)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }

        /*
         * DelegateWithMissingTypeOutParameter
         */
        DELEGATE_IMPL2(DelegateWithMissingTypeOutParameter, MissingConstructs::IMissingInterface ** value, MissingConstructs::IMissingInterface **returnValue)
        {
            if (value != nullptr || returnValue != nullptr) //We expect nullptr
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }

        DELEGATE_IMPL2(DelegateWithPartialStructOutParameter, PartialStruct* value, PartialStruct* returnValue)
        {
            if (value != nullptr && returnValue != nullptr)
            {
                return E_FAIL;
            }

            return E_INVALIDARG;
        }

        DELEGATE_IMPL2(DelegateWithMissingEnumOutParameter, MissingConstructs::MissingEnum* outValue, MissingConstructs::MissingEnum* retValue)
        {
            if (outValue != nullptr || retValue != nullptr) //We expect nullptr
            {
                return E_FAIL;
            }
            return E_INVALIDARG;
        }

        DELEGATE_IMPL(DelegateWithMissingEnumInParameter, MissingConstructs::MissingEnum value)
        {
            if (value == 0)
            {
                return E_FAIL;
            }

            return E_FAIL;
        }

        DELEGATE_IMPL2(DelegateWithMissingBoolOutParameter, MissingConstructs::MissingBool* outValue, MissingConstructs::MissingBool* retValue)
        {
            if (outValue == nullptr || retValue == nullptr) //We expect nullptr
            {
                return E_INVALIDARG;
            }
            *outValue = true;
            *retValue = false;
            return true;;
        }

        DELEGATE_IMPL(DelegateWithMissingBoolInParameter, MissingConstructs::MissingBool value)
        {
            return value == (MissingConstructs::MissingBool)true || value == (MissingConstructs::MissingBool)false ? S_OK : E_INVALIDARG;
        }

        DELEGATE_IMPL(MissingDelegate, MissingConstructs::IMissingInterface * value)
        {
            if (value == nullptr)
            {
                return S_OK;
            }

            return E_FAIL;
        }

        DELEGATE_IMPL(DelegateWithMissingInterfaceInParameter, MissingConstructs::IMissingInterface* value)
        {
            if (value == nullptr)
            {
                return E_INVALIDARG;
            }

            return value->None();
        }

        /*
         * MissingAndPartialInterfaceClass
         */
        MissingAndPartialInterfaceClass::MissingAndPartialInterfaceClass()
        {

        }

        MissingAndPartialInterfaceClass::~MissingAndPartialInterfaceClass()
        {

        }

        //IMissingInterface implementation
        IFACEMETHODIMP MissingAndPartialInterfaceClass::None()
        {
            return S_OK;
        }

        //IPartialInterface implementation
        IFACEMETHODIMP MissingAndPartialInterfaceClass::PartialStructMethod(PartialStruct value)
        {
            if (value.Value1 != 0)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }

        IFACEMETHODIMP MissingAndPartialInterfaceClass::Add(int a, int b, int *c)
        {
            if (c == nullptr)
            {
                return E_INVALIDARG;
            }
            *c = a + b + 42;
            return S_OK;
        }


        IFACEMETHODIMP MissingAndPartialInterfaceClass::get_MissingTypeProp(MissingConstructs::IMissingInterface **value)
        {
            if (value == nullptr)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }

        IFACEMETHODIMP MissingAndPartialInterfaceClass::put_MissingTypeProp(MissingConstructs::IMissingInterface* value)
        {
            if (value == nullptr)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }


        IFACEMETHODIMP MissingAndPartialInterfaceClass::get_MissingInterfaceProp(MissingConstructs::IMissingInterface** value)
        {
            if (value == nullptr)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }

        IFACEMETHODIMP MissingAndPartialInterfaceClass::put_MissingInterfaceProp(MissingConstructs::IMissingInterface* value)
        {
            if (value == nullptr)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }

        /*
         * PartialAndMissingInterfaceClass
         */

        PartialAndMissingInterfaceClass::PartialAndMissingInterfaceClass()
        {

        }

        PartialAndMissingInterfaceClass::~PartialAndMissingInterfaceClass()
        {

        }

        //IMissingInterface implementation
        IFACEMETHODIMP PartialAndMissingInterfaceClass::None()
        {
            return S_OK;
        }

        //IPartialInterface implementation
        IFACEMETHODIMP PartialAndMissingInterfaceClass::PartialStructMethod(PartialStruct value)
        {
            if (value.Value1 != 0)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }

        IFACEMETHODIMP PartialAndMissingInterfaceClass::Add(int a, int b, int *c)
        {
            if (c == nullptr)
            {
                return E_INVALIDARG;
            }
            *c = a + b + 42;
            return S_OK;
        }


        IFACEMETHODIMP PartialAndMissingInterfaceClass::get_MissingTypeProp(MissingConstructs::IMissingInterface **value)
        {
            if (value == nullptr)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }

        IFACEMETHODIMP PartialAndMissingInterfaceClass::put_MissingTypeProp(MissingConstructs::IMissingInterface* value)
        {
            if (value == nullptr)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }


        IFACEMETHODIMP PartialAndMissingInterfaceClass::get_MissingInterfaceProp(MissingConstructs::IMissingInterface** value)
        {
            if (value == nullptr)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }

        IFACEMETHODIMP PartialAndMissingInterfaceClass::put_MissingInterfaceProp(MissingConstructs::IMissingInterface* value)
        {
            if (value == nullptr)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }

        /*
         * ExtendsInterfaceClass
         */
        ExtendsInterfaceClass::ExtendsInterfaceClass()
        {

        }

        ExtendsInterfaceClass::~ExtendsInterfaceClass()
        {

        }

        //IExtendsMissingInterface
        IFACEMETHODIMP ExtendsInterfaceClass::Subtract(int a, int b, int *c)
        {
            if (c == nullptr)
            {
                return E_INVALIDARG;
            }
            *c = a - b + 42;
            return S_OK;
        }

        //IExtendsPartialInterface
        IFACEMETHODIMP ExtendsInterfaceClass::Divide(int a, int b, int *c)
        {
            if (c == nullptr)
            {
                return E_INVALIDARG;
            }
            *c = a / b + 42;
            return S_OK;
        }

        //IMissingInterface implementation
        IFACEMETHODIMP ExtendsInterfaceClass::None()
        {
            return S_OK;
        }

        //IPartialInterface implementation
        IFACEMETHODIMP ExtendsInterfaceClass::PartialStructMethod(PartialStruct value)
        {
            if (value.Value1 != 0)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }

        IFACEMETHODIMP ExtendsInterfaceClass::Add(int a, int b, int *c)
        {
            if (c == nullptr)
            {
                return E_INVALIDARG;
            }
            *c = a + b + 42;
            return S_OK;
        }


        IFACEMETHODIMP ExtendsInterfaceClass::get_MissingTypeProp(MissingConstructs::IMissingInterface **value)
        {
            if (value == nullptr)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }

        IFACEMETHODIMP ExtendsInterfaceClass::put_MissingTypeProp(MissingConstructs::IMissingInterface* value)
        {
            if (value == nullptr)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }


        IFACEMETHODIMP ExtendsInterfaceClass::get_MissingInterfaceProp(MissingConstructs::IMissingInterface** value)
        {
            if (value == nullptr)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }

        IFACEMETHODIMP ExtendsInterfaceClass::put_MissingInterfaceProp(MissingConstructs::IMissingInterface* value)
        {
            if (value == nullptr)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }

        /*
         * ExtendsReverseInterfaceClass
         */ 

        ExtendsReverseInterfaceClass::ExtendsReverseInterfaceClass()
        {

        }

        ExtendsReverseInterfaceClass::~ExtendsReverseInterfaceClass()
        {

        }

        //IExtendsMissingInterface
        IFACEMETHODIMP ExtendsReverseInterfaceClass::Subtract(int a, int b, int *c)
        {
            if (c == nullptr)
            {
                return E_INVALIDARG;
            }
            *c = a - b + 42;
            return S_OK;
        }

        //IExtendsPartialInterface
        IFACEMETHODIMP ExtendsReverseInterfaceClass::Divide(int a, int b, int *c)
        {
            if (c == nullptr)
            {
                return E_INVALIDARG;
            }
            *c = a / b + 42;
            return S_OK;
        }

        //IMissingInterface implementation
        IFACEMETHODIMP ExtendsReverseInterfaceClass::None()
        {
            return S_OK;
        }

        //IPartialInterface implementation
        IFACEMETHODIMP ExtendsReverseInterfaceClass::PartialStructMethod(PartialStruct value)
        {
            if (value.Value1 != 0)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }

        IFACEMETHODIMP ExtendsReverseInterfaceClass::Add(int a, int b, int *c)
        {
            if (c == nullptr)
            {
                return E_INVALIDARG;
            }
            *c = a + b + 42;
            return S_OK;
        }


        IFACEMETHODIMP ExtendsReverseInterfaceClass::get_MissingTypeProp(MissingConstructs::IMissingInterface **value)
        {
            if (value == nullptr)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }

        IFACEMETHODIMP ExtendsReverseInterfaceClass::put_MissingTypeProp(MissingConstructs::IMissingInterface* value)
        {
            if (value == nullptr)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }


        IFACEMETHODIMP ExtendsReverseInterfaceClass::get_MissingInterfaceProp(MissingConstructs::IMissingInterface** value)
        {
            if (value == nullptr)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }

        IFACEMETHODIMP ExtendsReverseInterfaceClass::put_MissingInterfaceProp(MissingConstructs::IMissingInterface* value)
        {
            if (value == nullptr)
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }

        /*
         * TestingClass
         */
        TestingClass::TestingClass()
        {

        }

        TestingClass::~TestingClass()
        {

        }

        //ITestRuntimeClasses
#define TEST_TYPE_MISSING(Name, Type) \
    IFACEMETHODIMP TestingClass::Check##Name(Type value) \
        { \
        void * addr = &value; \
        return addr != nullptr ? E_FAIL : E_INVALIDARG; \
        } \
        IFACEMETHODIMP TestingClass::Check##Name##Out(Type* value) \
        { \
        return value != nullptr ? E_FAIL : E_INVALIDARG; \
        } \
        IFACEMETHODIMP TestingClass::get_##Name##Prop(Type* value) \
        { \
        return value != nullptr ? E_FAIL : E_INVALIDARG; \
        } \
        IFACEMETHODIMP TestingClass::put_##Name##Prop(Type value) \
        { \
        void * addr = &value; \
        return addr != nullptr ? E_FAIL : E_INVALIDARG; \
        }

#define TEST_TYPE_MISSING_EVENT(Name, Type) \
    IFACEMETHODIMP TestingClass::add_##Name##Event(IDelegateWith##Name##InParameter* clickHandler, EventRegistrationToken * eventCookie) \
        { \
        return event##Name.Add(clickHandler, eventCookie); \
        } \
        IFACEMETHODIMP TestingClass::remove_##Name##Event(EventRegistrationToken eventCookie) \
        { \
        return event##Name.Remove(eventCookie); \
        }

#define TEST_TYPE_RESOLVED_EVENT(Name, Type, OutValueExpr) \
    IFACEMETHODIMP TestingClass::add_##Name##Event(IDelegateWith##Name##InParameter* clickHandler, EventRegistrationToken * eventCookie) \
        { \
        if (FAILED(event##Name.Add(clickHandler, eventCookie))) \
        { \
        return E_FAIL; \
        } \
        Type valueBacking; \
        Type *value = &valueBacking; \
        OutValueExpr; \
        return event##Name.InvokeAll(valueBacking); \
        } \
        IFACEMETHODIMP TestingClass::remove_##Name##Event(EventRegistrationToken eventCookie) \
        { \
        return event##Name.Remove(eventCookie); \
        }

#define TEST_TYPE_RESOLVED(Name, Type, OutValueExpr) TEST_TYPE_RESOLVED_CHECK(Name, Type, OutValueExpr, return &value != nullptr ? S_OK : E_INVALIDARG)

#define TEST_TYPE_RESOLVED_CHECK(Name, Type, OutValueExpr, CheckExpr) \
    IFACEMETHODIMP TestingClass::Check##Name(Type value) \
        { \
            CheckExpr; \
        } \
        IFACEMETHODIMP TestingClass::Check##Name##Out(Type* value) \
        { \
        if (value == nullptr) \
        { \
        return E_INVALIDARG; \
        } \
        OutValueExpr; \
        return S_OK; \
        } \
        IFACEMETHODIMP TestingClass::get_##Name##Prop(Type* value) \
        { \
        return value != nullptr ? S_OK : E_INVALIDARG; \
        } \
        IFACEMETHODIMP TestingClass::put_##Name##Prop(Type value) \
        { \
        void * addr = &value; \
        return addr != nullptr ? S_OK : E_INVALIDARG; \
        }
        

        TEST_TYPE_MISSING(MissingStruct, MissingConstructs::MissingStruct)
        TEST_TYPE_MISSING_EVENT(MissingStruct, MissingConstructs::MissingStruct)

        IFACEMETHODIMP TestingClass::CheckMissingStructByRef(const MissingConstructs::MissingStruct* value)
        {
            return value != nullptr ? S_OK : E_INVALIDARG;
        }


        TEST_TYPE_MISSING(PartialStruct, PartialStruct)
        TEST_TYPE_MISSING_EVENT(PartialStruct, PartialStruct)

        IFACEMETHODIMP TestingClass::CheckPartialStructByRef(const PartialStruct* value)
        {
            return value != nullptr ? S_OK : E_INVALIDARG;
        }


        TEST_TYPE_MISSING(PartialStructWithEnum, PartialStructWithEnum)
        TEST_TYPE_MISSING_EVENT(PartialStructWithEnum, PartialStructWithEnum)

        IFACEMETHODIMP TestingClass::CheckPartialStructWithEnumByRef(const PartialStructWithEnum* value)
        {
            return value != nullptr ? S_OK : E_INVALIDARG;
        }

        TEST_TYPE_RESOLVED(MissingBool, MissingConstructs::MissingBool, *value = true)
        TEST_TYPE_RESOLVED_EVENT(MissingBool, MissingConstructs::MissingBool, *value = true)

        TEST_TYPE_MISSING(MissingEnum, MissingConstructs::MissingEnum)
        TEST_TYPE_MISSING_EVENT(MissingEnum, MissingConstructs::MissingEnum)
                
        TEST_TYPE_RESOLVED(PartialAndMissingInterfaceClass, IPartialInterface*, Details::MakeAndInitialize<PartialAndMissingInterfaceClass>(value))
        TEST_TYPE_RESOLVED_EVENT(PartialAndMissingInterfaceClass, IPartialInterface*, Details::MakeAndInitialize<PartialAndMissingInterfaceClass>(value))

        TEST_TYPE_MISSING(MissingAndPartialInterfaceClass, MissingConstructs::IMissingInterface*)
        TEST_TYPE_RESOLVED_EVENT(MissingAndPartialInterfaceClass, MissingConstructs::IMissingInterface*, Details::MakeAndInitialize<MissingAndPartialInterfaceClass>(value))

        TEST_TYPE_RESOLVED(PartialInterface, IPartialInterface*, Details::MakeAndInitialize<PartialAndMissingInterfaceClass>(value))
        TEST_TYPE_RESOLVED_EVENT(PartialInterface, IPartialInterface*, Details::MakeAndInitialize<PartialAndMissingInterfaceClass>(value))

        TEST_TYPE_MISSING(MissingInterface, MissingConstructs::IMissingInterface*)
        TEST_TYPE_RESOLVED_EVENT(MissingInterface, MissingConstructs::IMissingInterface*, Details::MakeAndInitialize<MissingAndPartialInterfaceClass>(value))
        
        TEST_TYPE_RESOLVED(ExtendsPartialInterface, IExtendsPartialInterface*, Details::MakeAndInitialize<ExtendsInterfaceClass>(value))
        TEST_TYPE_RESOLVED_EVENT(ExtendsPartialInterface, IExtendsPartialInterface*, Details::MakeAndInitialize<ExtendsInterfaceClass>(value))
        
        TEST_TYPE_RESOLVED(ExtendsMissingInterface, IExtendsMissingInterface*, Details::MakeAndInitialize<ExtendsReverseInterfaceClass>(value))
        TEST_TYPE_RESOLVED_EVENT(ExtendsMissingInterface, IExtendsMissingInterface*, Details::MakeAndInitialize<ExtendsReverseInterfaceClass>(value))

        TEST_TYPE_RESOLVED(ExtendsInterfaceClass, IExtendsPartialInterface*, Details::MakeAndInitialize<ExtendsInterfaceClass>(value))
        TEST_TYPE_RESOLVED_EVENT(ExtendsInterfaceClass, IExtendsPartialInterface*, Details::MakeAndInitialize<ExtendsInterfaceClass>(value))

        TEST_TYPE_RESOLVED(ExtendsReverseInterfaceClass, IExtendsMissingInterface*, Details::MakeAndInitialize<ExtendsReverseInterfaceClass>(value))
        TEST_TYPE_RESOLVED_EVENT(ExtendsReverseInterfaceClass, IExtendsMissingInterface*, Details::MakeAndInitialize<ExtendsReverseInterfaceClass>(value))

        template <typename T, typename IT>
        void MakeDelegate(IT **value)
        {
            ComPtr<T> cRetDelegate = Make<T>();
            cRetDelegate.CopyTo(value);
        }
        TEST_TYPE_MISSING(MissingDelegate, MissingConstructs::IMissingDelegate*);
        TEST_TYPE_RESOLVED_EVENT(MissingDelegate, MissingConstructs::IMissingDelegate*, MakeDelegate<MissingDelegate COMMA MissingConstructs::IMissingDelegate>(value));

        HRESULT InvokeIDelegateWithMissingInterfaceInParameter(IDelegateWithMissingInterfaceInParameter* del)
        {
            MissingConstructs::IMissingInterface* value = nullptr;
            Details::MakeAndInitialize<MissingAndPartialInterfaceClass>(&value);
            return del->Invoke(value);
        }

        TEST_TYPE_RESOLVED_CHECK(DelegateWithMissingInterfaceInParameter, IDelegateWithMissingInterfaceInParameter*, MakeDelegate<DelegateWithMissingInterfaceInParameter COMMA IDelegateWithMissingInterfaceInParameter>(value), return InvokeIDelegateWithMissingInterfaceInParameter(value));

        //ITestDelegates Members
        IFACEMETHODIMP TestingClass::TestDelegateWithMissingBoolOutParameter(IDelegateWithMissingBoolOutParameter* inDelegate, IDelegateWithMissingBoolOutParameter** outDelegate, IDelegateWithMissingBoolOutParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
            inDelegate->AddRef();
            ComPtr<DelegateWithMissingBoolOutParameter> cRetDelegate = Make<DelegateWithMissingBoolOutParameter>();
            cRetDelegate.CopyTo(retDelegate);

            return S_OK;
        }
        IFACEMETHODIMP TestingClass::TestDelegateWithMissingBoolInParameter(IDelegateWithMissingBoolInParameter* inDelegate, IDelegateWithMissingBoolInParameter** outDelegate, IDelegateWithMissingBoolInParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
            inDelegate->AddRef();
            ComPtr<DelegateWithMissingBoolInParameter> cRetDelegate = Make<DelegateWithMissingBoolInParameter>();
            cRetDelegate.CopyTo(retDelegate);

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithMissingEnumOutParameter(IDelegateWithMissingEnumOutParameter* inDelegate, IDelegateWithMissingEnumOutParameter** outDelegate, IDelegateWithMissingEnumOutParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
            inDelegate->AddRef();
            ComPtr<DelegateWithMissingEnumOutParameter> cRetDelegate = Make<DelegateWithMissingEnumOutParameter>();
            cRetDelegate.CopyTo(retDelegate);

            return S_OK;
        }
        IFACEMETHODIMP TestingClass::TestDelegateWithMissingEnumInParameter(IDelegateWithMissingEnumInParameter* inDelegate, IDelegateWithMissingEnumInParameter** outDelegate, IDelegateWithMissingEnumInParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
            inDelegate->AddRef();
            ComPtr<DelegateWithMissingEnumInParameter> cRetDelegate = Make<DelegateWithMissingEnumInParameter>();
            cRetDelegate.CopyTo(retDelegate);

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithMissingTypeOutParameter(IDelegateWithMissingTypeOutParameter* inDelegate, IDelegateWithMissingTypeOutParameter** outDelegate, IDelegateWithMissingTypeOutParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
			inDelegate->AddRef();
            ComPtr<DelegateWithMissingTypeOutParameter> cRetDelegate = Make<DelegateWithMissingTypeOutParameter>();
            cRetDelegate.CopyTo(retDelegate);

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithMissingTypeInParameter(IDelegateWithMissingTypeInParameter* inDelegate, IDelegateWithMissingTypeInParameter** outDelegate, IDelegateWithMissingTypeInParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
			inDelegate->AddRef();
            *retDelegate = inDelegate;
			inDelegate->AddRef();

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithPartialStructOutParameter(IDelegateWithPartialStructOutParameter* inDelegate, IDelegateWithPartialStructOutParameter** outDelegate, IDelegateWithPartialStructOutParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
			inDelegate->AddRef();
            ComPtr<DelegateWithPartialStructOutParameter> cRetDelegate = Make<DelegateWithPartialStructOutParameter>();
            cRetDelegate.CopyTo(retDelegate);

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithPartialStructInParameter(IDelegateWithPartialStructInParameter* inDelegate, IDelegateWithPartialStructInParameter** outDelegate, IDelegateWithPartialStructInParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
			inDelegate->AddRef();
            *retDelegate = inDelegate;
			inDelegate->AddRef();

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithPartialStructInRefParameter(IDelegateWithPartialStructInRefParameter* inDelegate, IDelegateWithPartialStructInRefParameter** outDelegate, IDelegateWithPartialStructInRefParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
			inDelegate->AddRef();
            *retDelegate = inDelegate;
			inDelegate->AddRef();

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithPartialInterfaceOutParameter(IDelegateWithPartialInterfaceOutParameter* inDelegate, IDelegateWithPartialInterfaceOutParameter** outDelegate, IDelegateWithPartialInterfaceOutParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
			inDelegate->AddRef();
            *retDelegate = inDelegate;
			inDelegate->AddRef();

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithPartialInterfaceInParameter(IDelegateWithPartialInterfaceInParameter* inDelegate, IDelegateWithPartialInterfaceInParameter** outDelegate, IDelegateWithPartialInterfaceInParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
			inDelegate->AddRef();
            *retDelegate = inDelegate;
			inDelegate->AddRef();

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithExtendsPartialInterfaceOutParameter(IDelegateWithExtendsPartialInterfaceOutParameter* inDelegate, IDelegateWithExtendsPartialInterfaceOutParameter** outDelegate, IDelegateWithExtendsPartialInterfaceOutParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
			inDelegate->AddRef();
            *retDelegate = inDelegate;
			inDelegate->AddRef();

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithExtendsPartialInterfaceInParameter(IDelegateWithExtendsPartialInterfaceInParameter* inDelegate, IDelegateWithExtendsPartialInterfaceInParameter** outDelegate, IDelegateWithExtendsPartialInterfaceInParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
			inDelegate->AddRef();
            *retDelegate = inDelegate;
			inDelegate->AddRef();

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithExtendsMissingInterfaceOutParameter(IDelegateWithExtendsMissingInterfaceOutParameter* inDelegate, IDelegateWithExtendsMissingInterfaceOutParameter** outDelegate, IDelegateWithExtendsMissingInterfaceOutParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
			inDelegate->AddRef();
            *retDelegate = inDelegate;
			inDelegate->AddRef();

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithExtendsMissingInterfaceInParameter(IDelegateWithExtendsMissingInterfaceInParameter* inDelegate, IDelegateWithExtendsMissingInterfaceInParameter** outDelegate, IDelegateWithExtendsMissingInterfaceInParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
			inDelegate->AddRef();
            *retDelegate = inDelegate;
			inDelegate->AddRef();

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithPartialAndMissingInterfaceClassOutParameter(IDelegateWithPartialAndMissingInterfaceClassOutParameter* inDelegate, IDelegateWithPartialAndMissingInterfaceClassOutParameter** outDelegate, IDelegateWithPartialAndMissingInterfaceClassOutParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
			inDelegate->AddRef();
            *retDelegate = inDelegate;
			inDelegate->AddRef();

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithPartialAndMissingInterfaceClassInParameter(IDelegateWithPartialAndMissingInterfaceClassInParameter* inDelegate, IDelegateWithPartialAndMissingInterfaceClassInParameter** outDelegate, IDelegateWithPartialAndMissingInterfaceClassInParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
			inDelegate->AddRef();
            *retDelegate = inDelegate;
			inDelegate->AddRef();

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithMissingAndPartialInterfaceClassOutParameter(IDelegateWithMissingAndPartialInterfaceClassOutParameter* inDelegate, IDelegateWithMissingAndPartialInterfaceClassOutParameter** outDelegate, IDelegateWithMissingAndPartialInterfaceClassOutParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
			inDelegate->AddRef();
            *retDelegate = inDelegate;
			inDelegate->AddRef();

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithMissingAndPartialInterfaceClassInParameter(IDelegateWithMissingAndPartialInterfaceClassInParameter* inDelegate, IDelegateWithMissingAndPartialInterfaceClassInParameter** outDelegate, IDelegateWithMissingAndPartialInterfaceClassInParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
			inDelegate->AddRef();
            ComPtr<DelegateWithMissingAndPartialInterfaceClassInParameter> cRetDelegate = Make<DelegateWithMissingAndPartialInterfaceClassInParameter>();
            cRetDelegate.CopyTo(retDelegate);

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithExtendsInterfaceClassOutParameter(IDelegateWithExtendsInterfaceClassOutParameter* inDelegate, IDelegateWithExtendsInterfaceClassOutParameter** outDelegate, IDelegateWithExtendsInterfaceClassOutParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
			inDelegate->AddRef();
            *retDelegate = inDelegate;
			inDelegate->AddRef();

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithExtendsInterfaceClassInParameter(IDelegateWithExtendsInterfaceClassInParameter* inDelegate, IDelegateWithExtendsInterfaceClassInParameter** outDelegate, IDelegateWithExtendsInterfaceClassInParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
			inDelegate->AddRef();
            *retDelegate = inDelegate;
			inDelegate->AddRef();

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithExtendsReverseInterfaceClassOutParameter(IDelegateWithExtendsReverseInterfaceClassOutParameter* inDelegate, IDelegateWithExtendsReverseInterfaceClassOutParameter** outDelegate, IDelegateWithExtendsReverseInterfaceClassOutParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
			inDelegate->AddRef();
            *retDelegate = inDelegate;
			inDelegate->AddRef();

            return S_OK;
        }

        IFACEMETHODIMP TestingClass::TestDelegateWithExtendsReverseInterfaceClassInParameter(IDelegateWithExtendsReverseInterfaceClassInParameter* inDelegate, IDelegateWithExtendsReverseInterfaceClassInParameter** outDelegate, IDelegateWithExtendsReverseInterfaceClassInParameter** retDelegate)
        {
            if (inDelegate == nullptr)
            {
                return E_INVALIDARG;
            }

            *outDelegate = inDelegate;
			inDelegate->AddRef();
            *retDelegate = inDelegate;
			inDelegate->AddRef();

            return S_OK;
        }
    }
}