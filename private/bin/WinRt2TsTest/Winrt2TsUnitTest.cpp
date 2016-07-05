//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "MetadataString.h"
#include "MetadataResolver.h"
#include "CommandLineReader.h"

using namespace Metadata;
using namespace std;
using namespace WEX::TestExecution;

void RunWinRt2Ts(const Configuration& config);

class Winrt2TsUnitTest
{
    // Refer to TestMetadata.idl for the WinRT input for each test case.
    // Each top-level namespace in the file corresponds to a test case name here.
    // TestMetadata.winmd is generated at build time from TestMetadata.idl. 
    // At test execution WinRt2Ts takes this resulting winmd file as input.
    // Winrt2Ts is run once in ClassSetup, then each test case validates the result for its corresponding input.

    BEGIN_TEST_CLASS(Winrt2TsUnitTest)
    END_TEST_CLASS()

    TEST_CLASS_SETUP(ClassSetup);
    
    // Type definitions that should be hidden due to custom attributes
    TEST_METHOD(TypeDefWebHostHiddenInterface);
    TEST_METHOD(TypeDefWebHostHiddenRuntimeClass);
    TEST_METHOD(RuntimeClassImplementsExclusiveInterface);

    // Enum type definition
    TEST_METHOD(TypeDefEnum);

    // Runtime class definitions
    TEST_METHOD(RuntimeClassWithNoMembersNoBasesNotActivatable);
    TEST_METHOD(RuntimeClassVectorViewSpecialization);
    TEST_METHOD(RuntimeClassVectorSpecialization);
    TEST_METHOD(RuntimeClassVectorSpecializationInvisibleElementType);
    TEST_METHOD(RuntimeClassMapSpecialization);
    TEST_METHOD(RuntimeClassMapViewSpecialization);
    TEST_METHOD(RuntimeClassMapSpecializationInvisibleType);
    TEST_METHOD(RuntimeClassMapViewSpecializationInvisibleType);
    TEST_METHOD(RuntimeClassPromiseSpecializationWithResultType);
    TEST_METHOD(RuntimeClassPromiseSpecializationWithInvisibleResultType);
    TEST_METHOD(RuntimeClassPromiseSpecializationNoResultType);
    TEST_METHOD(RuntimeClassActivatableSingleConstructor);
    TEST_METHOD(RuntimeClassActivatableMultipleConstructors);
    TEST_METHOD(RuntimeClassActivatableInvisibleConstructor);
    TEST_METHOD(RuntimeClassStaticMembers);
    TEST_METHOD(RuntimeClassInstanceMembers);
    TEST_METHOD(RuntimeClassImplementsMissingInterface);

    // Interface definitions
    TEST_METHOD(InterfaceNoTypeMembersNoBasesNoSpecialization);
    TEST_METHOD(InterfaceOnlyOwnMembers);
    TEST_METHOD(InterfaceOwnAndInheritedMembers);
    TEST_METHOD(InterfaceVectorSpecialization);
    TEST_METHOD(InterfaceVectorViewSpecialization);
    TEST_METHOD(InterfaceVectorSpecializationInvisibleElementType);
    TEST_METHOD(InterfaceMapSpecialization);
    TEST_METHOD(InterfaceMapViewSpecialization);
    TEST_METHOD(InterfaceMapSpecializationInvisibleType);
    TEST_METHOD(InterfaceExtendsInvisibleType);

    // Struct definitions
    TEST_METHOD(StructWithFields);
    TEST_METHOD(StructInvisibleTypeField);

    // Delegate definitions
    TEST_METHOD(DelegateWithInvokeParameters);
    TEST_METHOD(DelegateInvisibleInvokeParameterType);

    // Usage of various types
    TEST_METHOD(StringTypes); // String, char
    TEST_METHOD(BoolType); // Boolean
    TEST_METHOD(NumberTypes); // I1, U1, I2, U2, I4, U4, I8, U8, R4, R8
    TEST_METHOD(ObjectType); // IInspectable
    TEST_METHOD(KnownTypes); // DateTime, TimeSpan, EventRegistrationToken, HResult, Guid
    TEST_METHOD(NestedTypes);  // ByRef, Array, IReference, Generic TypeRef Instantiation
    TEST_METHOD(NestedTypesInvisible); 

    // Methods
    TEST_METHOD(MethodParametersInOut);

    // Properties
    TEST_METHOD(PropertyGettersSetters);

    // Properties and methods visibility
    TEST_METHOD(MethodParameterVisibility);
    TEST_METHOD(PropertiesAndMethodsVisibility);

    // Fully qualified names
    TEST_METHOD(InterfaceHasFullyQualifiedMethodClassDoesNot);
    TEST_METHOD(StaticInterfaceHasFullyQualifiedMethodClassDoesNot);
    TEST_METHOD(InterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes);
    TEST_METHOD(ExclusiveInterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes);
    TEST_METHOD(InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent);
    TEST_METHOD(StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent);

