#include "stdafx.h"
#define COMMA ,

#define DELEGATE(T, ARGUMENTS) \
    class T : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, I##T> \
    { \
    public: \
    T(); \
    ~T(); \
    \
    IFACEMETHOD(Invoke)(ARGUMENTS); \
    }

#define DELEGATE_NS(T, NS, ARGUMENTS) \
class T : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, NS::I##T> \
    { \
    public: \
    T(); \
    ~T(); \
    \
    IFACEMETHOD(Invoke)(ARGUMENTS); \
    }

#define DELEGATE_IMPL(T, ARGUMENTS) \
    T::T() \
    { \
    \
    } \
    \
    T::~T() \
    { \
    \
    } \
    \
    IFACEMETHODIMP T::Invoke(ARGUMENTS)

#define DELEGATE2(T, ARG1, ARGUMENTS) DELEGATE(T, ARG1 COMMA ARGUMENTS)
#define DELEGATE_IMPL2(T, ARG1, ARGUMENTS) DELEGATE_IMPL(T, ARG1 COMMA ARGUMENTS)

namespace Animals
{


    namespace VariableProjection
    {

        DELEGATE2(DelegateWithMissingTypeOutParameter, MissingConstructs::IMissingInterface ** outValue, MissingConstructs::IMissingInterface **returnValue);

        DELEGATE2(DelegateWithPartialStructOutParameter, PartialStruct* value, PartialStruct* returnValue);

        DELEGATE(DelegateWithMissingAndPartialInterfaceClassInParameter, MissingConstructs::IMissingInterface* value);

        DELEGATE2(DelegateWithMissingEnumOutParameter, MissingConstructs::MissingEnum* outValue, MissingConstructs::MissingEnum* retValue);

        DELEGATE(DelegateWithMissingEnumInParameter, MissingConstructs::MissingEnum value);

        DELEGATE2(DelegateWithMissingBoolOutParameter, MissingConstructs::MissingBool* outValue, MissingConstructs::MissingBool* retValue);

        DELEGATE(DelegateWithMissingBoolInParameter, MissingConstructs::MissingBool value);

        DELEGATE_NS(MissingDelegate, MissingConstructs, MissingConstructs::IMissingInterface * value);

        DELEGATE(DelegateWithMissingInterfaceInParameter, MissingConstructs::IMissingInterface* value);

        class MissingAndPartialInterfaceClass : public Microsoft::WRL::RuntimeClass<MissingConstructs::IMissingInterface, IPartialInterface>
        {
            InspectableClass(L"Animals.VariableProjection.MissingAndPartialInterfaceClass", BaseTrust);

        public:
            MissingAndPartialInterfaceClass();
            ~MissingAndPartialInterfaceClass();

            //IMissingInterface implementation
            IFACEMETHOD(None)() override;

            //IPartialInterface implementation
            IFACEMETHOD(PartialStructMethod)(PartialStruct value) override;
            IFACEMETHOD(Add)(int a, int b, int *c) override;

            IFACEMETHOD(get_MissingTypeProp)(MissingConstructs::IMissingInterface **value) override;
            IFACEMETHOD(put_MissingTypeProp)(MissingConstructs::IMissingInterface* value) override;

            IFACEMETHOD(get_MissingInterfaceProp)(MissingConstructs::IMissingInterface** value) override;
            IFACEMETHOD(put_MissingInterfaceProp)(MissingConstructs::IMissingInterface* value) override;

        };

        class PartialAndMissingInterfaceClass : public Microsoft::WRL::RuntimeClass<IPartialInterface, MissingConstructs::IMissingInterface>
        {
            InspectableClass(L"Animals.VariableProjection.PartialAndMissingInterfaceClass", BaseTrust);

        public:
            PartialAndMissingInterfaceClass();
            ~PartialAndMissingInterfaceClass();

            //IMissingInterface implementation
            IFACEMETHOD(None)() override;

            //IPartialInterface implementation
            IFACEMETHOD(PartialStructMethod)(PartialStruct value) override;
            IFACEMETHOD(Add)(int a, int b, int *c) override;