    bool FindInResult(LPCWSTR content);

    wstring m_originalResult;
    wstring m_errors;

    map<wstring,wstring> m_resultsByNamespace;
};

static wstring CollapseWhitespace(const wstring& input)
{
    wstring result;
    bool lastCharWasWhitespace = false;
    for (auto ch : input)
    {
        if (ch == L'\r' || ch == L'\n' || ch == L' ')
        {
            if (!lastCharWasWhitespace)
            {
                result += L' ';
            }
            lastCharWasWhitespace = true;
        }
        else
        {
            lastCharWasWhitespace = false;
            result += ch;
        }
    }

    return result;
}

static map<wstring, wstring> BreakUpByNamespace(const wstring& input)
{
    map<wstring, wstring> m_resultsByNamespace;

    wstring tempBuffer = input;
    wchar_t* ptr = nullptr;
    auto end = wcstok_s(&tempBuffer[0], L" ", &ptr);
    enum class State
    {
        None,
        AtNamespaceName,
        InNamespace
    };
    State currentState = State::None;
    int braceLevel = 0;
    auto currentNamespace = m_resultsByNamespace.begin();
    do
    {
        auto begin = end;
        end = wcstok_s(nullptr, L" ", &ptr);

        switch (currentState)
        {
        case State::AtNamespaceName:
        {
            currentNamespace = m_resultsByNamespace.insert(make_pair(begin, wstring())).first;
            currentState = State::InNamespace;
            braceLevel = 0;
        }
        break;
        case State::InNamespace:
            currentNamespace->second += begin;
            currentNamespace->second += L" ";
            if (wcsncmp(begin, L"{", 1) == 0)
            {
                braceLevel++;
            }
            else if (wcsncmp(begin, L"}", 1) == 0)
            {
                braceLevel--;
                if (braceLevel == 0)
                {
                    currentState = State::None;
                }
            }
            break;
        case State::None:
            if (wcscmp(begin, L"namespace") == 0)
            {
                currentState = State::AtNamespaceName;
            }
            break;
        }
    } while (end != nullptr);

    return m_resultsByNamespace;
}

static bool BeginsWith(const wstring& string, LPCWSTR substring)
{
    return string.compare(0, string.size(), substring, string.size()) == 0;
}

bool Winrt2TsUnitTest::ClassSetup()
{
    WEX::Common::String testMetadataWinmdPath;
    if (FAILED(RuntimeParameters::TryGetValue(L"TestMetadataWinmd", testMetadataWinmdPath)))
    {
        return false;
    }

    WEX::Common::String windowsFoundationWinmdPath;
    if (FAILED(RuntimeParameters::TryGetValue(L"WindowsFoundationWinmd", windowsFoundationWinmdPath)))
    {
        return false;
    }

    Configuration config;
    config.winmds.push_back(testMetadataWinmdPath.GetBuffer());
    config.winmds.push_back(windowsFoundationWinmdPath.GetBuffer());

    // Redirect stdout to a string buffer
    wstringstream outStream;
    auto oldStdOut = wcout.rdbuf(outStream.rdbuf());

    // Redirect stderr to another string buffer
    wstringstream errStream;
    auto oldStdErr = wcerr.rdbuf(errStream.rdbuf());

    // Generate the TypeScript definitions from WinMDs
    // This is the code under test
    RunWinRt2Ts(config);

    m_originalResult = outStream.str();
    m_errors = errStream.str();

    // Collapse whitespace of result
    wstring result = CollapseWhitespace(m_originalResult);

    // Break up into namespaces
    m_resultsByNamespace = BreakUpByNamespace(result);

    // Restore streams
    wcout.rdbuf(oldStdOut);
    wcerr.rdbuf(oldStdErr);

    return true;
}

bool Winrt2TsUnitTest::FindInResult(LPCWSTR content)
{
    // Get the currently running test's name and search under that namespace in the result

    WEX::Common::String namespaceName;
    VERIFY_SUCCEEDED(RuntimeParameters::TryGetValue(L"TestName", namespaceName));
    namespaceName.Replace(L"Winrt2TsUnitTest::", L"");
    auto result = m_resultsByNamespace[namespaceName.GetBuffer()];

    WEX::Logging::Log::Comment(result.c_str());

    auto found = result.find(content, 0);
    return (found != wstring::npos);
}

void Winrt2TsUnitTest::TypeDefWebHostHiddenInterface()
{
    // An interface decorated with the [webhosthidden] attribute should not appear in the projection.

    VERIFY_IS_FALSE(FindInResult(L"interface IHiddenInterface"));
}

void Winrt2TsUnitTest::TypeDefWebHostHiddenRuntimeClass()
{
    // A runtime class decorated with the [webhosthidden] attribute should not appear in the projection.

    VERIFY_IS_FALSE(FindInResult(L"class HiddenRuntimeClass"));
}

void Winrt2TsUnitTest::RuntimeClassImplementsExclusiveInterface()
{
    // An interface decorated with the [exclusiveto] attribute should not appear in the projection.

    VERIFY_IS_FALSE(FindInResult(L"interface IInterfaceWithExclusiveTo"));

    VERIFY_IS_TRUE(FindInResult(L"class ClassWithExclusiveInterface { method(): void; }"));
}

void Winrt2TsUnitTest::TypeDefEnum()
{
    // Enums should be defined as TS enums.

    VERIFY_IS_TRUE(FindInResult(L"enum Enum { value1, value2, }"));
}

void Winrt2TsUnitTest::RuntimeClassWithNoMembersNoBasesNotActivatable()
{
    // A runtime class that isn't activatable should be defined as abstract.

    VERIFY_IS_TRUE(FindInResult(L"abstract class RuntimeClass { }"));
}

void Winrt2TsUnitTest::RuntimeClassVectorViewSpecialization()
{
    // A runtime class that implements IVectorView<T> should also extend Array<T> in the TS definition.

    VERIFY_IS_TRUE(FindInResult(L"abstract class RuntimeClass extends Array<number> implements Windows.Foundation.Collections.IIterable<number>, Windows.Foundation.Collections.IVectorView<number> { "
        L"size: number; "
        L"first(): Windows.Foundation.Collections.IIterator<number>; "
        L"getAt(index: number): number; "
        L"getMany(startIndex: number, items: number[]): number; "
        L"indexOf(value: number): any; /* { index: number; returnValue: boolean; } */ "
        L"}"));
}

void Winrt2TsUnitTest::RuntimeClassVectorSpecialization()
{
    // A runtime class that implements IVector<T> should also extend Array<T> in the TS definition.

    VERIFY_IS_TRUE(FindInResult(L"abstract class RuntimeClass extends Array<number> implements Windows.Foundation.Collections.IIterable<number>, Windows.Foundation.Collections.IVector<number> { "
        "size: number; "
        "append(value: number): void; "
        "clear(): void; "
        "first(): Windows.Foundation.Collections.IIterator<number>; "
        "getAt(index: number): number; "
        "getMany(startIndex: number, items: number[]): number; "
        "getView(): Windows.Foundation.Collections.IVectorView<number>; "
        "indexOf(value: number): any; /* { index: number; returnValue: boolean; } */ "
        "insertAt(index: number, value: number): void; "
        "removeAt(index: number): void; "
        "removeAtEnd(): void; "
        "replaceAll(items: number[]): void; "
        "setAt(index: number, value: number): void; "
        "}"));
}

void Winrt2TsUnitTest::RuntimeClassVectorSpecializationInvisibleElementType()
{
    // A runtime class that implements IVector<T> where T is hidden shouldn't be defined as implementing IVector<T> or Array<T>.

    VERIFY_IS_TRUE(FindInResult(L"abstract class RuntimeClass { }"));
}

void Winrt2TsUnitTest::RuntimeClassMapSpecialization()
{
    // A runtime class that implements IMap<HSTRING,V> should be defined with a string index signature.

    VERIFY_IS_TRUE(FindInResult(L"abstract class RuntimeClass implements Windows.Foundation.Collections.IIterable<Windows.Foundation.Collections.IKeyValuePair<string, number>>, Windows.Foundation.Collections.IMap<string, number> { "
        "size: number; "
        "clear(): void; "
        "first(): Windows.Foundation.Collections.IIterator<Windows.Foundation.Collections.IKeyValuePair<string, number>>; "
        "getView(): Windows.Foundation.Collections.IMapView<string, number>; "
        "hasKey(key: string): boolean; "
        "insert(key: string, value: number): boolean; "
        "lookup(key: string): number; "
        "remove(key: string): void; "
        "[index: string]: any; "
        "}"));
}

void Winrt2TsUnitTest::RuntimeClassMapViewSpecialization()
{
    // A runtime class that implements IMapView<HSTRING,V> should be defined with a string index signature.

    VERIFY_IS_TRUE(FindInResult(L"abstract class RuntimeClass implements Windows.Foundation.Collections.IIterable<Windows.Foundation.Collections.IKeyValuePair<string, number>>, Windows.Foundation.Collections.IMapView<string, number> { "
        "size: number; "
        "first(): Windows.Foundation.Collections.IIterator<Windows.Foundation.Collections.IKeyValuePair<string, number>>; "
        "hasKey(key: string): boolean; "
        "lookup(key: string): number; "
        "split(): { first: Windows.Foundation.Collections.IMapView<string, number>; second: Windows.Foundation.Collections.IMapView<string, number>; }; "
        "[index: string]: any; "
        "}"));
}