            IFACEMETHOD(get_MissingTypeProp)(MissingConstructs::IMissingInterface **value) override;
            IFACEMETHOD(put_MissingTypeProp)(MissingConstructs::IMissingInterface* value) override;

            IFACEMETHOD(get_MissingInterfaceProp)(MissingConstructs::IMissingInterface** value) override;
            IFACEMETHOD(put_MissingInterfaceProp)(MissingConstructs::IMissingInterface* value) override;
        };

        class ExtendsInterfaceClass : public Microsoft::WRL::RuntimeClass<IExtendsPartialInterface, IExtendsMissingInterface, IPartialInterface, MissingConstructs::IMissingInterface>
        {
            InspectableClass(L"Animals.VariableProjection.ExtendsInterfaceClass", BaseTrust);

        public:
            ExtendsInterfaceClass();
            ~ExtendsInterfaceClass();

            //IExtendsMissingInterface
            IFACEMETHOD(Subtract)(int a, int b, int *c);

            //IExtendsPartialInterface
            IFACEMETHOD(Divide)(int a, int b, int *c);

            //IMissingInterface implementation
            IFACEMETHOD(None)() override;

            //IPartialInterface implementation
            IFACEMETHOD(PartialStructMethod)(PartialStruct value) override;
            IFACEMETHOD(Add)(int a, int b, int *c) override;

            IFACEMETHOD(get_MissingTypeProp)(MissingConstructs::IMissingInterface **value) override;
            IFACEMETHOD(put_MissingTypeProp)(MissingConstructs::IMissingInterface* value) override;

            IFACEMETHOD(get_MissingInterfaceProp)(MissingConstructs::IMissingInterface** value) override;
            IFACEMETHOD(put_MissingInterfaceProp)(MissingConstructs::IMissingInterface* value) override;
        };

        class ExtendsReverseInterfaceClass : public Microsoft::WRL::RuntimeClass<IExtendsMissingInterface, IExtendsPartialInterface, IPartialInterface, MissingConstructs::IMissingInterface>
        {
            InspectableClass(L"Animals.VariableProjection.ExtendsReverseInterfaceClass", BaseTrust);

        public:
            ExtendsReverseInterfaceClass();
            ~ExtendsReverseInterfaceClass();

            //IExtendsMissingInterface
            IFACEMETHOD(Subtract)(int a, int b, int *c);

            //IExtendsPartialInterface
            IFACEMETHOD(Divide)(int a, int b, int *c);

            //IMissingInterface implementation
            IFACEMETHOD(None)() override;

            //IPartialInterface implementation
            IFACEMETHOD(PartialStructMethod)(PartialStruct value) override;
            IFACEMETHOD(Add)(int a, int b, int *c) override;

            IFACEMETHOD(get_MissingTypeProp)(MissingConstructs::IMissingInterface **value) override;
            IFACEMETHOD(put_MissingTypeProp)(MissingConstructs::IMissingInterface* value) override;