void Winrt2TsUnitTest::RuntimeClassMapSpecializationInvisibleType()
{
    // A runtime class that implements IMap<K,V> where V is hidden shouldn't be defined as implementing IMap<K,V> or have an index signature.

    VERIFY_IS_TRUE(FindInResult(L"abstract class RuntimeClass { }"));
}

void Winrt2TsUnitTest::RuntimeClassMapViewSpecializationInvisibleType()
{
    // A runtime class that implements IMapView<K,V> where V is hidden shouldn't be defined as implementing IMap<K,V> or have an index signature.

    VERIFY_IS_TRUE(FindInResult(L"abstract class RuntimeClass { }"));
}

void Winrt2TsUnitTest::RuntimeClassPromiseSpecializationWithResultType()
{
    // A runtime class that implements IAsyncOperation<T> should be defined as extending Promise<T>.

    VERIFY_IS_TRUE(FindInResult(L"abstract class RuntimeClass extends Windows.Foundation.Projections.Promise<number> implements Windows.Foundation.IAsyncInfo, Windows.Foundation.IAsyncOperation<number> { "
        "completed: Windows.Foundation.AsyncOperationCompletedHandler<number>; "
        "errorCode: number; "
        "id: number; "
        "status: Windows.Foundation.AsyncStatus; "
        "cancel(): void; "
        "close(): void; "
        "getResults(): number; "
        "}"));
}

void Winrt2TsUnitTest::RuntimeClassPromiseSpecializationWithInvisibleResultType()
{
    // A runtime class that implements IAsyncOperation<T> where T is hidden shouldn't be defined as extending IAsyncOperation<T> or Promise<T>.

    VERIFY_IS_TRUE(FindInResult(L"abstract class RuntimeClass implements Windows.Foundation.IAsyncInfo { "
        "errorCode: number; "
        "id: number; "
        "status: Windows.Foundation.AsyncStatus; "
        "cancel(): void; "
        "close(): void; "
        "}"));
}

void Winrt2TsUnitTest::RuntimeClassPromiseSpecializationNoResultType()
{
    // A runtime class that implements IAsyncAction or IAsyncActionWithProgres<T> should be defined as extending Promise<void>.

    VERIFY_IS_TRUE(FindInResult(L"abstract class RuntimeClass extends Windows.Foundation.Projections.Promise<void> implements Windows.Foundation.IAsyncInfo, Windows.Foundation.IAsyncActionWithProgress<number> { "
        "completed: Windows.Foundation.AsyncActionWithProgressCompletedHandler<number>; "
        "errorCode: number; "
        "id: number; "
        "progress: Windows.Foundation.AsyncActionProgressHandler<number>; "
        "status: Windows.Foundation.AsyncStatus; "
        "cancel(): void; "
        "close(): void; "
        "getResults(): void; "
        "}"));
}

void Winrt2TsUnitTest::RuntimeClassActivatableSingleConstructor()
{
    // An activatable runtime class should have a constructor emitted.

    VERIFY_IS_TRUE(FindInResult(L"class RuntimeClass { constructor(); }"));
}

void Winrt2TsUnitTest::RuntimeClassActivatableMultipleConstructors()
{
    // An activatable runtime class with multiple constructor overloads should have each overload emitted.

    VERIFY_IS_TRUE(FindInResult(L"class RuntimeClass { "
        "constructor(param1: number, param2: string); "
        "constructor(param: string); "
        "}"));
}

void Winrt2TsUnitTest::RuntimeClassActivatableInvisibleConstructor()
{
    // An constructor that takes an invisible type should itself be hidden.

    VERIFY_IS_TRUE(FindInResult(L"class RuntimeClass { }"));
}

void Winrt2TsUnitTest::RuntimeClassStaticMembers()
{
    // Static members on a runtime class should get defined as static members.

    VERIFY_IS_TRUE(FindInResult(L"class RuntimeClass { static method(): void; }"));
}

void Winrt2TsUnitTest::RuntimeClassInstanceMembers()
{
    // Instance members on a runtime class should get defined as instance members.

    VERIFY_IS_TRUE(FindInResult(L"class RuntimeClass { method(): void; }"));
}

void Winrt2TsUnitTest::RuntimeClassImplementsMissingInterface()
{
    // If a runtime class implements an interface that is [webhosthidden] or otherwise missing, the interface should be omitted from the implements clause.

    VERIFY_IS_TRUE(FindInResult(L"class RuntimeClass { }"));
}

void Winrt2TsUnitTest::InterfaceNoTypeMembersNoBasesNoSpecialization()
{
    // An empty interface should get defined as an empty TS interface.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface { } "));
}

void Winrt2TsUnitTest::InterfaceOnlyOwnMembers()
{
    // An interface with no parents should get projected as normal.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface { method(): void; } "));
}

void Winrt2TsUnitTest::InterfaceOwnAndInheritedMembers()
{
    // An interface that extends another interface should not include members from its parent in the TS definition.

    VERIFY_IS_TRUE(FindInResult(L"interface IChildInterface extends IParentInterface { "
        L"addEventListener(type: \"baseevent\", listener: Windows.Foundation.EventHandler<any>): void; "
        L"addEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"method(): void; "
        L"removeEventListener(type: \"baseevent\", listener: Windows.Foundation.EventHandler<any>): void; "
        L"removeEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"} "));
}

void Winrt2TsUnitTest::InterfaceVectorSpecialization()
{
    // An interface that inherits from IVector<T> should also extend Array<T> in the TS definition.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface extends Windows.Foundation.Collections.IVector<number>, Windows.Foundation.Collections.IIterable<number>, Array<number> { "
        "indexOf(value: number): any; /* { index: number; returnValue: boolean; } */ "
        "method(): void; "
        "}"));
}

void Winrt2TsUnitTest::InterfaceVectorViewSpecialization()
{
    // An interface that inherits from IVectorView<T> should also extend Array<T> in the TS definition.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface extends Windows.Foundation.Collections.IVectorView<number>, Windows.Foundation.Collections.IIterable<number>, Array<number> { "
        "indexOf(value: number): any; /* { index: number; returnValue: boolean; } */ "
        "method(): void; "
        "}"));
}

void Winrt2TsUnitTest::InterfaceVectorSpecializationInvisibleElementType()
{
    // An interface that inherits from IVectorView<T> where T is hidden shouldn't be defined as extending IVector<T> or Array<T>.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface { } "));
}

void Winrt2TsUnitTest::InterfaceMapSpecialization()
{
    // An interface that inherits from IMap<HSTRING,V> should be defined with a string index signature.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface extends Windows.Foundation.Collections.IMap<string, number>, Windows.Foundation.Collections.IIterable<Windows.Foundation.Collections.IKeyValuePair<string, number>> { "
        "method(): void; "
        "[index: string]: any; "
        "}"));
}

void Winrt2TsUnitTest::InterfaceMapViewSpecialization()
{
    // An interface that inherits from IMapView<HSTRING,V> should be defined with a string index signature.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface extends Windows.Foundation.Collections.IMapView<string, number>, Windows.Foundation.Collections.IIterable<Windows.Foundation.Collections.IKeyValuePair<string, number>> { "
        "method(): void; "
        "[index: string]: any; "
        "}"));
}

void Winrt2TsUnitTest::InterfaceMapSpecializationInvisibleType()
{
    // An interface that inherits from IMap<K,V> where V is hidden shouldn't be defined as extending IMap<K,V> or have an index signature.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface { method(): void; }"));
}

void Winrt2TsUnitTest::InterfaceExtendsInvisibleType()
{
    // If an interface inherits from another interface that is [webhosthidden] or otherwise missing, the missing interface should be omitted from the extends clause.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface { }"));
}

void Winrt2TsUnitTest::StructWithFields()
{
    // Struct is projected as a TS interface.

    VERIFY_IS_TRUE(FindInResult(L"interface Struct { x: number; h: string; }"));
}

void Winrt2TsUnitTest::StructInvisibleTypeField()
{
    // A struct with a field whose type is missing or invisible should also be invisible.

    VERIFY_IS_FALSE(FindInResult(L"interface Struct"));
}

void Winrt2TsUnitTest::DelegateWithInvokeParameters()
{
    // A delegate should be defined as an interface with a call signature.

    VERIFY_IS_TRUE(FindInResult(L"interface Delegate { (param: number): void }"));
}

void Winrt2TsUnitTest::DelegateInvisibleInvokeParameterType()
{
    // If the delegate has any parameters that are hidden or missing, the call signature shouldn't be defined.

    VERIFY_IS_TRUE(FindInResult(L"interface Delegate { }"));
}

void Winrt2TsUnitTest::StringTypes()
{
    // HSTRING and WCHAR WinRT types are projected as string.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface { "
        "charMethod(param: string): string; "
        "stringMethod(param: string): string; "
        "}"));
}

void Winrt2TsUnitTest::BoolType()
{
    // The boolean WinRT type is projected as boolean.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface { "
        "boolMethod(param: boolean): boolean; "
        "}"));
}