            IFACEMETHOD(get_MissingInterfaceProp)(MissingConstructs::IMissingInterface** value) override;
            IFACEMETHOD(put_MissingInterfaceProp)(MissingConstructs::IMissingInterface* value) override;
        };

#define TEST_TYPE(Name, Type) \
    TEST_TYPE_CORE(Name, Type) \
    TEST_TYPE_EVENTS(Name, Type) \

#define TEST_TYPE_CORE(Name, Type) \
        IFACEMETHOD(Check##Name)(Type value) override; \
        IFACEMETHOD(Check##Name##Out)(Type* value) override; \
        IFACEMETHOD(get_##Name##Prop)(Type* value) override; \
        IFACEMETHOD(put_##Name##Prop)(Type value) override; \

#define TEST_TYPE_EVENTS(Name, Type) \
    private: \
             Microsoft::WRL::EventSource<IDelegateWith##Name##InParameter> event##Name; \
    public: \
        IFACEMETHOD(add_##Name##Event)(IDelegateWith##Name##InParameter* clickHandler, EventRegistrationToken * eventCookie) override; \
        IFACEMETHOD(remove_##Name##Event)(EventRegistrationToken eventCookie) override


        class TestingClass : public Microsoft::WRL::RuntimeClass<ITestRuntimeClasses, ITestDelegates>
        {
            InspectableClass(L"Animals.VariableProjection.TestingClass", BaseTrust);
        private:
            IExtendsMissingInterface* extendsReverseInterfaceClass;
            MissingConstructs::IMissingInterface *missingAndPartialInterfaceClass;
        public:
            TestingClass();
            ~TestingClass();

            //ITestRuntimeClasses
            TEST_TYPE(MissingStruct, MissingConstructs::MissingStruct);
            IFACEMETHOD(CheckMissingStructByRef)(const MissingConstructs::MissingStruct* value) override;

            TEST_TYPE(PartialStruct, PartialStruct);
            IFACEMETHOD(CheckPartialStructByRef)(const PartialStruct* value) override;

            TEST_TYPE(PartialStructWithEnum, PartialStructWithEnum);
            IFACEMETHOD(CheckPartialStructWithEnumByRef)(const PartialStructWithEnum* value) override;

            TEST_TYPE(MissingBool, MissingConstructs::MissingBool);
            
            TEST_TYPE(MissingEnum, MissingConstructs::MissingEnum);
            
            TEST_TYPE(PartialAndMissingInterfaceClass, IPartialInterface*);
            
            TEST_TYPE(MissingAndPartialInterfaceClass, MissingConstructs::IMissingInterface*);
            
            TEST_TYPE(PartialInterface, IPartialInterface*);
            
            TEST_TYPE(MissingInterface, MissingConstructs::IMissingInterface*);
            
            TEST_TYPE(ExtendsPartialInterface, IExtendsPartialInterface*);
           
            TEST_TYPE(ExtendsMissingInterface, IExtendsMissingInterface*);

            TEST_TYPE(ExtendsInterfaceClass, IExtendsPartialInterface*);
            
            TEST_TYPE(ExtendsReverseInterfaceClass, IExtendsMissingInterface*);
            //TEST_TYPE_CORE(ExtendsReverseInterfaceClass, MissingConstructs::IMissingInterface*); //overload

            TEST_TYPE(MissingDelegate, MissingConstructs::IMissingDelegate*);
            //TEST_TYPE_CORE(MissingDelegate, MissingConstructs::IMissingInterface*); //overload
            
            TEST_TYPE_CORE(DelegateWithMissingInterfaceInParameter, IDelegateWithMissingInterfaceInParameter*);

            //ITestDelegates Members
            IFACEMETHOD(TestDelegateWithMissingBoolOutParameter)(IDelegateWithMissingBoolOutParameter* inDelegate, IDelegateWithMissingBoolOutParameter** outDelegate, IDelegateWithMissingBoolOutParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithMissingBoolInParameter)(IDelegateWithMissingBoolInParameter* inDelegate, IDelegateWithMissingBoolInParameter** outDelegate, IDelegateWithMissingBoolInParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithMissingEnumOutParameter)(IDelegateWithMissingEnumOutParameter* inDelegate, IDelegateWithMissingEnumOutParameter** outDelegate, IDelegateWithMissingEnumOutParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithMissingEnumInParameter)(IDelegateWithMissingEnumInParameter* inDelegate, IDelegateWithMissingEnumInParameter** outDelegate, IDelegateWithMissingEnumInParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithMissingTypeOutParameter)(IDelegateWithMissingTypeOutParameter* inDelegate, IDelegateWithMissingTypeOutParameter** outDelegate, IDelegateWithMissingTypeOutParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithMissingTypeInParameter)(IDelegateWithMissingTypeInParameter* inDelegate, IDelegateWithMissingTypeInParameter** outDelegate, IDelegateWithMissingTypeInParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithPartialStructOutParameter)(IDelegateWithPartialStructOutParameter* inDelegate, IDelegateWithPartialStructOutParameter** outDelegate, IDelegateWithPartialStructOutParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithPartialStructInParameter)(IDelegateWithPartialStructInParameter* inDelegate, IDelegateWithPartialStructInParameter** outDelegate, IDelegateWithPartialStructInParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithPartialStructInRefParameter)(IDelegateWithPartialStructInRefParameter* inDelegate, IDelegateWithPartialStructInRefParameter** outDelegate, IDelegateWithPartialStructInRefParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithPartialInterfaceOutParameter)(IDelegateWithPartialInterfaceOutParameter* inDelegate, IDelegateWithPartialInterfaceOutParameter** outDelegate, IDelegateWithPartialInterfaceOutParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithPartialInterfaceInParameter)(IDelegateWithPartialInterfaceInParameter* inDelegate, IDelegateWithPartialInterfaceInParameter** outDelegate, IDelegateWithPartialInterfaceInParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithExtendsPartialInterfaceOutParameter)(IDelegateWithExtendsPartialInterfaceOutParameter* inDelegate, IDelegateWithExtendsPartialInterfaceOutParameter** outDelegate, IDelegateWithExtendsPartialInterfaceOutParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithExtendsPartialInterfaceInParameter)(IDelegateWithExtendsPartialInterfaceInParameter* inDelegate, IDelegateWithExtendsPartialInterfaceInParameter** outDelegate, IDelegateWithExtendsPartialInterfaceInParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithExtendsMissingInterfaceOutParameter)(IDelegateWithExtendsMissingInterfaceOutParameter* inDelegate, IDelegateWithExtendsMissingInterfaceOutParameter** outDelegate, IDelegateWithExtendsMissingInterfaceOutParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithExtendsMissingInterfaceInParameter)(IDelegateWithExtendsMissingInterfaceInParameter* inDelegate, IDelegateWithExtendsMissingInterfaceInParameter** outDelegate, IDelegateWithExtendsMissingInterfaceInParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithPartialAndMissingInterfaceClassOutParameter)(IDelegateWithPartialAndMissingInterfaceClassOutParameter* inDelegate, IDelegateWithPartialAndMissingInterfaceClassOutParameter** outDelegate, IDelegateWithPartialAndMissingInterfaceClassOutParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithPartialAndMissingInterfaceClassInParameter)(IDelegateWithPartialAndMissingInterfaceClassInParameter* inDelegate, IDelegateWithPartialAndMissingInterfaceClassInParameter** outDelegate, IDelegateWithPartialAndMissingInterfaceClassInParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithMissingAndPartialInterfaceClassOutParameter)(IDelegateWithMissingAndPartialInterfaceClassOutParameter* inDelegate, IDelegateWithMissingAndPartialInterfaceClassOutParameter** outDelegate, IDelegateWithMissingAndPartialInterfaceClassOutParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithMissingAndPartialInterfaceClassInParameter)(IDelegateWithMissingAndPartialInterfaceClassInParameter* inDelegate, IDelegateWithMissingAndPartialInterfaceClassInParameter** outDelegate, IDelegateWithMissingAndPartialInterfaceClassInParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithExtendsInterfaceClassOutParameter)(IDelegateWithExtendsInterfaceClassOutParameter* inDelegate, IDelegateWithExtendsInterfaceClassOutParameter** outDelegate, IDelegateWithExtendsInterfaceClassOutParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithExtendsInterfaceClassInParameter)(IDelegateWithExtendsInterfaceClassInParameter* inDelegate, IDelegateWithExtendsInterfaceClassInParameter** outDelegate, IDelegateWithExtendsInterfaceClassInParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithExtendsReverseInterfaceClassOutParameter)(IDelegateWithExtendsReverseInterfaceClassOutParameter* inDelegate, IDelegateWithExtendsReverseInterfaceClassOutParameter** outDelegate, IDelegateWithExtendsReverseInterfaceClassOutParameter** retDelegate) override;
            IFACEMETHOD(TestDelegateWithExtendsReverseInterfaceClassInParameter)(IDelegateWithExtendsReverseInterfaceClassInParameter* inDelegate, IDelegateWithExtendsReverseInterfaceClassInParameter** outDelegate, IDelegateWithExtendsReverseInterfaceClassInParameter** retDelegate) override;
        };
    }

}