void Winrt2TsUnitTest::NumberTypes()
{
    // The various integer and floating point WinRT types are all projected as number.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface { "
        L"doubleMethod(param: number): number; "
        L"floatMethod(param: number): number; "
        L"int16Method(param: number): number; "
        L"int32Method(param: number): number; "
        L"int64Method(param: number): number; "
        L"uint16Method(param: number): number; "
        L"uint32Method(param: number): number; "
        L"uint64Method(param: number): number; "
        L"uint8Method(param: number): number; "
        L"}"));
}

void Winrt2TsUnitTest::ObjectType()
{
    // The IInspectable* WinRT type should be defined as any.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface { "
        "iinspectableMethod(param: any): any; "
        "}"));
}

void Winrt2TsUnitTest::KnownTypes()
{
    // Windows.Foundation.DateTime => Date
    // Windows.Foundation.TimeSpan => number
    // Windows.Foundation.EventRegistrationToken => number
    // System.Guid => string
    // HRESULT => number

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface { "
        "dateTimeMethod(param: Date): Date; "
        "eventRegistrationTokenMethod(param: number): number; "
        "guidMethod(param: string): string; "
        "hresultMethod(param: number): number; "
        "timeSpanMethod(param: number): number; "
        "}"));
}

void Winrt2TsUnitTest::NestedTypes()
{
    // Types that reference other types. WinRT arrays get projected as JS arrays, generic interfaces get projected as generic TS types,
    // Windows.Foundation.IReference<T> gets projected as T.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface { "
        "arrayMethod(param: number[], result: number[]): void; "
        "genericTypeMethod(param: Windows.Foundation.Collections.IKeyValuePair<number, number>): Windows.Foundation.Collections.IKeyValuePair<number, number>; "
        "ireferenceMethod(param: number): number; "
        "}"));
}

void Winrt2TsUnitTest::NestedTypesInvisible()
{
    // If a type references another type that is hidden, that type is also considered hidden.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface { }"));
}

void Winrt2TsUnitTest::MethodParametersInOut()
{
    // WinRT input parameters get mapped to JS function parameters, WinRT output parameters get mapped to JS return types.
    // Multiple WinRT output parameters are projected as a JS object return type.
    // WinRT array parameters are considered in/out parameters and are projected as JS array parameters.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface { "
        L"methodWithMultipleInOutParameters(value: number[], value2: string[]): void; "
        L"methodWithMultipleInParameters(param: number, param2: string): void; "
        L"methodWithMultipleOutParameters(): { param1: string; param2: number; }; "
        L"methodWithNoParameters(): void; "
        L"methodWithOneInOutParameter(value: number[]): void; "
        L"methodWithOneInParameter(param: number): void; "
        L"methodWithOneOutParameter(): number; "
        L"}"));
}

void Winrt2TsUnitTest::PropertyGettersSetters()
{
    // WinRT properties get projected as JS properties.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface { "
        L"propertyGetAndSet: number; "
        L"propertyGetterOnly: number; "
        L"}"));
}

void Winrt2TsUnitTest::MethodParameterVisibility()
{
    // A method that references a [webhosthidden] or otherwise missing type in its parameters or return type should itself be hidden.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface { "
        L"methodAllParametersVisible(param: number, param2: string): string; "
        L"}"));
}

void Winrt2TsUnitTest::PropertiesAndMethodsVisibility()
{
    // An event or property that references a [webhosthidden] or otherwise missing type in its parameters or return type should itself be hidden.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface { "
        L"oneventvisibletype: Windows.Foundation.EventHandler<any>; "
        L"propertyVisibleType: number; "
        L"addEventListener(type: \"eventvisibletype\", listener: Windows.Foundation.EventHandler<any>): void; "
        L"addEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"overloadedMethod(param: number, param2: string): void; "
        L"overloadedMethod(param: number): void; "
        L"overloadedMethodWithInvisibleOverload(param: number): void; "
        L"removeEventListener(type: \"eventvisibletype\", listener: Windows.Foundation.EventHandler<any>): void; "
        L"removeEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"}"));
}

void Winrt2TsUnitTest::InterfaceHasFullyQualifiedMethodClassDoesNot()
{
    // Fully qualified members of an interface should be defined as optional.

    VERIFY_IS_TRUE(FindInResult(L"interface IChildInterface extends IParentInterface { "
        L"\"InterfaceHasFullyQualifiedMethodClassDoesNot.IChildInterface.conflictMethod\"?(value: boolean, other: number): void; "
        L"\"InterfaceHasFullyQualifiedMethodClassDoesNot.IParentInterface.conflictMethod\"?(myInt: number): void; "
        L"} "));

    VERIFY_IS_TRUE(FindInResult(L"interface IParentInterface { "
        L"conflictMethod(myInt: number): void; "
        L"} "));

    VERIFY_IS_TRUE(FindInResult(L"class ImplementsChildInterface implements IParentInterface, IChildInterface { "
        L"constructor(); "
        L"conflictMethod(value: boolean, other: number): void; "
        L"conflictMethod(myInt: number): void; "
        L"}"));
}

void Winrt2TsUnitTest::StaticInterfaceHasFullyQualifiedMethodClassDoesNot()
{
    // Fully qualified members of an interface should be defined as optional.

    VERIFY_IS_TRUE(FindInResult(L"interface IChildInterface extends IParentInterface { "
        L"\"StaticInterfaceHasFullyQualifiedMethodClassDoesNot.IChildInterface.conflictMethod\"?(value: boolean, other: number): void; "
        L"\"StaticInterfaceHasFullyQualifiedMethodClassDoesNot.IParentInterface.conflictMethod\"?(myInt: number): void; "
        L"}"));

    VERIFY_IS_TRUE(FindInResult(L"interface IParentInterface { "
        L"conflictMethod(myInt: number): void; "
        L"}"));

    VERIFY_IS_TRUE(FindInResult(L"abstract class ImplementsStaticChildInterface { "
        L"static conflictMethod(value: boolean, other: number): void; "
        L"static conflictMethod(myInt: number): void; "
        L"}"));
}

void Winrt2TsUnitTest::InterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes()
{
    // If a fully qualified member name appears in a runtime class, the unqualified name should also be emitted,
    // satisfying the parent interfaces' definitions.

    VERIFY_IS_TRUE(FindInResult(L"interface IInterface { "
        L"conflictProperty: number; "
        L"onconflictevent: Windows.Foundation.EventHandler<string>; "
        L"addEventListener(type: \"conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"addEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"conflictMethod(myString: string): void; "
        L"removeEventListener(type: \"conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"removeEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"}"));

    VERIFY_IS_TRUE(FindInResult(L"interface IOtherInterface { "
        L"conflictProperty: string; "
        L"onconflictevent: Windows.Foundation.EventHandler<any>; "
        L"addEventListener(type: \"conflictevent\", listener: Windows.Foundation.EventHandler<any>): void; "
        L"addEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"conflictMethod(myInt: number): void; "
        L"removeEventListener(type: \"conflictevent\", listener: Windows.Foundation.EventHandler<any>): void; "
        L"removeEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"}"));

    VERIFY_IS_TRUE(FindInResult(L"class ImplementsConflictingInterfaces implements IInterface, IOtherInterface { "
        L"constructor(); "
        L"\"InterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IInterface.conflictProperty\": number; "
        L"conflictProperty: any; "
        L"\"InterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IInterface.onconflictevent\": Windows.Foundation.EventHandler<string>; "
        L"onconflictevent: (...args: any[]) => any; "
        L"\"InterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IOtherInterface.conflictProperty\": string; "
        L"\"InterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IOtherInterface.onconflictevent\": Windows.Foundation.EventHandler<any>; "
        L"\"InterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IInterface.conflictMethod\"(myString: string): void; "
        L"conflictMethod(myString: string): void; "
        L"\"InterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IOtherInterface.conflictMethod\"(myInt: number): void; "
        L"conflictMethod(myInt: number): void; "
        L"addEventListener(type: \"InterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IOtherInterface.conflictevent\", listener: Windows.Foundation.EventHandler<any>): void; "
        L"addEventListener(type: \"InterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"addEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"removeEventListener(type: \"InterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IOtherInterface.conflictevent\", listener: Windows.Foundation.EventHandler<any>): void; "
        L"removeEventListener(type: \"InterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"removeEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"}"));
}

void Winrt2TsUnitTest::InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent()
{
    // Fully qualified members of an interface should be defined as optional.
    // If a fully qualified member name appears in a runtime class, the unqualified name should also be emitted,
    // satisfying the parent interfaces' definitions.

    VERIFY_IS_TRUE(FindInResult(L"interface IChildInterface extends IParentInterface { "
        L"\"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.conflictProperty\"?: number; "
        L"\"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.onconflictevent\"?: Windows.Foundation.EventHandler<string>; "
        L"\"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.conflictProperty\"?: number; "
        L"\"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.onconflictevent\"?: Windows.Foundation.EventHandler<string>; "
        L"\"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.conflictMethod\"?(myString: string): void; "
        L"\"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.conflictMethod\"?(myString: string): void; "
        L"addEventListener(type: \"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"addEventListener(type: \"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"addEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"removeEventListener(type: \"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"removeEventListener(type: \"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"removeEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"}"));

    VERIFY_IS_TRUE(FindInResult(L"interface IParentInterface { "
        L"conflictProperty: number; "
        L"onconflictevent: Windows.Foundation.EventHandler<string>; "
        L"addEventListener(type: \"conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"addEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"conflictMethod(myString: string): void; "
        L"removeEventListener(type: \"conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"removeEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"}"));

    VERIFY_IS_TRUE(FindInResult(L"class ImplementChildInterface implements IParentInterface, IChildInterface { "
        L"constructor(); "
        L"\"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.conflictProperty\": number; "
        L"conflictProperty: any; "
        L"\"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.onconflictevent\": Windows.Foundation.EventHandler<string>; "
        L"onconflictevent: (...args: any[]) => any; "
        L"\"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.conflictProperty\": number; "
        L"\"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.onconflictevent\": Windows.Foundation.EventHandler<string>; "
        L"\"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.conflictMethod\"(myString: string): void; "
        L"conflictMethod(myString: string): void; "
        L"\"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.conflictMethod\"(myString: string): void; "
        L"conflictMethod(myString: string): void; "
        L"addEventListener(type: \"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"addEventListener(type: \"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"addEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"removeEventListener(type: \"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"removeEventListener(type: \"InterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"removeEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"}"));
}

void Winrt2TsUnitTest::StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent()
{
    // Fully qualified members of an interface should be defined as optional.
    // If a fully qualified member name appears in a runtime class as a static member, 
    // there is no need to emit the unqualified name as there is no inheritance relationship between the class and its static interface.

    VERIFY_IS_TRUE(FindInResult(L"interface IChildInterface extends IParentInterface { "
        L"\"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.conflictProperty\"?: number; "
        L"\"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.onconflictevent\"?: Windows.Foundation.EventHandler<string>; "
        L"\"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.conflictProperty\"?: number; "
        L"\"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.onconflictevent\"?: Windows.Foundation.EventHandler<string>; "
        L"\"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.conflictMethod\"?(myInt: number): void; "
        L"\"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.conflictMethod\"?(myString: string): void; "
        L"addEventListener(type: \"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"addEventListener(type: \"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"addEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"removeEventListener(type: \"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"removeEventListener(type: \"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"removeEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"}"));

    VERIFY_IS_TRUE(FindInResult(L"interface IParentInterface { "
        L"conflictProperty: number; "
        L"onconflictevent: Windows.Foundation.EventHandler<string>; "
        L"addEventListener(type: \"conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"addEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"conflictMethod(myString: string): void; "
        L"removeEventListener(type: \"conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"removeEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"}"));

    VERIFY_IS_TRUE(FindInResult(L"abstract class StaticImplementsIChildInterface { "
        L"static \"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.conflictProperty\": number; "
        L"static \"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.onconflictevent\": Windows.Foundation.EventHandler<string>; "
        L"static \"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.conflictProperty\": number; "
        L"static \"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.onconflictevent\": Windows.Foundation.EventHandler<string>; "
        L"static \"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.conflictMethod\"(myInt: number): void; "
        L"static \"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.conflictMethod\"(myString: string): void; "
        L"static addEventListener(type: \"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"static addEventListener(type: \"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"static addEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"static removeEventListener(type: \"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IChildInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"static removeEventListener(type: \"StaticInterfaceAndClassHaveFullyQualifiedPropertyMethodEvent.IParentInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"static removeEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"}"));
}

void Winrt2TsUnitTest::ExclusiveInterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes()
{
    // Fully qualified members of an interface should be defined as optional.
    // If a fully qualified member name appears in a runtime class as a static member, 
    // there is no need to emit the unqualified name as there is no inheritance relationship between the class and its static interface.

    VERIFY_IS_TRUE(FindInResult(L"class ImplementsConflictingInterfaces { "
        L"constructor(); "
        L"\"ExclusiveInterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IInterface.conflictProperty\": number; "
        L"\"ExclusiveInterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IInterface.onconflictevent\": Windows.Foundation.EventHandler<string>; "
        L"\"ExclusiveInterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IOtherInterface.conflictProperty\": string; "
        L"\"ExclusiveInterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IOtherInterface.onconflictevent\": Windows.Foundation.EventHandler<any>; "
        L"\"ExclusiveInterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IInterface.conflictMethod\"(myString: string): void; "
        L"\"ExclusiveInterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IOtherInterface.conflictMethod\"(myInt: number): void; "
        L"addEventListener(type: \"ExclusiveInterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"addEventListener(type: \"ExclusiveInterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IOtherInterface.conflictevent\", listener: Windows.Foundation.EventHandler<any>): void; "
        L"addEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"removeEventListener(type: \"ExclusiveInterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IInterface.conflictevent\", listener: Windows.Foundation.EventHandler<string>): void; "
        L"removeEventListener(type: \"ExclusiveInterfaceDoesNotHaveFullyQualifiedPropertyMethodEventClassDoes.IOtherInterface.conflictevent\", listener: Windows.Foundation.EventHandler<any>): void; "
        L"removeEventListener(type: string, listener: (...args: any[]) => any): void; "
        L"}"));
}