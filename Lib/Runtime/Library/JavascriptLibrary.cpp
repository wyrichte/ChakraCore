//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

#ifdef F_JSETW
#include <IERESP_mshtml.h>
#include "microsoft-scripting-jscript9.internalevents.h"
#endif

namespace Js
{
    SimplePropertyDescriptor JavascriptLibrary::SharedFunctionPropertyDescriptors[2] =
    {
        SimplePropertyDescriptor(BuiltInPropertyRecords::prototype, PropertyWritable),
        SimplePropertyDescriptor(BuiltInPropertyRecords::name, PropertyConfigurable | PropertyWritable)
    };

    SimpleTypeHandler<1> JavascriptLibrary::SharedPrototypeTypeHandler(BuiltInPropertyRecords::constructor, PropertyWritable | PropertyConfigurable, PropertyTypesWritableDataOnly, 4, sizeof(DynamicObject));
    SimpleTypeHandler<1> JavascriptLibrary::SharedFunctionWithoutPrototypeTypeHandler(BuiltInPropertyRecords::name, PropertyConfigurable | PropertyWritable);
    SimpleTypeHandler<1> JavascriptLibrary::SharedFunctionWithPrototypeTypeHandlerV11(BuiltInPropertyRecords::prototype, PropertyWritable);
    SimpleTypeHandler<2> JavascriptLibrary::SharedFunctionWithPrototypeTypeHandler(SharedFunctionPropertyDescriptors);
    SimpleTypeHandler<1> JavascriptLibrary::SharedIdMappedFunctionWithPrototypeTypeHandler(BuiltInPropertyRecords::prototype);
    SimpleTypeHandler<1> JavascriptLibrary::SharedFunctionWithLengthTypeHandler(BuiltInPropertyRecords::length);
    MissingPropertyTypeHandler JavascriptLibrary::MissingPropertyHolderTypeHandler;


    SimplePropertyDescriptor JavascriptLibrary::HeapArgumentsPropertyDescriptorsV11[2] =
    {
        SimplePropertyDescriptor(BuiltInPropertyRecords::length, PropertyConfigurable | PropertyWritable),
        SimplePropertyDescriptor(BuiltInPropertyRecords::callee, PropertyConfigurable | PropertyWritable)
    };

    SimplePropertyDescriptor JavascriptLibrary::HeapArgumentsPropertyDescriptors[3] =
    {
        SimplePropertyDescriptor(BuiltInPropertyRecords::length, PropertyConfigurable | PropertyWritable),
        SimplePropertyDescriptor(BuiltInPropertyRecords::callee, PropertyConfigurable | PropertyWritable),
        SimplePropertyDescriptor(BuiltInPropertyRecords::_symbolIterator, PropertyConfigurable | PropertyWritable)
    };

    SimplePropertyDescriptor JavascriptLibrary::FunctionWithLengthAndPrototypeTypeDescriptors[2] =
    {
        SimplePropertyDescriptor(BuiltInPropertyRecords::prototype, PropertyNone),
        SimplePropertyDescriptor(BuiltInPropertyRecords::length, PropertyConfigurable)
    };

    void JavascriptLibrary::Initialize(ScriptContext* scriptContext, GlobalObject * globalObject)
    {
        PROBE_STACK(scriptContext, Js::Constants::MinStackDefault);
#ifdef PROFILE_EXEC
        scriptContext->ProfileBegin(Js::LibInitPhase);
#endif
        this->scriptContext = scriptContext;
        this->recycler = scriptContext->GetRecycler();
        this->undeclBlockVarSentinel = RecyclerNew(recycler, UndeclaredBlockVariable, StaticType::New(scriptContext, TypeIds_Null, nullptr, nullptr));

        typesEnsuredToHaveOnlyWritableDataPropertiesInItAndPrototypeChain = RecyclerNew(recycler, JsUtil::List<Type *>, recycler);

        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // WARNING: Any objects created here using DeferredTypeHandler need to appear in EnsureLibraryReadyForHybridDebugging
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        // Library is not zero-initialized. memset the memory occcupied by builtinFunctions array to 0.
        memset(builtinFunctions, 0, sizeof(JavascriptFunction *) * BuiltinFunction::Count);

        // Note: InitializePrototypes and InitializeTypes must be called first.
        InitializePrototypes();
        InitializeTypes();
        InitializeGlobal(globalObject);
        InitializeDebug();
        InitializeComplexThings();
        InitializeStaticValues();

        // Do an early check of hybrid debugging. Note that script engine is not ready, so objects that run script in initializer
        // can't be un-deferred yet. However, we can possibly mark isHybridDebugging and avoid deferring new runtime objects.
        EnsureReadyIfHybridDebugging(/*isScriptEngineReady*/false);

        if (!PHASE_OFF1(CopyOnAccessArrayPhase))
        {
            this->cacheForCopyOnAccessArraySegments = RecyclerNewZ(this->recycler, CacheForCopyOnAccessArraySegments);
        }
#ifdef PROFILE_EXEC
        scriptContext->ProfileEnd(Js::LibInitPhase);
#endif
    }

    void JavascriptLibrary::Uninitialize()
    {
        this->globalObject = nullptr;
    }

    void JavascriptLibrary::InitializePrototypes()
    {
        Recycler* recycler = this->GetRecycler();

        nullValue = RecyclerNew(recycler, RecyclableObject, StaticType::New(scriptContext, TypeIds_Null, nullptr, nullptr));
        nullValue->GetType()->SetHasSpecialPrototype(true);

        ArrayBuffer* baseArrayBuffer;

        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // WARNING: Any objects here using DeferredTypeHandler need to appear in EnsureLibraryReadyForHybridDebugging
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        // The prototype property of the object prototype is null.
        objectPrototype = ObjectPrototypeObject::New(recycler,
            DynamicType::New(scriptContext, TypeIds_Object, nullValue, nullptr,
            DeferredTypeHandler<InitializeObjectPrototype>::GetDefaultInstance()));

        constructorPrototypeObjectType = DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
            &SharedPrototypeTypeHandler, true, true);

        constructorPrototypeObjectType->SetHasNoEnumerableProperties(true);

        if (scriptContext->GetConfig()->IsTypedArrayEnabled())
        {
            if (scriptContext->GetConfig()->IsES6TypedArrayExtensionsEnabled())
            {
                arrayBufferPrototype = DynamicObject::New(recycler,
                    DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                    DeferredTypeHandler<InitializeArrayBufferPrototype>::GetDefaultInstance()));

                arrayBufferType = DynamicType::New(scriptContext, TypeIds_ArrayBuffer, arrayBufferPrototype, nullptr,
                    SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);

                dataViewPrototype = DynamicObject::New(recycler,
                    DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                    DeferredTypeHandler<InitializeDataViewPrototype>::GetDefaultInstance()));

                typedArrayPrototype = DynamicObject::New(recycler,
                    DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                    DeferredTypeHandler<InitializeTypedArrayPrototype>::GetDefaultInstance()));

                Int8ArrayPrototype = DynamicObject::New(recycler,
                    DynamicType::New(scriptContext, TypeIds_Object, typedArrayPrototype, nullptr,
                    DeferredTypeHandler<InitializeInt8ArrayPrototype>::GetDefaultInstance()));

                Uint8ArrayPrototype = DynamicObject::New(recycler,
                    DynamicType::New(scriptContext, TypeIds_Object, typedArrayPrototype, nullptr,
                    DeferredTypeHandler<InitializeUint8ArrayPrototype>::GetDefaultInstance()));

                // If ES6 TypedArrays are enabled, we have Khronos Interop mode enabled
                Uint8ClampedArrayPrototype = DynamicObject::New(recycler,
                    DynamicType::New(scriptContext, TypeIds_Object, typedArrayPrototype, nullptr,
                    DeferredTypeHandler<InitializeUint8ClampedArrayPrototype>::GetDefaultInstance()));

                Int16ArrayPrototype = DynamicObject::New(recycler,
                    DynamicType::New(scriptContext, TypeIds_Object, typedArrayPrototype, nullptr,
                    DeferredTypeHandler<InitializeInt16ArrayPrototype>::GetDefaultInstance()));

                Uint16ArrayPrototype = DynamicObject::New(recycler,
                    DynamicType::New(scriptContext, TypeIds_Object, typedArrayPrototype, nullptr,
                    DeferredTypeHandler<InitializeUint16ArrayPrototype>::GetDefaultInstance()));

                Int32ArrayPrototype = DynamicObject::New(recycler,
                    DynamicType::New(scriptContext, TypeIds_Object, typedArrayPrototype, nullptr,
                    DeferredTypeHandler<InitializeInt32ArrayPrototype>::GetDefaultInstance()));

                Uint32ArrayPrototype = DynamicObject::New(recycler,
                    DynamicType::New(scriptContext, TypeIds_Object, typedArrayPrototype, nullptr,
                    DeferredTypeHandler<InitializeUint32ArrayPrototype>::GetDefaultInstance()));

                Float32ArrayPrototype = DynamicObject::New(recycler,
                    DynamicType::New(scriptContext, TypeIds_Object, typedArrayPrototype, nullptr,
                    DeferredTypeHandler<InitializeFloat32ArrayPrototype>::GetDefaultInstance()));

                Float64ArrayPrototype = DynamicObject::New(recycler,
                    DynamicType::New(scriptContext, TypeIds_Object, typedArrayPrototype, nullptr,
                    DeferredTypeHandler<InitializeFloat64ArrayPrototype>::GetDefaultInstance()));

                Int64ArrayPrototype = DynamicObject::New(recycler,
                    DynamicType::New(scriptContext, TypeIds_Object, typedArrayPrototype, nullptr,
                    DeferredTypeHandler<InitializeInt64ArrayPrototype>::GetDefaultInstance()));

                Uint64ArrayPrototype = DynamicObject::New(recycler,
                    DynamicType::New(scriptContext, TypeIds_Object, typedArrayPrototype, nullptr,
                    DeferredTypeHandler<InitializeUint64ArrayPrototype>::GetDefaultInstance()));

                BoolArrayPrototype = DynamicObject::New(recycler,
                    DynamicType::New(scriptContext, TypeIds_Object, typedArrayPrototype, nullptr,
                    DeferredTypeHandler<InitializeBoolArrayPrototype>::GetDefaultInstance()));

                CharArrayPrototype = DynamicObject::New(recycler,
                    DynamicType::New(scriptContext, TypeIds_Object, typedArrayPrototype, nullptr,
                    DeferredTypeHandler<InitializeCharArrayPrototype>::GetDefaultInstance()));
            }
            else
            {
                arrayBufferPrototype = JavascriptArrayBuffer::Create(0,
                    DynamicType::New(scriptContext, TypeIds_ArrayBuffer, objectPrototype, nullptr,
                    DeferredTypeHandler<InitializeArrayBufferPrototype>::GetDefaultInstance()));

                arrayBufferType = DynamicType::New(scriptContext, TypeIds_ArrayBuffer, arrayBufferPrototype, nullptr,
                    SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);

                baseArrayBuffer = JavascriptArrayBuffer::Create(0, arrayBufferType);
                Int8ArrayPrototype = RecyclerNew(recycler, Int8Array, baseArrayBuffer, 0, 0,
                    DynamicType::New(scriptContext, TypeIds_Int8Array, objectPrototype, nullptr,
                    DeferredTypeHandler<InitializeInt8ArrayPrototype>::GetDefaultInstance()));

                baseArrayBuffer = JavascriptArrayBuffer::Create(0, arrayBufferType);
                Uint8ArrayPrototype = RecyclerNew(recycler, Uint8Array, baseArrayBuffer, 0, 0,
                    DynamicType::New(scriptContext, TypeIds_Uint8Array, objectPrototype, nullptr,
                    DeferredTypeHandler<InitializeUint8ArrayPrototype>::GetDefaultInstance()));

                if (scriptContext->GetConfig()->IsKhronosInteropEnabled())
                {
                    dataViewPrototype = DynamicObject::New(recycler,
                        DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                        DeferredTypeHandler<InitializeDataViewPrototype>::GetDefaultInstance()));

                    baseArrayBuffer = JavascriptArrayBuffer::Create(0, arrayBufferType);
                    Uint8ClampedArrayPrototype = RecyclerNew(recycler, Uint8ClampedArray, baseArrayBuffer, 0, 0,
                        DynamicType::New(scriptContext, TypeIds_Uint8ClampedArray, objectPrototype, nullptr,
                        DeferredTypeHandler<InitializeUint8ClampedArrayPrototype>::GetDefaultInstance()));
                }
                else
                {
                    baseArrayBuffer = JavascriptArrayBuffer::Create(0, arrayBufferType);
                    dataViewPrototype = RecyclerNew(recycler, DataView, baseArrayBuffer, 0, 0,
                        DynamicType::New(scriptContext, TypeIds_DataView, objectPrototype, nullptr,
                        DeferredTypeHandler<InitializeDataViewPrototype>::GetDefaultInstance()));

                    Uint8ClampedArrayPrototype = nullptr;
                }

                baseArrayBuffer = JavascriptArrayBuffer::Create(0, arrayBufferType);
                Int16ArrayPrototype = RecyclerNew(recycler, Int16Array, baseArrayBuffer, 0, 0,
                    DynamicType::New(scriptContext, TypeIds_Int16Array, objectPrototype, nullptr,
                    DeferredTypeHandler<InitializeInt16ArrayPrototype>::GetDefaultInstance()));

                baseArrayBuffer = JavascriptArrayBuffer::Create(0, arrayBufferType);
                Uint16ArrayPrototype = RecyclerNew(recycler, Uint16Array, baseArrayBuffer, 0, 0,
                    DynamicType::New(scriptContext, TypeIds_Uint16Array, objectPrototype, nullptr,
                    DeferredTypeHandler<InitializeUint16ArrayPrototype>::GetDefaultInstance()));

                baseArrayBuffer = JavascriptArrayBuffer::Create(0, arrayBufferType);
                Int32ArrayPrototype = RecyclerNew(recycler, Int32Array, baseArrayBuffer, 0, 0,
                    DynamicType::New(scriptContext, TypeIds_Int32Array, objectPrototype, nullptr,
                    DeferredTypeHandler<InitializeInt32ArrayPrototype>::GetDefaultInstance()));

                baseArrayBuffer = JavascriptArrayBuffer::Create(0, arrayBufferType);
                Uint32ArrayPrototype = RecyclerNew(recycler, Uint32Array, baseArrayBuffer, 0, 0,
                    DynamicType::New(scriptContext, TypeIds_Uint32Array, objectPrototype, nullptr,
                    DeferredTypeHandler<InitializeUint32ArrayPrototype>::GetDefaultInstance()));

                baseArrayBuffer = JavascriptArrayBuffer::Create(0, arrayBufferType);
                Float32ArrayPrototype = RecyclerNew(recycler, Float32Array, baseArrayBuffer, 0, 0,
                    DynamicType::New(scriptContext, TypeIds_Float32Array, objectPrototype, nullptr,
                    DeferredTypeHandler<InitializeFloat32ArrayPrototype>::GetDefaultInstance()));

                baseArrayBuffer = JavascriptArrayBuffer::Create(0, arrayBufferType);
                Float64ArrayPrototype = RecyclerNew(recycler, Float64Array, baseArrayBuffer, 0, 0,
                    DynamicType::New(scriptContext, TypeIds_Float64Array, objectPrototype, nullptr,
                    DeferredTypeHandler<InitializeFloat64ArrayPrototype>::GetDefaultInstance()));

                baseArrayBuffer = JavascriptArrayBuffer::Create(0, arrayBufferType);
                Int64ArrayPrototype = RecyclerNew(recycler, Int64Array, baseArrayBuffer, 0, 0,
                    DynamicType::New(scriptContext, TypeIds_Int64Array, objectPrototype, nullptr,
                    DeferredTypeHandler<InitializeInt64ArrayPrototype>::GetDefaultInstance()));

                baseArrayBuffer = JavascriptArrayBuffer::Create(0, arrayBufferType);
                Uint64ArrayPrototype = RecyclerNew(recycler, Uint64Array, baseArrayBuffer, 0, 0,
                    DynamicType::New(scriptContext, TypeIds_Uint64Array, objectPrototype, nullptr,
                    DeferredTypeHandler<InitializeUint64ArrayPrototype>::GetDefaultInstance()));

                baseArrayBuffer = JavascriptArrayBuffer::Create(0, arrayBufferType);
                BoolArrayPrototype = RecyclerNew(recycler, BoolArray, baseArrayBuffer, 0, 0,
                    DynamicType::New(scriptContext, TypeIds_BoolArray, objectPrototype, nullptr,
                    DeferredTypeHandler<InitializeBoolArrayPrototype>::GetDefaultInstance()));

                baseArrayBuffer = JavascriptArrayBuffer::Create(0, arrayBufferType);
                CharArrayPrototype = RecyclerNew(recycler, CharArray, baseArrayBuffer, 0, 0,
                    DynamicType::New(scriptContext, TypeIds_CharArray, objectPrototype, nullptr,
                    DeferredTypeHandler<InitializeCharArrayPrototype>::GetDefaultInstance()));
            }
        }
        else
        {
            arrayBufferType = nullptr;

            arrayBufferPrototype = nullptr;
            dataViewPrototype = nullptr;
            Int8ArrayPrototype = nullptr;
            Uint8ArrayPrototype = nullptr;
            Uint8ClampedArrayPrototype = nullptr;
            Int16ArrayPrototype = nullptr;
            Uint16ArrayPrototype = nullptr;
            Int32ArrayPrototype = nullptr;
            Uint32ArrayPrototype = nullptr;
            Float32ArrayPrototype = nullptr;
            Float64ArrayPrototype = nullptr;
            Int64ArrayPrototype = nullptr;
            Uint64ArrayPrototype = nullptr;
            BoolArrayPrototype = nullptr;
            CharArrayPrototype = nullptr;
        }

        if (!scriptContext->GetConfig()->IsES6TypedArrayExtensionsEnabled())
        {
            pixelArrayPrototype = RecyclerNew(recycler, JavascriptPixelArray, 0,
                DynamicType::New(scriptContext, TypeIds_PixelArray, objectPrototype, nullptr,
                DeferredTypeHandler<InitializePixelArrayPrototype>::GetDefaultInstance()));
        }
        else
        {
            pixelArrayPrototype = nullptr;
        }

        arrayPrototype = JavascriptArray::New<Var, JavascriptArray, 0>(0,
            DynamicType::New(scriptContext, TypeIds_Array, objectPrototype, nullptr,
            DeferredTypeHandler<InitializeArrayPrototype>::GetDefaultInstance()), recycler);

        if (scriptContext->GetConfig()->IsES6PrototypeChain())
        {
            booleanPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeBooleanPrototype>::GetDefaultInstance()));

            datePrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeDatePrototype>::GetDefaultInstance()));

            numberPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeNumberPrototype>::GetDefaultInstance()));

            stringPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeStringPrototype>::GetDefaultInstance()));

            regexPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeRegexPrototype>::GetDefaultInstance()));
        }
        else
        {
            booleanPrototype = RecyclerNew(recycler, JavascriptBooleanObject, nullptr,
                DynamicType::New(scriptContext, TypeIds_BooleanObject, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeBooleanPrototype>::GetDefaultInstance()));

            double initDateValue = JavascriptNumber::NaN;

            datePrototype = RecyclerNewZ(recycler, JavascriptDate, initDateValue,
                DynamicType::New(scriptContext, TypeIds_Date, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeDatePrototype>::GetDefaultInstance()));

            numberPrototype = RecyclerNew(recycler, JavascriptNumberObject, TaggedInt::ToVarUnchecked(0),
                DynamicType::New(scriptContext, TypeIds_NumberObject, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeNumberPrototype>::GetDefaultInstance()));

            stringPrototype = RecyclerNew(recycler, JavascriptStringObject, nullptr,
                DynamicType::New(scriptContext, TypeIds_StringObject, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeStringPrototype>::GetDefaultInstance()));
        }

        winrtErrorPrototype = nullptr;

        if (scriptContext->GetConfig()->IsES6PrototypeChain())
        {
            errorPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeErrorPrototype>::GetDefaultInstance()));

            evalErrorPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, errorPrototype, nullptr,
                DeferredTypeHandler<InitializeEvalErrorPrototype>::GetDefaultInstance()));

            rangeErrorPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, errorPrototype, nullptr,
                DeferredTypeHandler<InitializeRangeErrorPrototype>::GetDefaultInstance()));

            referenceErrorPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, errorPrototype, nullptr,
                DeferredTypeHandler<InitializeReferenceErrorPrototype>::GetDefaultInstance()));

            syntaxErrorPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, errorPrototype, nullptr,
                DeferredTypeHandler<InitializeSyntaxErrorPrototype>::GetDefaultInstance()));

            typeErrorPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, errorPrototype, nullptr,
                DeferredTypeHandler<InitializeTypeErrorPrototype>::GetDefaultInstance()));

            uriErrorPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, errorPrototype, nullptr,
                DeferredTypeHandler<InitializeURIErrorPrototype>::GetDefaultInstance()));

            if (scriptContext->GetConfig()->IsWinRTEnabled())
            {
                winrtErrorPrototype = DynamicObject::New(recycler,
                    DynamicType::New(scriptContext, TypeIds_Object, errorPrototype, nullptr,
                    DeferredTypeHandler<InitializeWinRTErrorPrototype>::GetDefaultInstance()));
            }
        }
        else
        {
            errorPrototype = RecyclerNew(this->GetRecycler(), JavascriptError,
                DynamicType::New(scriptContext, TypeIds_Error, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeErrorPrototype>::GetDefaultInstance()),
                /*isExternalError*/FALSE, /*isPrototype*/TRUE);

            evalErrorPrototype = RecyclerNew(this->GetRecycler(), JavascriptError,
                DynamicType::New(scriptContext, TypeIds_Error, errorPrototype, nullptr,
                DeferredTypeHandler<InitializeEvalErrorPrototype>::GetDefaultInstance()),
                /*isExternalError*/FALSE, /*isPrototype*/TRUE);

            rangeErrorPrototype = RecyclerNew(this->GetRecycler(), JavascriptError,
                DynamicType::New(scriptContext, TypeIds_Error, errorPrototype, nullptr,
                DeferredTypeHandler<InitializeRangeErrorPrototype>::GetDefaultInstance()),
                /*isExternalError*/FALSE, /*isPrototype*/TRUE);

            referenceErrorPrototype = RecyclerNew(this->GetRecycler(), JavascriptError,
                DynamicType::New(scriptContext, TypeIds_Error, errorPrototype, nullptr,
                DeferredTypeHandler<InitializeReferenceErrorPrototype>::GetDefaultInstance()),
                /*isExternalError*/FALSE, /*isPrototype*/TRUE);

            syntaxErrorPrototype = RecyclerNew(this->GetRecycler(), JavascriptError,
                DynamicType::New(scriptContext, TypeIds_Error, errorPrototype, nullptr,
                DeferredTypeHandler<InitializeSyntaxErrorPrototype>::GetDefaultInstance()),
                /*isExternalError*/FALSE, /*isPrototype*/TRUE);

            typeErrorPrototype = RecyclerNew(this->GetRecycler(), JavascriptError,
                DynamicType::New(scriptContext, TypeIds_Error, errorPrototype, nullptr,
                DeferredTypeHandler<InitializeTypeErrorPrototype>::GetDefaultInstance()),
                /*isExternalError*/FALSE, /*isPrototype*/TRUE);

            uriErrorPrototype = RecyclerNew(this->GetRecycler(), JavascriptError,
                DynamicType::New(scriptContext, TypeIds_Error, errorPrototype, nullptr,
                DeferredTypeHandler<InitializeURIErrorPrototype>::GetDefaultInstance()),
                /*isExternalError*/FALSE, /*isPrototype*/TRUE);

            if (scriptContext->GetConfig()->IsWinRTEnabled())
            {
                winrtErrorPrototype = RecyclerNew(this->GetRecycler(), JavascriptError,
                    DynamicType::New(scriptContext, TypeIds_Error, errorPrototype, nullptr,
                    DeferredTypeHandler<InitializeWinRTErrorPrototype>::GetDefaultInstance()),
                    /*isExternalError*/FALSE, /*isPrototype*/TRUE);
            }
        }

        functionPrototype = RecyclerNew(recycler, JavascriptFunction,
            DynamicType::New(scriptContext, TypeIds_Function, objectPrototype, JavascriptFunction::PrototypeEntryPoint,
            DeferredTypeHandler<InitializeFunctionPrototype>::GetDefaultInstance()), &JavascriptFunction::EntryInfo::PrototypeEntryPoint);

        symbolPrototype = nullptr;
        mapPrototype = nullptr;
        setPrototype = nullptr;
        weakMapPrototype = nullptr;
        weakSetPrototype = nullptr;
        arrayIteratorPrototype = nullptr;
        mapIteratorPrototype = nullptr;
        setIteratorPrototype = nullptr;
        stringIteratorPrototype = nullptr;
        promisePrototype = nullptr;
        javascriptEnumeratorIteratorPrototype = nullptr;
        generatorFunctionPrototype = nullptr;
        generatorPrototype = nullptr;

        if (scriptContext->GetConfig()->IsES6SymbolEnabled())
        {
            symbolPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeSymbolPrototype>::GetDefaultInstance()));
        }

        if (scriptContext->GetConfig()->IsES6MapEnabled())
        {
            mapPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeMapPrototype>::GetDefaultInstance()));
        }

        if (scriptContext->GetConfig()->IsES6SetEnabled())
        {
            setPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeSetPrototype>::GetDefaultInstance()));
        }

        if (scriptContext->GetConfig()->IsES6WeakMapEnabled())
        {
            weakMapPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeWeakMapPrototype>::GetDefaultInstance()));
        }

        if (scriptContext->GetConfig()->IsES6WeakSetEnabled())
        {
            weakSetPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeWeakSetPrototype>::GetDefaultInstance()));
        }

        if (scriptContext->GetConfig()->IsES6IteratorsEnabled())
        {
            arrayIteratorPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeArrayIteratorPrototype>::GetDefaultInstance()));
            mapIteratorPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeMapIteratorPrototype>::GetDefaultInstance()));
            setIteratorPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeSetIteratorPrototype>::GetDefaultInstance()));
            stringIteratorPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeStringIteratorPrototype>::GetDefaultInstance()));
        }

        if (scriptContext->GetConfig()->IsES6PromiseEnabled())
        {
            promisePrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializePromisePrototype>::GetDefaultInstance()));
        }

        if (scriptContext->GetConfig()->IsES6ProxyEnabled())
        {
            javascriptEnumeratorIteratorPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeJavascriptEnumeratorIteratorPrototype>::GetDefaultInstance()));
        }

        if (scriptContext->GetConfig()->IsES6GeneratorsEnabled())
        {
            generatorFunctionPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, functionPrototype, nullptr,
                DeferredTypeHandler<InitializeGeneratorFunctionProtoype>::GetDefaultInstance()));

            generatorPrototype = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeGeneratorProtoype>::GetDefaultInstance()));
        }
    }

    void JavascriptLibrary::InitializeTypes()
    {
        Recycler* recycler = this->GetRecycler();
        ScriptConfiguration const *config = scriptContext->GetConfig();

        enumeratorType = StaticType::New(scriptContext, TypeIds_Enumerator, objectPrototype, nullptr);

        // Initialize Array/Argument types
        uint heapArgumentPropertyDescriptorsCount = 0;
        SimplePropertyDescriptor* heapArgumentPropertyDescriptors = nullptr;
        if (config->IsES6IteratorsEnabled())
        {
            heapArgumentPropertyDescriptors = HeapArgumentsPropertyDescriptors;
            heapArgumentPropertyDescriptorsCount = _countof(HeapArgumentsPropertyDescriptors);
        }
        else
        {
            heapArgumentPropertyDescriptorsCount = _countof(HeapArgumentsPropertyDescriptorsV11);
            heapArgumentPropertyDescriptors = HeapArgumentsPropertyDescriptorsV11;
        }
        heapArgumentsType = DynamicType::New(scriptContext, TypeIds_Arguments, objectPrototype, nullptr,
            SimpleDictionaryTypeHandler::New(scriptContext, heapArgumentPropertyDescriptors, heapArgumentPropertyDescriptorsCount, 0, 0, true, true), true, true);
        activationObjectType = DynamicType::New(scriptContext, TypeIds_ActivationObject, nullValue, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        arrayType = DynamicType::New(scriptContext, TypeIds_Array, arrayPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        nativeIntArrayType = DynamicType::New(scriptContext, TypeIds_NativeIntArray, arrayPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        copyOnAccessNativeIntArrayType = DynamicType::New(scriptContext, TypeIds_CopyOnAccessNativeIntArray, arrayPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        nativeFloatArrayType = DynamicType::New(scriptContext, TypeIds_NativeFloatArray, arrayPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);

        arrayBufferType = DynamicType::New(scriptContext, TypeIds_ArrayBuffer, arrayBufferPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        dataViewType = DynamicType::New(scriptContext, TypeIds_DataView, dataViewPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);

        int8ArrayType = DynamicType::New(scriptContext, TypeIds_Int8Array, Int8ArrayPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);

        uint8ArrayType = DynamicType::New(scriptContext, TypeIds_Uint8Array, Uint8ArrayPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        uint8ClampedArrayType = DynamicType::New(scriptContext, TypeIds_Uint8ClampedArray, Uint8ClampedArrayPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        int16ArrayType = DynamicType::New(scriptContext, TypeIds_Int16Array, Int16ArrayPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        uint16ArrayType = DynamicType::New(scriptContext, TypeIds_Uint16Array, Uint16ArrayPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        int32ArrayType = DynamicType::New(scriptContext, TypeIds_Int32Array, Int32ArrayPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        uint32ArrayType = DynamicType::New(scriptContext, TypeIds_Uint32Array, Uint32ArrayPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        float32ArrayType = DynamicType::New(scriptContext, TypeIds_Float32Array, Float32ArrayPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        float64ArrayType = DynamicType::New(scriptContext, TypeIds_Float64Array, Float64ArrayPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        int64ArrayType = DynamicType::New(scriptContext, TypeIds_Int64Array, Int64ArrayPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        uint64ArrayType = DynamicType::New(scriptContext, TypeIds_Uint64Array, Uint64ArrayPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        boolArrayType = DynamicType::New(scriptContext, TypeIds_BoolArray, BoolArrayPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        charArrayType = DynamicType::New(scriptContext, TypeIds_CharArray, CharArrayPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);

        errorType = DynamicType::New(scriptContext, TypeIds_Error, errorPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        evalErrorType = DynamicType::New(scriptContext, TypeIds_Error, evalErrorPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        rangeErrorType = DynamicType::New(scriptContext, TypeIds_Error, rangeErrorPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        referenceErrorType = DynamicType::New(scriptContext, TypeIds_Error, referenceErrorPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        syntaxErrorType = DynamicType::New(scriptContext, TypeIds_Error, syntaxErrorPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        typeErrorType = DynamicType::New(scriptContext, TypeIds_Error, typeErrorPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        uriErrorType = DynamicType::New(scriptContext, TypeIds_Error, uriErrorPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);

        if (config->IsWinRTEnabled())
        {
            winrtErrorType = DynamicType::New(scriptContext, TypeIds_Error, winrtErrorPrototype, nullptr,
                SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        }

        pixelArrayType = nullptr;
        symbolTypeStatic = nullptr;
        symbolTypeDynamic = nullptr;
        withType    = nullptr;
        proxyType   = nullptr;
        promiseType = nullptr;
        javascriptEnumeratorIteratorType = nullptr;

        if (!config->IsES6TypedArrayExtensionsEnabled())
        {
            pixelArrayType = DynamicType::New(scriptContext, TypeIds_PixelArray, pixelArrayPrototype, nullptr,
                SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        }

        // Initialize boolean types
        booleanTypeStatic = StaticType::New(scriptContext, TypeIds_Boolean, booleanPrototype, nullptr);
        booleanTypeDynamic = DynamicType::New(scriptContext, TypeIds_BooleanObject, booleanPrototype, nullptr, NullTypeHandler<false>::GetDefaultInstance(), true, true);

        if (config->IsES6SymbolEnabled())
        {
            // Initialize symbol types
            symbolTypeStatic = StaticType::New(scriptContext, TypeIds_Symbol, symbolPrototype, nullptr);
            symbolTypeDynamic = DynamicType::New(scriptContext, TypeIds_SymbolObject, symbolPrototype, nullptr, NullTypeHandler<false>::GetDefaultInstance(), true, true);
        }

        if (config->IsES6UnscopablesEnabled())
        {
            withType = StaticType::New(scriptContext, TypeIds_WithScopeObject, GetNull(), nullptr);
        }

        if (config->IsES6SpreadEnabled())
        {
            SpreadArgumentType = DynamicType::New(scriptContext, TypeIds_SpreadArgument, GetNull(), nullptr, NullTypeHandler<false>::GetDefaultInstance(), true, true);
        }

        if (config->IsES6ProxyEnabled())
        {
            // proxy's prototype is not actually used. once a proxy is used, the GetType()->GetPrototype() is not used in instanceOf style usage as they are trapped.
            // We can use GetType()->GetPrototype() in [[get]], [[set]], and [[hasProperty]] to force the prototype walk to stop at prototype so we don't need to
            // continue prototype chain walk after proxy.
            proxyType = DynamicType::New(scriptContext, TypeIds_Proxy, GetNull(), nullptr, NullTypeHandler<false>::GetDefaultInstance(), true, true);

            javascriptEnumeratorIteratorType = DynamicType::New(scriptContext, TypeIds_JavascriptEnumeratorIterator, javascriptEnumeratorIteratorPrototype, nullptr,

                SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        }

        if (config->IsES6PromiseEnabled())
        {
            promiseType = DynamicType::New(scriptContext, TypeIds_Promise, promisePrototype, nullptr, NullTypeHandler<false>::GetDefaultInstance(), true, true);
        }

        // Initialize Date types
        dateType = DynamicType::New(scriptContext, TypeIds_Date, datePrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        winrtDateType = DynamicType::New(scriptContext, TypeIds_WinRTDate, datePrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        variantDateType = StaticType::New(scriptContext, TypeIds_VariantDate, nullValue, nullptr);

        //  Initialize function types
        if (config->IsES6FunctionNameEnabled())
        {
            functionTypeHandler = &SharedFunctionWithoutPrototypeTypeHandler;
        }
        else
        {
            functionTypeHandler = NullTypeHandler<false>::GetDefaultInstance();
        }

        if (config->IsES6FunctionNameEnabled())
        {
            functionWithPrototypeTypeHandler = &SharedFunctionWithPrototypeTypeHandler;
        }
        else
        {
            functionWithPrototypeTypeHandler = &SharedFunctionWithPrototypeTypeHandlerV11;
        }
        functionWithPrototypeTypeHandler->SetHasKnownSlot0();

        externalFunctionWithDeferredPrototypeType = CreateDeferredPrototypeFunctionType(JavascriptExternalFunction::ExternalFunctionThunk, true);
        wrappedFunctionWithDeferredPrototypeType = CreateDeferredPrototypeFunctionType(JavascriptExternalFunction::WrappedFunctionThunk, true);
        stdCallFunctionWithDeferredPrototypeType = CreateDeferredPrototypeFunctionType(JavascriptExternalFunction::StdCallExternalFunctionThunk, true);
        idMappedFunctionWithPrototypeType = DynamicType::New(scriptContext, TypeIds_Function, functionPrototype, JavascriptExternalFunction::ExternalFunctionThunk,
            &SharedIdMappedFunctionWithPrototypeTypeHandler, true, true);
        externalConstructorFunctionWithDeferredPrototypeType = DynamicType::New(scriptContext, TypeIds_Function, functionPrototype, JavascriptExternalFunction::ExternalFunctionThunk,
            Js::DeferredTypeHandler<Js::JavascriptExternalFunction::DeferredInitializer>::GetDefaultInstance(), true, true);

        if (config->IsES6FunctionNameEnabled())
        {
            boundFunctionType = DynamicType::New(scriptContext, TypeIds_Function, functionPrototype, BoundFunction::NewInstance,
                GetDeferredBoundFunctionTypeHandler(scriptContext), true, true);
        }
        else
        {
            boundFunctionType = DynamicType::New(scriptContext, TypeIds_Function, functionPrototype, BoundFunction::NewInstance,
                SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        }
        crossSiteDeferredPrototypeFunctionType = CreateDeferredPrototypeFunctionType(
            scriptContext->CurrentCrossSiteThunk, true);
        crossSiteIdMappedFunctionWithPrototypeType = DynamicType::New(scriptContext, TypeIds_Function, functionPrototype, scriptContext->CurrentCrossSiteThunk,
            &SharedIdMappedFunctionWithPrototypeTypeHandler, true, true);
        crossSiteExternalConstructFunctionWithPrototypeType = DynamicType::New(scriptContext, TypeIds_Function, functionPrototype, scriptContext->CurrentCrossSiteThunk,
            Js::DeferredTypeHandler<Js::JavascriptExternalFunction::DeferredInitializer>::GetDefaultInstance(), true, true);

        // Initialize Number types
        numberTypeStatic = StaticType::New(scriptContext, TypeIds_Number, numberPrototype, nullptr);
        int64NumberTypeStatic = StaticType::New(scriptContext, TypeIds_Int64Number, numberPrototype, nullptr);
        uint64NumberTypeStatic = StaticType::New(scriptContext, TypeIds_UInt64Number, numberPrototype, nullptr);
        numberTypeDynamic = DynamicType::New(scriptContext, TypeIds_NumberObject, numberPrototype, nullptr, NullTypeHandler<false>::GetDefaultInstance(), true, true);

#ifdef SIMD_JS_ENABLED
        // SIMD
        // Initialize types
        if (Js::Configuration::Global.flags.Simdjs)
        {
            simdFloat32x4TypeStatic = StaticType::New(scriptContext, TypeIds_SIMDFloat32x4, nullValue /*prototype*/, nullptr);
            simdFloat64x2TypeStatic = StaticType::New(scriptContext, TypeIds_SIMDFloat64x2, nullValue /*prototype*/, nullptr);
            simdInt32x4TypeStatic = StaticType::New(scriptContext, TypeIds_SIMDInt32x4, nullValue /*prototype*/, nullptr);
        }
#endif

        // Initialize Object types
        for (int16 i = 0; i < PreInitializedObjectTypeCount; i++)
        {
            SimplePathTypeHandler * typeHandler =
                SimplePathTypeHandler::New(
                    scriptContext,
                    scriptContext->GetRootPath(),
                    0,
                    i * InlineSlotCountIncrement,
                    sizeof(DynamicObject),
                    true,
                    true);
            typeHandler->SetIsInlineSlotCapacityLocked();
            objectTypes[i] = DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr, typeHandler, true, true);
        }
        for (int16 i = 0; i < PreInitializedObjectTypeCount; i++)
        {
            SimplePathTypeHandler * typeHandler =
                SimplePathTypeHandler::New(
                    scriptContext,
                    scriptContext->GetRootPath(),
                    0,
                    DynamicTypeHandler::GetObjectHeaderInlinableSlotCapacity() + i * InlineSlotCountIncrement,
                    DynamicTypeHandler::GetOffsetOfObjectHeaderInlineSlots(),
                    true,
                    true);
            typeHandler->SetIsInlineSlotCapacityLocked();
            objectHeaderInlinedTypes[i] =
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr, typeHandler, true, true);
        }

        // Initialize regex types
        TypePath *const regexResultPath = TypePath::New(recycler);
        regexResultPath->Add(BuiltInPropertyRecords::input);
        regexResultPath->Add(BuiltInPropertyRecords::index);
        regexResultType = DynamicType::New(scriptContext, TypeIds_Array, arrayPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, regexResultPath, regexResultPath->GetPathLength(), JavascriptRegularExpressionResult::InlineSlotCount, sizeof(JavascriptArray), true, true), true, true);

        // Intiialize string types
        stringTypeStatic = StaticType::New(scriptContext, TypeIds_String, stringPrototype, nullptr);
        stringTypeDynamic = DynamicType::New(scriptContext, TypeIds_StringObject, stringPrototype, nullptr, NullTypeHandler<false>::GetDefaultInstance(), true, true);

        // Intiailzed Throw error object type
        throwErrorObjectType = StaticType::New(scriptContext, TypeIds_Undefined, nullValue, ThrowErrorObject::DefaultEntryPoint);

        mapType = nullptr;
        setType = nullptr;
        weakMapType = nullptr;
        weakSetType = nullptr;
        iteratorResultType = nullptr;
        arrayIteratorType = nullptr;
        mapIteratorType = nullptr;
        setIteratorType = nullptr;
        stringIteratorType = nullptr;

        if (config->IsES6MapEnabled())
        {
            mapType = DynamicType::New(scriptContext, TypeIds_Map, mapPrototype, nullptr,
                SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        }

        if (config->IsES6SetEnabled())
        {
            setType = DynamicType::New(scriptContext, TypeIds_Set, setPrototype, nullptr,
                SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        }

        if (config->IsES6WeakMapEnabled())
        {
            weakMapType = DynamicType::New(scriptContext, TypeIds_WeakMap, weakMapPrototype, nullptr,
                SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        }

        if (config->IsES6WeakSetEnabled())
        {
            weakSetType = DynamicType::New(scriptContext, TypeIds_WeakSet, weakSetPrototype, nullptr,
                SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        }

        if (config->IsES6IteratorsEnabled())
        {
            TypePath *const iteratorResultPath = TypePath::New(recycler);
            iteratorResultPath->Add(BuiltInPropertyRecords::value);
            iteratorResultPath->Add(BuiltInPropertyRecords::done);
            iteratorResultType = DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                SimplePathTypeHandler::New(scriptContext, iteratorResultPath, iteratorResultPath->GetPathLength(), 2, sizeof(DynamicObject), true, true), true, true);

            arrayIteratorType = DynamicType::New(scriptContext, TypeIds_ArrayIterator, arrayIteratorPrototype, nullptr,
                SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
            mapIteratorType = DynamicType::New(scriptContext, TypeIds_MapIterator, mapIteratorPrototype, nullptr,
                SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
            setIteratorType = DynamicType::New(scriptContext, TypeIds_SetIterator, setIteratorPrototype, nullptr,
                SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
            stringIteratorType = DynamicType::New(scriptContext, TypeIds_StringIterator, stringIteratorPrototype, nullptr,
                SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        }

        if (config->IsES6GeneratorsEnabled())
        {
            generatorConstructorPrototypeObjectType = DynamicType::New(scriptContext, TypeIds_Object, generatorPrototype, nullptr,
                // TODO[ianhall][generators]: NullTypeHandler<true>?
                NullTypeHandler<false>::GetDefaultInstance(), true, true);

            generatorConstructorPrototypeObjectType->SetHasNoEnumerableProperties(true);
        }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        debugDisposableObjectType = DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);

        debugFuncExecutorInDisposeObjectType = DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
#endif
    }

    void JavascriptLibrary::InitializeGeneratorFunction(DynamicObject *function, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(function, function->GetType()->GetLibrary()->functionWithPrototypeTypeHandler);
        function->SetPropertyWithAttributes(PropertyIds::prototype, function->GetType()->GetLibrary()->CreateGeneratorConstructorPrototypeObject(), PropertyWritable, nullptr);

        if (function->GetScriptContext()->GetConfig()->IsES6FunctionNameEnabled())
        {
            function->SetPropertyWithAttributes(PropertyIds::name, ((Js::JavascriptFunction*)function)->GetDisplayName(true), PropertyConfigurable, nullptr);
        }
    }


    template<bool addPrototype>
    void JavascriptLibrary::InitializeFunction(DynamicObject *function, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        JavascriptLibrary* javascriptLibrary = function->GetType()->GetLibrary();
        if (!addPrototype)
        {
            typeHandler->Convert(function, javascriptLibrary->functionTypeHandler);
        }
        else
        {
            typeHandler->Convert(function, javascriptLibrary->functionWithPrototypeTypeHandler);
            function->SetProperty(PropertyIds::prototype, javascriptLibrary->CreateConstructorPrototypeObject((Js::JavascriptFunction *)function), PropertyOperation_None, nullptr);
        }

        if (function->GetScriptContext()->GetConfig()->IsES6FunctionNameEnabled())
        {
            if (ScriptFunction::Is(function))
            {
                ScriptFunction *scriptFunction  = Js::ScriptFunction::FromVar(function);
                if (scriptFunction->GetFunctionProxy()->EnsureDeserialized()->GetIsStaticNameFunction())
                {
                    return;
                }
            }
            function->SetPropertyWithAttributes(PropertyIds::name, ((Js::JavascriptFunction*)function)->GetDisplayName(true), PropertyConfigurable, nullptr);
        }
    }


    template<bool isNameAvailable>
    class InitializeBoundFunctionDeferredTypeHandlerFilter
    {
    public:
        static bool HasFilter() { return true; }
        static bool HasProperty(PropertyId propertyId)
        {
            switch (propertyId)
            {
            case PropertyIds::name:
                return isNameAvailable;
            }
            return false;
        }
    };

    template<bool isNameAvailable>
    class InitializeFunctionDeferredTypeHandlerFilter
    {
    public:
        static bool HasFilter() { return true; }
        static bool HasProperty(PropertyId propertyId)
        {
            switch (propertyId)
            {
            case PropertyIds::prototype:
                return true;
            case PropertyIds::name:
                return isNameAvailable;
            }
            return false;
        }
    };

    DynamicTypeHandler * JavascriptLibrary::GetDeferredBoundFunctionTypeHandler(ScriptContext* scriptContext)
    {
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            return DeferredTypeHandler<InitializeFunction<false>, InitializeBoundFunctionDeferredTypeHandlerFilter<true>>::GetDefaultInstance();
        }
        else
        {
            return DeferredTypeHandler<InitializeFunction<false>, InitializeBoundFunctionDeferredTypeHandlerFilter<false>>::GetDefaultInstance();
        }
    }

    DynamicTypeHandler * JavascriptLibrary::GetDeferredPrototypeGeneratorFunctionTypeHandler(ScriptContext* scriptContext)
    {
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            return DeferredTypeHandler<InitializeGeneratorFunction, InitializeFunctionDeferredTypeHandlerFilter<true>>::GetDefaultInstance();
        }
        else
        {
            return DeferredTypeHandler<InitializeGeneratorFunction, InitializeFunctionDeferredTypeHandlerFilter<false>>::GetDefaultInstance();
        }

    }
    DynamicTypeHandler * JavascriptLibrary::GetDeferredPrototypeFunctionTypeHandler(ScriptContext* scriptContext)
    {
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            return DeferredTypeHandler<InitializeFunction<true>, InitializeFunctionDeferredTypeHandlerFilter<true>>::GetDefaultInstance();
        }
        else
        {
            return DeferredTypeHandler<InitializeFunction<true>, InitializeFunctionDeferredTypeHandlerFilter<false>>::GetDefaultInstance();
        }
    }
    DynamicType * JavascriptLibrary::CreateDeferredPrototypeGeneratorFunctionType(JavascriptMethod entrypoint, bool isShared)
    {
        return DynamicType::New(scriptContext, TypeIds_Function, generatorFunctionPrototype, entrypoint,
            GetDeferredPrototypeGeneratorFunctionTypeHandler(scriptContext), isShared, isShared);
    }

    DynamicTypeHandler * JavascriptLibrary::GetDeferredFunctionTypeHandler()
    {
        if (this->GetScriptContext()->GetConfig()->IsES6FunctionNameEnabled())
        {
            class InitializeFunctionDeferredTypeHandlerFilterName
            {
            public:
                static bool HasFilter() { return true; }
                static bool HasProperty(PropertyId propertyId)
                {
                    switch (propertyId)
                    {
                    case PropertyIds::name:
                        return true;
                    }
                    return false;
                }
            };
            return DeferredTypeHandler<InitializeFunction<false>, InitializeFunctionDeferredTypeHandlerFilterName>::GetDefaultInstance();
        }
        return functionTypeHandler;
    }

    DynamicType * JavascriptLibrary::CreateDeferredPrototypeFunctionType(JavascriptMethod entrypoint, bool isShared)
    {
        return DynamicType::New(scriptContext, TypeIds_Function, functionPrototype, entrypoint,
            GetDeferredPrototypeFunctionTypeHandler(scriptContext), isShared, isShared);
    }
    DynamicType * JavascriptLibrary::CreateFunctionType(JavascriptMethod entrypoint, RecyclableObject* prototype)
    {
        if (prototype == nullptr)
        {
            prototype = functionPrototype;
        }

        return DynamicType::New(scriptContext, TypeIds_Function, prototype, entrypoint,
            GetDeferredFunctionTypeHandler(), false, false);
    }

    DynamicType * JavascriptLibrary::CreateFunctionWithLengthType(FunctionInfo * functionInfo)
    {
        return CreateFunctionWithLengthType(this->GetFunctionPrototype(), functionInfo);
    }

    DynamicType * JavascriptLibrary::CreateFunctionWithLengthAndPrototypeType(FunctionInfo * functionInfo)
    {
        return CreateFunctionWithLengthAndPrototypeType(this->GetFunctionPrototype(), functionInfo);
    }

    DynamicType * JavascriptLibrary::CreateFunctionWithLengthType(DynamicObject * prototype, FunctionInfo * functionInfo)
    {
        Assert(!functionInfo->HasBody());
        return DynamicType::New(scriptContext, TypeIds_Function, prototype,
            this->inProfileMode? ProfileEntryThunk : functionInfo->GetOriginalEntryPoint(),
            &SharedFunctionWithLengthTypeHandler);
    }

    DynamicType * JavascriptLibrary::CreateFunctionWithLengthAndPrototypeType(DynamicObject * prototype, FunctionInfo * functionInfo)
    {
        Assert(!functionInfo->HasBody());
        return DynamicType::New(scriptContext, TypeIds_Function, prototype,
            this->inProfileMode? ProfileEntryThunk : functionInfo->GetOriginalEntryPoint(),
            SimpleDictionaryTypeHandler::New(scriptContext, FunctionWithLengthAndPrototypeTypeDescriptors, _countof(FunctionWithLengthAndPrototypeTypeDescriptors), 0, 0));
    }

    void JavascriptLibrary::InitializeProperties(ThreadContext * threadContext)
    {
        if ( threadContext->GetMaxPropertyId() < PropertyIds::_countJSOnlyProperty )
        {
            threadContext->UncheckedAddBuiltInPropertyId();
        }
    }

    void JavascriptLibrary::CopyOnWriteSpecialGlobals(ScriptContext * scriptContext, GlobalObject * globalObject, JavascriptLibrary * originalLibrary)
    {
        VERIFY_COPY_ON_WRITE_ENABLED();

        // These globals are referenced directly by copy-on-write and therefore need to be initialized early.
        undefinedValue = RecyclerNew(recycler, RecyclableObject,
            StaticType::New(scriptContext, TypeIds_Undefined, nullValue, nullptr));
        scriptContext->RecordCopyOnWrite(originalLibrary->undefinedValue, undefinedValue);

        // These are special because they are used in copy-on-write for boolean. They don't need to be recorded
        // because they are primative values which are not recorded.
        booleanTrue = RecyclerNew(recycler, JavascriptBoolean, true, booleanTypeStatic);
        booleanFalse = RecyclerNew(recycler, JavascriptBoolean, false, booleanTypeStatic);

        // These are special because they are used when cloning a string
        // Empty string is special since it is referenced by string copy on write so just create a new one.
        emptyString = CreateEmptyString();
        scriptContext->RecordCopyOnWrite(originalLibrary->emptyString, emptyString);

        // Null string is special since it must be distinct from empty.
        nullString = CreateEmptyString();
        scriptContext->RecordCopyOnWrite(originalLibrary->nullString, nullString);
    }

    template <class T>
    inline T *CopyObject(ScriptContext * scriptContext, T *instance)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        return instance ? (T *)scriptContext->CopyOnWrite(instance) : nullptr;
    }

    void JavascriptLibraryBase::Finalize(bool isShutdown)
    {
        if (scriptContext)
        {
            // Clear the weak reference dictionary so we don't need to clean them
            // during PostCollectCallBack before Dispose deleting the script context.
            scriptContext->ResetWeakReferenceDicitionaryList();
        }
    }

    void JavascriptLibraryBase::Dispose(bool isShutdown)
    {
        if (scriptContext)
        {
            if (isShutdown)
            {
                // during shut down the global object might not be closed yet.
                // Clear the global object from the script context so it doesn't
                // get unpinned (which may fail because the recycler is shutting down)
                scriptContext->Close(true);
                scriptContext->ClearGlobalObject();
            }
            else
            {
                Assert(scriptContext->IsClosed());
            }
            HeapDelete(scriptContext);
            scriptContext = nullptr;
        }
    }

    void JavascriptLibrary::CopyOnWriteGlobal(ScriptContext * scriptContext, GlobalObject * globalObject, JavascriptLibrary * originalLibrary)
    {
        VERIFY_COPY_ON_WRITE_ENABLED();

        // VTable addresses are just static values, we can copy them directly.
        memcpy_s(vtableAddresses, VTableValue::Count * sizeof(INT_PTR), originalLibrary->vtableAddresses, VTableValue::Count * sizeof(INT_PTR));
        constructorCacheDefaultInstance = originalLibrary->constructorCacheDefaultInstance;
        absDoubleCst = originalLibrary->absDoubleCst;
        uintConvertConst = originalLibrary->uintConvertConst;
        jnHelperMethods = originalLibrary->jnHelperMethods;

        pi = CopyObject(scriptContext, originalLibrary->pi);
        nan = CopyObject(scriptContext, originalLibrary->nan);
        negativeInfinite = CopyObject(scriptContext, originalLibrary->negativeInfinite);
        positiveInfinite = CopyObject(scriptContext, originalLibrary->positiveInfinite);
        minValue = CopyObject(scriptContext, originalLibrary->minValue);
        maxValue = CopyObject(scriptContext, originalLibrary->maxValue);
        negativeZero = CopyObject(scriptContext, originalLibrary->negativeZero);

        parseIntFunctionObject = CopyObject(scriptContext, originalLibrary->parseIntFunctionObject);
        parseFloatFunctionObject = CopyObject(scriptContext, originalLibrary->parseFloatFunctionObject);

        // Empty and Null strings are handled in CopyOnWriteSpecialGlobals
        whackString = CopyObject(scriptContext, originalLibrary->whackString);

        undefinedDisplayString = CopyObject(scriptContext, originalLibrary->undefinedDisplayString);
        nanDisplayString = CopyObject(scriptContext, originalLibrary->nanDisplayString);
        quotesString = CopyObject(scriptContext, originalLibrary->quotesString);
        nullDisplayString = CopyObject(scriptContext, originalLibrary->nullDisplayString);
        unknownDisplayString = CopyObject(scriptContext, originalLibrary->unknownDisplayString);
        commaDisplayString = CopyObject(scriptContext, originalLibrary->commaDisplayString);
        commaSpaceDisplayString = CopyObject(scriptContext, originalLibrary->commaSpaceDisplayString);
        trueDisplayString = CopyObject(scriptContext, originalLibrary->trueDisplayString);
        falseDisplayString = CopyObject(scriptContext, originalLibrary->falseDisplayString);
        lengthDisplayString = CopyObject(scriptContext, originalLibrary->lengthDisplayString);
        objectDisplayString = CopyObject(scriptContext, originalLibrary->objectDisplayString);
        errorDisplayString = CopyObject(scriptContext, originalLibrary->errorDisplayString);
        stringTypeDisplayString = CopyObject(scriptContext, originalLibrary->stringTypeDisplayString);
        functionPrefixString = CopyObject(scriptContext, originalLibrary->functionPrefixString);
        generatorFunctionPrefixString = CopyObject(scriptContext, originalLibrary->generatorFunctionPrefixString);
        functionDisplayString = CopyObject(scriptContext, originalLibrary->functionDisplayString);
        xDomainFunctionDisplayString = CopyObject(scriptContext, originalLibrary->xDomainFunctionDisplayString);
        invalidDateString = CopyObject(scriptContext, originalLibrary->invalidDateString);
        objectTypeDisplayString = CopyObject(scriptContext, originalLibrary->objectTypeDisplayString);
        functionTypeDisplayString = CopyObject(scriptContext, originalLibrary->functionTypeDisplayString);
        booleanTypeDisplayString = CopyObject(scriptContext, originalLibrary->booleanTypeDisplayString);
        numberTypeDisplayString = CopyObject(scriptContext, originalLibrary->numberTypeDisplayString);

#ifdef SIMD_JS_ENABLED
        if (Js::Configuration::Global.flags.Simdjs)
        {
            simdFloat32x4DisplayString = CopyObject(scriptContext, originalLibrary->simdFloat32x4DisplayString);
            simdFloat64x2DisplayString = CopyObject(scriptContext, originalLibrary->simdFloat64x2DisplayString);
            simdInt32x4DisplayString = CopyObject(scriptContext, originalLibrary->simdInt32x4DisplayString);
        }
#endif

        symbolTypeDisplayString = CopyObject(scriptContext, originalLibrary->symbolTypeDisplayString);

        defaultAccessorFunction = CreateNonProfiledFunction(&JavascriptOperators::EntryInfo::DefaultAccessor);
        stackTraceAccessorFunction = CopyObject(scriptContext, originalLibrary->stackTraceAccessorFunction);

        throwTypeErrorAccessorFunction = CopyObject(scriptContext, originalLibrary->throwTypeErrorAccessorFunction);
        throwTypeErrorCallerAccessorFunction = CopyObject(scriptContext, originalLibrary->throwTypeErrorCallerAccessorFunction);
        throwTypeErrorCalleeAccessorFunction = CopyObject(scriptContext, originalLibrary->throwTypeErrorCalleeAccessorFunction);
        throwTypeErrorArgumentsAccessorFunction = CopyObject(scriptContext, originalLibrary->throwTypeErrorArgumentsAccessorFunction);
        debugObjectNonUserGetterFunction = CopyObject(scriptContext, originalLibrary->debugObjectNonUserGetterFunction);
        debugObjectNonUserSetterFunction = CopyObject(scriptContext, originalLibrary->debugObjectNonUserSetterFunction);
        debugObjectDebugModeGetterFunction = CopyObject(scriptContext, originalLibrary->debugObjectDebugModeGetterFunction);

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        debugObjectFaultInjectionCookieGetterFunction = CopyObject(scriptContext, originalLibrary->debugObjectFaultInjectionCookieGetterFunction);
        debugObjectFaultInjectionCookieSetterFunction = CopyObject(scriptContext, originalLibrary->debugObjectFaultInjectionCookieSetterFunction);
#endif

        __proto__getterFunction = CopyObject(scriptContext, originalLibrary->__proto__getterFunction);
        __proto__setterFunction = CopyObject(scriptContext, originalLibrary->__proto__setterFunction);

        arrayPrototypeToStringFunction = CopyObject(scriptContext, originalLibrary->arrayPrototypeToStringFunction);
        arrayPrototypeToLocaleStringFunction = CopyObject(scriptContext, originalLibrary->arrayPrototypeToLocaleStringFunction);

        identityFunction = CopyObject(scriptContext, originalLibrary->identityFunction);
        throwerFunction = CopyObject(scriptContext, originalLibrary->throwerFunction);

        randSeed = originalLibrary->randSeed;

        // These are added directly to ensure they get the right properties.
        PropertyAttributes attributes = PropertyNone;
        AddMember(globalObject, PropertyIds::NaN, nan, attributes);
        AddMember(globalObject, PropertyIds::Infinity, positiveInfinite, attributes);
        AddMember(globalObject, PropertyIds::undefined, undefinedValue, attributes);

        // The built-in functions are skipped here because the default value copying from GlobalObject::CopyOnWrite() will copy them.
        // Special code here is only necessary if JavascriptLibrary maintains a cached copy of the global.

        // Similarly we only need to cache the copy on write versions here, the generic copy code will create the
        // property slots.
        objectConstructor = CopyObject(scriptContext, originalLibrary->objectConstructor);
        arrayConstructor = CopyObject(scriptContext, originalLibrary->arrayConstructor);
        booleanConstructor = CopyObject(scriptContext, originalLibrary->booleanConstructor);
        dateConstructor = CopyObject(scriptContext, originalLibrary->dateConstructor);
        functionConstructor = CopyObject(scriptContext, originalLibrary->functionConstructor);

        mathObject = CopyObject(scriptContext, originalLibrary->mathObject);
#ifdef SIMD_JS_ENABLED
        if (AutoSystemInfo::Data.SSE2Available() && Js::Configuration::Global.flags.Simdjs)
        {
            simdObject = CopyObject(scriptContext, originalLibrary->simdObject);
        }
#endif
        debugObject = CopyObject(scriptContext, originalLibrary->debugObject);
        diagnosticsScriptObject = CopyObject(scriptContext, originalLibrary->diagnosticsScriptObject);

        numberConstructor = CopyObject(scriptContext, originalLibrary->numberConstructor);
        stringConstructor = CopyObject(scriptContext, originalLibrary->stringConstructor);
        regexConstructor = CopyObject(scriptContext, originalLibrary->regexConstructor);
        pixelArrayConstructor = CopyObject(scriptContext, originalLibrary->pixelArrayConstructor);

        // Note that CopyObject handles nullptr. Initialize to nullptr if these were nullptr in original library.
        arrayBufferConstructor = CopyObject(scriptContext, originalLibrary->arrayBufferConstructor);
        typedArrayConstructor = CopyObject(scriptContext, originalLibrary->typedArrayConstructor);
        dataViewConstructor = CopyObject(scriptContext, originalLibrary->dataViewConstructor);
        Int8ArrayConstructor = CopyObject(scriptContext, originalLibrary->Int8ArrayConstructor);
        Uint8ArrayConstructor = CopyObject(scriptContext, originalLibrary->Uint8ArrayConstructor);
        Uint8ClampedArrayConstructor = CopyObject(scriptContext, originalLibrary->Uint8ClampedArrayConstructor);
        Int16ArrayConstructor = CopyObject(scriptContext, originalLibrary->Int16ArrayConstructor);
        Uint16ArrayConstructor = CopyObject(scriptContext, originalLibrary->Uint16ArrayConstructor);
        Int32ArrayConstructor = CopyObject(scriptContext, originalLibrary->Int32ArrayConstructor);
        Uint32ArrayConstructor = CopyObject(scriptContext, originalLibrary->Uint32ArrayConstructor);
        Float32ArrayConstructor = CopyObject(scriptContext, originalLibrary->Float32ArrayConstructor);
        Float64ArrayConstructor = CopyObject(scriptContext, originalLibrary->Float64ArrayConstructor);

        errorConstructor = CopyObject(scriptContext, originalLibrary->errorConstructor);
        evalErrorConstructor = CopyObject(scriptContext, originalLibrary->evalErrorConstructor);
        rangeErrorConstructor = CopyObject(scriptContext, originalLibrary->rangeErrorConstructor);
        referenceErrorConstructor = CopyObject(scriptContext, originalLibrary->referenceErrorConstructor);
        syntaxErrorConstructor = CopyObject(scriptContext, originalLibrary->syntaxErrorConstructor);
        typeErrorConstructor = CopyObject(scriptContext, originalLibrary->typeErrorConstructor);
        uriErrorConstructor = CopyObject(scriptContext, originalLibrary->uriErrorConstructor);
        winrtErrorConstructor = CopyObject(scriptContext, originalLibrary->winrtErrorConstructor);

        mapConstructor = CopyObject(scriptContext, originalLibrary->mapConstructor);
        setConstructor = CopyObject(scriptContext, originalLibrary->setConstructor);
        weakMapConstructor = CopyObject(scriptContext, originalLibrary->weakMapConstructor);
        weakSetConstructor = CopyObject(scriptContext, originalLibrary->weakSetConstructor);

        proxyConstructor = CopyObject(scriptContext, originalLibrary->proxyConstructor);
        reflectObject = CopyObject(scriptContext, originalLibrary->reflectObject);

        symbolConstructor = CopyObject(scriptContext, originalLibrary->symbolConstructor);
        promiseConstructor = CopyObject(scriptContext, originalLibrary->promiseConstructor);

        generatorFunctionConstructor = CopyObject(scriptContext, originalLibrary->generatorFunctionConstructor);

        debuggerDeadZoneBlockVariableString = CopyObject(scriptContext, originalLibrary->debuggerDeadZoneBlockVariableString);

        JSONObject = nullptr;
        IntlObject = nullptr;
        winRTPromiseConstructor = nullptr;

        if (originalLibrary->JSONObject)
        {
            JSONObject = CopyObject(scriptContext, originalLibrary->JSONObject);
        }
        else
        {
            JSONObject = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeJSONObject>::GetDefaultInstance()));
            AddMember(globalObject, PropertyIds::JSON, JSONObject);
        }

#ifdef ENABLE_INTL_OBJECT
        //If we are or win8 or later; then the default value for the flag; or enabled value for the flag (-Intl and -Intl-) dictates if Intl is available.
        //If we are on win7; then the flag has to be specified force it on (or off); the default value is ignored
        if (scriptContext->GetConfig()->IsIntlEnabled())
        {
            if (originalLibrary->IntlObject)
            {
                IntlObject = CopyObject(scriptContext, originalLibrary->IntlObject);
            }
            else
            {
                IntlObject = DynamicObject::New(recycler,
                    DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                    DeferredTypeHandler<InitializeIntlObject>::GetDefaultInstance()));
                AddMember(globalObject, PropertyIds::Intl, IntlObject);
            }
        }
#endif

#ifdef ENABLE_PROJECTION
        if (scriptContext->GetConfig()->IsWinRTEnabled()
            && originalLibrary->winRTPromiseConstructor)
        {
            winRTPromiseConstructor = CopyObject(scriptContext, originalLibrary->winRTPromiseConstructor);
        }
#endif

        if (originalLibrary->engineInterfaceObject)
        {
            this->engineInterfaceObject = CopyObject(scriptContext, originalLibrary->engineInterfaceObject);
        }
        else
        {
            this->engineInterfaceObject = EngineInterfaceObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_EngineInterfaceObject, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeEngineInterfaceObject>::GetDefaultInstance()));
        }

        // The nullEnumerators instances do not need to be tracked between copies so don't bother
        // recording the copy.
        nullEnumerator = RecyclerNew(this->recycler, NullEnumerator, scriptContext);

        defaultPropertyDescriptor.SetValue(undefinedValue);
        defaultPropertyDescriptor.SetWritable(false);
        defaultPropertyDescriptor.SetGetter(defaultAccessorFunction);
        defaultPropertyDescriptor.SetSetter(defaultAccessorFunction);
        defaultPropertyDescriptor.SetEnumerable(false);
        defaultPropertyDescriptor.SetConfigurable(false);
    }

#define  ADD_TYPEDARRAY_CONSTRUCTOR(typedarrayConstructor, TypedArray) \
    typedarrayConstructor = CreateBuiltinConstructor(&TypedArray##::EntryInfo::NewInstance, \
    DeferredTypeHandler<Initialize##TypedArray##Constructor>::GetDefaultInstance()); \
            AddFunction(globalObject, PropertyIds::TypedArray, typedarrayConstructor); \

    void JavascriptLibrary::InitializeGlobal(GlobalObject * globalObject)
    {
        // !!! Warning !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // Any changes to this function might require a corresponding change to CopyOnWriteGlobal() above.
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // WARNING: Any objects here using DeferredTypeHandler need to appear in EnsureLibraryReadyForHybridDebugging
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        RecyclableObject* globalObjectPrototype = GetObjectPrototype();
        globalObject->SetPrototype(globalObjectPrototype);
        Recycler * recycler = this->GetRecycler();

        pi = JavascriptNumber::New(Math::PI, scriptContext);
        nan = JavascriptNumber::New(JavascriptNumber::NaN, scriptContext);
        negativeInfinite = JavascriptNumber::New(JavascriptNumber::NEGATIVE_INFINITY, scriptContext);
        positiveInfinite = JavascriptNumber::New(JavascriptNumber::POSITIVE_INFINITY, scriptContext);
        minValue = JavascriptNumber::New(JavascriptNumber::MIN_VALUE, scriptContext);
        maxValue = JavascriptNumber::New(JavascriptNumber::MAX_VALUE, scriptContext);
        negativeZero = JavascriptNumber::New(JavascriptNumber::NEG_ZERO, scriptContext);
        undefinedValue = RecyclerNew(recycler, RecyclableObject,
            StaticType::New(scriptContext, TypeIds_Undefined, nullValue, nullptr));
        missingPropertyHolder = RecyclerNewPlus(recycler, sizeof(Var), DynamicObject,
            DynamicType::New(scriptContext, TypeIds_Object, nullValue, nullptr, &MissingPropertyHolderTypeHandler));
        MissingPropertyTypeHandler::SetUndefinedPropertySlot(missingPropertyHolder);

        emptyString = CreateEmptyString(); // Must be created before other calls to CreateString
        nullString = CreateEmptyString(); // Must be distinct from emptyString (for the DOM)
        quotesString = CreateStringFromCppLiteral(L"\"\"");
        whackString = CreateStringFromCppLiteral(L"/");
        undefinedDisplayString = CreateStringFromCppLiteral(L"undefined");
        nanDisplayString = CreateStringFromCppLiteral(L"NaN");
        nullDisplayString = CreateStringFromCppLiteral(L"null");
        unknownDisplayString = CreateStringFromCppLiteral(L"unknown");
        commaDisplayString = CreateStringFromCppLiteral(L",");
        commaSpaceDisplayString = CreateStringFromCppLiteral(L", ");
        trueDisplayString = CreateStringFromCppLiteral(L"true");
        falseDisplayString = CreateStringFromCppLiteral(L"false");
        lengthDisplayString = CreateStringFromCppLiteral(L"length");
        objectDisplayString = CreateStringFromCppLiteral(L"[object Object]");
        errorDisplayString = CreateStringFromCppLiteral(L"[object Error]");
        stringTypeDisplayString = CreateStringFromCppLiteral(L"string");
        functionPrefixString = CreateStringFromCppLiteral(L"function ");
        generatorFunctionPrefixString = CreateStringFromCppLiteral(L"function* ");
        functionDisplayString = CreateStringFromCppLiteral(JS_DISPLAY_STRING_FUNCTION_ANONYMOUS);
        xDomainFunctionDisplayString = CreateStringFromCppLiteral(L"\012function anonymous() {\012    [x-domain code]\012}\012");
        invalidDateString = CreateStringFromCppLiteral(L"Invalid Date");
        objectTypeDisplayString = CreateStringFromCppLiteral(L"object");
        functionTypeDisplayString = CreateStringFromCppLiteral(L"function");
        booleanTypeDisplayString = CreateStringFromCppLiteral(L"boolean");
        numberTypeDisplayString = CreateStringFromCppLiteral(L"number");

#ifdef SIMD_JS_ENABLED
        if (Js::Configuration::Global.flags.Simdjs)
        {
            simdFloat32x4DisplayString = CreateStringFromCppLiteral(L"float32x4");
            simdFloat64x2DisplayString = CreateStringFromCppLiteral(L"float64x2");
            simdInt32x4DisplayString = CreateStringFromCppLiteral(L"int32x4");
        }
#endif

        if (scriptContext->GetConfig()->IsES6SymbolEnabled())
        {
            symbolTypeDisplayString = CreateStringFromCppLiteral(L"symbol");

            symbolHasInstance = CreateSymbol(BuiltInPropertyRecords::_symbolHasInstance);
            symbolIsConcatSpreadable = CreateSymbol(BuiltInPropertyRecords::_symbolIsConcatSpreadable);
            symbolIterator = CreateSymbol(BuiltInPropertyRecords::_symbolIterator);
            symbolSpecies = CreateSymbol(BuiltInPropertyRecords::_symbolSpecies);
            symbolToPrimitive = CreateSymbol(BuiltInPropertyRecords::_symbolToPrimitive);
            symbolToStringTag = CreateSymbol(BuiltInPropertyRecords::_symbolToStringTag);
            symbolUnscopables = CreateSymbol(BuiltInPropertyRecords::_symbolUnscopables);
        }
        else
        {
            symbolTypeDisplayString = nullptr;
            symbolHasInstance = nullptr;
            symbolIsConcatSpreadable = nullptr;
            symbolIterator = nullptr;
            symbolSpecies = nullptr;
            symbolToPrimitive = nullptr;
            symbolToStringTag = nullptr;
            symbolUnscopables = nullptr;
        }

        debuggerDeadZoneBlockVariableString = CreateStringFromCppLiteral(L"[Uninitialized block variable]");
        defaultAccessorFunction = CreateNonProfiledFunction(&JavascriptOperators::EntryInfo::DefaultAccessor);

        if (scriptContext->GetConfig()->IsErrorStackTraceEnabled())
        {
            stackTraceAccessorFunction = CreateNonProfiledFunction(&JavascriptExceptionOperators::EntryInfo::StackTraceAccessor);
            stackTraceAccessorFunction->SetPropertyWithAttributes(PropertyIds::length, TaggedInt::ToVarUnchecked(0), PropertyNone, nullptr);
        }

        throwTypeErrorAccessorFunction = CreateNonProfiledFunction(&JavascriptExceptionOperators::EntryInfo::ThrowTypeErrorAccessor);
        throwTypeErrorAccessorFunction->SetPropertyWithAttributes(PropertyIds::length, TaggedInt::ToVarUnchecked(0), PropertyNone, nullptr);

        throwTypeErrorCallerAccessorFunction = CreateNonProfiledFunction(&JavascriptExceptionOperators::EntryInfo::ThrowTypeErrorCallerAccessor);
        throwTypeErrorCallerAccessorFunction->SetPropertyWithAttributes(PropertyIds::length, TaggedInt::ToVarUnchecked(0), PropertyNone, nullptr);

        throwTypeErrorCalleeAccessorFunction = CreateNonProfiledFunction(&JavascriptExceptionOperators::EntryInfo::ThrowTypeErrorCalleeAccessor);
        throwTypeErrorCalleeAccessorFunction->SetPropertyWithAttributes(PropertyIds::length, TaggedInt::ToVarUnchecked(0), PropertyNone, nullptr);

        throwTypeErrorArgumentsAccessorFunction = CreateNonProfiledFunction(&JavascriptExceptionOperators::EntryInfo::ThrowTypeErrorArgumentsAccessor);
        throwTypeErrorArgumentsAccessorFunction->SetPropertyWithAttributes(PropertyIds::length, TaggedInt::ToVarUnchecked(0), PropertyNone, nullptr);

        if (scriptContext->GetConfig()->Is__proto__Enabled())
        {
            __proto__getterFunction = CreateNonProfiledFunction(&ObjectPrototypeObject::EntryInfo::__proto__getter);
            __proto__getterFunction->SetPropertyWithAttributes(PropertyIds::length, TaggedInt::ToVarUnchecked(0), PropertyNone, nullptr);

            __proto__setterFunction = CreateNonProfiledFunction(&ObjectPrototypeObject::EntryInfo::__proto__setter);
            __proto__setterFunction->SetPropertyWithAttributes(PropertyIds::length, TaggedInt::ToVarUnchecked(1), PropertyNone, nullptr);
        }

        if (scriptContext->GetConfig()->IsES6PromiseEnabled())
        {
            identityFunction = CreateNonProfiledFunction(&JavascriptPromise::EntryInfo::Identity);
            identityFunction->SetPropertyWithAttributes(PropertyIds::length, TaggedInt::ToVarUnchecked(1), PropertyNone, nullptr);

            throwerFunction = CreateNonProfiledFunction(&JavascriptPromise::EntryInfo::Thrower);
            throwerFunction->SetPropertyWithAttributes(PropertyIds::length, TaggedInt::ToVarUnchecked(1), PropertyNone, nullptr);
        }

        booleanTrue = RecyclerNew(recycler, JavascriptBoolean, true, booleanTypeStatic);
        booleanFalse = RecyclerNew(recycler, JavascriptBoolean, false, booleanTypeStatic);
        randSeed = 0;

        AddMember(globalObject, PropertyIds::NaN, nan, PropertyNone);
        AddMember(globalObject, PropertyIds::Infinity, positiveInfinite, PropertyNone);
        AddMember(globalObject, PropertyIds::undefined, undefinedValue, PropertyNone);

        // Note: Any global function added/removed/changed here should also be updated in ScriptContext::RegisterBuiltinFunctions
        // so that the new functions show up in the profiler too.
        JavascriptFunction ** builtinFuncs = this->GetBuiltinFunctions();

        evalFunctionObject = AddFunctionToLibraryObject(globalObject, PropertyIds::eval, &GlobalObject::EntryInfo::Eval, 1);
        parseIntFunctionObject = AddFunctionToLibraryObject(globalObject, PropertyIds::parseInt, &GlobalObject::EntryInfo::ParseInt, 2);
        builtinFuncs[BuiltinFunction::GlobalObject_ParseInt] = parseIntFunctionObject;
        parseFloatFunctionObject = AddFunctionToLibraryObject(globalObject, PropertyIds::parseFloat, &GlobalObject::EntryInfo::ParseFloat, 1);
        AddFunctionToLibraryObject(globalObject, PropertyIds::isNaN, &GlobalObject::EntryInfo::IsNaN, 1);
        AddFunctionToLibraryObject(globalObject, PropertyIds::isFinite, &GlobalObject::EntryInfo::IsFinite, 1);
        AddFunctionToLibraryObject(globalObject, PropertyIds::decodeURI, &GlobalObject::EntryInfo::DecodeURI, 1);
        AddFunctionToLibraryObject(globalObject, PropertyIds::decodeURIComponent, &GlobalObject::EntryInfo::DecodeURIComponent, 1);
        AddFunctionToLibraryObject(globalObject, PropertyIds::encodeURI, &GlobalObject::EntryInfo::EncodeURI, 1);
        AddFunctionToLibraryObject(globalObject, PropertyIds::encodeURIComponent, &GlobalObject::EntryInfo::EncodeURIComponent, 1);
        AddFunctionToLibraryObject(globalObject, PropertyIds::escape, &GlobalObject::EntryInfo::Escape, 1);
        AddFunctionToLibraryObject(globalObject, PropertyIds::unescape, &GlobalObject::EntryInfo::UnEscape, 1);

        if (scriptContext->GetConfig()->SupportsES3Extensions())
        {
            AddFunctionToLibraryObject(globalObject, PropertyIds::ScriptEngine, &GlobalObject::EntryInfo::ScriptEngine, 0);
            AddFunctionToLibraryObject(globalObject, PropertyIds::ScriptEngineMajorVersion, &GlobalObject::EntryInfo::ScriptEngineMajorVersion, 0);
            AddFunctionToLibraryObject(globalObject, PropertyIds::ScriptEngineMinorVersion, &GlobalObject::EntryInfo::ScriptEngineMinorVersion, 0);
            AddFunctionToLibraryObject(globalObject, PropertyIds::ScriptEngineBuildVersion, &GlobalObject::EntryInfo::ScriptEngineBuildVersion, 0);
            AddFunctionToLibraryObject(globalObject, PropertyIds::CollectGarbage, &GlobalObject::EntryInfo::CollectGarbage, 0);
        }

#ifdef IR_VIEWER
        if (Js::Configuration::Global.flags.IsEnabled(Js::IRViewerFlag))
        {
            AddFunctionToLibraryObjectWithPropertyName(globalObject, L"parseIR", &GlobalObject::EntryInfo::ParseIR, 1);
            AddFunctionToLibraryObjectWithPropertyName(globalObject, L"functionList", &GlobalObject::EntryInfo::FunctionList, 1);
            AddFunctionToLibraryObjectWithPropertyName(globalObject, L"rejitFunction", &GlobalObject::EntryInfo::RejitFunction, 2);
        }
#endif /* IR_VIEWER */

        DebugOnly(CheckRegisteredBuiltIns(builtinFuncs));

        builtInConstructorCache = RecyclerNew(this->GetRecycler(), ConstructorCache);
        builtInConstructorCache->PopulateForSkipDefaultNewObject(this->GetScriptContext());

        objectConstructor = CreateBuiltinConstructor(&JavascriptObject::EntryInfo::NewInstance,
            DeferredTypeHandler<InitializeObjectConstructor>::GetDefaultInstance());
        AddFunction(globalObject, PropertyIds::Object, objectConstructor);
        arrayConstructor = CreateBuiltinConstructor(&JavascriptArray::EntryInfo::NewInstance,
            DeferredTypeHandler<InitializeArrayConstructor>::GetDefaultInstance());
        AddFunction(globalObject, PropertyIds::Array, arrayConstructor);
        booleanConstructor = CreateBuiltinConstructor(&JavascriptBoolean::EntryInfo::NewInstance,
            DeferredTypeHandler<InitializeBooleanConstructor>::GetDefaultInstance());
        AddFunction(globalObject, PropertyIds::Boolean, booleanConstructor);

        symbolConstructor = nullptr;
        proxyConstructor = nullptr;
        promiseConstructor = nullptr;
        reflectObject = nullptr;

        if (scriptContext->GetConfig()->IsES6SymbolEnabled())
        {
            symbolConstructor = CreateBuiltinConstructor(&JavascriptSymbol::EntryInfo::NewInstance,
                DeferredTypeHandler<InitializeSymbolConstructor>::GetDefaultInstance());
            AddFunction(globalObject, PropertyIds::Symbol, symbolConstructor);
        }

        if (scriptContext->GetConfig()->IsES6ProxyEnabled())
        {
            proxyConstructor = CreateBuiltinConstructor(&JavascriptProxy::EntryInfo::NewInstance,
                DeferredTypeHandler<InitializeProxyConstructor>::GetDefaultInstance());
            AddFunction(globalObject, PropertyIds::Proxy, proxyConstructor);
            reflectObject = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeReflectObject>::GetDefaultInstance()));
            AddMember(globalObject, PropertyIds::Reflect, reflectObject);
        }

        if (scriptContext->GetConfig()->IsES6PromiseEnabled())
        {
            promiseConstructor = CreateBuiltinConstructor(&JavascriptPromise::EntryInfo::NewInstance,
                DeferredTypeHandler<InitializePromiseConstructor>::GetDefaultInstance());
            AddFunction(globalObject, PropertyIds::Promise, promiseConstructor);
        }

        dateConstructor = CreateBuiltinConstructor(&JavascriptDate::EntryInfo::NewInstance,
            DeferredTypeHandler<InitializeDateConstructor>::GetDefaultInstance());
        AddFunction(globalObject, PropertyIds::Date, dateConstructor);
        functionConstructor = CreateBuiltinConstructor(&JavascriptFunction::EntryInfo::NewInstance,
            DeferredTypeHandler<InitializeFunctionConstructor>::GetDefaultInstance());
        AddFunction(globalObject, PropertyIds::Function, functionConstructor);
        mathObject = DynamicObject::New(recycler,
            DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
            DeferredTypeHandler<InitializeMathObject>::GetDefaultInstance()));
        AddMember(globalObject, PropertyIds::Math, mathObject);

#ifdef SIMD_JS_ENABLED
        // SIMD
        // we declare global objects and lib functions only if SSE2 is available. Else, we use the polyfill.
        if (AutoSystemInfo::Data.SSE2Available() && Js::Configuration::Global.flags.Simdjs)
        {
            simdObject = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeSIMDObject>::GetDefaultInstance()));
            AddMember(globalObject, PropertyIds::SIMD, simdObject, PropertyNone);
        }
#endif

        debugObject = nullptr;
        diagnosticsScriptObject = nullptr;

        numberConstructor = CreateBuiltinConstructor(&JavascriptNumber::EntryInfo::NewInstance,
            DeferredTypeHandler<InitializeNumberConstructor>::GetDefaultInstance());
        AddFunction(globalObject, PropertyIds::Number, numberConstructor);
        stringConstructor = CreateBuiltinConstructor(&JavascriptString::EntryInfo::NewInstance,
            DeferredTypeHandler<InitializeStringConstructor>::GetDefaultInstance());
        AddFunction(globalObject, PropertyIds::String, stringConstructor);
        regexConstructorType = DynamicType::New(scriptContext, TypeIds_Function, functionPrototype, JavascriptRegExp::NewInstance,
            DeferredTypeHandler<InitializeRegexConstructor>::GetDefaultInstance());
        regexConstructor = RecyclerNewEnumClass(recycler, EnumFunctionClass, JavascriptRegExpConstructor, regexConstructorType);
        AddFunction(globalObject, PropertyIds::RegExp, regexConstructor);

        if (scriptContext->GetConfig()->IsTypedArrayEnabled())
        {
            arrayBufferConstructor = CreateBuiltinConstructor(&ArrayBuffer::EntryInfo::NewInstance,
                DeferredTypeHandler<InitializeArrayBufferConstructor>::GetDefaultInstance());
            AddFunction(globalObject, PropertyIds::ArrayBuffer, arrayBufferConstructor);
            dataViewConstructor = CreateBuiltinConstructor(&DataView::EntryInfo::NewInstance,
                DeferredTypeHandler<InitializeDataViewConstructor>::GetDefaultInstance());
            AddFunction(globalObject, PropertyIds::DataView, dataViewConstructor);

            if (scriptContext->GetConfig()->IsES6TypedArrayExtensionsEnabled())
            {
                typedArrayConstructor = CreateBuiltinConstructor(&TypedArrayBase::EntryInfo::NewInstance,
                    DeferredTypeHandler<InitializeTypedArrayConstructor>::GetDefaultInstance(),
                    functionPrototype);

                Int8ArrayConstructor = CreateBuiltinConstructor(&Int8Array::EntryInfo::NewInstance,
                    DeferredTypeHandler<InitializeInt8ArrayConstructor>::GetDefaultInstance(),
                    typedArrayConstructor);
                AddFunction(globalObject, PropertyIds::Int8Array, Int8ArrayConstructor);

                Uint8ArrayConstructor = CreateBuiltinConstructor(&Uint8Array::EntryInfo::NewInstance,
                    DeferredTypeHandler<InitializeUint8ArrayConstructor>::GetDefaultInstance(),
                    typedArrayConstructor);
                AddFunction(globalObject, PropertyIds::Uint8Array, Uint8ArrayConstructor);

                Uint8ClampedArrayConstructor = CreateBuiltinConstructor(&Uint8ClampedArray::EntryInfo::NewInstance,
                    DeferredTypeHandler<InitializeUint8ClampedArrayConstructor>::GetDefaultInstance(),
                    typedArrayConstructor);
                AddFunction(globalObject, PropertyIds::Uint8ClampedArray, Uint8ClampedArrayConstructor);

                Int16ArrayConstructor = CreateBuiltinConstructor(&Int16Array::EntryInfo::NewInstance,
                    DeferredTypeHandler<InitializeInt16ArrayConstructor>::GetDefaultInstance(),
                    typedArrayConstructor);
                AddFunction(globalObject, PropertyIds::Int16Array, Int16ArrayConstructor);

                Uint16ArrayConstructor = CreateBuiltinConstructor(&Uint16Array::EntryInfo::NewInstance,
                    DeferredTypeHandler<InitializeUint16ArrayConstructor>::GetDefaultInstance(),
                    typedArrayConstructor);
                AddFunction(globalObject, PropertyIds::Uint16Array, Uint16ArrayConstructor);

                Int32ArrayConstructor = CreateBuiltinConstructor(&Int32Array::EntryInfo::NewInstance,
                    DeferredTypeHandler<InitializeInt32ArrayConstructor>::GetDefaultInstance(),
                    typedArrayConstructor);
                AddFunction(globalObject, PropertyIds::Int32Array, Int32ArrayConstructor);

                Uint32ArrayConstructor = CreateBuiltinConstructor(&Uint32Array::EntryInfo::NewInstance,
                    DeferredTypeHandler<InitializeUint32ArrayConstructor>::GetDefaultInstance(),
                    typedArrayConstructor);
                AddFunction(globalObject, PropertyIds::Uint32Array, Uint32ArrayConstructor);

                Float32ArrayConstructor = CreateBuiltinConstructor(&Float32Array::EntryInfo::NewInstance,
                    DeferredTypeHandler<InitializeFloat32ArrayConstructor>::GetDefaultInstance(),
                    typedArrayConstructor);
                AddFunction(globalObject, PropertyIds::Float32Array, Float32ArrayConstructor);

                Float64ArrayConstructor = CreateBuiltinConstructor(&Float64Array::EntryInfo::NewInstance,
                    DeferredTypeHandler<InitializeFloat64ArrayConstructor>::GetDefaultInstance(),
                    typedArrayConstructor);
                AddFunction(globalObject, PropertyIds::Float64Array, Float64ArrayConstructor);
            }
            else
            {
                ADD_TYPEDARRAY_CONSTRUCTOR(Int8ArrayConstructor, Int8Array);
                ADD_TYPEDARRAY_CONSTRUCTOR(Uint8ArrayConstructor, Uint8Array);

                if (scriptContext->GetConfig()->IsKhronosInteropEnabled())
                {
                    ADD_TYPEDARRAY_CONSTRUCTOR(Uint8ClampedArrayConstructor, Uint8ClampedArray);
                }
                else
                {
                    Uint8ClampedArrayConstructor = nullptr;
                }

                ADD_TYPEDARRAY_CONSTRUCTOR(Int16ArrayConstructor, Int16Array);
                ADD_TYPEDARRAY_CONSTRUCTOR(Uint16ArrayConstructor, Uint16Array);
                ADD_TYPEDARRAY_CONSTRUCTOR(Int32ArrayConstructor, Int32Array);
                ADD_TYPEDARRAY_CONSTRUCTOR(Uint32ArrayConstructor, Uint32Array);
                ADD_TYPEDARRAY_CONSTRUCTOR(Float32ArrayConstructor, Float32Array);
                ADD_TYPEDARRAY_CONSTRUCTOR(Float64ArrayConstructor, Float64Array);
            }
        }
        else
        {
            arrayBufferConstructor = nullptr;
            Int8ArrayConstructor = nullptr;
            Uint8ArrayConstructor = nullptr;
            Uint8ClampedArrayConstructor = nullptr;
            Int16ArrayConstructor = nullptr;
            Uint16ArrayConstructor = nullptr;
            Int32ArrayConstructor = nullptr;
            Uint32ArrayConstructor = nullptr;
            Float32ArrayConstructor = nullptr;
            Float64ArrayConstructor = nullptr;
            dataViewConstructor = nullptr;
        }

        if (!scriptContext->GetConfig()->IsES6TypedArrayExtensionsEnabled())
        {
            pixelArrayConstructor = CreateBuiltinConstructor(&JavascriptPixelArray::EntryInfo::NewInstance,
                DeferredTypeHandler<InitializePixelArrayConstructor>::GetDefaultInstance());
            AddFunction(globalObject, PropertyIds::CanvasPixelArray, pixelArrayConstructor);
        }
        else
        {
            pixelArrayConstructor = nullptr;
        }


        JSONObject = DynamicObject::New(recycler,
            DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
            DeferredTypeHandler<InitializeJSONObject>::GetDefaultInstance()));
        AddMember(globalObject, PropertyIds::JSON, JSONObject);

#ifdef ENABLE_INTL_OBJECT
        //If we are or win8 or later; then the default value for the flag; or enabled value for the flag (-Intl and -Intl-) dictates if Intl is available.
        //If we are on win7; then the flag has to be specified force it on (or off); the default value is ignored
        if (scriptContext->GetConfig()->IsIntlEnabled())
        {
            IntlObject = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeIntlObject>::GetDefaultInstance()));
            AddMember(globalObject, PropertyIds::Intl, IntlObject);
        }
        else
#endif
        {
            IntlObject = nullptr;
        }

        winRTPromiseConstructor = nullptr;

        engineInterfaceObject = EngineInterfaceObject::New(recycler,
            DynamicType::New(scriptContext, TypeIds_EngineInterfaceObject, objectPrototype, nullptr,
            DeferredTypeHandler<InitializeEngineInterfaceObject>::GetDefaultInstance()));

        mapConstructor = nullptr;
        setConstructor = nullptr;
        weakMapConstructor = nullptr;
        weakSetConstructor = nullptr;
        generatorFunctionConstructor = nullptr;

        if (scriptContext->GetConfig()->IsES6MapEnabled())
        {
            mapConstructor = CreateBuiltinConstructor(&JavascriptMap::EntryInfo::NewInstance,
                DeferredTypeHandler<InitializeMapConstructor>::GetDefaultInstance());
            AddFunction(globalObject, PropertyIds::Map, mapConstructor);
        }

        if (scriptContext->GetConfig()->IsES6SetEnabled())
        {
            setConstructor = CreateBuiltinConstructor(&JavascriptSet::EntryInfo::NewInstance,
                DeferredTypeHandler<InitializeSetConstructor>::GetDefaultInstance());
            AddFunction(globalObject, PropertyIds::Set, setConstructor);
        }

        if (scriptContext->GetConfig()->IsES6WeakMapEnabled())
        {
            weakMapConstructor = CreateBuiltinConstructor(&JavascriptWeakMap::EntryInfo::NewInstance,
                DeferredTypeHandler<InitializeWeakMapConstructor>::GetDefaultInstance());
            AddFunction(globalObject, PropertyIds::WeakMap, weakMapConstructor);
        }

        if (scriptContext->GetConfig()->IsES6WeakSetEnabled())
        {
            weakSetConstructor = CreateBuiltinConstructor(&JavascriptWeakSet::EntryInfo::NewInstance,
                DeferredTypeHandler<InitializeWeakSetConstructor>::GetDefaultInstance());
            AddFunction(globalObject, PropertyIds::WeakSet, weakSetConstructor);
        }

        if (scriptContext->GetConfig()->IsES6GeneratorsEnabled())
        {
            generatorFunctionConstructor = CreateBuiltinConstructor(&JavascriptGeneratorFunction::EntryInfo::NewInstance,
                DeferredTypeHandler<InitializeGeneratorFunctionConstructor>::GetDefaultInstance(),
                functionConstructor);
            // GeneratorFunction is not a global property by ES6 spec so don't add it to the global object
        }

        errorConstructor = CreateBuiltinConstructor(&JavascriptError::EntryInfo::NewErrorInstance,
            DeferredTypeHandler<InitializeErrorConstructor>::GetDefaultInstance());
        AddFunction(globalObject, PropertyIds::Error, errorConstructor);

        winrtErrorConstructor = nullptr;

                RuntimeFunction* nativeErrorPrototype = nullptr;
        if (scriptContext->GetConfig()->IsES6PrototypeChain())
        {
            nativeErrorPrototype = errorConstructor;
        }

        evalErrorConstructor = CreateBuiltinConstructor(&JavascriptError::EntryInfo::NewEvalErrorInstance,
            DeferredTypeHandler<InitializeEvalErrorConstructor>::GetDefaultInstance(),
            nativeErrorPrototype);
        AddFunction(globalObject, PropertyIds::EvalError, evalErrorConstructor);

        rangeErrorConstructor = CreateBuiltinConstructor(&JavascriptError::EntryInfo::NewRangeErrorInstance,
            DeferredTypeHandler<InitializeRangeErrorConstructor>::GetDefaultInstance(),
            nativeErrorPrototype);
        AddFunction(globalObject, PropertyIds::RangeError, rangeErrorConstructor);

        referenceErrorConstructor = CreateBuiltinConstructor(&JavascriptError::EntryInfo::NewReferenceErrorInstance,
            DeferredTypeHandler<InitializeReferenceErrorConstructor>::GetDefaultInstance(),
            nativeErrorPrototype);
        AddFunction(globalObject, PropertyIds::ReferenceError, referenceErrorConstructor);

        syntaxErrorConstructor = CreateBuiltinConstructor(&JavascriptError::EntryInfo::NewSyntaxErrorInstance,
            DeferredTypeHandler<InitializeSyntaxErrorConstructor>::GetDefaultInstance(),
            nativeErrorPrototype);
        AddFunction(globalObject, PropertyIds::SyntaxError, syntaxErrorConstructor);

        typeErrorConstructor = CreateBuiltinConstructor(&JavascriptError::EntryInfo::NewTypeErrorInstance,
            DeferredTypeHandler<InitializeTypeErrorConstructor>::GetDefaultInstance(),
            nativeErrorPrototype);
        AddFunction(globalObject, PropertyIds::TypeError, typeErrorConstructor);

        uriErrorConstructor = CreateBuiltinConstructor(&JavascriptError::EntryInfo::NewURIErrorInstance,
            DeferredTypeHandler<InitializeURIErrorConstructor>::GetDefaultInstance(),
            nativeErrorPrototype);
        AddFunction(globalObject, PropertyIds::URIError, uriErrorConstructor);

        if (scriptContext->GetConfig()->IsWinRTEnabled())
        {
            winrtErrorConstructor = CreateBuiltinConstructor(&JavascriptError::EntryInfo::NewWinRTErrorInstance,
                DeferredTypeHandler<InitializeWinRTErrorConstructor>::GetDefaultInstance(),
                nativeErrorPrototype);
            AddFunction(globalObject, PropertyIds::WinRTError, winrtErrorConstructor);
        }
        nullEnumerator = RecyclerNew(this->recycler, NullEnumerator, scriptContext);
    }

    void JavascriptLibrary::EnsureDebugObject(DynamicObject* newDebugObject)
    {
        Assert(!debugObject);

        if (!debugObject)
        {
            this->debugObject = newDebugObject;
            AddMember(globalObject, PropertyIds::Debug, debugObject);
        }
    }

    void JavascriptLibrary::InitializeDiagnosticsScriptObject(DiagnosticsScriptObject* newDiagnosticsScriptObject)
    {
        Assert(this->diagnosticsScriptObject == nullptr);
        this->diagnosticsScriptObject = newDiagnosticsScriptObject;
        AddMember(globalObject, globalObject->GetScriptContext()->GetOrAddPropertyIdTracked(L"diagnosticsScript"), this->diagnosticsScriptObject);
    }

    void JavascriptLibrary::SetDebugObjectNonUserAccessor(FunctionInfo *funcGetter, FunctionInfo *funcSetter)
    {
        Assert(funcGetter);
        Assert(funcSetter);

        debugObjectNonUserGetterFunction = CreateNonProfiledFunction(funcGetter);
        debugObjectNonUserGetterFunction->SetPropertyWithAttributes(PropertyIds::length, TaggedInt::ToVarUnchecked(0), PropertyNone, nullptr);

        debugObjectNonUserSetterFunction = CreateNonProfiledFunction(funcSetter);
        debugObjectNonUserSetterFunction->SetPropertyWithAttributes(PropertyIds::length, TaggedInt::ToVarUnchecked(1), PropertyNone, nullptr);
    }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    void JavascriptLibrary::SetDebugObjectFaultInjectionCookieGetterAccessor(FunctionInfo *funcGetter, FunctionInfo *funcSetter)
    {
        Assert(funcGetter);
        Assert(funcSetter);

        debugObjectFaultInjectionCookieGetterFunction = CreateNonProfiledFunction(funcGetter);
        debugObjectFaultInjectionCookieGetterFunction->SetPropertyWithAttributes(PropertyIds::length, TaggedInt::ToVarUnchecked(0), PropertyNone, nullptr);

        debugObjectFaultInjectionCookieSetterFunction = CreateNonProfiledFunction(funcSetter);
        debugObjectFaultInjectionCookieSetterFunction->SetPropertyWithAttributes(PropertyIds::length, TaggedInt::ToVarUnchecked(1), PropertyNone, nullptr);
    }
#endif

    void JavascriptLibrary::SetDebugObjectDebugModeAccessor(FunctionInfo *funcGetter)
    {
        Assert(funcGetter);

        debugObjectDebugModeGetterFunction = CreateNonProfiledFunction(funcGetter);
        debugObjectDebugModeGetterFunction->SetPropertyWithAttributes(PropertyIds::length, TaggedInt::ToVarUnchecked(0), PropertyNone, nullptr);
    }

    void JavascriptLibrary::InitializeArrayConstructor(DynamicObject* arrayConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(arrayConstructor, mode, 6);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterArray
        // so that the update is in sync with profiler
        ScriptContext* scriptContext = arrayConstructor->GetScriptContext();
        JavascriptLibrary* library = arrayConstructor->GetLibrary();
        JavascriptFunction ** builtinFuncs = library->GetBuiltinFunctions();

        library->AddMember(arrayConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(1), PropertyNone);
        library->AddMember(arrayConstructor, PropertyIds::prototype, scriptContext->GetLibrary()->arrayPrototype, PropertyNone);
        library->AddAccessorsToLibraryObject(arrayConstructor, PropertyIds::_symbolSpecies, &JavascriptArray::EntryInfo::GetterSymbolSpecies, nullptr);

        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(arrayConstructor, PropertyIds::name, scriptContext->GetPropertyString(PropertyIds::Array), PropertyConfigurable);
        }
        builtinFuncs[BuiltinFunction::Array_IsArray] = library->AddFunctionToLibraryObject(arrayConstructor, PropertyIds::isArray, &JavascriptArray::EntryInfo::IsArray, 1);

        // Array.from and Array.of are implemented as part of the ES6 TypedArray feature
        if (scriptContext->GetConfig()->IsES6TypedArrayExtensionsEnabled())
        {
            library->AddFunctionToLibraryObject(arrayConstructor, PropertyIds::from, &JavascriptArray::EntryInfo::From, 1);
            library->AddFunctionToLibraryObject(arrayConstructor, PropertyIds::of, &JavascriptArray::EntryInfo::Of, 0);
        }

        DebugOnly(CheckRegisteredBuiltIns(builtinFuncs));

        arrayConstructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeArrayPrototype(DynamicObject* arrayPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(arrayPrototype, mode, 24);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterArray
        // so that the update is in sync with profiler

        ScriptContext* scriptContext = arrayPrototype->GetScriptContext();
        JavascriptLibrary* library = arrayPrototype->GetLibrary();
        library->AddMember(arrayPrototype, PropertyIds::constructor, library->arrayConstructor);

        JavascriptFunction ** builtinFuncs = library->GetBuiltinFunctions();

        builtinFuncs[BuiltinFunction::Array_Push]               = library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::push,            &JavascriptArray::EntryInfo::Push,              1);
        builtinFuncs[BuiltinFunction::Array_Concat]             = library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::concat,          &JavascriptArray::EntryInfo::Concat,            1);
        builtinFuncs[BuiltinFunction::Array_Join]               = library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::join,            &JavascriptArray::EntryInfo::Join,              1);
        builtinFuncs[BuiltinFunction::Array_Pop]                = library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::pop,             &JavascriptArray::EntryInfo::Pop,               0);
        builtinFuncs[BuiltinFunction::Array_Reverse]            = library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::reverse,         &JavascriptArray::EntryInfo::Reverse,           0);
        builtinFuncs[BuiltinFunction::Array_Shift]              = library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::shift,           &JavascriptArray::EntryInfo::Shift,             0);
        builtinFuncs[BuiltinFunction::Array_Slice]              = library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::slice,           &JavascriptArray::EntryInfo::Slice,             2);
        /* No inlining                Array_Sort               */ library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::sort,            &JavascriptArray::EntryInfo::Sort,              1);
        builtinFuncs[BuiltinFunction::Array_Splice]             = library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::splice,          &JavascriptArray::EntryInfo::Splice,            2);

        // The toString and toLocaleString properties are shared between Array.prototype and %TypedArray%.prototype.
        // Whichever prototype is initialized first will create the functions, the other should just load the existing function objects.
        if (library->arrayPrototypeToStringFunction == nullptr)
        {
            Assert(library->arrayPrototypeToLocaleStringFunction == nullptr);

            library->arrayPrototypeToLocaleStringFunction = /* No inlining       Array_ToLocaleString */ library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::toLocaleString, &JavascriptArray::EntryInfo::ToLocaleString, 0);
            library->arrayPrototypeToStringFunction =       /* No inlining       Array_ToString       */ library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::toString, &JavascriptArray::EntryInfo::ToString, 0);
        }
        else
        {
            Assert(library->arrayPrototypeToLocaleStringFunction);

            library->AddMember(arrayPrototype, PropertyIds::toLocaleString, library->arrayPrototypeToLocaleStringFunction);
            library->AddMember(arrayPrototype, PropertyIds::toString, library->arrayPrototypeToStringFunction);
        }

        builtinFuncs[BuiltinFunction::Array_Unshift]            = library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::unshift,         &JavascriptArray::EntryInfo::Unshift,           1);


        builtinFuncs[BuiltinFunction::Array_IndexOf]        = library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::indexOf,         &JavascriptArray::EntryInfo::IndexOf,           1);
        /* No inlining                Array_Every          */ library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::every,           &JavascriptArray::EntryInfo::Every,             1);
        /* No inlining                Array_Filter         */ library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::filter,          &JavascriptArray::EntryInfo::Filter,            1);
        /* No inlining                Array_ForEach        */ library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::forEach,         &JavascriptArray::EntryInfo::ForEach,           1);
        builtinFuncs[BuiltinFunction::Array_LastIndexOf]    = library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::lastIndexOf,     &JavascriptArray::EntryInfo::LastIndexOf,       1);
        /* No inlining                Array_Map            */ library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::map,             &JavascriptArray::EntryInfo::Map,               1);
        /* No inlining                Array_Reduce         */ library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::reduce,          &JavascriptArray::EntryInfo::Reduce,            1);
        /* No inlining                Array_ReduceRight    */ library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::reduceRight,     &JavascriptArray::EntryInfo::ReduceRight,       1);
        /* No inlining                Array_Some           */ library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::some,            &JavascriptArray::EntryInfo::Some,              1);

        if (scriptContext->GetConfig()->IsES6StringExtensionsEnabled()) // This is not a typo, Array.prototype.find and .findIndex are part of the ES6 Improved String APIs feature
        {
            /* No inlining                Array_Find           */ library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::find,            &JavascriptArray::EntryInfo::Find,              1);
            /* No inlining                Array_FindIndex      */ library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::findIndex,       &JavascriptArray::EntryInfo::FindIndex,         1);
        }

        if (scriptContext->GetConfig()->IsES6IteratorsEnabled())
        {
            /* No inlining								Array_Entries        */ library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::entries, &JavascriptArray::EntryInfo::Entries, 0);
            /* No inlining								Array_Keys           */ library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::keys, &JavascriptArray::EntryInfo::Keys, 0);

            JavascriptFunction *values = library->arrayPrototypeValuesFunction ? library->arrayPrototypeValuesFunction : /* No inlining Array_Values     */ library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::values, &JavascriptArray::EntryInfo::Values, 0);
            /* No inlining							Array_SymbolIterator */ library->AddMember(arrayPrototype, PropertyIds::_symbolIterator, values);
        }

        if (scriptContext->GetConfig()->IsES6UnscopablesEnabled())
        {
            DynamicObject* unscopables_blacklist = scriptContext->GetLibrary()->CreateObject(true, 7);
            unscopables_blacklist->SetProperty(PropertyIds::find, JavascriptBoolean::ToVar(true, scriptContext), PropertyOperation_None, nullptr);
            unscopables_blacklist->SetProperty(PropertyIds::findIndex, JavascriptBoolean::ToVar(true, scriptContext), PropertyOperation_None, nullptr);
            unscopables_blacklist->SetProperty(PropertyIds::fill, JavascriptBoolean::ToVar(true, scriptContext), PropertyOperation_None, nullptr);
            unscopables_blacklist->SetProperty(PropertyIds::copyWithin, JavascriptBoolean::ToVar(true, scriptContext), PropertyOperation_None, nullptr);
            unscopables_blacklist->SetProperty(PropertyIds::entries, JavascriptBoolean::ToVar(true, scriptContext), PropertyOperation_None, nullptr);
            unscopables_blacklist->SetProperty(PropertyIds::keys, JavascriptBoolean::ToVar(true, scriptContext), PropertyOperation_None, nullptr);
            unscopables_blacklist->SetProperty(PropertyIds::values, JavascriptBoolean::ToVar(true, scriptContext), PropertyOperation_None, nullptr);
            library->AddMember(arrayPrototype, PropertyIds::_symbolUnscopables, unscopables_blacklist);
        }

        if (scriptContext->GetConfig()->IsES6TypedArrayExtensionsEnabled()) // This is not a typo, Array.prototype.fill and .copyWithin are part of the ES6 TypedArray feature
        {
            /* No inlining                Array_Fill           */ library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::fill, &JavascriptArray::EntryInfo::Fill, 1);
            /* No inlining                Array_CopyWithin     */ library->AddFunctionToLibraryObject(arrayPrototype, PropertyIds::copyWithin, &JavascriptArray::EntryInfo::CopyWithin, 2);
        }

        DebugOnly(CheckRegisteredBuiltIns(builtinFuncs));

        arrayPrototype->SetHasNoEnumerableProperties(true);
    }

    void  JavascriptLibrary::InitializeArrayBufferConstructor(DynamicObject* arrayBufferConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(arrayBufferConstructor, mode, 4);

        ScriptContext* scriptContext = arrayBufferConstructor->GetScriptContext();
        JavascriptLibrary* library = arrayBufferConstructor->GetLibrary();
        library->AddMember(arrayBufferConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(1), PropertyNone);
        library->AddMember(arrayBufferConstructor, PropertyIds::prototype, scriptContext->GetLibrary()->arrayBufferPrototype, PropertyNone);
        library->AddAccessorsToLibraryObject(arrayBufferConstructor, PropertyIds::_symbolSpecies, &ArrayBuffer::EntryInfo::GetterSymbolSpecies, nullptr);
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(arrayBufferConstructor, PropertyIds::name, scriptContext->GetPropertyString(PropertyIds::ArrayBuffer), PropertyConfigurable);
        }
        if (scriptContext->GetConfig()->IsKhronosInteropEnabled())
        {
            library->AddFunctionToLibraryObject(arrayBufferConstructor, PropertyIds::isView, &ArrayBuffer::EntryInfo::IsView, 1);
        }

        arrayBufferConstructor->SetHasNoEnumerableProperties(true);
    }

    void  JavascriptLibrary::InitializeArrayBufferPrototype(DynamicObject* arrayBufferPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(arrayBufferPrototype, mode, 2);

        ScriptContext* scriptContext = arrayBufferPrototype->GetScriptContext();
        JavascriptLibrary* library = arrayBufferPrototype->GetLibrary();
        library->AddMember(arrayBufferPrototype, PropertyIds::constructor, library->arrayBufferConstructor);

        if (scriptContext->GetConfig()->IsKhronosInteropEnabled())
        {
            library->AddFunctionToLibraryObject(arrayBufferPrototype, PropertyIds::slice, &ArrayBuffer::EntryInfo::Slice, 2);
        }

        if (scriptContext->GetConfig()->IsES6TypedArrayExtensionsEnabled())
        {
            library->AddAccessorsToLibraryObject(arrayBufferPrototype, PropertyIds::byteLength, &ArrayBuffer::EntryInfo::GetterByteLength, nullptr);
        }

        if (scriptContext->GetConfig()->IsES6ToStringTagEnabled())
        {
            library->AddMember(arrayBufferPrototype, PropertyIds::_symbolToStringTag, library->CreateStringFromCppLiteral(L"ArrayBuffer"), PropertyConfigurable);
        }

        arrayBufferPrototype->SetHasNoEnumerableProperties(true);
    }

    void  JavascriptLibrary::InitializeDataViewConstructor(DynamicObject* dataViewConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(dataViewConstructor, mode, 3);

        ScriptContext* scriptContext = dataViewConstructor->GetScriptContext();
        JavascriptLibrary* library = dataViewConstructor->GetLibrary();
        library->AddMember(dataViewConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(1), PropertyNone);
        library->AddMember(dataViewConstructor, PropertyIds::prototype, scriptContext->GetLibrary()->dataViewPrototype, PropertyNone);
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(dataViewConstructor, PropertyIds::name, scriptContext->GetPropertyString(PropertyIds::DataView), PropertyConfigurable);
        }
        dataViewConstructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeDataViewPrototype(DynamicObject* dataViewPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(dataViewPrototype, mode, 2);

        ScriptContext* scriptContext = dataViewPrototype->GetScriptContext();
        JavascriptLibrary* library = dataViewPrototype->GetLibrary();
        library->AddMember(dataViewPrototype, PropertyIds::constructor, library->dataViewConstructor);
        library->AddFunctionToLibraryObject(dataViewPrototype, PropertyIds::setInt8, &DataView::EntryInfo::SetInt8, 1);
        library->AddFunctionToLibraryObject(dataViewPrototype, PropertyIds::setUint8, &DataView::EntryInfo::SetUint8, 1);
        library->AddFunctionToLibraryObject(dataViewPrototype, PropertyIds::setInt16, &DataView::EntryInfo::SetInt16, 1);
        library->AddFunctionToLibraryObject(dataViewPrototype, PropertyIds::setUint16, &DataView::EntryInfo::SetUint16, 1);
        library->AddFunctionToLibraryObject(dataViewPrototype, PropertyIds::setInt32, &DataView::EntryInfo::SetInt32, 1);
        library->AddFunctionToLibraryObject(dataViewPrototype, PropertyIds::setUint32, &DataView::EntryInfo::SetUint32, 1);
        library->AddFunctionToLibraryObject(dataViewPrototype, PropertyIds::setFloat32, &DataView::EntryInfo::SetFloat32, 1);
        library->AddFunctionToLibraryObject(dataViewPrototype, PropertyIds::setFloat64, &DataView::EntryInfo::SetFloat64, 1);
        library->AddFunctionToLibraryObject(dataViewPrototype, PropertyIds::getInt8, &DataView::EntryInfo::GetInt8, 1);
        library->AddFunctionToLibraryObject(dataViewPrototype, PropertyIds::getUint8, &DataView::EntryInfo::GetUint8, 1);
        library->AddFunctionToLibraryObject(dataViewPrototype, PropertyIds::getInt16, &DataView::EntryInfo::GetInt16, 1);
        library->AddFunctionToLibraryObject(dataViewPrototype, PropertyIds::getUint16, &DataView::EntryInfo::GetUint16, 1);
        library->AddFunctionToLibraryObject(dataViewPrototype, PropertyIds::getInt32, &DataView::EntryInfo::GetInt32, 1);
        library->AddFunctionToLibraryObject(dataViewPrototype, PropertyIds::getUint32, &DataView::EntryInfo::GetUint32, 1);
        library->AddFunctionToLibraryObject(dataViewPrototype, PropertyIds::getFloat32, &DataView::EntryInfo::GetFloat32, 1);
        library->AddFunctionToLibraryObject(dataViewPrototype, PropertyIds::getFloat64, &DataView::EntryInfo::GetFloat64, 1);

        if (scriptContext->GetConfig()->IsKhronosInteropEnabled())
        {
            library->AddAccessorsToLibraryObject(dataViewPrototype, PropertyIds::buffer, &DataView::EntryInfo::GetterBuffer, nullptr);
            library->AddAccessorsToLibraryObject(dataViewPrototype, PropertyIds::byteLength, &DataView::EntryInfo::GetterByteLength, nullptr);
            library->AddAccessorsToLibraryObject(dataViewPrototype, PropertyIds::byteOffset, &DataView::EntryInfo::GetterByteOffset, nullptr);
        }

        if (scriptContext->GetConfig()->IsES6ToStringTagEnabled())
        {
            library->AddMember(dataViewPrototype, PropertyIds::_symbolToStringTag, library->CreateStringFromCppLiteral(L"DataView"), PropertyConfigurable);
        }

        dataViewPrototype->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeTypedArrayConstructor(DynamicObject* typedArrayConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(typedArrayConstructor, mode, 5);

        ScriptContext* scriptContext = typedArrayConstructor->GetScriptContext();
        JavascriptLibrary* library = typedArrayConstructor->GetLibrary();

        Assert(scriptContext->GetConfig()->IsES6TypedArrayExtensionsEnabled());

        library->AddMember(typedArrayConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(3), PropertyNone);
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(typedArrayConstructor, PropertyIds::name, library->CreateStringFromCppLiteral(L"TypedArray"), PropertyConfigurable);
        }
        library->AddMember(typedArrayConstructor, PropertyIds::prototype, library->typedArrayPrototype, PropertyNone);

        library->AddFunctionToLibraryObject(typedArrayConstructor, PropertyIds::from, &TypedArrayBase::EntryInfo::From, 1);
        library->AddFunctionToLibraryObject(typedArrayConstructor, PropertyIds::of, &TypedArrayBase::EntryInfo::Of, 0);
        library->AddAccessorsToLibraryObject(typedArrayConstructor, PropertyIds::_symbolSpecies, &TypedArrayBase::EntryInfo::GetterSymbolSpecies, nullptr);

        typedArrayConstructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeTypedArrayPrototype(DynamicObject* typedarrayPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(typedarrayPrototype, mode, 31);

        ScriptContext* scriptContext = typedarrayPrototype->GetScriptContext();
        JavascriptLibrary* library = typedarrayPrototype->GetLibrary();

        Assert(scriptContext->GetConfig()->IsES6TypedArrayExtensionsEnabled());

        library->AddMember(typedarrayPrototype, PropertyIds::constructor, library->typedArrayConstructor);
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::set, &TypedArrayBase::EntryInfo::Set, 2);
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::subarray, &TypedArrayBase::EntryInfo::Subarray, 2);
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::copyWithin, &TypedArrayBase::EntryInfo::CopyWithin, 2);
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::every, &TypedArrayBase::EntryInfo::Every, 1);
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::fill, &TypedArrayBase::EntryInfo::Fill, 1);
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::filter, &TypedArrayBase::EntryInfo::Filter, 1);
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::find, &TypedArrayBase::EntryInfo::Find, 1);
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::findIndex, &TypedArrayBase::EntryInfo::FindIndex, 1);
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::forEach, &TypedArrayBase::EntryInfo::ForEach, 1);
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::indexOf, &TypedArrayBase::EntryInfo::IndexOf, 1);
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::join, &TypedArrayBase::EntryInfo::Join, 1);
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::lastIndexOf, &TypedArrayBase::EntryInfo::LastIndexOf, 1);
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::map, &TypedArrayBase::EntryInfo::Map, 1);
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::reduce, &TypedArrayBase::EntryInfo::Reduce, 1);
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::reduceRight, &TypedArrayBase::EntryInfo::ReduceRight, 1);
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::reverse, &TypedArrayBase::EntryInfo::Reverse, 0);
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::slice, &TypedArrayBase::EntryInfo::Slice, 2);
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::some, &TypedArrayBase::EntryInfo::Some, 1);
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::sort, &TypedArrayBase::EntryInfo::Sort, 1);
        if (scriptContext->GetConfig()->IsES6IteratorsEnabled())
        {
            library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::entries, &TypedArrayBase::EntryInfo::Entries, 0);
            library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::keys, &TypedArrayBase::EntryInfo::Keys, 0);
            JavascriptFunction* valuesFunc = library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::values, &TypedArrayBase::EntryInfo::Values, 0);
            library->AddMember(typedarrayPrototype, PropertyIds::_symbolIterator, valuesFunc);
        }

        library->AddAccessorsToLibraryObject(typedarrayPrototype, PropertyIds::buffer, &TypedArrayBase::EntryInfo::GetterBuffer, nullptr);
        library->AddAccessorsToLibraryObject(typedarrayPrototype, PropertyIds::byteLength, &TypedArrayBase::EntryInfo::GetterByteLength, nullptr);
        library->AddAccessorsToLibraryObject(typedarrayPrototype, PropertyIds::byteOffset, &TypedArrayBase::EntryInfo::GetterByteOffset, nullptr);
        library->AddAccessorsToLibraryObject(typedarrayPrototype, PropertyIds::length, &TypedArrayBase::EntryInfo::GetterLength, nullptr);

        if (scriptContext->GetConfig()->IsES6ToStringTagEnabled())
        {
            library->AddAccessorsToLibraryObjectWithName(typedarrayPrototype, PropertyIds::_symbolToStringTag,
                PropertyIds::_RuntimeFunctionNameId_toStringTag, &TypedArrayBase::EntryInfo::GetterSymbolToStringTag, nullptr);
        }
        // The toString and toLocaleString properties are shared between Array.prototype and %TypedArray%.prototype.
        // Whichever prototype is initialized first will create the functions, the other should just load the existing function objects.
        if (library->arrayPrototypeToStringFunction == nullptr)
        {
            Assert(library->arrayPrototypeToLocaleStringFunction == nullptr);

            library->arrayPrototypeToLocaleStringFunction = library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::toLocaleString, &JavascriptArray::EntryInfo::ToLocaleString, 0);
            library->arrayPrototypeToStringFunction = library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::toString, &JavascriptArray::EntryInfo::ToString, 0);
        }
        else
        {
            Assert(library->arrayPrototypeToLocaleStringFunction);

            library->AddMember(typedarrayPrototype, PropertyIds::toLocaleString, library->arrayPrototypeToLocaleStringFunction);
            library->AddMember(typedarrayPrototype, PropertyIds::toString, library->arrayPrototypeToStringFunction);
        }

        typedarrayPrototype->SetHasNoEnumerableProperties(true);
    }

#define INIT_TYPEDARRAY_CONSTRUCTOR(typedArray, typedarrayPrototype, TypeName) \
    void  JavascriptLibrary::Initialize##typedArray##Constructor(DynamicObject* typedArrayConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode) \
    { \
        typeHandler->Convert(typedArrayConstructor, mode, 4); \
        ScriptContext* scriptContext = typedArrayConstructor->GetScriptContext(); \
        JavascriptLibrary* library = typedArrayConstructor->GetLibrary(); \
        if (scriptContext->GetConfig()->IsES6TypedArrayExtensionsEnabled()) \
        { \
            library->AddMember(typedArrayConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(3), PropertyNone); \
            if (scriptContext->GetConfig()->IsES6FunctionNameEnabled()) \
            { \
                library->AddMember(typedArrayConstructor, PropertyIds::name, library->CreateStringFromCppLiteral(L#typedArray), PropertyConfigurable); \
            } \
        } \
        else \
        { \
            library->AddMember(typedArrayConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(1), PropertyNone); \
        } \
        library->AddMember(typedArrayConstructor, PropertyIds::BYTES_PER_ELEMENT, TaggedInt::ToVarUnchecked(sizeof(TypeName)), PropertyNone); \
        library->AddMember(typedArrayConstructor, PropertyIds::prototype, scriptContext->GetLibrary()->##typedarrayPrototype##, PropertyNone); \
        typedArrayConstructor->SetHasNoEnumerableProperties(true); \
    } \

    INIT_TYPEDARRAY_CONSTRUCTOR(Int8Array, Int8ArrayPrototype, int8);
    INIT_TYPEDARRAY_CONSTRUCTOR(Uint8Array, Uint8ArrayPrototype, uint8);
    INIT_TYPEDARRAY_CONSTRUCTOR(Uint8ClampedArray, Uint8ClampedArrayPrototype, uint8);
    INIT_TYPEDARRAY_CONSTRUCTOR(Int16Array, Int16ArrayPrototype, int16);
    INIT_TYPEDARRAY_CONSTRUCTOR(Uint16Array, Uint16ArrayPrototype, uint16);
    INIT_TYPEDARRAY_CONSTRUCTOR(Int32Array, Int32ArrayPrototype, int32);
    INIT_TYPEDARRAY_CONSTRUCTOR(Uint32Array, Uint32ArrayPrototype, uint32);
    INIT_TYPEDARRAY_CONSTRUCTOR(Float32Array, Float32ArrayPrototype, float);
    INIT_TYPEDARRAY_CONSTRUCTOR(Float64Array, Float64ArrayPrototype, double);

#define INIT_TYPEDARRAY_PROTOTYPE(typedArray, typedarrayPrototype, TypeName) \
    void JavascriptLibrary::Initialize##typedarrayPrototype##(DynamicObject* typedarrayPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode) \
    { \
        typeHandler->Convert(typedarrayPrototype, mode, 2); \
        ScriptContext* scriptContext = typedarrayPrototype->GetScriptContext(); \
        JavascriptLibrary* library = typedarrayPrototype->GetLibrary(); \
        library->AddMember(typedarrayPrototype, PropertyIds::constructor, library->##typedArray##Constructor); \
        if (scriptContext->GetConfig()->IsES6TypedArrayExtensionsEnabled()) \
        { \
            library->AddMember(typedarrayPrototype, PropertyIds::BYTES_PER_ELEMENT, TaggedInt::ToVarUnchecked(sizeof(TypeName)), PropertyNone); \
        } \
        else \
        { \
            library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::set, &##typedArray##::EntryInfo::Set, 2); \
            library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::subarray, &##typedArray##::EntryInfo::Subarray, 2); \
        } \
        typedarrayPrototype->SetHasNoEnumerableProperties(true); \
    } \

    INIT_TYPEDARRAY_PROTOTYPE(Int8Array, Int8ArrayPrototype, int8);
    INIT_TYPEDARRAY_PROTOTYPE(Uint8Array, Uint8ArrayPrototype, uint8);
    INIT_TYPEDARRAY_PROTOTYPE(Uint8ClampedArray, Uint8ClampedArrayPrototype, uint8);
    INIT_TYPEDARRAY_PROTOTYPE(Int16Array, Int16ArrayPrototype, int16);
    INIT_TYPEDARRAY_PROTOTYPE(Uint16Array, Uint16ArrayPrototype, uint16);
    INIT_TYPEDARRAY_PROTOTYPE(Int32Array, Int32ArrayPrototype, int32);
    INIT_TYPEDARRAY_PROTOTYPE(Uint32Array, Uint32ArrayPrototype, uint32);
    INIT_TYPEDARRAY_PROTOTYPE(Float32Array, Float32ArrayPrototype, float);
    INIT_TYPEDARRAY_PROTOTYPE(Float64Array, Float64ArrayPrototype, double);

    // For Microsoft extension typed array, like Int64Array, BoolArray, we don't have constructor as they can only be created from the projection arguments.
    // there is no subarray method either as that's another way to create typed array.
#define INIT_MSINTERNAL_TYPEDARRAY_PROTOTYPE(typedArray, typedarrayPrototype) \
    void JavascriptLibrary::Initialize##typedarrayPrototype##(DynamicObject* typedarrayPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)  \
    {   \
        typeHandler->Convert(typedarrayPrototype, mode, 1); \
        JavascriptLibrary* library = typedarrayPrototype->GetLibrary(); \
        library->AddFunctionToLibraryObject(typedarrayPrototype, PropertyIds::set, &##typedArray##::EntryInfo::Set, 2); \
        typedarrayPrototype->SetHasNoEnumerableProperties(true); \
    }   \

    INIT_MSINTERNAL_TYPEDARRAY_PROTOTYPE(Int64Array, Int64ArrayPrototype);
    INIT_MSINTERNAL_TYPEDARRAY_PROTOTYPE(Uint64Array, Uint64ArrayPrototype);
    INIT_MSINTERNAL_TYPEDARRAY_PROTOTYPE(BoolArray, BoolArrayPrototype);
    INIT_MSINTERNAL_TYPEDARRAY_PROTOTYPE(CharArray, CharArrayPrototype);

    void JavascriptLibrary::InitializePixelArrayConstructor(DynamicObject* pixelArrayConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(pixelArrayConstructor, mode, 2);

        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterPixelArray
        // so that the update is in sync with profiler

        ScriptContext* scriptContext = pixelArrayConstructor->GetScriptContext();
        JavascriptLibrary* library = pixelArrayConstructor->GetLibrary();
        library->AddMember(pixelArrayConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(1), PropertyNone);
        library->AddMember(pixelArrayConstructor, PropertyIds::prototype, scriptContext->GetLibrary()->pixelArrayPrototype, PropertyNone);

        // Note: PixelArray constructor object does not have a .name property per spec, unlike other function object built-ins.  Further,
        // the deprecation of PixelArrays no longer warrant the addition of IE12+ features

        pixelArrayConstructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializePixelArrayPrototype(DynamicObject* arrayPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(arrayPrototype, mode, 1);

        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterPixelArray
        // so that the update is in sync with profiler

        JavascriptLibrary* library = arrayPrototype->GetLibrary();
        library->AddMember(arrayPrototype, PropertyIds::constructor, library->arrayConstructor);

        arrayPrototype->SetHasNoEnumerableProperties(true);
    }

    bool JavascriptLibrary::GetPixelArrayBuffer(Var instance, BYTE **ppBuffer, UINT *pBufferLength)
    {
        AssertMsg(pixelArrayType, "Where's pixelArrayType?");

#if ENABLE_DEBUG_CONFIG_OPTIONS
        // unwrap the autoproxy'ed proxy to make the call go through.
        if (Js::Configuration::Global.flags.IsEnabled(Js::autoProxyFlag) && JavascriptProxy::Is(instance))
        {
            instance = JavascriptProxy::FromVar(instance)->GetTarget();
        }
#endif
        if (JavascriptPixelArray::Is(instance))
        {
            JavascriptPixelArray *pixelArray = JavascriptPixelArray::FromVar(instance);
            *ppBuffer = pixelArray->GetBufferPointer();
            *pBufferLength = pixelArray->GetBufferLength();
            return true;
        }

        return false;
    }

    void JavascriptLibrary::InitializeErrorConstructor(DynamicObject* constructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(constructor, mode, 4);

        ScriptContext* scriptContext = constructor->GetScriptContext();
        JavascriptLibrary* library = constructor->GetLibrary();

        library->AddMember(constructor, PropertyIds::prototype, library->errorPrototype, PropertyNone);
        library->AddMember(constructor, PropertyIds::length, TaggedInt::ToVarUnchecked(1), PropertyNone);

        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(constructor, PropertyIds::name, scriptContext->GetPropertyString(PropertyIds::Error), PropertyConfigurable);
        }
        if (scriptContext->GetConfig()->IsErrorStackTraceEnabled())
        {
            library->AddMember(constructor, PropertyIds::stackTraceLimit, JavascriptNumber::ToVar(JavascriptExceptionOperators::DefaultStackTraceLimit, scriptContext), PropertyConfigurable | PropertyWritable | PropertyEnumerable);
        }

        constructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeErrorPrototype(DynamicObject* prototype, DeferredTypeHandlerBase* typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(prototype, mode, 4);

        JavascriptLibrary* library = prototype->GetLibrary();

        library->AddMember(prototype, PropertyIds::constructor, library->errorConstructor);

        bool hasNoEnumerableProperties = true;
        PropertyAttributes prototypeNameMessageAttributes = PropertyConfigurable | PropertyWritable;

        library->AddMember(prototype, PropertyIds::name, library->CreateStringFromCppLiteral(L"Error"), prototypeNameMessageAttributes);
        library->AddMember(prototype, PropertyIds::message, library->GetEmptyString(), prototypeNameMessageAttributes);
        library->AddFunctionToLibraryObject(prototype, PropertyIds::toString, &JavascriptError::EntryInfo::ToString, 0);

        prototype->SetHasNoEnumerableProperties(hasNoEnumerableProperties);
    }

#define INIT_ERROR_CONSTRUCTOR(error) \
    void JavascriptLibrary::Initialize##error##Constructor(DynamicObject* constructor, DeferredTypeHandlerBase* typeHandler, DeferredInitializeMode mode) \
    { \
        typeHandler->Convert(constructor, mode, 3); \
        ScriptContext* scriptContext = constructor->GetScriptContext(); \
        JavascriptLibrary* library = constructor->GetLibrary(); \
        library->AddMember(constructor, PropertyIds::prototype, library->Get##error##Prototype(), PropertyNone); \
        library->AddMember(constructor, PropertyIds::length, TaggedInt::ToVarUnchecked(1), PropertyNone); \
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled()) \
        { \
            PropertyAttributes prototypeNameMessageAttributes = PropertyConfigurable; \
            library->AddMember(constructor, PropertyIds::name, library->CreateStringFromCppLiteral(L#error), prototypeNameMessageAttributes); \
        } \
        constructor->SetHasNoEnumerableProperties(true); \
    } \

    INIT_ERROR_CONSTRUCTOR(EvalError);
    INIT_ERROR_CONSTRUCTOR(RangeError);
    INIT_ERROR_CONSTRUCTOR(ReferenceError);
    INIT_ERROR_CONSTRUCTOR(SyntaxError);
    INIT_ERROR_CONSTRUCTOR(TypeError);
    INIT_ERROR_CONSTRUCTOR(URIError);
    INIT_ERROR_CONSTRUCTOR(WinRTError);

#define INIT_ERROR_PROTOTYPE(error) \
    void JavascriptLibrary::Initialize##error##Prototype(DynamicObject* prototype, DeferredTypeHandlerBase* typeHandler, DeferredInitializeMode mode) \
    { \
        typeHandler->Convert(prototype, mode, 4); \
        JavascriptLibrary* library = prototype->GetLibrary(); \
        library->AddMember(prototype, PropertyIds::constructor, library->Get##error##Constructor()); \
        bool hasNoEnumerableProperties = true; \
        PropertyAttributes prototypeNameMessageAttributes = PropertyConfigurable | PropertyWritable; \
        library->AddMember(prototype, PropertyIds::name, library->CreateStringFromCppLiteral(L#error), prototypeNameMessageAttributes); \
        library->AddMember(prototype, PropertyIds::message, library->GetEmptyString(), prototypeNameMessageAttributes); \
        library->AddFunctionToLibraryObject(prototype, PropertyIds::toString, &JavascriptError::EntryInfo::ToString, 0); \
        prototype->SetHasNoEnumerableProperties(hasNoEnumerableProperties); \
    } \

    INIT_ERROR_PROTOTYPE(EvalError);
    INIT_ERROR_PROTOTYPE(RangeError);
    INIT_ERROR_PROTOTYPE(ReferenceError);
    INIT_ERROR_PROTOTYPE(SyntaxError);
    INIT_ERROR_PROTOTYPE(TypeError);
    INIT_ERROR_PROTOTYPE(URIError);
    INIT_ERROR_PROTOTYPE(WinRTError);

    void JavascriptLibrary::InitializeBooleanConstructor(DynamicObject* booleanConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(booleanConstructor, mode, 3);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterBoolean
        // so that the update is in sync with profiler
        ScriptContext* scriptContext = booleanConstructor->GetScriptContext();
        JavascriptLibrary* library = booleanConstructor->GetLibrary();
        library->AddMember(booleanConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(1), PropertyNone);
        library->AddMember(booleanConstructor, PropertyIds::prototype, library->booleanPrototype, PropertyNone);
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(booleanConstructor, PropertyIds::name, scriptContext->GetPropertyString(PropertyIds::Boolean), PropertyConfigurable);
        }

        booleanConstructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeBooleanPrototype(DynamicObject* booleanPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(booleanPrototype, mode, 3);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterBoolean
        // so that the update is in sync with profiler
        JavascriptLibrary* library = booleanPrototype->GetLibrary();
        ScriptContext* scriptContext = booleanPrototype->GetScriptContext();
        library->AddMember(booleanPrototype, PropertyIds::constructor, library->booleanConstructor);
        scriptContext->SetBuiltInLibraryFunction(JavascriptBoolean::EntryInfo::ValueOf.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(booleanPrototype, PropertyIds::valueOf, &JavascriptBoolean::EntryInfo::ValueOf, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptBoolean::EntryInfo::ToString.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(booleanPrototype, PropertyIds::toString, &JavascriptBoolean::EntryInfo::ToString, 0));

        booleanPrototype->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeSymbolConstructor(DynamicObject* symbolConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(symbolConstructor, mode, 12);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterSymbol
        // so that the update is in sync with profiler
        JavascriptLibrary* library = symbolConstructor->GetLibrary();
        ScriptContext* scriptContext = symbolConstructor->GetScriptContext();
        library->AddMember(symbolConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(0), PropertyNone);
        library->AddMember(symbolConstructor, PropertyIds::prototype, library->symbolPrototype, PropertyNone);
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(symbolConstructor, PropertyIds::name, scriptContext->GetPropertyString(PropertyIds::Symbol), PropertyConfigurable);
        }
        library->AddMember(symbolConstructor, PropertyIds::hasInstance, library->GetSymbolHasInstance(), PropertyNone);
        if (scriptContext->GetConfig()->IsES6IsConcatSpreadableEnabled())
        {
            library->AddMember(symbolConstructor, PropertyIds::isConcatSpreadable, library->GetSymbolIsConcatSpreadable(), PropertyNone);
        }
        library->AddMember(symbolConstructor, PropertyIds::iterator, library->GetSymbolIterator(), PropertyNone);
        library->AddMember(symbolConstructor, PropertyIds::species, library->GetSymbolSpecies(), PropertyNone);

        if (scriptContext->GetConfig()->IsES6ToPrimitiveEnabled())
        {
            library->AddMember(symbolConstructor, PropertyIds::toPrimitive, library->GetSymbolToPrimitive(), PropertyNone);
        }

        if (scriptContext->GetConfig()->IsES6ToStringTagEnabled())
        {
            library->AddMember(symbolConstructor, PropertyIds::toStringTag, library->GetSymbolToStringTag(), PropertyNone);
        }
        library->AddMember(symbolConstructor, PropertyIds::unscopables, library->GetSymbolUnscopables(), PropertyNone);

        library->AddFunctionToLibraryObject(symbolConstructor, PropertyIds::for_, &JavascriptSymbol::EntryInfo::For, 1);
        library->AddFunctionToLibraryObject(symbolConstructor, PropertyIds::keyFor, &JavascriptSymbol::EntryInfo::KeyFor, 1);

        symbolConstructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeSymbolPrototype(DynamicObject* symbolPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(symbolPrototype, mode, 5);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterSymbol
        // so that the update is in sync with profiler
        JavascriptLibrary* library = symbolPrototype->GetLibrary();
        ScriptContext* scriptContext = symbolPrototype->GetScriptContext();

        library->AddMember(symbolPrototype, PropertyIds::constructor, library->symbolConstructor);

        if (scriptContext->GetConfig()->IsES6ToStringTagEnabled())
        {
            library->AddMember(symbolPrototype, PropertyIds::_symbolToStringTag, library->CreateStringFromCppLiteral(L"Symbol"), PropertyConfigurable);
        }
        scriptContext->SetBuiltInLibraryFunction(JavascriptSymbol::EntryInfo::ValueOf.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(symbolPrototype, PropertyIds::valueOf, &JavascriptSymbol::EntryInfo::ValueOf, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptSymbol::EntryInfo::ToString.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(symbolPrototype, PropertyIds::toString, &JavascriptSymbol::EntryInfo::ToString, 0));

        if (scriptContext->GetConfig()->IsES6ToPrimitiveEnabled())
        {
            scriptContext->SetBuiltInLibraryFunction(JavascriptSymbol::EntryInfo::SymbolToPrimitive.GetOriginalEntryPoint(),
                library->AddFunctionToLibraryObjectWithName(symbolPrototype, PropertyIds::_symbolToPrimitive, PropertyIds::_RuntimeFunctionNameId_toPrimitive,
                &JavascriptSymbol::EntryInfo::SymbolToPrimitive, 1));
            symbolPrototype->SetWritable(PropertyIds::_symbolToPrimitive, false);
        }
        symbolPrototype->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializePromiseConstructor(DynamicObject* promiseConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(promiseConstructor, mode, 8);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterPromise
        // so that the update is in sync with profiler
        JavascriptLibrary* library = promiseConstructor->GetLibrary();
        ScriptContext* scriptContext = promiseConstructor->GetScriptContext();
        library->AddMember(promiseConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(1), PropertyNone);
        library->AddMember(promiseConstructor, PropertyIds::prototype, library->promisePrototype, PropertyNone);
        library->AddAccessorsToLibraryObject(promiseConstructor, PropertyIds::_symbolSpecies, &JavascriptPromise::EntryInfo::GetterSymbolSpecies, nullptr);
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(promiseConstructor, PropertyIds::name, scriptContext->GetPropertyString(PropertyIds::Promise), PropertyConfigurable);
        }
        library->AddFunctionToLibraryObject(promiseConstructor, PropertyIds::all, &JavascriptPromise::EntryInfo::All, 1);
        library->AddFunctionToLibraryObject(promiseConstructor, PropertyIds::race, &JavascriptPromise::EntryInfo::Race, 1);
        library->AddFunctionToLibraryObject(promiseConstructor, PropertyIds::reject, &JavascriptPromise::EntryInfo::Reject, 1);
        library->AddFunctionToLibraryObject(promiseConstructor, PropertyIds::resolve, &JavascriptPromise::EntryInfo::Resolve, 1);

        promiseConstructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializePromisePrototype(DynamicObject* promisePrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(promisePrototype, mode, 4);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterPromise
        // so that the update is in sync with profiler
        JavascriptLibrary* library = promisePrototype->GetLibrary();
        ScriptContext* scriptContext = promisePrototype->GetScriptContext();

        library->AddMember(promisePrototype, PropertyIds::constructor, library->promiseConstructor);
        if (scriptContext->GetConfig()->IsES6ToStringTagEnabled())
        {
            library->AddMember(promisePrototype, PropertyIds::_symbolToStringTag, library->CreateStringFromCppLiteral(L"Promise"), PropertyConfigurable);
        }
        scriptContext->SetBuiltInLibraryFunction(JavascriptPromise::EntryInfo::Catch.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(promisePrototype, PropertyIds::catch_, &JavascriptPromise::EntryInfo::Catch, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptPromise::EntryInfo::Then.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(promisePrototype, PropertyIds::then, &JavascriptPromise::EntryInfo::Then, 2));

        promisePrototype->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeGeneratorFunctionConstructor(DynamicObject* generatorFunctionConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(generatorFunctionConstructor, mode, 3);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterGeneratorFunction
        // so that the update is in sync with profiler
        JavascriptLibrary* library = generatorFunctionConstructor->GetLibrary();
        ScriptContext* scriptContext = generatorFunctionConstructor->GetScriptContext();
        library->AddMember(generatorFunctionConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(1), PropertyConfigurable);
        library->AddMember(generatorFunctionConstructor, PropertyIds::prototype, library->generatorFunctionPrototype, PropertyNone);
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(generatorFunctionConstructor, PropertyIds::name, library->CreateStringFromCppLiteral(L"GeneratorFunction"), PropertyConfigurable);
        }
        generatorFunctionConstructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeGeneratorFunctionProtoype(DynamicObject* generatorFunctionPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(generatorFunctionPrototype, mode, 3);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterGeneratorFunction
        // so that the update is in sync with profiler
        JavascriptLibrary* library = generatorFunctionPrototype->GetLibrary();
        ScriptContext* scriptContext = library->GetScriptContext();

        library->AddMember(generatorFunctionPrototype, PropertyIds::constructor, library->generatorFunctionConstructor, PropertyConfigurable);
        library->AddMember(generatorFunctionPrototype, PropertyIds::prototype, library->generatorPrototype, PropertyConfigurable);
        if (scriptContext->GetConfig()->IsES6ToStringTagEnabled())
        {
            library->AddMember(generatorFunctionPrototype, PropertyIds::_symbolToStringTag, library->CreateStringFromCppLiteral(L"GeneratorFunction"), PropertyConfigurable);
        }
        generatorFunctionPrototype->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeGeneratorProtoype(DynamicObject* generatorPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(generatorPrototype, mode, 6);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterGenerator
        // so that the update is in sync with profiler
        JavascriptLibrary* library = generatorPrototype->GetLibrary();
        ScriptContext* scriptContext = library->GetScriptContext();

        library->AddMember(generatorPrototype, PropertyIds::constructor, library->generatorFunctionPrototype, PropertyConfigurable);
        if (scriptContext->GetConfig()->IsES6ToStringTagEnabled())
        {
            library->AddMember(generatorPrototype, PropertyIds::_symbolToStringTag, library->CreateStringFromCppLiteral(L"Generator"), PropertyConfigurable);
        }
        library->AddFunctionToLibraryObject(generatorPrototype, PropertyIds::next, &JavascriptGenerator::EntryInfo::Next, 1);
        library->AddFunctionToLibraryObject(generatorPrototype, PropertyIds::return_, &JavascriptGenerator::EntryInfo::Return, 1);
        library->AddFunctionToLibraryObject(generatorPrototype, PropertyIds::throw_, &JavascriptGenerator::EntryInfo::Throw, 1);
        library->AddFunctionToLibraryObjectWithName(generatorPrototype, PropertyIds::_symbolIterator,
            PropertyIds::_RuntimeFunctionNameId_iterator, &JavascriptGenerator::EntryInfo::SymbolIterator, 0);

        generatorPrototype->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeProxyConstructor(DynamicObject* proxyConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(proxyConstructor, mode, 4);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterSymbol
        // so that the update is in sync with profiler
        JavascriptLibrary* library = proxyConstructor->GetLibrary();
        ScriptContext* scriptContext = proxyConstructor->GetScriptContext();
        library->AddMember(proxyConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(2), PropertyNone);
        library->AddMember(proxyConstructor, PropertyIds::prototype, library->GetProxyPrototype(), PropertyNone);
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(proxyConstructor, PropertyIds::name, scriptContext->GetPropertyString(PropertyIds::Proxy), PropertyConfigurable);
        }
        library->AddFunctionToLibraryObject(proxyConstructor, PropertyIds::revocable, &JavascriptProxy::EntryInfo::Revocable, PropertyNone);

        proxyConstructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeProxyPrototype(DynamicObject* proxyPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(proxyPrototype, mode, 1);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterSymbol
        // so that the update is in sync with profiler
        JavascriptLibrary* library = proxyPrototype->GetLibrary();
        library->AddMember(proxyPrototype, PropertyIds::constructor, library->proxyConstructor);

        proxyPrototype->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeDateConstructor(DynamicObject* dateConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(dateConstructor, mode, 6);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterDate
        // so that the update is in sync with profiler
        JavascriptLibrary* library = dateConstructor->GetLibrary();
        ScriptContext* scriptContext = dateConstructor->GetScriptContext();
        library->AddMember(dateConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(7), PropertyNone);
        library->AddMember(dateConstructor, PropertyIds::prototype, library->datePrototype, PropertyNone);
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(dateConstructor, PropertyIds::name, scriptContext->GetPropertyString(PropertyIds::Date), PropertyConfigurable);
        }
        library->AddFunctionToLibraryObject(dateConstructor, PropertyIds::parse, &JavascriptDate::EntryInfo::Parse, 1); // should be static
        library->AddFunctionToLibraryObject(dateConstructor, PropertyIds::now, &JavascriptDate::EntryInfo::Now, 0);     // should be static
        library->AddFunctionToLibraryObject(dateConstructor, PropertyIds::UTC, &JavascriptDate::EntryInfo::UTC, 7);     // should be static

        dateConstructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeDatePrototype(DynamicObject* datePrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(datePrototype, mode, 51);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterDate
        // so that the update is in sync with profiler
        ScriptContext* scriptContext = datePrototype->GetScriptContext();
        JavascriptLibrary* library = datePrototype->GetLibrary();
        library->AddMember(datePrototype, PropertyIds::constructor, library->dateConstructor);
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetDate.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getDate, &JavascriptDate::EntryInfo::GetDate, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetDay.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getDay, &JavascriptDate::EntryInfo::GetDay, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetFullYear.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getFullYear, &JavascriptDate::EntryInfo::GetFullYear, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetHours.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getHours, &JavascriptDate::EntryInfo::GetHours, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetMilliseconds.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getMilliseconds, &JavascriptDate::EntryInfo::GetMilliseconds, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetMinutes.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getMinutes, &JavascriptDate::EntryInfo::GetMinutes, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetMonth.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getMonth, &JavascriptDate::EntryInfo::GetMonth, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetSeconds.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getSeconds, &JavascriptDate::EntryInfo::GetSeconds, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetTime.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getTime, &JavascriptDate::EntryInfo::GetTime, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetTimezoneOffset.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getTimezoneOffset, &JavascriptDate::EntryInfo::GetTimezoneOffset, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetUTCDate.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getUTCDate, &JavascriptDate::EntryInfo::GetUTCDate, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetUTCDay.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getUTCDay, &JavascriptDate::EntryInfo::GetUTCDay, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetUTCFullYear.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getUTCFullYear, &JavascriptDate::EntryInfo::GetUTCFullYear, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetUTCHours.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getUTCHours, &JavascriptDate::EntryInfo::GetUTCHours, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetUTCMilliseconds.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getUTCMilliseconds, &JavascriptDate::EntryInfo::GetUTCMilliseconds, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetUTCMinutes.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getUTCMinutes, &JavascriptDate::EntryInfo::GetUTCMinutes, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetUTCMonth.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getUTCMonth, &JavascriptDate::EntryInfo::GetUTCMonth, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetUTCSeconds.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getUTCSeconds, &JavascriptDate::EntryInfo::GetUTCSeconds, 0));
        if (scriptContext->GetConfig()->SupportsES3Extensions() && scriptContext->GetConfig()->GetHostType() != HostTypeApplication)
        {
            scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetVarDate.GetOriginalEntryPoint(),
                library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getVarDate, &JavascriptDate::EntryInfo::GetVarDate, 0));
        }
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::GetYear.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::getYear, &JavascriptDate::EntryInfo::GetYear, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::SetDate.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::setDate, &JavascriptDate::EntryInfo::SetDate, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::SetFullYear.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::setFullYear, &JavascriptDate::EntryInfo::SetFullYear, 3));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::SetHours.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::setHours, &JavascriptDate::EntryInfo::SetHours, 4));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::SetMilliseconds.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::setMilliseconds, &JavascriptDate::EntryInfo::SetMilliseconds, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::SetMinutes.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::setMinutes, &JavascriptDate::EntryInfo::SetMinutes, 3));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::SetMonth.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::setMonth, &JavascriptDate::EntryInfo::SetMonth, 2));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::SetSeconds.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::setSeconds, &JavascriptDate::EntryInfo::SetSeconds, 2));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::SetTime.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::setTime, &JavascriptDate::EntryInfo::SetTime, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::SetUTCDate.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::setUTCDate, &JavascriptDate::EntryInfo::SetUTCDate, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::SetUTCFullYear.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::setUTCFullYear, &JavascriptDate::EntryInfo::SetUTCFullYear, 3));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::SetUTCHours.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::setUTCHours, &JavascriptDate::EntryInfo::SetUTCHours, 4));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::SetUTCMilliseconds.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::setUTCMilliseconds, &JavascriptDate::EntryInfo::SetUTCMilliseconds, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::SetUTCMinutes.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::setUTCMinutes, &JavascriptDate::EntryInfo::SetUTCMinutes, 3));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::SetUTCMonth.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::setUTCMonth, &JavascriptDate::EntryInfo::SetUTCMonth, 2));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::SetUTCSeconds.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::setUTCSeconds, &JavascriptDate::EntryInfo::SetUTCSeconds, 2));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::SetYear.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::setYear, &JavascriptDate::EntryInfo::SetYear, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::ToDateString.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::toDateString, &JavascriptDate::EntryInfo::ToDateString, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::ToISOString.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::toISOString, &JavascriptDate::EntryInfo::ToISOString, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::ToJSON.GetOriginalEntryPoint(),
             library->AddFunctionToLibraryObject(datePrototype, PropertyIds::toJSON, &JavascriptDate::EntryInfo::ToJSON, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::ToLocaleDateString.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::toLocaleDateString, &JavascriptDate::EntryInfo::ToLocaleDateString, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::ToLocaleString.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::toLocaleString, &JavascriptDate::EntryInfo::ToLocaleString, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::ToLocaleTimeString.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::toLocaleTimeString, &JavascriptDate::EntryInfo::ToLocaleTimeString, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::ToString.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::toString, &JavascriptDate::EntryInfo::ToString, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::ToTimeString.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::toTimeString, &JavascriptDate::EntryInfo::ToTimeString, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::ToUTCString.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::toUTCString, &JavascriptDate::EntryInfo::ToUTCString, 0));
        library->AddFunctionToLibraryObject(datePrototype, PropertyIds::toGMTString, &JavascriptDate::EntryInfo::ToGMTString, 0);
        scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::ValueOf.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(datePrototype, PropertyIds::valueOf, &JavascriptDate::EntryInfo::ValueOf, 0));

        if (scriptContext->GetConfig()->IsES6ToPrimitiveEnabled())
        {
            scriptContext->SetBuiltInLibraryFunction(JavascriptDate::EntryInfo::SymbolToPrimitive.GetOriginalEntryPoint(),
                library->AddFunctionToLibraryObjectWithName(datePrototype, PropertyIds::_symbolToPrimitive, PropertyIds::_RuntimeFunctionNameId_toPrimitive,
                &JavascriptDate::EntryInfo::SymbolToPrimitive, 1));
            datePrototype->SetWritable(PropertyIds::_symbolToPrimitive, false);
        }
        datePrototype->SetHasNoEnumerableProperties(true);
    }


    void JavascriptLibrary::InitializeFunctionConstructor(DynamicObject* functionConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(functionConstructor, mode, 3);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterFunction
        // so that the update is in sync with profiler
        ScriptContext* scriptContext = functionConstructor->GetScriptContext();
        JavascriptLibrary* library = functionConstructor->GetLibrary();
        library->AddMember(functionConstructor, PropertyIds::prototype, library->functionPrototype, PropertyNone);
        library->AddMember(functionConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(1), PropertyConfigurable);
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(functionConstructor, PropertyIds::name, scriptContext->GetPropertyString(PropertyIds::Function), PropertyConfigurable);
        }
        functionConstructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeFunctionPrototype(DynamicObject* functionPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(functionPrototype, mode, 7);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterFunction
        // so that the update is in sync with profiler
        ScriptContext* scriptContext = functionPrototype->GetScriptContext();
        JavascriptLibrary* library = functionPrototype->GetLibrary();
        JavascriptFunction ** builtinFuncs = library->GetBuiltinFunctions();

        library->AddMember(functionPrototype, PropertyIds::constructor, library->functionConstructor);
        library->AddMember(functionPrototype, PropertyIds::length, TaggedInt::ToVarUnchecked(0), PropertyConfigurable);

        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(functionPrototype, PropertyIds::name, LiteralString::CreateEmptyString(scriptContext->GetLibrary()->GetStringTypeStatic()), PropertyConfigurable);
        }

        JavascriptFunction *func = library->AddFunctionToLibraryObject(functionPrototype, PropertyIds::apply, &JavascriptFunction::EntryInfo::Apply, 2);
        builtinFuncs[BuiltinFunction::Function_Apply] = func;

        library->AddFunctionToLibraryObject(functionPrototype, PropertyIds::bind, &JavascriptFunction::EntryInfo::Bind, 1);
        func = library->AddFunctionToLibraryObject(functionPrototype, PropertyIds::call, &JavascriptFunction::EntryInfo::Call, 1);
        builtinFuncs[BuiltinFunction::Function_Call] = func;
        library->AddFunctionToLibraryObject(functionPrototype, PropertyIds::toString, &JavascriptFunction::EntryInfo::ToString, 0);

        if (scriptContext->GetConfig()->IsES6ClassAndExtendsEnabled())
        {
            library->AddFunctionToLibraryObject(functionPrototype, PropertyIds::toMethod, &JavascriptFunction::EntryInfo::ToMethod, 1);
        }

        DebugOnly(CheckRegisteredBuiltIns(builtinFuncs));

        functionPrototype->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeDebug()
    {
        // !!! Warning !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // Any changes to this function might require a corresponding change to CopyOnWriteGlobal() above.
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    }

    void JavascriptLibrary::InitializeComplexThings()
    {
        // !!! Warning !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // Any changes to this function might require a corresponding change to CopyOnWriteGlobal() above.
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        Recycler *const recycler = GetRecycler();

        // Creating the regex prototype object requires compiling an empty regex, which may require error types to be
        // initialized first (such as when a stack probe fails). So, the regex prototype and other things that depend on it are
        // initialized here, which will be after the dependency types are initialized.
        //
        // In ES6, RegExp.prototype is not a RegExp object itself so we do not need to wait and create an empty RegExp.
        // Instead, we just create an ordinary object prototype for RegExp.prototype in InitializePrototypes.
        if (!scriptContext->GetConfig()->IsES6PrototypeChain() && regexPrototype == nullptr)
        {
            // This will be non-null during CopyOnWrite().
            regexPrototype = RecyclerNew(recycler, JavascriptRegExp, RegexHelper::CompileDynamic(scriptContext, L"", 0, L"", 0, false),
                DynamicType::New(scriptContext, TypeIds_RegEx, objectPrototype, nullptr,
                DeferredTypeHandler<InitializeRegexPrototype>::GetDefaultInstance()));
        }

        regexType = DynamicType::New(scriptContext, TypeIds_RegEx, regexPrototype, nullptr,
            SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);

        // See JavascriptRegExp::IsWritable for special non-writable properties
        regexType->GetTypeHandler()->ClearHasOnlyWritableDataProperties();
    }

    void JavascriptLibrary::InitializeMathObject(DynamicObject* mathObject, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(mathObject, mode, 42);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterMath
        // so that the update is in sync with profiler
        ScriptContext* scriptContext = mathObject->GetScriptContext();
        JavascriptLibrary* library = mathObject->GetLibrary();

        library->AddMember(mathObject, PropertyIds::E,       JavascriptNumber::New(Math::E,       scriptContext), PropertyNone);
        library->AddMember(mathObject, PropertyIds::LN10,    JavascriptNumber::New(Math::LN10,    scriptContext), PropertyNone);
        library->AddMember(mathObject, PropertyIds::LN2,     JavascriptNumber::New(Math::LN2,     scriptContext), PropertyNone);
        library->AddMember(mathObject, PropertyIds::LOG2E,   JavascriptNumber::New(Math::LOG2E,   scriptContext), PropertyNone);
        library->AddMember(mathObject, PropertyIds::LOG10E,  JavascriptNumber::New(Math::LOG10E,  scriptContext), PropertyNone);
        library->AddMember(mathObject, PropertyIds::PI,      library->pi,                                         PropertyNone);
        library->AddMember(mathObject, PropertyIds::SQRT1_2, JavascriptNumber::New(Math::SQRT1_2, scriptContext), PropertyNone);
        library->AddMember(mathObject, PropertyIds::SQRT2,   JavascriptNumber::New(Math::SQRT2,   scriptContext), PropertyNone);

        JavascriptFunction ** builtinFuncs = library->GetBuiltinFunctions();

        builtinFuncs[BuiltinFunction::Math_Abs]    = library->AddFunctionToLibraryObject(mathObject, PropertyIds::abs,    &Math::EntryInfo::Abs,    1);
        builtinFuncs[BuiltinFunction::Math_Acos]   = library->AddFunctionToLibraryObject(mathObject, PropertyIds::acos,   &Math::EntryInfo::Acos,   1);
        builtinFuncs[BuiltinFunction::Math_Asin]   = library->AddFunctionToLibraryObject(mathObject, PropertyIds::asin,   &Math::EntryInfo::Asin,   1);
        builtinFuncs[BuiltinFunction::Math_Atan]   = library->AddFunctionToLibraryObject(mathObject, PropertyIds::atan,   &Math::EntryInfo::Atan,   1);
        builtinFuncs[BuiltinFunction::Math_Atan2]  = library->AddFunctionToLibraryObject(mathObject, PropertyIds::atan2,  &Math::EntryInfo::Atan2,  2);
        builtinFuncs[BuiltinFunction::Math_Cos]    = library->AddFunctionToLibraryObject(mathObject, PropertyIds::cos,    &Math::EntryInfo::Cos,    1);
        builtinFuncs[BuiltinFunction::Math_Ceil]   = library->AddFunctionToLibraryObject(mathObject, PropertyIds::ceil,   &Math::EntryInfo::Ceil,   1);
        builtinFuncs[BuiltinFunction::Math_Exp]    = library->AddFunctionToLibraryObject(mathObject, PropertyIds::exp,    &Math::EntryInfo::Exp,    1);
        builtinFuncs[BuiltinFunction::Math_Floor]  = library->AddFunctionToLibraryObject(mathObject, PropertyIds::floor,  &Math::EntryInfo::Floor,  1);
        builtinFuncs[BuiltinFunction::Math_Log]    = library->AddFunctionToLibraryObject(mathObject, PropertyIds::log,    &Math::EntryInfo::Log,    1);
        builtinFuncs[BuiltinFunction::Math_Max]    = library->AddFunctionToLibraryObject(mathObject, PropertyIds::max,    &Math::EntryInfo::Max,    2);
        builtinFuncs[BuiltinFunction::Math_Min]    = library->AddFunctionToLibraryObject(mathObject, PropertyIds::min,    &Math::EntryInfo::Min,    2);
        builtinFuncs[BuiltinFunction::Math_Pow]    = library->AddFunctionToLibraryObject(mathObject, PropertyIds::pow,    &Math::EntryInfo::Pow,    2);
        builtinFuncs[BuiltinFunction::Math_Random] = library->AddFunctionToLibraryObject(mathObject, PropertyIds::random, &Math::EntryInfo::Random, 0);
        builtinFuncs[BuiltinFunction::Math_Round]  = library->AddFunctionToLibraryObject(mathObject, PropertyIds::round,  &Math::EntryInfo::Round,  1);
        builtinFuncs[BuiltinFunction::Math_Sin]    = library->AddFunctionToLibraryObject(mathObject, PropertyIds::sin,    &Math::EntryInfo::Sin,    1);
        builtinFuncs[BuiltinFunction::Math_Sqrt]   = library->AddFunctionToLibraryObject(mathObject, PropertyIds::sqrt,   &Math::EntryInfo::Sqrt,   1);
        builtinFuncs[BuiltinFunction::Math_Tan]    = library->AddFunctionToLibraryObject(mathObject, PropertyIds::tan,    &Math::EntryInfo::Tan,    1);

        if (scriptContext->GetConfig()->IsES6MathExtensionsEnabled())
        {
            builtinFuncs[BuiltinFunction::Math_Imul] = library->AddFunctionToLibraryObject(mathObject, PropertyIds::imul, &Math::EntryInfo::Imul, 2);
            builtinFuncs[BuiltinFunction::Math_Fround] = library->AddFunctionToLibraryObject(mathObject, PropertyIds::fround, &Math::EntryInfo::Fround, 1);
            // TODO: Implement inlining of the following ES6 Math functions
            /*builtinFuncs[BuiltinFunction::Math_Log10] =*/ library->AddFunctionToLibraryObject(mathObject, PropertyIds::log10, &Math::EntryInfo::Log10, 1);
            /*builtinFuncs[BuiltinFunction::Math_Log2]  =*/ library->AddFunctionToLibraryObject(mathObject, PropertyIds::log2,  &Math::EntryInfo::Log2,  1);
            /*builtinFuncs[BuiltinFunction::Math_Log1p] =*/ library->AddFunctionToLibraryObject(mathObject, PropertyIds::log1p, &Math::EntryInfo::Log1p, 1);
            /*builtinFuncs[BuiltinFunction::Math_Expm1] =*/ library->AddFunctionToLibraryObject(mathObject, PropertyIds::expm1, &Math::EntryInfo::Expm1, 1);
            /*builtinFuncs[BuiltinFunction::Math_Cosh]  =*/ library->AddFunctionToLibraryObject(mathObject, PropertyIds::cosh,  &Math::EntryInfo::Cosh,  1);
            /*builtinFuncs[BuiltinFunction::Math_Sinh]  =*/ library->AddFunctionToLibraryObject(mathObject, PropertyIds::sinh,  &Math::EntryInfo::Sinh,  1);
            /*builtinFuncs[BuiltinFunction::Math_Tanh]  =*/ library->AddFunctionToLibraryObject(mathObject, PropertyIds::tanh,  &Math::EntryInfo::Tanh,  1);
            /*builtinFuncs[BuiltinFunction::Math_Acosh] =*/ library->AddFunctionToLibraryObject(mathObject, PropertyIds::acosh, &Math::EntryInfo::Acosh, 1);
            /*builtinFuncs[BuiltinFunction::Math_Asinh] =*/ library->AddFunctionToLibraryObject(mathObject, PropertyIds::asinh, &Math::EntryInfo::Asinh, 1);
            /*builtinFuncs[BuiltinFunction::Math_Atanh] =*/ library->AddFunctionToLibraryObject(mathObject, PropertyIds::atanh, &Math::EntryInfo::Atanh, 1);
            /*builtinFuncs[BuiltinFunction::Math_Hypot] =*/ library->AddFunctionToLibraryObject(mathObject, PropertyIds::hypot, &Math::EntryInfo::Hypot, 2);
            /*builtinFuncs[BuiltinFunction::Math_Trunc] =*/ library->AddFunctionToLibraryObject(mathObject, PropertyIds::trunc, &Math::EntryInfo::Trunc, 1);
            /*builtinFuncs[BuiltinFunction::Math_Sign]  =*/ library->AddFunctionToLibraryObject(mathObject, PropertyIds::sign,  &Math::EntryInfo::Sign,  1);
            /*builtinFuncs[BuiltinFunction::Math_Cbrt]  =*/ library->AddFunctionToLibraryObject(mathObject, PropertyIds::cbrt,  &Math::EntryInfo::Cbrt,  1);
            /*builtinFuncs[BuiltinFunction::Math_Clz32] =*/ library->AddFunctionToLibraryObject(mathObject, PropertyIds::clz32, &Math::EntryInfo::Clz32, 1);
        }

        if (scriptContext->GetConfig()->IsES6ToStringTagEnabled())
        {
            library->AddMember(mathObject, PropertyIds::_symbolToStringTag, library->CreateStringFromCppLiteral(L"Math"), PropertyConfigurable);
        }

        DebugOnly(CheckRegisteredBuiltIns(builtinFuncs));

        mathObject->SetHasNoEnumerableProperties(true);
    }

#ifdef SIMD_JS_ENABLED
    // SIMD
    void JavascriptLibrary::InitializeSIMDObject(DynamicObject* simdObject, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        // TODO: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterSIMD so that the update is in sync with profiler
        typeHandler->Convert(simdObject, mode, 2);
        ScriptContext* scriptContext = simdObject->GetScriptContext();
        JavascriptLibrary* library = simdObject->GetLibrary();

        // TODO: Add these function to SIMD prototype objects
        // SIMD review: considering using same machanism as signMask?
        library->simdFloat32x4ToStringFunction = library->DefaultCreateFunction(&JavascriptSIMDFloat32x4::EntryInfo::ToString, 1, nullptr, nullptr, PropertyIds::toString);
        library->simdFloat64x2ToStringFunction = library->DefaultCreateFunction(&JavascriptSIMDFloat64x2::EntryInfo::ToString, 1, nullptr, nullptr, PropertyIds::toString);
        library->simdInt32x4ToStringFunction = library->DefaultCreateFunction(&JavascriptSIMDInt32x4::EntryInfo::ToString, 1, nullptr, nullptr, PropertyIds::toString);

        // Q: Do we need to do scriptContext->SetBuiltInLibraryFunction(..) ?
        // TODO: make sure all SIMD properties are Non-writable and Non-configurable

        // Float32x4
        JavascriptFunction* float32x4Function = library->AddFunctionToLibraryObject(simdObject, PropertyIds::float32x4, &SIMDFloat32x4Lib::EntryInfo::Float32x4, 5, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::zero, &SIMDFloat32x4Lib::EntryInfo::Zero, 1, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::splat, &SIMDFloat32x4Lib::EntryInfo::Splat, 2, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::withX, &SIMDFloat32x4Lib::EntryInfo::WithX, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::withY, &SIMDFloat32x4Lib::EntryInfo::WithY, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::withZ, &SIMDFloat32x4Lib::EntryInfo::WithZ, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::withW, &SIMDFloat32x4Lib::EntryInfo::WithW, 3, PropertyNone);
        // type conversions
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::fromFloat64x2, &SIMDFloat32x4Lib::EntryInfo::FromFloat64x2, 2, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::fromFloat64x2Bits, &SIMDFloat32x4Lib::EntryInfo::FromFloat64x2Bits, 2, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::fromInt32x4, &SIMDFloat32x4Lib::EntryInfo::FromInt32x4, 2, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::fromInt32x4Bits, &SIMDFloat32x4Lib::EntryInfo::FromInt32x4Bits, 2, PropertyNone);
        // binary ops
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::add, &SIMDFloat32x4Lib::EntryInfo::Add, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::sub, &SIMDFloat32x4Lib::EntryInfo::Sub, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::mul, &SIMDFloat32x4Lib::EntryInfo::Mul, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::div, &SIMDFloat32x4Lib::EntryInfo::Div, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::and, &SIMDFloat32x4Lib::EntryInfo::And, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::or, &SIMDFloat32x4Lib::EntryInfo::Or, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::xor, &SIMDFloat32x4Lib::EntryInfo::Xor, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::min, &SIMDFloat32x4Lib::EntryInfo::Min, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::max, &SIMDFloat32x4Lib::EntryInfo::Max, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::scale, &SIMDFloat32x4Lib::EntryInfo::Scale, 3, PropertyNone);
        // unary ops
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::abs, &SIMDFloat32x4Lib::EntryInfo::Abs, 2, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::neg, &SIMDFloat32x4Lib::EntryInfo::Neg, 2, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::not, &SIMDFloat32x4Lib::EntryInfo::Not, 2, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::sqrt, &SIMDFloat32x4Lib::EntryInfo::Sqrt, 2, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::reciprocal, &SIMDFloat32x4Lib::EntryInfo::Reciprocal, 2, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::reciprocalSqrt, &SIMDFloat32x4Lib::EntryInfo::ReciprocalSqrt, 2, PropertyNone);
        // compare ops
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::lessThan, &SIMDFloat32x4Lib::EntryInfo::LessThan, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::lessThanOrEqual, &SIMDFloat32x4Lib::EntryInfo::LessThanOrEqual, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::equal, &SIMDFloat32x4Lib::EntryInfo::Equal, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::notEqual, &SIMDFloat32x4Lib::EntryInfo::NotEqual, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::greaterThan, &SIMDFloat32x4Lib::EntryInfo::GreaterThan, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::greaterThanOrEqual, &SIMDFloat32x4Lib::EntryInfo::GreaterThanOrEqual, 3, PropertyNone);
        // others
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::shuffle, &SIMDFloat32x4Lib::EntryInfo::Shuffle, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::shuffleMix, &SIMDFloat32x4Lib::EntryInfo::ShuffleMix, 4, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::clamp, &SIMDFloat32x4Lib::EntryInfo::Clamp, 4, PropertyNone);
        library->AddFunctionToLibraryObject(float32x4Function, PropertyIds::select, &SIMDFloat32x4Lib::EntryInfo::Select, 4, PropertyNone);
        // end Float32x4

        // Float64x2
        JavascriptFunction* float64x2Function = library->AddFunctionToLibraryObject(simdObject, PropertyIds::float64x2, &SIMDFloat64x2Lib::EntryInfo::Float64x2, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::zero, &SIMDFloat64x2Lib::EntryInfo::Zero, 1, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::splat, &SIMDFloat64x2Lib::EntryInfo::Splat, 2, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::withX, &SIMDFloat64x2Lib::EntryInfo::WithX, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::withY, &SIMDFloat64x2Lib::EntryInfo::WithY, 3, PropertyNone);
        // type conversions
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::fromFloat32x4, &SIMDFloat64x2Lib::EntryInfo::FromFloat32x4, 2, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::fromFloat32x4Bits, &SIMDFloat64x2Lib::EntryInfo::FromFloat32x4Bits, 2, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::fromInt32x4, &SIMDFloat64x2Lib::EntryInfo::FromInt32x4, 2, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::fromInt32x4Bits, &SIMDFloat64x2Lib::EntryInfo::FromInt32x4Bits, 2, PropertyNone);
        // binary ops
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::add, &SIMDFloat64x2Lib::EntryInfo::Add, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::sub, &SIMDFloat64x2Lib::EntryInfo::Sub, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::mul, &SIMDFloat64x2Lib::EntryInfo::Mul, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::div, &SIMDFloat64x2Lib::EntryInfo::Div, 3, PropertyNone);

        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::min, &SIMDFloat64x2Lib::EntryInfo::Min, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::max, &SIMDFloat64x2Lib::EntryInfo::Max, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::scale, &SIMDFloat64x2Lib::EntryInfo::Scale, 3, PropertyNone);
        // unary ops
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::abs, &SIMDFloat64x2Lib::EntryInfo::Abs, 2, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::neg, &SIMDFloat64x2Lib::EntryInfo::Neg, 2, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::sqrt, &SIMDFloat64x2Lib::EntryInfo::Sqrt, 2, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::reciprocal, &SIMDFloat64x2Lib::EntryInfo::Reciprocal, 2, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::reciprocalSqrt, &SIMDFloat64x2Lib::EntryInfo::ReciprocalSqrt, 2, PropertyNone);
        // compare ops
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::lessThan, &SIMDFloat64x2Lib::EntryInfo::LessThan, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::lessThanOrEqual, &SIMDFloat64x2Lib::EntryInfo::LessThanOrEqual, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::equal, &SIMDFloat64x2Lib::EntryInfo::Equal, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::notEqual, &SIMDFloat64x2Lib::EntryInfo::NotEqual, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::greaterThan, &SIMDFloat64x2Lib::EntryInfo::GreaterThan, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::greaterThanOrEqual, &SIMDFloat64x2Lib::EntryInfo::GreaterThanOrEqual, 3, PropertyNone);
        // others
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::shuffle, &SIMDFloat64x2Lib::EntryInfo::Shuffle, 3, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::shuffleMix, &SIMDFloat64x2Lib::EntryInfo::ShuffleMix, 4, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::clamp, &SIMDFloat64x2Lib::EntryInfo::Clamp, 4, PropertyNone);
        library->AddFunctionToLibraryObject(float64x2Function, PropertyIds::select, &SIMDFloat64x2Lib::EntryInfo::Select, 4, PropertyNone);
        // end Float64x2

        // Int32x4
        JavascriptFunction* int32x4Function = library->AddFunctionToLibraryObject(simdObject, PropertyIds::int32x4, &SIMDInt32x4Lib::EntryInfo::Int32x4, 5, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::zero, &SIMDInt32x4Lib::EntryInfo::Zero, 1, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::splat, &SIMDInt32x4Lib::EntryInfo::Splat, 2, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::bool_, &SIMDInt32x4Lib::EntryInfo::Bool, 5, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::withX, &SIMDInt32x4Lib::EntryInfo::WithX, 3, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::withY, &SIMDInt32x4Lib::EntryInfo::WithY, 3, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::withZ, &SIMDInt32x4Lib::EntryInfo::WithZ, 3, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::withW, &SIMDInt32x4Lib::EntryInfo::WithW, 3, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::withFlagX, &SIMDInt32x4Lib::EntryInfo::WithFlagX, 3, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::withFlagY, &SIMDInt32x4Lib::EntryInfo::WithFlagY, 3, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::withFlagZ, &SIMDInt32x4Lib::EntryInfo::WithFlagZ, 3, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::withFlagW, &SIMDInt32x4Lib::EntryInfo::WithFlagW, 3, PropertyNone);
        // type conversions
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::fromFloat64x2, &SIMDInt32x4Lib::EntryInfo::FromFloat64x2, 2, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::fromFloat64x2Bits, &SIMDInt32x4Lib::EntryInfo::FromFloat64x2Bits, 2, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::fromFloat32x4, &SIMDInt32x4Lib::EntryInfo::FromFloat32x4, 2, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::fromFloat32x4Bits, &SIMDInt32x4Lib::EntryInfo::FromFloat32x4Bits, 2, PropertyNone);
        // binary ops
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::add, &SIMDInt32x4Lib::EntryInfo::Add, 3, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::sub, &SIMDInt32x4Lib::EntryInfo::Sub, 3, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::mul, &SIMDInt32x4Lib::EntryInfo::Mul, 3, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::and, &SIMDInt32x4Lib::EntryInfo::And, 3, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::or, &SIMDInt32x4Lib::EntryInfo::Or, 3, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::xor, &SIMDInt32x4Lib::EntryInfo::Xor, 3, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::neg, &SIMDInt32x4Lib::EntryInfo::Neg, 2, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::not, &SIMDInt32x4Lib::EntryInfo::Not, 2, PropertyNone);
        // compare ops
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::lessThan, &SIMDInt32x4Lib::EntryInfo::LessThan, 3, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::equal, &SIMDInt32x4Lib::EntryInfo::Equal, 3, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::greaterThan, &SIMDInt32x4Lib::EntryInfo::GreaterThan, 3, PropertyNone);
        // others
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::shuffle, &SIMDInt32x4Lib::EntryInfo::Shuffle, 3, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::shuffleMix, &SIMDInt32x4Lib::EntryInfo::ShuffleMix, 4, PropertyNone);
        // shift
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::shiftLeft, &SIMDInt32x4Lib::EntryInfo::ShiftLeft, 3, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::shiftRightLogical, &SIMDInt32x4Lib::EntryInfo::ShiftRightLogical, 3, PropertyNone);
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::shiftRightArithmetic, &SIMDInt32x4Lib::EntryInfo::ShiftRightArithmetic, 3, PropertyNone);
        // select
        library->AddFunctionToLibraryObject(int32x4Function, PropertyIds::select, &SIMDInt32x4Lib::EntryInfo::Select, 4, PropertyNone);
        // end Int32x4

        // SIMD shuffle masks
        {
#define MACRO(maskName, maskValue) library->AddMember(simdObject, PropertyIds:: ## maskName, JavascriptNumber::New(maskValue, scriptContext), PropertyNone);
#include "SIMDShuffleMasks.h"
        }
    }
#endif

    void JavascriptLibrary::InitializeReflectObject(DynamicObject* reflectObject, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(reflectObject, mode, 12);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterReflect
        // so that the update is in sync with profiler
        ScriptContext* scriptContext = reflectObject->GetScriptContext();
        JavascriptLibrary* library = reflectObject->GetLibrary();
        scriptContext->SetBuiltInLibraryFunction(JavascriptReflect::EntryInfo::DefineProperty.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(reflectObject, PropertyIds::defineProperty, &JavascriptReflect::EntryInfo::DefineProperty, 3));
        scriptContext->SetBuiltInLibraryFunction(JavascriptReflect::EntryInfo::DeleteProperty.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(reflectObject, PropertyIds::deleteProperty, &JavascriptReflect::EntryInfo::DeleteProperty, 2));
        scriptContext->SetBuiltInLibraryFunction(JavascriptReflect::EntryInfo::Enumerate.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(reflectObject, PropertyIds::enumerate, &JavascriptReflect::EntryInfo::Enumerate, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptReflect::EntryInfo::Get.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(reflectObject, PropertyIds::get, &JavascriptReflect::EntryInfo::Get, 3));
        scriptContext->SetBuiltInLibraryFunction(JavascriptReflect::EntryInfo::GetOwnPropertyDescriptor.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(reflectObject, PropertyIds::getOwnPropertyDescriptor, &JavascriptReflect::EntryInfo::GetOwnPropertyDescriptor, 2));
        scriptContext->SetBuiltInLibraryFunction(JavascriptReflect::EntryInfo::GetPrototypeOf.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(reflectObject, PropertyIds::getPrototypeOf, &JavascriptReflect::EntryInfo::GetPrototypeOf, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptReflect::EntryInfo::Has.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(reflectObject, PropertyIds::has, &JavascriptReflect::EntryInfo::Has, 2));
        scriptContext->SetBuiltInLibraryFunction(JavascriptReflect::EntryInfo::IsExtensible.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(reflectObject, PropertyIds::isExtensible, &JavascriptReflect::EntryInfo::IsExtensible, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptReflect::EntryInfo::OwnKeys.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(reflectObject, PropertyIds::ownKeys, &JavascriptReflect::EntryInfo::OwnKeys, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptReflect::EntryInfo::PreventExtensions.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(reflectObject, PropertyIds::preventExtensions, &JavascriptReflect::EntryInfo::PreventExtensions, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptReflect::EntryInfo::Set.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(reflectObject, PropertyIds::set, &JavascriptReflect::EntryInfo::Set, 4));
        scriptContext->SetBuiltInLibraryFunction(JavascriptReflect::EntryInfo::SetPrototypeOf.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(reflectObject, PropertyIds::setPrototypeOf, &JavascriptReflect::EntryInfo::SetPrototypeOf, 3));
        scriptContext->SetBuiltInLibraryFunction(JavascriptReflect::EntryInfo::Apply.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(reflectObject, PropertyIds::apply, &JavascriptReflect::EntryInfo::Apply, 3));
        scriptContext->SetBuiltInLibraryFunction(JavascriptReflect::EntryInfo::Construct.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(reflectObject, PropertyIds::construct, &JavascriptReflect::EntryInfo::Construct, 2));
    }

    void JavascriptLibrary::InitializeStaticValues()
    {
        constructorCacheDefaultInstance = &Js::ConstructorCache::DefaultInstance;
        absDoubleCst = Js::JavascriptNumber::AbsDoubleCst;
        uintConvertConst = Js::JavascriptNumber::UIntConvertConst;
        jnHelperMethods = IR::GetHelperMethods();

        defaultPropertyDescriptor.SetValue(undefinedValue);
        defaultPropertyDescriptor.SetWritable(false);
        defaultPropertyDescriptor.SetGetter(defaultAccessorFunction);
        defaultPropertyDescriptor.SetSetter(defaultAccessorFunction);
        defaultPropertyDescriptor.SetEnumerable(false);
        defaultPropertyDescriptor.SetConfigurable(false);

#if !defined(_M_X64_OR_ARM64)
        vtableAddresses[VTableValue::VtableJavascriptNumber] = VirtualTableInfo<Js::JavascriptNumber>::Address;
#endif
        vtableAddresses[VTableValue::VtableDynamicObject] = VirtualTableInfo<Js::DynamicObject>::Address;
        vtableAddresses[VTableValue::VtableInvalid] = Js::ScriptContextOptimizationOverrideInfo::InvalidVtable;
        vtableAddresses[VTableValue::VtablePropertyString] = VirtualTableInfo<Js::PropertyString>::Address;
        vtableAddresses[VTableValue::VtableJavascriptBoolean] = VirtualTableInfo<Js::JavascriptBoolean>::Address;
        vtableAddresses[VTableValue::VtableSmallDynamicObjectSnapshotEnumeratorWPCache] = VirtualTableInfo<Js::DynamicObjectSnapshotEnumeratorWPCache<Js::BigPropertyIndex,true,false>>::Address;
        vtableAddresses[VTableValue::VtableJavascriptArray] = VirtualTableInfo<Js::JavascriptArray>::Address;
        vtableAddresses[VTableValue::VtableInt8Array] = VirtualTableInfo<Js::Int8Array>::Address;
        vtableAddresses[VTableValue::VtableUint8Array] = VirtualTableInfo<Js::Uint8Array>::Address;
        vtableAddresses[VTableValue::VtableUint8ClampedArray] = VirtualTableInfo<Js::Uint8ClampedArray>::Address;
        vtableAddresses[VTableValue::VtableInt16Array] = VirtualTableInfo<Js::Int16Array>::Address;
        vtableAddresses[VTableValue::VtableUint16Array] = VirtualTableInfo<Js::Uint16Array>::Address;
        vtableAddresses[VTableValue::VtableInt32Array] = VirtualTableInfo<Js::Int32Array>::Address;
        vtableAddresses[VTableValue::VtableUint32Array] = VirtualTableInfo<Js::Uint32Array>::Address;
        vtableAddresses[VTableValue::VtableFloat32Array] = VirtualTableInfo<Js::Float32Array>::Address;
        vtableAddresses[VTableValue::VtableFloat64Array] = VirtualTableInfo<Js::Float64Array>::Address;
        vtableAddresses[VTableValue::VtableJavascriptPixelArray] = VirtualTableInfo<Js::JavascriptPixelArray>::Address;
        vtableAddresses[VTableValue::VtableInt64Array] = VirtualTableInfo<Js::Int64Array>::Address;
        vtableAddresses[VTableValue::VtableUint64Array] = VirtualTableInfo<Js::Uint64Array>::Address;

        vtableAddresses[VTableValue::VtableInt8VirtualArray] = VirtualTableInfo<Js::Int8VirtualArray>::Address;
        vtableAddresses[VTableValue::VtableUint8VirtualArray] = VirtualTableInfo<Js::Uint8VirtualArray>::Address;
        vtableAddresses[VTableValue::VtableUint8ClampedVirtualArray] = VirtualTableInfo<Js::Uint8ClampedVirtualArray>::Address;
        vtableAddresses[VTableValue::VtableInt16VirtualArray] = VirtualTableInfo<Js::Int16VirtualArray>::Address;
        vtableAddresses[VTableValue::VtableUint16VirtualArray] = VirtualTableInfo<Js::Uint16VirtualArray>::Address;
        vtableAddresses[VTableValue::VtableInt32VirtualArray] = VirtualTableInfo<Js::Int32VirtualArray>::Address;
        vtableAddresses[VTableValue::VtableUint32VirtualArray] = VirtualTableInfo<Js::Uint32VirtualArray>::Address;
        vtableAddresses[VTableValue::VtableFloat32VirtualArray] = VirtualTableInfo<Js::Float32VirtualArray>::Address;
        vtableAddresses[VTableValue::VtableFloat64VirtualArray] = VirtualTableInfo<Js::Float64VirtualArray>::Address;

        vtableAddresses[VTableValue::VtableBoolArray] = VirtualTableInfo<Js::BoolArray>::Address;
        vtableAddresses[VTableValue::VtableCharArray] = VirtualTableInfo<Js::CharArray>::Address;
        vtableAddresses[VTableValue::VtableNativeIntArray] = VirtualTableInfo<Js::JavascriptNativeIntArray>::Address;
        vtableAddresses[VTableValue::VtableCopyOnAccessNativeIntArray] = VirtualTableInfo<Js::JavascriptCopyOnAccessNativeIntArray>::Address;
        vtableAddresses[VTableValue::VtableNativeFloatArray] = VirtualTableInfo<Js::JavascriptNativeFloatArray>::Address;
        vtableAddresses[VTableValue::VtableJavascriptNativeIntArray] = VirtualTableInfo<Js::JavascriptNativeIntArray>::Address;
        vtableAddresses[VTableValue::VtableJavascriptRegExp] = VirtualTableInfo<Js::JavascriptRegExp>::Address;
        vtableAddresses[VTableValue::VtableStackScriptFunction] = VirtualTableInfo<Js::StackScriptFunction>::Address;
        vtableAddresses[VTableValue::VtableConcatStringMulti] = VirtualTableInfo<Js::ConcatStringMulti>::Address;
        vtableAddresses[VTableValue::VtableCompoundString] = VirtualTableInfo<Js::CompoundString>::Address;
    }

    //
    // Ensure library ready if started hybrid debugging. Call this to support hybrid debugging when engine starts debug mode.
    //
    HRESULT JavascriptLibrary::EnsureReadyIfHybridDebugging(bool isScriptEngineReady /*= true*/)
    {
        HRESULT hr = S_OK;

        this->isHybridDebugging = Js::Configuration::Global.IsHybridDebugging();

        // If just started hybrid debugging, ensure library objects ready (but only if we can run script now)
        if (this->isHybridDebugging && !this->isLibraryReadyForHybridDebugging && isScriptEngineReady)
        {
            BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT_NESTED(scriptContext, /*doCleanup*/false)
            {
                EnsureLibraryReadyForHybridDebugging();
            }
            END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
        }

        return hr;
    }

    //
    // If under hybrid debugging, ensure one object ready (not using deferred type handler).
    //
    DynamicObject* JavascriptLibrary::EnsureReadyIfHybridDebugging(DynamicObject* obj)
    {
        if (IsHybridDebugging())
        {
            obj->EnsureObjectReady();
        }
        return obj;
    }

    //
    // When starting hybrid debugging, ensure all library objects ready (not using deferred type handler).
    //
    void JavascriptLibrary::EnsureLibraryReadyForHybridDebugging()
    {
        Assert(IsHybridDebugging() && !isLibraryReadyForHybridDebugging);

        //
        // WARNING: List all library objects that use DeferredTypeHandler!
        //
        DynamicObject* objects[] =
        {
            this->objectPrototype,
            this->arrayPrototype,
            this->arrayBufferPrototype,
            this->dataViewPrototype,
            this->typedArrayPrototype,
            this->Int8ArrayPrototype,
            this->Uint8ArrayPrototype,
            this->Uint8ClampedArrayPrototype,
            this->Int16ArrayPrototype,
            this->Uint16ArrayPrototype,
            this->Int32ArrayPrototype,
            this->Uint32ArrayPrototype,
            this->Float32ArrayPrototype,
            this->Float64ArrayPrototype,
            this->Int64ArrayPrototype,
            this->Uint64ArrayPrototype,
            this->BoolArrayPrototype,
            this->CharArrayPrototype,
            this->pixelArrayPrototype,
            this->booleanPrototype,
            this->datePrototype,
            this->functionPrototype,
            this->numberPrototype,
            this->stringPrototype,
            this->mapPrototype,
            this->setPrototype,
            this->weakMapPrototype,
            this->weakSetPrototype,
            this->regexPrototype,
            this->symbolPrototype,
            this->arrayIteratorPrototype,
            this->promisePrototype,
            this->javascriptEnumeratorIteratorPrototype,
            this->generatorFunctionPrototype,
            this->generatorPrototype,
            this->errorPrototype,
            this->evalErrorPrototype,
            this->rangeErrorPrototype,
            this->referenceErrorPrototype,
            this->syntaxErrorPrototype,
            this->typeErrorPrototype,
            this->uriErrorPrototype,
            this->winrtErrorPrototype,

            this->objectConstructor,
            this->arrayConstructor,
            this->booleanConstructor,
            this->dateConstructor,
            this->functionConstructor,
            this->debugObject,
            this->mathObject,
            this->numberConstructor,
            this->stringConstructor,
            this->regexConstructor,
            this->arrayBufferConstructor,
            this->dataViewConstructor,
            this->typedArrayConstructor,
            this->Int8ArrayConstructor,
            this->Uint8ArrayConstructor,
            this->Uint8ClampedArrayConstructor,
            this->Int16ArrayConstructor,
            this->Uint16ArrayConstructor,
            this->Int32ArrayConstructor,
            this->Uint32ArrayConstructor,
            this->Float32ArrayConstructor,
            this->Float64ArrayConstructor,
            this->pixelArrayConstructor,
            this->JSONObject,
            this->IntlObject,  //TODO: InitializeIntlObject executes script, needs from CallRootFunction
            this->mapConstructor,
            this->setConstructor,
            this->weakMapConstructor,
            this->weakSetConstructor,
            this->symbolConstructor,
            this->promiseConstructor,
            this->proxyConstructor,
            this->generatorFunctionConstructor,
            this->errorConstructor,
            this->evalErrorConstructor,
            this->rangeErrorConstructor,
            this->referenceErrorConstructor,
            this->syntaxErrorConstructor,
            this->typeErrorConstructor,
            this->uriErrorConstructor,
            this->winrtErrorConstructor
        };

        for (int i = 0; i < _countof(objects); i++)
        {
            if (objects[i]) // may be NULL for compat mode
            {
                objects[i]->EnsureObjectReady();
            }
        }

        isLibraryReadyForHybridDebugging = true; // Done!
    }

#ifdef ENABLE_NATIVE_CODEGEN
    // Seems that this is for all built-ins that can be inlined (not only Math.*).
    // static
    /*TODO: This function is only used in float preferencing scenarios. Should remove it once we do away with float preferencing.
        Moreover, cases like,
            case PropertyIds::concat:
            case PropertyIds::indexOf:
            case PropertyIds::lastIndexOf:
            case PropertyIds::slice:

            which have same names for Array and String cannot be resolved just by the property id
    */
    BuiltinFunction JavascriptLibrary::GetBuiltinFunctionForPropId(PropertyId id)
    {
        switch (id)
        {
        case PropertyIds::abs:
            return BuiltinFunction::Math_Abs;

        case PropertyIds::acos:
            return BuiltinFunction::Math_Acos;

        case PropertyIds::asin:
            return BuiltinFunction::Math_Asin;

        case PropertyIds::atan:
            return BuiltinFunction::Math_Atan;

#if 0
        // For now, avoid mapping Math.atan2 to a direct CRT call, as the
        // fast CRT helper doesn't handle denormals correctly.
        case PropertyIds::atan2:
            return BuiltinFunction::Atan2;
#endif
        case PropertyIds::cos:
            return BuiltinFunction::Math_Cos;

        case PropertyIds::exp:
            return BuiltinFunction::Math_Exp;

        case PropertyIds::log:
            return BuiltinFunction::Math_Log;

        case PropertyIds::pow:
            return BuiltinFunction::Math_Pow;

        case PropertyIds::random:
            return BuiltinFunction::Math_Random;

        case PropertyIds::sin:
            return BuiltinFunction::Math_Sin;

        case PropertyIds::sqrt:
            return BuiltinFunction::Math_Sqrt;

        case PropertyIds::tan:
            return BuiltinFunction::Math_Tan;

        case PropertyIds::floor:
            return BuiltinFunction::Math_Floor;

        case PropertyIds::ceil:
            return BuiltinFunction::Math_Ceil;

        case PropertyIds::round:
            return BuiltinFunction::Math_Round;

         case PropertyIds::max:
            return BuiltinFunction::Math_Max;

        case PropertyIds::min:
            return BuiltinFunction::Math_Min;

        case PropertyIds::imul:
            return BuiltinFunction::Math_Imul;

        case PropertyIds::fround:
            return BuiltinFunction::Math_Fround;

        case PropertyIds::codePointAt:
            return BuiltinFunction::String_CodePointAt;

        case PropertyIds::push:
            return BuiltinFunction::Array_Push;

        case PropertyIds::concat:
            return BuiltinFunction::Array_Concat;

        case PropertyIds::indexOf:
            return BuiltinFunction::Array_IndexOf;

        case PropertyIds::isArray:
            return BuiltinFunction::Array_IsArray;

        case PropertyIds::join:
            return BuiltinFunction::Array_Join;

        case PropertyIds::lastIndexOf:
            return BuiltinFunction::Array_LastIndexOf;

        case PropertyIds::reverse:
            return BuiltinFunction::Array_Reverse;

        case PropertyIds::shift:
            return BuiltinFunction::Array_Shift;

        case PropertyIds::slice:
            return BuiltinFunction::Array_Slice;

        case PropertyIds::splice:
            return BuiltinFunction::Array_Splice;

        case PropertyIds::unshift:
            return BuiltinFunction::Array_Unshift;

        case PropertyIds::apply:
            return BuiltinFunction::Function_Apply;

        /*case PropertyIds::concat:
            return BuiltinFunction::String_Concat;*/

        case PropertyIds::charAt:
            return BuiltinFunction::String_CharAt;

        case PropertyIds::charCodeAt:
            return BuiltinFunction::String_CharCodeAt;

        case PropertyIds::fromCharCode:
            return BuiltinFunction::String_FromCharCode;

        case PropertyIds::fromCodePoint:
                return BuiltinFunction::String_FromCodePoint;

        /*case PropertyIds::indexOf:
            return BuiltinFunction::String_IndexOf;

        case PropertyIds::lastIndexOf:
            return BuiltinFunction::String_LastIndexOf;*/

        case PropertyIds::link:
            return BuiltinFunction::String_Link;

        case PropertyIds::localeCompare:
            return BuiltinFunction::String_LocaleCompare;

        case PropertyIds::match:
            return BuiltinFunction::String_Match;

        case PropertyIds::replace:
            return BuiltinFunction::String_Replace;

        case PropertyIds::search:
            return BuiltinFunction::String_Search;

        /*case PropertyIds::slice:
            return BuiltinFunction::String_Slice;*/

        case PropertyIds::split:
            return BuiltinFunction::String_Split;

        case PropertyIds::substr:
            return BuiltinFunction::String_Substr;

        case PropertyIds::substring:
            return BuiltinFunction::String_Substring;

        case PropertyIds::toLocaleLowerCase:
            return BuiltinFunction::String_ToLocaleLowerCase;

        case PropertyIds::toLocaleUpperCase:
            return BuiltinFunction::String_ToLocaleUpperCase;

        case PropertyIds::toLowerCase:
            return BuiltinFunction::String_ToLowerCase;

        case PropertyIds::toUpperCase:
            return BuiltinFunction::String_ToUpperCase;

        case PropertyIds::trim:
            return BuiltinFunction::String_Trim;

        case PropertyIds::trimLeft:
            return BuiltinFunction::String_TrimLeft;

        case PropertyIds::trimRight:
            return BuiltinFunction::String_TrimRight;

        case PropertyIds::exec:
            return BuiltinFunction::RegExp_Exec;

        default:
            return BuiltinFunction::None;
        }
    }

    // Returns built-in enum value for given funcInfo. Ultimately this will work for all built-ins (not only Math.*).
    // Used by inliner.
    //static
    BuiltinFunction JavascriptLibrary::GetBuiltInForFuncInfo(FunctionInfo* funcInfo)
    {
        Assert(funcInfo);

        if (funcInfo == &Math::EntryInfo::Abs)
        {
            return BuiltinFunction::Math_Abs;
        }
        else if (funcInfo == &Math::EntryInfo::Acos)
        {
            return BuiltinFunction::Math_Acos;
        }
        else if (funcInfo == &Math::EntryInfo::Asin)
        {
            return BuiltinFunction::Math_Asin;
        }
        else if (funcInfo == &Math::EntryInfo::Atan)
        {
            return BuiltinFunction::Math_Atan;
        }
        else if (funcInfo == &Math::EntryInfo::Atan2)
        {
            // Even that atan2 can't currently be float-preferenced, it's fine to support it here
            // as we check later for JavascriptLibrary::CanFloatPreferenceFunc.
            return BuiltinFunction::Math_Atan2;
        }
        else if (funcInfo == &Math::EntryInfo::Cos)
        {
            return BuiltinFunction::Math_Cos;
        }
        else if (funcInfo == &Math::EntryInfo::Exp)
        {
            return BuiltinFunction::Math_Exp;
        }
        else if (funcInfo == &Math::EntryInfo::Log)
        {
            return BuiltinFunction::Math_Log;
        }
        else if (funcInfo == &Math::EntryInfo::Pow)
        {
            return BuiltinFunction::Math_Pow;
        }
        else if (funcInfo == &Math::EntryInfo::Random)
        {
            return BuiltinFunction::Math_Random;
        }
        else if (funcInfo == &Math::EntryInfo::Sin)
        {
            return BuiltinFunction::Math_Sin;
        }
        else if (funcInfo == &Math::EntryInfo::Sqrt)
        {
            return BuiltinFunction::Math_Sqrt;
        }
        else if (funcInfo == &Math::EntryInfo::Tan)
        {
            return BuiltinFunction::Math_Tan;
        }
        else if (funcInfo == &Math::EntryInfo::Floor)
        {
            return BuiltinFunction::Math_Floor;
        }
        else if (funcInfo == &Math::EntryInfo::Ceil)
        {
            return BuiltinFunction::Math_Ceil;
        }
        else if (funcInfo == &Math::EntryInfo::Round)
        {
            return BuiltinFunction::Math_Round;
        }
        else if (funcInfo == &Math::EntryInfo::Fround)
        {
            return BuiltinFunction::Math_Fround;
        }
        else if (funcInfo == &JavascriptArray::EntryInfo::Concat)
        {
            return BuiltinFunction::Array_Concat;
        }
        else if (funcInfo == &JavascriptArray::EntryInfo::IndexOf)
        {
            return BuiltinFunction::Array_IndexOf;
        }
        else if (funcInfo == &JavascriptArray::EntryInfo::IsArray)
        {
            return BuiltinFunction::Array_IsArray;
        }
        else if (funcInfo == &JavascriptArray::EntryInfo::Join)
        {
            return BuiltinFunction::Array_Join;
        }
        else if (funcInfo == &JavascriptArray::EntryInfo::LastIndexOf)
        {
            return BuiltinFunction::Array_LastIndexOf;
        }
        else if (funcInfo == &JavascriptArray::EntryInfo::Push)
        {
            return BuiltinFunction::Array_Push;
        }
        else if (funcInfo == &JavascriptArray::EntryInfo::Reverse)
        {
            return BuiltinFunction::Array_Reverse;
        }
        else if (funcInfo == &JavascriptArray::EntryInfo::Shift)
        {
            return BuiltinFunction::Array_Shift;
        }
        else if (funcInfo == &JavascriptArray::EntryInfo::Slice)
        {
            return BuiltinFunction::Array_Slice;
        }
        else if (funcInfo == &JavascriptArray::EntryInfo::Splice)
        {
            return BuiltinFunction::Array_Splice;
        }
        else if (funcInfo == &JavascriptArray::EntryInfo::Unshift)
        {
            return BuiltinFunction::Array_Unshift;
        }

        else if (funcInfo == &JavascriptString::EntryInfo::Concat)
        {
            return BuiltinFunction::String_Concat;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::FromCharCode)
        {
            return BuiltinFunction::String_FromCharCode;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::FromCodePoint)
        {
            return BuiltinFunction::String_FromCodePoint;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::IndexOf)
        {
            return BuiltinFunction::String_IndexOf;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::LastIndexOf)
        {
            return BuiltinFunction::String_LastIndexOf;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::Link)
        {
            return BuiltinFunction::String_Link;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::LocaleCompare)
        {
            return BuiltinFunction::String_LocaleCompare;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::Match)
        {
            return BuiltinFunction::String_Match;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::Search)
        {
            return BuiltinFunction::String_Search;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::Slice)
        {
            return BuiltinFunction::String_Slice;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::Split)
        {
            return BuiltinFunction::String_Split;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::Substr)
        {
            return BuiltinFunction::String_Substr;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::Substring)
        {
            return BuiltinFunction::String_Substring;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::ToLocaleLowerCase)
        {
            return BuiltinFunction::String_ToLocaleLowerCase;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::ToLocaleUpperCase)
        {
            return BuiltinFunction::String_ToLocaleUpperCase;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::ToLowerCase)
        {
            return BuiltinFunction::String_ToLowerCase;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::ToUpperCase)
        {
            return BuiltinFunction::String_ToUpperCase;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::Trim)
        {
            return BuiltinFunction::String_Trim;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::TrimLeft)
        {
            return BuiltinFunction::String_TrimLeft;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::TrimRight)
        {
            return BuiltinFunction::String_TrimRight;
        }
        else if (funcInfo == &Math::EntryInfo::Ceil)
        {
            return BuiltinFunction::Math_Ceil;
        }
        else if (funcInfo == &Math::EntryInfo::Floor)
        {
            return BuiltinFunction::Math_Floor;
        }
        else if (funcInfo == &Math::EntryInfo::Max)
        {
            return BuiltinFunction::Math_Max;
        }
        else if (funcInfo == &Math::EntryInfo::Min)
        {
            return BuiltinFunction::Math_Min;
        }
        else if (funcInfo == &Math::EntryInfo::Imul)
        {
            return BuiltinFunction::Math_Imul;
        }
        else if (funcInfo == &Math::EntryInfo::Round)
        {
            return BuiltinFunction::Math_Round;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::CharAt)
        {
            return BuiltinFunction::String_CharAt;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::CharCodeAt)
        {
            return BuiltinFunction::String_CharCodeAt;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::CodePointAt)
        {
            return BuiltinFunction::String_CodePointAt;
        }
        else if (funcInfo == &JavascriptArray::EntryInfo::Push)
        {
            return BuiltinFunction::Array_Push;
        }
        else if (funcInfo == &JavascriptArray::EntryInfo::Pop)
        {
            return BuiltinFunction::Array_Pop;
        }
        else if (funcInfo == &JavascriptString::EntryInfo::Replace)
        {
            return BuiltinFunction::String_Replace;
        }
        else if (funcInfo == &JavascriptFunction::EntryInfo::Apply)
        {
            return BuiltinFunction::Function_Apply;
        }
        else if (funcInfo == &JavascriptFunction::EntryInfo::Call)
        {
            return BuiltinFunction::Function_Call;
        }
        else if (funcInfo == &GlobalObject::EntryInfo::ParseInt)
        {
            return BuiltinFunction::GlobalObject_ParseInt;
        }
        else if (funcInfo == &JavascriptRegExp::EntryInfo::Exec)
        {
            return BuiltinFunction::RegExp_Exec;
        }
        else if (funcInfo == &Math::EntryInfo::Clz32)
        {
            return BuiltinFunction::Math_Clz32;
        }
        /* TODO: Implement inlining of ES6 Math API extensions
        else if (funcInfo == &Math::EntryInfo::Log10)
        {
            return BuiltinFunction::Math_Log10;
        }
        else if (funcInfo == &Math::EntryInfo::Log2)
        {
            return BuiltinFunction::Math_Log2;
        }
        else if (funcInfo == &Math::EntryInfo::Log1p)
        {
            return BuiltinFunction::Math_Log1p;
        }
        else if (funcInfo == &Math::EntryInfo::Expm1)
        {
            return BuiltinFunction::Math_Expm1;
        }
        else if (funcInfo == &Math::EntryInfo::Cosh)
        {
            return BuiltinFunction::Math_Cosh;
        }
        else if (funcInfo == &Math::EntryInfo::Sinh)
        {
            return BuiltinFunction::Math_Sinh;
        }
        else if (funcInfo == &Math::EntryInfo::Tanh)
        {
            return BuiltinFunction::Math_Tanh;
        }
        else if (funcInfo == &Math::EntryInfo::Acosh)
        {
            return BuiltinFunction::Math_Acosh;
        }
        else if (funcInfo == &Math::EntryInfo::Asinh)
        {
            return BuiltinFunction::Math_Asinh;
        }
        else if (funcInfo == &Math::EntryInfo::Atanh)
        {
            return BuiltinFunction::Math_Atanh;
        }
        else if (funcInfo == &Math::EntryInfo::Hypot)
        {
            return BuiltinFunction::Math_Hypot;
        }
        else if (funcInfo == &Math::EntryInfo::Trunc)
        {
            return BuiltinFunction::Math_Trunc;
        }
        else if (funcInfo == &Math::EntryInfo::Sign)
        {
            return BuiltinFunction::Math_Sign;
        }
        else if (funcInfo == &Math::EntryInfo::Cbrt)
        {
            return BuiltinFunction::Math_Cbrt;
        }
        */

        // TODO: add string.prototype.concat.

        return BuiltinFunction::None;
    }

    // Returns true if the function's return type is always float.
    BOOL JavascriptLibrary::IsFltFunc(BuiltinFunction index)
    {
        // Note: MathFuncion is one of built-ins.
        if (!JavascriptLibrary::CanFloatPreferenceFunc(index))
        {
            return FALSE;
        }

        Js::BuiltInFlags builtInFlags = JavascriptLibrary::GetFlagsForBuiltIn(index);
        Js::BuiltInArgSpecizationType dstType = Js::JavascriptLibrary::GetBuiltInArgType(builtInFlags, Js::BuiltInArgShift::BIAS_Dst);
        bool isFloatFunc = dstType == Js::BuiltInArgSpecizationType::BIAST_Float;
        return isFloatFunc;
    }

    size_t JavascriptLibrary::LibraryFunctionArgC[] = {
#define LIBRARY_FUNCTION(obj, name, argc, flags) argc,
#include "LibraryFunction.h"
#undef LIBRARY_FUNCTION
        0
    };

#if ENABLE_DEBUG_CONFIG_OPTIONS
    wchar_t* JavascriptLibrary::LibraryFunctionName[] = {
#define LIBRARY_FUNCTION(obj, name, argc, flags) L#obj L"." L#name,
#include "LibraryFunction.h"
#undef LIBRARY_FUNCTION
        0
    };
#endif

    int JavascriptLibrary::LibraryFunctionFlags[] = {
#define LIBRARY_FUNCTION(obj, name, argc, flags) flags,
#include "LibraryFunction.h"
#undef LIBRARY_FUNCTION
        BIF_None
    };

    bool JavascriptLibrary::IsFloatFunctionCallsite(BuiltinFunction index, size_t argc)
    {
        if (IsFltFunc(index))
        {
            Assert(index < BuiltinFunction::Count);
            if (argc)
            {
                return JavascriptLibrary::LibraryFunctionArgC[index] <= (argc - 1 /* this */);
            }
        }

        return false;
    }

    // (TODO: confirm) Returns true if the function's return type can be float.
    // For abs, min, max -- return can be int or float, but still return true from here.
    BOOL JavascriptLibrary::CanFloatPreferenceFunc(BuiltinFunction index)
    {
        // Shortcut the common case:
        if (index == BuiltinFunction::None)
        {
            return FALSE;
        }

        switch (index)
        {
        case BuiltinFunction::Math_Abs:
        case BuiltinFunction::Math_Acos:
        case BuiltinFunction::Math_Asin:
        case BuiltinFunction::Math_Atan:
#if 0
        // For now, avoid mapping Math.atan2 to a direct CRT call, as the
        // fast CRT helper doesn't handle denormals correctly.
        case BuiltinFunction::Atan2:
#endif
        case BuiltinFunction::Math_Cos:
        case BuiltinFunction::Math_Exp:
        case BuiltinFunction::Math_Log:
        case BuiltinFunction::Math_Min:
        case BuiltinFunction::Math_Max:
        case BuiltinFunction::Math_Pow:
        case BuiltinFunction::Math_Random:
        case BuiltinFunction::Math_Sin:
        case BuiltinFunction::Math_Sqrt:
        case BuiltinFunction::Math_Tan:
        case BuiltinFunction::Math_Fround:
#if 0
        // The ones below will be enabled in IE11.
        case PropertyIds::max:
            return BuiltinFunction::Max;

        case PropertyIds::min:
            return BuiltinFunction::Min;
#endif
            return TRUE;
        }
        return FALSE;
    }

    bool JavascriptLibrary::IsFltBuiltInConst(PropertyId propertyId)
    {
        switch (propertyId)
        {
        case Js::PropertyIds::E:
        case Js::PropertyIds::LN10:
        case Js::PropertyIds::LN2:
        case Js::PropertyIds::LOG2E:
        case Js::PropertyIds::LOG10E:
        case Js::PropertyIds::PI:
        case Js::PropertyIds::SQRT1_2:
        case Js::PropertyIds::SQRT2:
            return true;
        }
        return false;
    }
#endif

    void JavascriptLibrary::TypeAndPrototypesAreEnsuredToHaveOnlyWritableDataProperties(Type *const type)
    {
        Assert(type);
        Assert(type->GetScriptContext() == scriptContext);
        Assert(type->AreThisAndPrototypesEnsuredToHaveOnlyWritableDataProperties());
        Assert(!scriptContext->IsClosed());

        if(typesEnsuredToHaveOnlyWritableDataPropertiesInItAndPrototypeChain->Count() == 0)
        {
            scriptContext->RegisterPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesScriptContext();
        }
        typesEnsuredToHaveOnlyWritableDataPropertiesInItAndPrototypeChain->Add(type);
    }

    void JavascriptLibrary::NoPrototypeChainsAreEnsuredToHaveOnlyWritableDataProperties()
    {
        for(int i = 0; i < typesEnsuredToHaveOnlyWritableDataPropertiesInItAndPrototypeChain->Count(); ++i)
        {
            typesEnsuredToHaveOnlyWritableDataPropertiesInItAndPrototypeChain
                ->Item(i)
                ->SetAreThisAndPrototypesEnsuredToHaveOnlyWritableDataProperties(false);
        }
        typesEnsuredToHaveOnlyWritableDataPropertiesInItAndPrototypeChain->ClearAndZero();
    }

    void JavascriptLibrary::InitializeNumberConstructor(DynamicObject* numberConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(numberConstructor, mode, 17);

        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterNumber
        // so that the update is in sync with profiler
        ScriptContext* scriptContext = numberConstructor->GetScriptContext();
        JavascriptLibrary* library = numberConstructor->GetLibrary();
        library->AddMember(numberConstructor, PropertyIds::length,            TaggedInt::ToVarUnchecked(1), PropertyNone);
        library->AddMember(numberConstructor, PropertyIds::prototype,         library->numberPrototype,     PropertyNone);
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(numberConstructor, PropertyIds::name, scriptContext->GetPropertyString(PropertyIds::Number), PropertyConfigurable);
        }
        library->AddMember(numberConstructor, PropertyIds::MAX_VALUE,         library->maxValue,            PropertyNone);
        library->AddMember(numberConstructor, PropertyIds::MIN_VALUE,         library->minValue,            PropertyNone);
        library->AddMember(numberConstructor, PropertyIds::NaN,               library->nan,                 PropertyNone);
        library->AddMember(numberConstructor, PropertyIds::NEGATIVE_INFINITY, library->negativeInfinite,    PropertyNone);
        library->AddMember(numberConstructor, PropertyIds::POSITIVE_INFINITY, library->positiveInfinite,    PropertyNone);

        if (scriptContext->GetConfig()->IsES6NumberExtensionsEnabled())
        {
#ifdef DBG
            double epsilon = 0.0;
            for (double next = 1.0; next + 1.0 != 1.0; next = next / 2.0)
            {
                epsilon = next;
            }
            Assert(epsilon == Math::EPSILON);
#endif
            library->AddMember(numberConstructor, PropertyIds::EPSILON,     JavascriptNumber::New(Math::EPSILON,     scriptContext), PropertyNone);
            library->AddMember(numberConstructor, PropertyIds::MAX_SAFE_INTEGER, JavascriptNumber::New(Math::MAX_SAFE_INTEGER, scriptContext), PropertyNone);
            library->AddMember(numberConstructor, PropertyIds::MIN_SAFE_INTEGER, JavascriptNumber::New(Math::MIN_SAFE_INTEGER, scriptContext), PropertyNone);

            AssertMsg(library->parseIntFunctionObject != nullptr, "Where is parseIntFunctionObject? Should have been initialized with Global object initialization");
            AssertMsg(library->parseFloatFunctionObject != nullptr, "Where is parseIntFunctionObject? Should have been initialized with Global object initialization");
            library->AddMember(numberConstructor, PropertyIds::parseInt, library->parseIntFunctionObject);
            library->AddMember(numberConstructor, PropertyIds::parseFloat, library->parseFloatFunctionObject);
            library->AddFunctionToLibraryObject(numberConstructor, PropertyIds::isNaN, &JavascriptNumber::EntryInfo::IsNaN, 1);
            library->AddFunctionToLibraryObject(numberConstructor, PropertyIds::isFinite, &JavascriptNumber::EntryInfo::IsFinite, 1);
            library->AddFunctionToLibraryObject(numberConstructor, PropertyIds::isInteger, &JavascriptNumber::EntryInfo::IsInteger, 1);
            library->AddFunctionToLibraryObject(numberConstructor, PropertyIds::isSafeInteger, &JavascriptNumber::EntryInfo::IsSafeInteger, 1);
        }

        numberConstructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeNumberPrototype(DynamicObject* numberPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(numberPrototype, mode, 8);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterNumber
        // so that the update is in sync with profiler
        ScriptContext* scriptContext = numberPrototype->GetScriptContext();
        JavascriptLibrary* library = numberPrototype->GetLibrary();
        library->AddMember(numberPrototype, PropertyIds::constructor, library->numberConstructor);
        scriptContext->SetBuiltInLibraryFunction(JavascriptNumber::EntryInfo::ToExponential.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(numberPrototype, PropertyIds::toExponential, &JavascriptNumber::EntryInfo::ToExponential, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptNumber::EntryInfo::ToFixed.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(numberPrototype, PropertyIds::toFixed, &JavascriptNumber::EntryInfo::ToFixed, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptNumber::EntryInfo::ToPrecision.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(numberPrototype, PropertyIds::toPrecision, &JavascriptNumber::EntryInfo::ToPrecision, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptNumber::EntryInfo::ToLocaleString.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(numberPrototype, PropertyIds::toLocaleString, &JavascriptNumber::EntryInfo::ToLocaleString, 0));
        scriptContext->SetBuiltInLibraryFunction(JavascriptNumber::EntryInfo::ToString.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(numberPrototype, PropertyIds::toString, &JavascriptNumber::EntryInfo::ToString, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptNumber::EntryInfo::ValueOf.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(numberPrototype, PropertyIds::valueOf, &JavascriptNumber::EntryInfo::ValueOf, 0));

        numberPrototype->SetHasNoEnumerableProperties(true);
    }


    void JavascriptLibrary::InitializeObjectConstructor(DynamicObject* objectConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterObject
        // so that the update is in sync with profiler
        JavascriptLibrary* library = objectConstructor->GetLibrary();
        ScriptContext* scriptContext = objectConstructor->GetScriptContext();
        int propertyCount = 17;
        if (scriptContext->GetConfig()->IsES6SymbolEnabled())
        {
            propertyCount += 1;
        }
        if (scriptContext->GetConfig()->IsES6ObjectExtensionsEnabled())
        {
            propertyCount += 2;
        }

        typeHandler->Convert(objectConstructor, mode, propertyCount);

        library->AddMember(objectConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(1), PropertyNone);
        library->AddMember(objectConstructor, PropertyIds::prototype, library->objectPrototype, PropertyNone);
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(objectConstructor, PropertyIds::name, scriptContext->GetPropertyString(PropertyIds::Object), PropertyConfigurable);
        }
        //Supported in IE8 standards and above
        scriptContext->SetBuiltInLibraryFunction(JavascriptObject::EntryInfo::DefineProperty.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(objectConstructor, PropertyIds::defineProperty, &JavascriptObject::EntryInfo::DefineProperty, 3));
        scriptContext->SetBuiltInLibraryFunction(JavascriptObject::EntryInfo::GetOwnPropertyDescriptor.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(objectConstructor, PropertyIds::getOwnPropertyDescriptor, &JavascriptObject::EntryInfo::GetOwnPropertyDescriptor, 2));
        scriptContext->SetBuiltInLibraryFunction(JavascriptObject::EntryInfo::DefineProperties.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(objectConstructor, PropertyIds::defineProperties, &JavascriptObject::EntryInfo::DefineProperties, 2));
        library->AddFunctionToLibraryObject(objectConstructor, PropertyIds::create, &JavascriptObject::EntryInfo::Create, 2);
        scriptContext->SetBuiltInLibraryFunction(JavascriptObject::EntryInfo::Seal.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(objectConstructor, PropertyIds::seal, &JavascriptObject::EntryInfo::Seal, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptObject::EntryInfo::Freeze.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(objectConstructor, PropertyIds::freeze, &JavascriptObject::EntryInfo::Freeze, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptObject::EntryInfo::PreventExtensions.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(objectConstructor, PropertyIds::preventExtensions, &JavascriptObject::EntryInfo::PreventExtensions, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptObject::EntryInfo::IsSealed.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(objectConstructor, PropertyIds::isSealed, &JavascriptObject::EntryInfo::IsSealed, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptObject::EntryInfo::IsFrozen.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(objectConstructor, PropertyIds::isFrozen, &JavascriptObject::EntryInfo::IsFrozen, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptObject::EntryInfo::IsExtensible.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(objectConstructor, PropertyIds::isExtensible, &JavascriptObject::EntryInfo::IsExtensible, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptObject::EntryInfo::GetPrototypeOf.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(objectConstructor, PropertyIds::getPrototypeOf, &JavascriptObject::EntryInfo::GetPrototypeOf, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptObject::EntryInfo::Keys.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(objectConstructor, PropertyIds::keys, &JavascriptObject::EntryInfo::Keys, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptObject::EntryInfo::GetOwnPropertyNames.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(objectConstructor, PropertyIds::getOwnPropertyNames, &JavascriptObject::EntryInfo::GetOwnPropertyNames, 1));
        scriptContext->SetBuiltInLibraryFunction(JavascriptObject::EntryInfo::SetPrototypeOf.GetOriginalEntryPoint(),
            library->AddFunctionToLibraryObject(objectConstructor, PropertyIds::setPrototypeOf, &JavascriptObject::EntryInfo::SetPrototypeOf, 2));
        if (scriptContext->GetConfig()->IsES6SymbolEnabled())
        {
            scriptContext->SetBuiltInLibraryFunction(JavascriptObject::EntryInfo::GetOwnPropertySymbols.GetOriginalEntryPoint(),
                library->AddFunctionToLibraryObject(objectConstructor, PropertyIds::getOwnPropertySymbols, &JavascriptObject::EntryInfo::GetOwnPropertySymbols, 1));
        }
        if (scriptContext->GetConfig()->IsES6ObjectExtensionsEnabled())
        {
            scriptContext->SetBuiltInLibraryFunction(JavascriptObject::EntryInfo::Is.GetOriginalEntryPoint(),
                library->AddFunctionToLibraryObject(objectConstructor, PropertyIds::is, &JavascriptObject::EntryInfo::Is, 2));
            scriptContext->SetBuiltInLibraryFunction(JavascriptObject::EntryInfo::Assign.GetOriginalEntryPoint(),
                library->AddFunctionToLibraryObject(objectConstructor, PropertyIds::assign, &JavascriptObject::EntryInfo::Assign, 2));
        }

        objectConstructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeObjectPrototype(DynamicObject* objectPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        JavascriptLibrary* library = objectPrototype->GetLibrary();
        ScriptContext* scriptContext = objectPrototype->GetScriptContext();

        bool hasAccessors = scriptContext->GetConfig()->Is__proto__Enabled();

        typeHandler->Convert(objectPrototype, mode, 11, hasAccessors);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterObject
        // so that the update is in sync with profiler
        library->AddMember(objectPrototype, PropertyIds::constructor, library->objectConstructor);
        library->AddFunctionToLibraryObject(objectPrototype, PropertyIds::hasOwnProperty, &JavascriptObject::EntryInfo::HasOwnProperty, 1);
        library->AddFunctionToLibraryObject(objectPrototype, PropertyIds::propertyIsEnumerable, &JavascriptObject::EntryInfo::PropertyIsEnumerable, 1);
        library->AddFunctionToLibraryObject(objectPrototype, PropertyIds::isPrototypeOf, &JavascriptObject::EntryInfo::IsPrototypeOf, 1);
        library->AddFunctionToLibraryObject(objectPrototype, PropertyIds::toLocaleString, &JavascriptObject::EntryInfo::ToLocaleString, 0);

        library->objectToStringFunction = library->AddFunctionToLibraryObject(objectPrototype, PropertyIds::toString, &JavascriptObject::EntryInfo::ToString, 0);
        library->objectValueOfFunction = library->AddFunctionToLibraryObject(objectPrototype, PropertyIds::valueOf, &JavascriptObject::EntryInfo::ValueOf, 0);

        scriptContext->SetBuiltInLibraryFunction(JavascriptObject::EntryInfo::ToString.GetOriginalEntryPoint(), library->objectToStringFunction);

        if (scriptContext->GetConfig()->Is__proto__Enabled())
        {
            bool hadOnlyWritableDataProperties = objectPrototype->GetDynamicType()->GetTypeHandler()->GetHasOnlyWritableDataProperties();
            objectPrototype->SetAccessors(PropertyIds::__proto__, library->Get__proto__getterFunction(), library->Get__proto__setterFunction(), PropertyOperation_NonFixedValue);
            objectPrototype->SetEnumerable(PropertyIds::__proto__, FALSE);
            // Let's pretend __proto__ is actually writable.  We'll make sure we always go through a special code path when writing to it.
            if (hadOnlyWritableDataProperties)
            {
                objectPrototype->GetDynamicType()->GetTypeHandler()->SetHasOnlyWritableDataProperties();
            }
        }

        if (scriptContext->GetConfig()->IsDefineGetterSetterEnabled())
        {
            library->AddFunctionToLibraryObject(objectPrototype, PropertyIds::__defineGetter__, &JavascriptObject::EntryInfo::DefineGetter, 2);
            library->AddFunctionToLibraryObject(objectPrototype, PropertyIds::__defineSetter__, &JavascriptObject::EntryInfo::DefineSetter, 2);
            library->AddFunctionToLibraryObject(objectPrototype, PropertyIds::__lookupGetter__, &JavascriptObject::EntryInfo::LookupGetter, 1);
            library->AddFunctionToLibraryObject(objectPrototype, PropertyIds::__lookupSetter__, &JavascriptObject::EntryInfo::LookupSetter, 1);
        }

        objectPrototype->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeRegexConstructor(DynamicObject* regexConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        JavascriptLibrary* library = regexConstructor->GetLibrary();
        ScriptContext* scriptContext = regexConstructor->GetScriptContext();
        typeHandler->Convert(regexConstructor, mode, 3);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterRegExp
        // so that the update is in sync with profiler
        library->AddMember(regexConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(2), PropertyNone);
        library->AddMember(regexConstructor, PropertyIds::prototype, library->regexPrototype, PropertyNone);
        library->AddAccessorsToLibraryObject(regexConstructor, PropertyIds::_symbolSpecies, &JavascriptRegExp::EntryInfo::GetterSymbolSpecies, nullptr);
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(regexConstructor, PropertyIds::name, scriptContext->GetPropertyString(PropertyIds::RegExp), PropertyConfigurable);
        }

        regexConstructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeRegexPrototype(DynamicObject* regexPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(regexPrototype, mode, 5);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterRegExp
        // so that the update is in sync with profiler
        JavascriptFunction * func;
        JavascriptLibrary* library = regexPrototype->GetLibrary();
        JavascriptFunction ** builtinFuncs = library->GetBuiltinFunctions();

        library->AddMember(regexPrototype, PropertyIds::constructor, library->regexConstructor);
        func = library->AddFunctionToLibraryObject(regexPrototype, PropertyIds::exec, &JavascriptRegExp::EntryInfo::Exec, 1);
        builtinFuncs[BuiltinFunction::RegExp_Exec] = func;
        library->AddFunctionToLibraryObject(regexPrototype, PropertyIds::test, &JavascriptRegExp::EntryInfo::Test, 1);
        library->AddFunctionToLibraryObject(regexPrototype, PropertyIds::toString, &JavascriptRegExp::EntryInfo::ToString, 0);
        // This is deprecated. Should be guarded with appropriate version flag.
        library->AddFunctionToLibraryObject(regexPrototype, PropertyIds::compile, &JavascriptRegExp::EntryInfo::Compile, 2);

        DebugOnly(CheckRegisteredBuiltIns(builtinFuncs));

        regexPrototype->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeStringConstructor(DynamicObject* stringConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(stringConstructor, mode, 6);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterString
        // so that the update is in sync with profiler
        JavascriptLibrary* library = stringConstructor->GetLibrary();
        ScriptContext* scriptContext = stringConstructor->GetScriptContext();

        JavascriptFunction ** builtinFuncs = library->GetBuiltinFunctions();
        library->AddMember(stringConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(1), PropertyNone);
        library->AddMember(stringConstructor, PropertyIds::prototype, library->stringPrototype, PropertyNone);

        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(stringConstructor, PropertyIds::name, scriptContext->GetPropertyString(PropertyIds::String), PropertyConfigurable);
        }


        builtinFuncs[BuiltinFunction::String_FromCharCode]  = library->AddFunctionToLibraryObject(stringConstructor, PropertyIds::fromCharCode,  &JavascriptString::EntryInfo::FromCharCode,  1);
        if(scriptContext->GetConfig()->IsES6UnicodeExtensionsEnabled())
        {
            builtinFuncs[BuiltinFunction::String_FromCodePoint] = library->AddFunctionToLibraryObject(stringConstructor, PropertyIds::fromCodePoint, &JavascriptString::EntryInfo::FromCodePoint, 1);
        }

        if (scriptContext->GetConfig()->IsES6StringTemplateEnabled())
        {
            /* No inlining                String_Raw           */ library->AddFunctionToLibraryObject(stringConstructor, PropertyIds::raw,           &JavascriptString::EntryInfo::Raw,           1);
        }

        DebugOnly(CheckRegisteredBuiltIns(builtinFuncs));

        stringConstructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeStringPrototype(DynamicObject* stringPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(stringPrototype, mode, 38);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterString
        // so that the update is in sync with profiler
        ScriptContext* scriptContext = stringPrototype->GetScriptContext();
        JavascriptLibrary* library = stringPrototype->GetLibrary();
        JavascriptFunction ** builtinFuncs = library->GetBuiltinFunctions();
        library->AddMember(stringPrototype, PropertyIds::constructor, library->stringConstructor);

        builtinFuncs[BuiltinFunction::String_IndexOf]       = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::indexOf,            &JavascriptString::EntryInfo::IndexOf,              1);
        builtinFuncs[BuiltinFunction::String_LastIndexOf]   = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::lastIndexOf,        &JavascriptString::EntryInfo::LastIndexOf,          1);
        builtinFuncs[BuiltinFunction::String_Replace]       = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::replace,            &JavascriptString::EntryInfo::Replace,              2);
        builtinFuncs[BuiltinFunction::String_Search]        = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::search,             &JavascriptString::EntryInfo::Search,               1);
        builtinFuncs[BuiltinFunction::String_Slice]         = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::slice,              &JavascriptString::EntryInfo::Slice,                2);

        if (CONFIG_FLAG(ES6Unicode))
        {
            builtinFuncs[BuiltinFunction::String_CodePointAt]   = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::codePointAt,        &JavascriptString::EntryInfo::CodePointAt,          1);
            /* builtinFuncs[BuiltinFunction::String_Normalize] =*/library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::normalize,          &JavascriptString::EntryInfo::Normalize,            0);
        }

        builtinFuncs[BuiltinFunction::String_CharAt]            = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::charAt,             &JavascriptString::EntryInfo::CharAt,               1);
        builtinFuncs[BuiltinFunction::String_CharCodeAt]        = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::charCodeAt,         &JavascriptString::EntryInfo::CharCodeAt,           1);
        builtinFuncs[BuiltinFunction::String_Concat]            = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::concat,             &JavascriptString::EntryInfo::Concat,               1);
        builtinFuncs[BuiltinFunction::String_LocaleCompare]     = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::localeCompare,      &JavascriptString::EntryInfo::LocaleCompare,        1);
        builtinFuncs[BuiltinFunction::String_Match]             = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::match,              &JavascriptString::EntryInfo::Match,                1);
        builtinFuncs[BuiltinFunction::String_Split]             = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::split,              &JavascriptString::EntryInfo::Split,                2);
        builtinFuncs[BuiltinFunction::String_Substring]         = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::substring,          &JavascriptString::EntryInfo::Substring,            2);
        builtinFuncs[BuiltinFunction::String_Substr]            = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::substr,             &JavascriptString::EntryInfo::Substr,               2);
        builtinFuncs[BuiltinFunction::String_ToLocaleLowerCase] = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::toLocaleLowerCase,  &JavascriptString::EntryInfo::ToLocaleLowerCase,    0);
        builtinFuncs[BuiltinFunction::String_ToLocaleUpperCase] = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::toLocaleUpperCase,  &JavascriptString::EntryInfo::ToLocaleUpperCase,    0);
        builtinFuncs[BuiltinFunction::String_ToLowerCase]       = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::toLowerCase,        &JavascriptString::EntryInfo::ToLowerCase,          0);
        scriptContext->SetBuiltInLibraryFunction(JavascriptString::EntryInfo::ToString.GetOriginalEntryPoint(),
                                                                  library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::toString,           &JavascriptString::EntryInfo::ToString,             0));
        builtinFuncs[BuiltinFunction::String_ToUpperCase]       = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::toUpperCase,        &JavascriptString::EntryInfo::ToUpperCase,          0);
        builtinFuncs[BuiltinFunction::String_Trim]              = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::trim,               &JavascriptString::EntryInfo::Trim,                 0);

        scriptContext->SetBuiltInLibraryFunction(JavascriptString::EntryInfo::ValueOf.GetOriginalEntryPoint(),
                                                                  library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::valueOf,            &JavascriptString::EntryInfo::ValueOf,              0));

        if (scriptContext->GetConfig()->SupportsES3Extensions() && scriptContext->GetConfig()->GetHostType() != HostTypeApplication)
        {
            /* No inlining                String_Anchor        */ library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::anchor,             &JavascriptString::EntryInfo::Anchor,               1);
            /* No inlining                String_Big           */ library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::big,                &JavascriptString::EntryInfo::Big,                  0);
            /* No inlining                String_Blink         */ library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::blink,              &JavascriptString::EntryInfo::Blink,                0);
            /* No inlining                String_Bold          */ library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::bold,               &JavascriptString::EntryInfo::Bold,                 0);
            /* No inlining                String_Fixed         */ library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::fixed,              &JavascriptString::EntryInfo::Fixed,                0);
            /* No inlining                String_FontColor     */ library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::fontcolor,          &JavascriptString::EntryInfo::FontColor,            1);
            /* No inlining                String_FontSize      */ library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::fontsize,           &JavascriptString::EntryInfo::FontSize,             1);
            /* No inlining                String_Italics       */ library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::italics,            &JavascriptString::EntryInfo::Italics,              0);
            builtinFuncs[BuiltinFunction::String_Link]          = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::link,               &JavascriptString::EntryInfo::Link,                 1);
            /* No inlining                String_Small         */ library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::Small,              &JavascriptString::EntryInfo::Small,                0);
            /* No inlining                String_Strike        */ library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::strike,             &JavascriptString::EntryInfo::Strike,               0);
            /* No inlining                String_Sub           */ library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::sub,                &JavascriptString::EntryInfo::Sub,                  0);
            /* No inlining                String_Sup           */ library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::sup,                &JavascriptString::EntryInfo::Sup,                  0);
        }

        if (scriptContext->GetConfig()->IsES6StringExtensionsEnabled())
        {
            /* No inlining                String_Repeat        */ library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::repeat,             &JavascriptString::EntryInfo::Repeat,               1);
            /* No inlining                String_StartsWith    */ library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::startsWith,         &JavascriptString::EntryInfo::StartsWith,           1);
            /* No inlining                String_EndsWith      */ library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::endsWith,           &JavascriptString::EntryInfo::EndsWith,             1);
            /* No inlining                String_Includes      */ library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::includes,           &JavascriptString::EntryInfo::Includes,             1);
            builtinFuncs[BuiltinFunction::String_TrimLeft]      = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::trimLeft,           &JavascriptString::EntryInfo::TrimLeft,             0);
            builtinFuncs[BuiltinFunction::String_TrimRight]     = library->AddFunctionToLibraryObject(stringPrototype, PropertyIds::trimRight,          &JavascriptString::EntryInfo::TrimRight,            0);
        }

        if (scriptContext->GetConfig()->IsES6IteratorsEnabled())
        {
            library->AddFunctionToLibraryObjectWithName(stringPrototype, PropertyIds::_symbolIterator, PropertyIds::_RuntimeFunctionNameId_iterator,
                &JavascriptString::EntryInfo::SymbolIterator, 0);
        }

        DebugOnly(CheckRegisteredBuiltIns(builtinFuncs));

        stringPrototype->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeMapConstructor(DynamicObject* mapConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(mapConstructor, mode, 3);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterMap
        // so that the update is in sync with profiler
        JavascriptLibrary* library = mapConstructor->GetLibrary();
        ScriptContext* scriptContext = mapConstructor->GetScriptContext();
        library->AddMember(mapConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(0), PropertyNone);
        library->AddMember(mapConstructor, PropertyIds::prototype, library->mapPrototype, PropertyNone);
        library->AddAccessorsToLibraryObject(mapConstructor, PropertyIds::_symbolSpecies, &JavascriptMap::EntryInfo::GetterSymbolSpecies, nullptr);
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(mapConstructor, PropertyIds::name, scriptContext->GetPropertyString(PropertyIds::Map), PropertyConfigurable);
        }

        mapConstructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeMapPrototype(DynamicObject* mapPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(mapPrototype, mode, 13, true);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterMap
        // so that the update is in sync with profiler
        ScriptContext* scriptContext = mapPrototype->GetScriptContext();
        JavascriptLibrary* library = mapPrototype->GetLibrary();
        library->AddMember(mapPrototype, PropertyIds::constructor, library->mapConstructor);

        library->AddFunctionToLibraryObject(mapPrototype, PropertyIds::clear, &JavascriptMap::EntryInfo::Clear, 0);
        library->AddFunctionToLibraryObject(mapPrototype, PropertyIds::delete_, &JavascriptMap::EntryInfo::Delete, 1);
        library->AddFunctionToLibraryObject(mapPrototype, PropertyIds::forEach, &JavascriptMap::EntryInfo::ForEach, 1);
        library->AddFunctionToLibraryObject(mapPrototype, PropertyIds::get, &JavascriptMap::EntryInfo::Get, 1);
        library->AddFunctionToLibraryObject(mapPrototype, PropertyIds::has, &JavascriptMap::EntryInfo::Has, 1);
        library->AddFunctionToLibraryObject(mapPrototype, PropertyIds::set, &JavascriptMap::EntryInfo::Set, 2);

        library->AddAccessorsToLibraryObject(mapPrototype, PropertyIds::size, &JavascriptMap::EntryInfo::SizeGetter, nullptr);

        if (scriptContext->GetConfig()->IsES6IteratorsEnabled())
        {
            JavascriptFunction* entriesFunc;
            entriesFunc = library->AddFunctionToLibraryObject(mapPrototype, PropertyIds::entries, &JavascriptMap::EntryInfo::Entries, 0);
            library->AddFunctionToLibraryObject(mapPrototype, PropertyIds::keys, &JavascriptMap::EntryInfo::Keys, 0);
            library->AddFunctionToLibraryObject(mapPrototype, PropertyIds::values, &JavascriptMap::EntryInfo::Values, 0);
            library->AddMember(mapPrototype, PropertyIds::_symbolIterator, entriesFunc);
        }
        if (scriptContext->GetConfig()->IsES6ToStringTagEnabled())
        {
            library->AddMember(mapPrototype, PropertyIds::_symbolToStringTag, library->CreateStringFromCppLiteral(L"Map"), PropertyConfigurable);
        }

        mapPrototype->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeSetConstructor(DynamicObject* setConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(setConstructor, mode, 3);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterSet
        // so that the update is in sync with profiler
        JavascriptLibrary* library = setConstructor->GetLibrary();
        ScriptContext* scriptContext = setConstructor->GetScriptContext();
        library->AddMember(setConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(0), PropertyNone);
        library->AddMember(setConstructor, PropertyIds::prototype, library->setPrototype, PropertyNone);
        library->AddAccessorsToLibraryObject(setConstructor, PropertyIds::_symbolSpecies, &JavascriptSet::EntryInfo::GetterSymbolSpecies, nullptr);
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(setConstructor, PropertyIds::name, scriptContext->GetPropertyString(PropertyIds::Set), PropertyConfigurable);
        }

        setConstructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeSetPrototype(DynamicObject* setPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(setPrototype, mode, 12, true);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterSet
        // so that the update is in sync with profiler
        ScriptContext* scriptContext = setPrototype->GetScriptContext();
        JavascriptLibrary* library = setPrototype->GetLibrary();
        library->AddMember(setPrototype, PropertyIds::constructor, library->setConstructor);

        library->AddFunctionToLibraryObject(setPrototype, PropertyIds::add, &JavascriptSet::EntryInfo::Add, 1);
        library->AddFunctionToLibraryObject(setPrototype, PropertyIds::clear, &JavascriptSet::EntryInfo::Clear, 0);
        library->AddFunctionToLibraryObject(setPrototype, PropertyIds::delete_, &JavascriptSet::EntryInfo::Delete, 1);
        library->AddFunctionToLibraryObject(setPrototype, PropertyIds::forEach, &JavascriptSet::EntryInfo::ForEach, 1);
        library->AddFunctionToLibraryObject(setPrototype, PropertyIds::has, &JavascriptSet::EntryInfo::Has, 1);

        library->AddAccessorsToLibraryObject(setPrototype, PropertyIds::size, &JavascriptSet::EntryInfo::SizeGetter, nullptr);

        if (scriptContext->GetConfig()->IsES6IteratorsEnabled())
        {
            JavascriptFunction* valuesFunc;
            library->AddFunctionToLibraryObject(setPrototype, PropertyIds::entries, &JavascriptSet::EntryInfo::Entries, 0);
            valuesFunc = library->AddFunctionToLibraryObject(setPrototype, PropertyIds::values, &JavascriptSet::EntryInfo::Values, 0);
            library->AddMember(setPrototype, PropertyIds::keys, valuesFunc);
            library->AddMember(setPrototype, PropertyIds::_symbolIterator, valuesFunc);
        }

        if (scriptContext->GetConfig()->IsES6ToStringTagEnabled())
        {
            library->AddMember(setPrototype, PropertyIds::_symbolToStringTag, library->CreateStringFromCppLiteral(L"Set"), PropertyConfigurable);
        }

        setPrototype->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeWeakMapConstructor(DynamicObject* weakMapConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(weakMapConstructor, mode, 3);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterWeakMap
        // so that the update is in sync with profiler
        JavascriptLibrary* library = weakMapConstructor->GetLibrary();
        ScriptContext* scriptContext = weakMapConstructor->GetScriptContext();
        library->AddMember(weakMapConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(0), PropertyNone);
        library->AddMember(weakMapConstructor, PropertyIds::prototype, library->weakMapPrototype, PropertyNone);
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(weakMapConstructor, PropertyIds::name, scriptContext->GetPropertyString(PropertyIds::WeakMap), PropertyConfigurable);
        }

        weakMapConstructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeWeakMapPrototype(DynamicObject* weakMapPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(weakMapPrototype, mode, 6);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterWeakMap
        // so that the update is in sync with profiler
        ScriptContext* scriptContext = weakMapPrototype->GetScriptContext();
        JavascriptLibrary* library = weakMapPrototype->GetLibrary();
        library->AddMember(weakMapPrototype, PropertyIds::constructor, library->weakMapConstructor);

        library->AddFunctionToLibraryObject(weakMapPrototype, PropertyIds::delete_, &JavascriptWeakMap::EntryInfo::Delete, 1);
        library->AddFunctionToLibraryObject(weakMapPrototype, PropertyIds::get, &JavascriptWeakMap::EntryInfo::Get, 1);
        library->AddFunctionToLibraryObject(weakMapPrototype, PropertyIds::has, &JavascriptWeakMap::EntryInfo::Has, 1);
        library->AddFunctionToLibraryObject(weakMapPrototype, PropertyIds::set, &JavascriptWeakMap::EntryInfo::Set, 2);

        if (scriptContext->GetConfig()->IsES6ToStringTagEnabled())
        {
            library->AddMember(weakMapPrototype, PropertyIds::_symbolToStringTag, library->CreateStringFromCppLiteral(L"WeakMap"), PropertyConfigurable);
        }

        weakMapPrototype->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeWeakSetConstructor(DynamicObject* weakSetConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(weakSetConstructor, mode, 3);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterWeakSet
        // so that the update is in sync with profiler
        JavascriptLibrary* library = weakSetConstructor->GetLibrary();
        ScriptContext* scriptContext = weakSetConstructor->GetScriptContext();
        library->AddMember(weakSetConstructor, PropertyIds::length, TaggedInt::ToVarUnchecked(0), PropertyNone);
        library->AddMember(weakSetConstructor, PropertyIds::prototype, library->weakSetPrototype, PropertyNone);
        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            library->AddMember(weakSetConstructor, PropertyIds::name, scriptContext->GetPropertyString(PropertyIds::WeakSet), PropertyConfigurable);
        }

        weakSetConstructor->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeWeakSetPrototype(DynamicObject* weakSetPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(weakSetPrototype, mode, 5);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterWeakSet
        // so that the update is in sync with profiler
        ScriptContext* scriptContext = weakSetPrototype->GetScriptContext();
        JavascriptLibrary* library = weakSetPrototype->GetLibrary();
        library->AddMember(weakSetPrototype, PropertyIds::constructor, library->weakSetConstructor);

        library->AddFunctionToLibraryObject(weakSetPrototype, PropertyIds::add, &JavascriptWeakSet::EntryInfo::Add, 1);
        library->AddFunctionToLibraryObject(weakSetPrototype, PropertyIds::delete_, &JavascriptWeakSet::EntryInfo::Delete, 1);
        library->AddFunctionToLibraryObject(weakSetPrototype, PropertyIds::has, &JavascriptWeakSet::EntryInfo::Has, 1);

        if (scriptContext->GetConfig()->IsES6ToStringTagEnabled())
        {
            library->AddMember(weakSetPrototype, PropertyIds::_symbolToStringTag, library->CreateStringFromCppLiteral(L"WeakSet"), PropertyConfigurable);
        }

        weakSetPrototype->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeArrayIteratorPrototype(DynamicObject* arrayIteratorPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(arrayIteratorPrototype, mode, 3);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterArrayIterator
        // so that the update is in sync with profiler

        JavascriptLibrary* library = arrayIteratorPrototype->GetLibrary();
        ScriptContext* scriptContext = library->GetScriptContext();

        library->AddFunctionToLibraryObject(arrayIteratorPrototype, PropertyIds::next, &JavascriptArrayIterator::EntryInfo::Next, 0);
        library->AddFunctionToLibraryObjectWithName(arrayIteratorPrototype, PropertyIds::_symbolIterator, PropertyIds::_RuntimeFunctionNameId_iterator,
            &JavascriptArrayIterator::EntryInfo::SymbolIterator, 0);

        if (scriptContext->GetConfig()->IsES6ToStringTagEnabled())
        {
            library->AddMember(arrayIteratorPrototype, PropertyIds::_symbolToStringTag, library->CreateStringFromCppLiteral(L"Array Iterator"), PropertyConfigurable);
        }
    }

    void JavascriptLibrary::InitializeMapIteratorPrototype(DynamicObject* mapIteratorPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(mapIteratorPrototype, mode, 3);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterMapIterator
        // so that the update is in sync with profiler

        JavascriptLibrary* library = mapIteratorPrototype->GetLibrary();
        ScriptContext* scriptContext = library->GetScriptContext();

        library->AddFunctionToLibraryObject(mapIteratorPrototype, PropertyIds::next, &JavascriptMapIterator::EntryInfo::Next, 0);
        library->AddFunctionToLibraryObjectWithName(mapIteratorPrototype, PropertyIds::_symbolIterator, PropertyIds::_RuntimeFunctionNameId_iterator,
            &JavascriptMapIterator::EntryInfo::SymbolIterator, 0);

        if (scriptContext->GetConfig()->IsES6ToStringTagEnabled())
        {
            library->AddMember(mapIteratorPrototype, PropertyIds::_symbolToStringTag, library->CreateStringFromCppLiteral(L"Map Iterator"), PropertyConfigurable);
        }
    }

    void JavascriptLibrary::InitializeSetIteratorPrototype(DynamicObject* setIteratorPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(setIteratorPrototype, mode, 3);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterSetIterator
        // so that the update is in sync with profiler

        JavascriptLibrary* library = setIteratorPrototype->GetLibrary();
        ScriptContext* scriptContext = library->GetScriptContext();
        library->AddFunctionToLibraryObject(setIteratorPrototype, PropertyIds::next, &JavascriptSetIterator::EntryInfo::Next, 0);
        library->AddFunctionToLibraryObjectWithName(setIteratorPrototype, PropertyIds::_symbolIterator, PropertyIds::_RuntimeFunctionNameId_iterator,
            &JavascriptSetIterator::EntryInfo::SymbolIterator, 0);

        if (scriptContext->GetConfig()->IsES6ToStringTagEnabled())
        {
            library->AddMember(setIteratorPrototype, PropertyIds::_symbolToStringTag, library->CreateStringFromCppLiteral(L"Set Iterator"), PropertyConfigurable);
        }
    }

    void JavascriptLibrary::InitializeStringIteratorPrototype(DynamicObject* stringIteratorPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(stringIteratorPrototype, mode, 3);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterStringIterator
        // so that the update is in sync with profiler

        JavascriptLibrary* library = stringIteratorPrototype->GetLibrary();
        ScriptContext* scriptContext = library->GetScriptContext();
        library->AddFunctionToLibraryObject(stringIteratorPrototype, PropertyIds::next, &JavascriptStringIterator::EntryInfo::Next, 0);
        library->AddFunctionToLibraryObjectWithName(stringIteratorPrototype, PropertyIds::_symbolIterator, PropertyIds::_RuntimeFunctionNameId_iterator,
            &JavascriptStringIterator::EntryInfo::SymbolIterator, 0);

        if (scriptContext->GetConfig()->IsES6ToStringTagEnabled())
        {
            library->AddMember(stringIteratorPrototype, PropertyIds::_symbolToStringTag, library->CreateStringFromCppLiteral(L"String Iterator"), PropertyConfigurable);
        }
    }

    void JavascriptLibrary::InitializeJavascriptEnumeratorIteratorPrototype(DynamicObject* javascriptEnumeratorIteratorPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(javascriptEnumeratorIteratorPrototype, mode, 1);
        // Note: Any new function addition/deletion/modification should also be updated in ScriptContext::RegisterEnumeratorIterator
        // so that the update is in sync with profiler

        JavascriptLibrary* library = javascriptEnumeratorIteratorPrototype->GetLibrary();
        ScriptContext* scriptContext = library->GetScriptContext();
        library->AddFunctionToLibraryObject(javascriptEnumeratorIteratorPrototype, PropertyIds::next, &JavascriptEnumeratorIterator::EntryInfo::Next, 0);
        library->AddFunctionToLibraryObjectWithName(javascriptEnumeratorIteratorPrototype, PropertyIds::_symbolIterator, PropertyIds::_RuntimeFunctionNameId_iterator,
            &JavascriptEnumeratorIterator::EntryInfo::SymbolIterator, 0);

        if (scriptContext->GetConfig()->IsES6ToStringTagEnabled())
        {
            library->AddMember(javascriptEnumeratorIteratorPrototype, PropertyIds::_symbolToStringTag, library->CreateStringFromCppLiteral(L"Enumerator Iterator"), PropertyConfigurable);
        }
    }

    RuntimeFunction* JavascriptLibrary::CreateBuiltinConstructor(FunctionInfo * functionInfo, DynamicTypeHandler * typeHandler, DynamicObject* prototype)
    {
        Assert((functionInfo->GetAttributes() & FunctionInfo::Attributes::ErrorOnNew) == 0);

        if (prototype == nullptr)
        {
            prototype = functionPrototype;
        }

        ConstructorCache* ctorCache = ((functionInfo->GetAttributes() & FunctionInfo::Attributes::SkipDefaultNewObject) != 0) ?
            this->builtInConstructorCache : &ConstructorCache::DefaultInstance;

        return RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, RuntimeFunction,
            DynamicType::New(scriptContext, TypeIds_Function, prototype, functionInfo->GetOriginalEntryPoint(), typeHandler),
            functionInfo, ctorCache);
    }

    JavascriptExternalFunction* JavascriptLibrary::CreateExternalConstructor(Js::JavascriptMethod entryPoint, PropertyId nameId, RecyclableObject * prototype)
    {
        Assert(nameId >= Js::InternalPropertyIds::Count && scriptContext->IsTrackedPropertyId(nameId));
        JavascriptExternalFunction* function = this->CreateIdMappedExternalFunction(entryPoint, idMappedFunctionWithPrototypeType);
        function->SetFunctionNameId(TaggedInt::ToVarUnchecked(nameId));

        Js::RecyclableObject* objPrototype;
        if (prototype == nullptr)
        {
            objPrototype = CreateConstructorPrototypeObject(function);
            Assert(!objPrototype->IsEnumerable(Js::PropertyIds::constructor));
        }
        else
        {
            objPrototype = Js::RecyclableObject::FromVar(prototype);
            Js::JavascriptOperators::InitProperty(objPrototype, Js::PropertyIds::constructor, function);
            objPrototype->SetEnumerable(Js::PropertyIds::constructor, false);
        }

        Assert(!function->IsEnumerable(Js::PropertyIds::prototype));
        Assert(!function->IsConfigurable(Js::PropertyIds::prototype));
        Assert(!function->IsWritable(Js::PropertyIds::prototype));
        function->SetPropertyWithAttributes(Js::PropertyIds::prototype, objPrototype, PropertyNone, nullptr);

        if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
        {
            function->SetPropertyWithAttributes(PropertyIds::name, function->GetDisplayName(true), PropertyConfigurable, nullptr);
        }

        return function;
    }

    JavascriptExternalFunction* JavascriptLibrary::CreateExternalConstructor(Js::JavascriptMethod entryPoint, PropertyId nameId, InitializeMethod method, unsigned short deferredTypeSlots, bool hasAccessors)
    {
        Assert(nameId >= Js::InternalPropertyIds::Count && scriptContext->IsTrackedPropertyId(nameId));

        // Make sure the actual entry point is never null.
        if (entryPoint == nullptr)
        {
            entryPoint = Js::RecyclableObject::DefaultEntryPoint;
        }

        JavascriptExternalFunction* function = RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, JavascriptExternalFunction, entryPoint,
            externalConstructorFunctionWithDeferredPrototypeType, method, deferredTypeSlots, hasAccessors);

        function->SetFunctionNameId(TaggedInt::ToVarUnchecked(nameId));

        return function;
    }

    void JavascriptLibrary::CopyOnWriteFrom(ScriptContext* scriptContext, GlobalObject * globalObject, JavascriptLibrary * originalLibrary)
    {
        VERIFY_COPY_ON_WRITE_ENABLED();

        // Initializing the library by copying the library from an existing library creating copy-on-write proxies for each object
        // to shield the original from mutations made in the new context.
        this->scriptContext = scriptContext;
        this->recycler = scriptContext->GetRecycler();
        AssertMsg(originalLibrary->recycler == this->recycler, "Copy-on-write assumes the same recycler is used in the original script context as the forked script context");

        typesEnsuredToHaveOnlyWritableDataPropertiesInItAndPrototypeChain = RecyclerNew(recycler, JsUtil::List<Type *>, recycler);

        nullValue = RecyclerNew(recycler, RecyclableObject, StaticType::New(scriptContext, TypeIds_Null, nullptr, nullptr));
        nullValue->GetType()->SetHasSpecialPrototype(true);

        // Copy the prototypes
        objectPrototype = (ObjectPrototypeObject *)scriptContext->CopyOnWrite(originalLibrary->objectPrototype);
        constructorPrototypeObjectType = CreateObjectTypeNoCache(objectPrototype, TypeIds_Object);
        generatorConstructorPrototypeObjectType = CreateObjectTypeNoCache(objectPrototype, TypeIds_Object);

        // Temporarily set the dyanmic types to what the prototype of their prototypes should be so the CopyOnWrite
        // object created will have this prototype. These values are then overwritten by InitializeTypes() to their
        // correct values with the corresponding prototype objects as their prototype. This is done this way to avoid
        // the prototype special case, which is encountered once per-context, having to appear in the MakeCopyOnWrite
        // methods which are executed once per copy.
        DynamicType *arrayPrototypeType = DynamicType::New(scriptContext, TypeIds_Array, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        arrayType = arrayPrototypeType;
        booleanTypeDynamic = DynamicType::New(scriptContext, TypeIds_BooleanObject, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        numberTypeDynamic = DynamicType::New(scriptContext, TypeIds_NumberObject, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        functionTypeHandler = SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true);
        stringTypeDynamic = DynamicType::New(scriptContext, TypeIds_StringObject, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);

        symbolTypeDynamic = nullptr;
        promiseType = nullptr;
        pixelArrayType = nullptr;

        if (scriptContext->GetConfig()->IsES6SymbolEnabled())
        {
            symbolTypeDynamic = DynamicType::New(scriptContext, TypeIds_SymbolObject, objectPrototype,
                RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        }

        if (scriptContext->GetConfig()->IsES6PromiseEnabled())
        {
            promiseType = DynamicType::New(scriptContext, TypeIds_Promise, objectPrototype,
                RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        }

        dateType = DynamicType::New(scriptContext, TypeIds_Date, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        regexType = DynamicType::New(scriptContext, TypeIds_RegEx, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);

        regexType->GetTypeHandler()->ClearHasOnlyWritableDataProperties();

        if (!scriptContext->GetConfig()->IsES6TypedArrayExtensionsEnabled())
        {
            pixelArrayType = DynamicType::New(scriptContext, TypeIds_PixelArray, objectPrototype,
                RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        }

        arrayBufferType = DynamicType::New(scriptContext, TypeIds_ArrayBuffer, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        int8ArrayType = DynamicType::New(scriptContext, TypeIds_Int8Array, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        uint8ArrayType = DynamicType::New(scriptContext, TypeIds_Uint8Array, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        uint8ClampedArrayType = DynamicType::New(scriptContext, TypeIds_Uint8ClampedArray, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        int16ArrayType = DynamicType::New(scriptContext, TypeIds_Int16Array, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        uint16ArrayType = DynamicType::New(scriptContext, TypeIds_Uint16Array, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        int32ArrayType = DynamicType::New(scriptContext, TypeIds_Int32Array, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        uint32ArrayType = DynamicType::New(scriptContext, TypeIds_Uint32Array, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        float32ArrayType = DynamicType::New(scriptContext, TypeIds_Float32Array, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        float64ArrayType = DynamicType::New(scriptContext, TypeIds_Float64Array, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        boolArrayType = DynamicType::New(scriptContext, TypeIds_BoolArray, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        charArrayType = DynamicType::New(scriptContext, TypeIds_CharArray, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        int64ArrayType = DynamicType::New(scriptContext, TypeIds_Int64Array, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        uint64ArrayType = DynamicType::New(scriptContext, TypeIds_Uint64Array, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);

        errorType = DynamicType::New(scriptContext, TypeIds_Error, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        evalErrorType = DynamicType::New(scriptContext, TypeIds_Error, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        rangeErrorType = DynamicType::New(scriptContext, TypeIds_Error, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        referenceErrorType = DynamicType::New(scriptContext, TypeIds_Error, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        syntaxErrorType = DynamicType::New(scriptContext, TypeIds_Error, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        typeErrorType = DynamicType::New(scriptContext, TypeIds_Error, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        uriErrorType = DynamicType::New(scriptContext, TypeIds_Error, objectPrototype,
            RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);

        if (scriptContext->GetConfig()->IsWinRTEnabled())
        {
            winrtErrorType = DynamicType::New(scriptContext, TypeIds_Error, objectPrototype,
                RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        }

        iteratorResultType = nullptr;
        arrayIteratorType = nullptr;
        mapIteratorType = nullptr;
        setIteratorType = nullptr;
        stringIteratorType = nullptr;

        if (scriptContext->GetConfig()->IsES6IteratorsEnabled())
        {
            iteratorResultType = DynamicType::New(scriptContext, TypeIds_Object, objectPrototype,
                RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);

            arrayIteratorType = DynamicType::New(scriptContext, TypeIds_ArrayIterator, objectPrototype,
                RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
            mapIteratorType = DynamicType::New(scriptContext, TypeIds_MapIterator, objectPrototype,
                RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
            setIteratorType = DynamicType::New(scriptContext, TypeIds_SetIterator, objectPrototype,
                RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
            stringIteratorType = DynamicType::New(scriptContext, TypeIds_StringIterator, objectPrototype,
                RecyclableObject::DefaultEntryPoint, SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
        }

        arrayPrototype = objectPrototype;  // This is set here so that the JavascriptArray::MakeCopyOnWrite called by CopyOnWrite gets the object prototype instead of itself
        DynamicObject *newArrayPrototype = (DynamicObject *)scriptContext->CopyOnWrite(originalLibrary->arrayPrototype);
        arrayPrototype = newArrayPrototype;

        booleanPrototype = (DynamicObject *)scriptContext->CopyOnWrite(originalLibrary->booleanPrototype);
        datePrototype = (DynamicObject *)scriptContext->CopyOnWrite(originalLibrary->datePrototype);
        functionPrototype = objectPrototype;  // This is set here so that the JavascriptFunction::MakeCopyOnWrite called by CopyOnWrite gets the object prototype instead of itself
        functionPrototype = (DynamicObject *)scriptContext->CopyOnWrite(originalLibrary->functionPrototype);
        numberPrototype = (DynamicObject *)scriptContext->CopyOnWrite(originalLibrary->numberPrototype);
        regexPrototype = (DynamicObject *)scriptContext->CopyOnWrite(originalLibrary->regexPrototype);
        stringPrototype = (DynamicObject *)scriptContext->CopyOnWrite(originalLibrary->stringPrototype);

        // Note that CopyObject handles nullptr. Initialize to nullptr if these were nullptr in original library.
        arrayBufferPrototype = CopyObject(scriptContext, originalLibrary->arrayBufferPrototype);
        typedArrayPrototype = CopyObject(scriptContext, originalLibrary->typedArrayPrototype);
        dataViewPrototype = CopyObject(scriptContext, originalLibrary->dataViewPrototype);
        Int8ArrayPrototype = CopyObject(scriptContext, originalLibrary->Int8ArrayPrototype);
        Uint8ArrayPrototype = CopyObject(scriptContext, originalLibrary->Uint8ArrayPrototype);
        Uint8ClampedArrayPrototype = CopyObject(scriptContext, originalLibrary->Uint8ClampedArrayPrototype);
        Int16ArrayPrototype = CopyObject(scriptContext, originalLibrary->Int16ArrayPrototype);
        Uint16ArrayPrototype = CopyObject(scriptContext, originalLibrary->Uint16ArrayPrototype);
        Int32ArrayPrototype = CopyObject(scriptContext, originalLibrary->Int32ArrayPrototype);
        Uint32ArrayPrototype = CopyObject(scriptContext, originalLibrary->Uint32ArrayPrototype);
        Float32ArrayPrototype = CopyObject(scriptContext, originalLibrary->Float32ArrayPrototype);
        Float64ArrayPrototype = CopyObject(scriptContext, originalLibrary->Float64ArrayPrototype);
        Int64ArrayPrototype = CopyObject(scriptContext, originalLibrary->Int64ArrayPrototype);
        Uint64ArrayPrototype = CopyObject(scriptContext, originalLibrary->Uint64ArrayPrototype);
        BoolArrayPrototype = CopyObject(scriptContext, originalLibrary->BoolArrayPrototype);
        CharArrayPrototype = CopyObject(scriptContext, originalLibrary->CharArrayPrototype);

        errorPrototype = CopyObject(scriptContext, originalLibrary->errorPrototype);
        evalErrorPrototype = CopyObject(scriptContext, originalLibrary->evalErrorPrototype);
        rangeErrorPrototype = CopyObject(scriptContext, originalLibrary->rangeErrorPrototype);
        referenceErrorPrototype = CopyObject(scriptContext, originalLibrary->referenceErrorPrototype);
        syntaxErrorPrototype = CopyObject(scriptContext, originalLibrary->syntaxErrorPrototype);
        typeErrorPrototype = CopyObject(scriptContext, originalLibrary->typeErrorPrototype);
        uriErrorPrototype = CopyObject(scriptContext, originalLibrary->uriErrorPrototype);
        winrtErrorPrototype = CopyObject(scriptContext, originalLibrary->winrtErrorPrototype);

        mapPrototype = CopyObject(scriptContext, originalLibrary->mapPrototype);
        setPrototype = CopyObject(scriptContext, originalLibrary->setPrototype);
        weakMapPrototype = CopyObject(scriptContext, originalLibrary->weakMapPrototype);
        weakSetPrototype = CopyObject(scriptContext, originalLibrary->weakSetPrototype);

        arrayIteratorPrototype = CopyObject(scriptContext, originalLibrary->arrayIteratorPrototype);
        mapIteratorPrototype = CopyObject(scriptContext, originalLibrary->mapIteratorPrototype);
        setIteratorPrototype = CopyObject(scriptContext, originalLibrary->setIteratorPrototype);
        stringIteratorPrototype = CopyObject(scriptContext, originalLibrary->stringIteratorPrototype);

        javascriptEnumeratorIteratorPrototype = CopyObject(scriptContext, originalLibrary->javascriptEnumeratorIteratorPrototype);

        pixelArrayPrototype = CopyObject(scriptContext, originalLibrary->pixelArrayPrototype);
        symbolPrototype = CopyObject(scriptContext, originalLibrary->symbolPrototype);
        promisePrototype = CopyObject(scriptContext, originalLibrary->promisePrototype);

        generatorFunctionPrototype = CopyObject(scriptContext, originalLibrary->generatorFunctionPrototype);
        generatorPrototype = CopyObject(scriptContext, originalLibrary->generatorPrototype);

        undeclBlockVarSentinel = RecyclerNew(recycler, UndeclaredBlockVariable, StaticType::New(scriptContext, TypeIds_Null, nullptr, nullptr));

        // Create the types
        InitializeTypes();

        CopyOnWriteSpecialGlobals(scriptContext, globalObject, originalLibrary);

        // The following call is performed earlier than during initialize because the regular expression
        // type must be valid before detaching the array prototype or trying to copy the regular expression
        // constructor because they early detach and they might try to copy a regular expression.
        InitializeComplexThings();

        // Force the array prototype to detach.  Delaying the detach of arrayType will cause the prototype object
        // to be get the wrong type. This is odd because JavascriptArray's prototype must be a JavascriptArray but not have
        // itself as a prototype. To do this we first we need to set the arrayPrototype back to object (to ensure
        // the array prototype's prototype is still object), then we detach the object. This cannot be done earlier
        // because the detach code uses an enumerator which is not usable until after the types have been initialized.
        arrayPrototype = objectPrototype;
        DynamicType *realArrayType = arrayType;
        arrayType = arrayPrototypeType;
        ((CopyOnWriteObject<JavascriptArray>*)newArrayPrototype)->Detach();
        arrayPrototype = newArrayPrototype;
        arrayType = realArrayType;

        // Initialize the global object prototype.
        globalObject->SetPrototype(objectPrototype);

        CopyOnWriteGlobal(scriptContext, globalObject, originalLibrary);
    }


    RuntimeFunction* JavascriptLibrary::DefaultCreateFunction(FunctionInfo * functionInfo, int length, DynamicObject * prototype, DynamicType * functionType, PropertyId nameId)
    {
        return DefaultCreateFunction(functionInfo, length, prototype, functionType, TaggedInt::ToVarUnchecked((int)nameId));
    }

    RuntimeFunction* JavascriptLibrary::DefaultCreateFunction(FunctionInfo * functionInfo, int length, DynamicObject * prototype, DynamicType * functionType, Var nameId)
    {
        RuntimeFunction * function;
        if (nullptr == functionType)
        {
            functionType = (nullptr == prototype) ?
                CreateFunctionWithLengthType(functionInfo) : CreateFunctionWithLengthAndPrototypeType(functionInfo);
        }

        function = RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, RuntimeFunction, functionType, functionInfo);

        if (prototype != nullptr)
        {
            // NOTE: Assume all built in function prototype doesn't contain valueOf or toString that has side effects
            function->SetPropertyWithAttributes(PropertyIds::prototype, prototype, PropertyNone, nullptr, PropertyOperation_None, SideEffects_None);
        }

        function->SetPropertyWithAttributes(PropertyIds::length, TaggedInt::ToVarUnchecked(length), PropertyConfigurable, nullptr);
        function->SetFunctionNameId(nameId);
        if (function->GetScriptContext()->GetConfig()->IsES6FunctionNameEnabled())
        {
            function->SetPropertyWithAttributes(PropertyIds::name, function->GetDisplayName(true), PropertyConfigurable, nullptr);
        }

#ifdef HEAP_ENUMERATION_VALIDATION
        if (prototype) prototype->SetHeapEnumValidationCookie(HEAP_ENUMERATION_LIBRARY_OBJECT_COOKIE);
        function->SetHeapEnumValidationCookie(HEAP_ENUMERATION_LIBRARY_OBJECT_COOKIE);
#endif
        return function;
    }

    JavascriptFunction* JavascriptLibrary::AddFunction(DynamicObject* object, PropertyId propertyId, RuntimeFunction* function)
    {

       AddMember(object, propertyId, function);
       function->SetFunctionNameId(TaggedInt::ToVarUnchecked((int)propertyId));
       return function;
    }

    JavascriptFunction * JavascriptLibrary::AddFunctionToLibraryObject(DynamicObject* object, PropertyId propertyId, FunctionInfo * functionInfo, int length, PropertyAttributes attributes)
    {
        RuntimeFunction* function = DefaultCreateFunction(functionInfo, length, nullptr, nullptr, propertyId);
        AddMember(object, propertyId, function, attributes);
        return function;
    }

    JavascriptFunction * JavascriptLibrary::AddFunctionToLibraryObjectWithPrototype(GlobalObject * object, PropertyId propertyId, FunctionInfo * functionInfo, int length, DynamicObject * prototype, DynamicType * functionType)
    {
        RuntimeFunction* function = DefaultCreateFunction(functionInfo, length, prototype, functionType, propertyId);
        AddMember(object, propertyId, function);
        return function;
    }

    JavascriptFunction * JavascriptLibrary::AddFunctionToLibraryObjectWithName(DynamicObject* object, PropertyId propertyId, PropertyId name, FunctionInfo * functionInfo, int length)
    {
        RuntimeFunction* function = DefaultCreateFunction(functionInfo, length, nullptr, nullptr, name);
        AddMember(object, propertyId, function);
        return function;
    }
    void JavascriptLibrary::AddAccessorsToLibraryObject(DynamicObject* object, PropertyId propertyId, FunctionInfo * getterFunctionInfo, FunctionInfo * setterFunctionInfo)
    {
        AddAccessorsToLibraryObjectWithName(object, propertyId, propertyId, getterFunctionInfo, setterFunctionInfo);
    }

    void JavascriptLibrary::AddAccessorsToLibraryObjectWithName(DynamicObject* object, PropertyId propertyId, PropertyId name, FunctionInfo * getterFunctionInfo, FunctionInfo * setterFunctionInfo)
    {
        Js::RecyclableObject * getter;
        Js::RecyclableObject * setter;

        if (getterFunctionInfo != nullptr)
        {
            Var name_withGetPrefix = LiteralString::Concat(LiteralString::NewCopySz(L"get ", scriptContext), scriptContext->GetPropertyString(name));
            RuntimeFunction* getterFunction = DefaultCreateFunction(getterFunctionInfo, 0, nullptr, nullptr, name_withGetPrefix);
            getterFunction->SetPropertyWithAttributes(PropertyIds::length, TaggedInt::ToVarUnchecked(0), PropertyNone, nullptr);
            getter = getterFunction;
        }
        else
        {
            getter = GetUndefined();
        }

        if (setterFunctionInfo != nullptr)
        {
            Var name_withSetPrefix = LiteralString::Concat(LiteralString::NewCopySz(L"set ", scriptContext), scriptContext->GetPropertyString(name));
            RuntimeFunction* setterFunction = DefaultCreateFunction(setterFunctionInfo, 0, nullptr, nullptr, name_withSetPrefix);
            setterFunction->SetPropertyWithAttributes(PropertyIds::length, TaggedInt::ToVarUnchecked(1), PropertyNone, nullptr);
            setter = setterFunction;
        }
        else
        {
            setter = GetUndefined();
        }
        object->SetAccessors(propertyId, getter, setter);
        object->SetAttributes(propertyId, PropertyConfigurable | PropertyWritable);
    }

    void JavascriptLibrary::AddMember(DynamicObject* object, PropertyId propertyId, Var value)
    {
        AddMember(object, propertyId, value, PropertyBuiltInMethodDefaults);
    }

    void JavascriptLibrary::AddMember(DynamicObject* object, PropertyId propertyId, Var value, PropertyAttributes attributes)
    {
        // NOTE: Assume all built in member doesn't have side effect
        object->SetPropertyWithAttributes(propertyId, value, attributes, nullptr, PropertyOperation_None, SideEffects_None);
    }

    void JavascriptLibrary::InitializeJSONObject(DynamicObject* JSONObject, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(JSONObject, mode, 3);
        JavascriptLibrary* library = JSONObject->GetLibrary();
        JSONObject->GetScriptContext()->SetBuiltInLibraryFunction(JSON::EntryInfo::Stringify.GetOriginalEntryPoint(),
        library->AddFunctionToLibraryObject(JSONObject, PropertyIds::stringify, &JSON::EntryInfo::Stringify, 3));
        library->AddFunctionToLibraryObject(JSONObject, PropertyIds::parse, &JSON::EntryInfo::Parse, 2);

        if (JSONObject->GetScriptContext()->GetConfig()->IsES6ToStringTagEnabled())
        {
            library->AddMember(JSONObject, PropertyIds::_symbolToStringTag, JSONObject->GetLibrary()->CreateStringFromCppLiteral(L"JSON"), PropertyConfigurable);
        }

        JSONObject->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeEngineInterfaceObject(DynamicObject* engineInterface, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(engineInterface, mode, 3);

        EngineInterfaceObject::FromVar(engineInterface)->Initialize();

        engineInterface->SetHasNoEnumerableProperties(true);
    }

    void JavascriptLibrary::InitializeWinRTPromiseConstructor()
    {
        Assert(this->engineInterfaceObject != nullptr);

        this->winRTPromiseConstructor = JavascriptFunction::FromVar(this->engineInterfaceObject->GetPromiseConstructor(scriptContext));
    }

    JavascriptFunction* JavascriptLibrary::GetWinRTPromiseConstructor()
    {
        if (this->winRTPromiseConstructor == nullptr)
        {
            this->InitializeWinRTPromiseConstructor();
        }

        Assert(this->winRTPromiseConstructor != nullptr);

        return this->winRTPromiseConstructor;
    }

    void JavascriptLibrary::InitializeHostPromiseContinuationFunction()
    {
        // TODO: Below loads and returns WScript.SetTimeout or window.setTimeout. Later, we should instead use the task queue.
        // NOTE: The code here is placeholder until we get the task queue from trident.
        //       If user code changes WScript.SetTimeout or window.setTimeout, the Promise feature won't work!
        PropertyId windowId = scriptContext->GetOrAddPropertyIdTracked(L"window");
        PropertyId setTimeoutId = scriptContext->GetOrAddPropertyIdTracked(L"setTimeout");
        Var global = this->GetGlobalObject();
        Var window;
        Var setTimeout;

        // Try to load window.setTimeout
        if (JavascriptOperators::GetRootProperty(global, windowId, &window, scriptContext) &&
            RecyclableObject::Is(window) &&
            JavascriptOperators::GetProperty(RecyclableObject::FromVar(window), setTimeoutId, &setTimeout, scriptContext) &&
            JavascriptConversion::IsCallable(setTimeout))
        {
            this->hostPromiseContinuationFunction = JavascriptFunction::FromVar(setTimeout);
            return;
        }
        else if (JavascriptOperators::GetRootProperty(global, setTimeoutId, &setTimeout, scriptContext) &&
                 JavascriptConversion::IsCallable(setTimeout))
        {
            // Workers do not have a window property, but they do have setTimeout on their global
            this->hostPromiseContinuationFunction = JavascriptFunction::FromVar(setTimeout);
            return;
        }

        PropertyId wscriptId = scriptContext->GetOrAddPropertyIdTracked(L"WScript");
        setTimeoutId = scriptContext->GetOrAddPropertyIdTracked(L"SetTimeout");
        Var wscript;

        // Try to load WScript.SetTimeout
        if (JavascriptOperators::GetRootProperty(global, wscriptId, &wscript, scriptContext) &&
            RecyclableObject::Is(wscript) &&
            JavascriptOperators::GetProperty(RecyclableObject::FromVar(wscript), setTimeoutId, &setTimeout, scriptContext) &&
            JavascriptConversion::IsCallable(setTimeout))
        {
            this->hostPromiseContinuationFunction = JavascriptFunction::FromVar(setTimeout);
            return;
        }

        // If we couldn't load either WScript.SetTimeout or window.setTimeout the Promise feature is not going to work.
        // We do need to use some kind of function here, so let's just use a thrower.
        this->hostPromiseContinuationFunction = this->throwerFunction;
    }

    JavascriptFunction* JavascriptLibrary::GetHostPromiseContinuationFunction()
    {
        if (this->hostPromiseContinuationFunction == nullptr)
        {
            this->InitializeHostPromiseContinuationFunction();
        }

        return this->hostPromiseContinuationFunction;
    }

    void JavascriptLibrary::SetNativeHostPromiseContinuationFunction(PromiseContinuationCallback function, void *state)
    {
        this->nativeHostPromiseContinuationFunction = function;
        this->nativeHostPromiseContinuationFunctionState = state;
    }

    void JavascriptLibrary::EnqueueTask(Var taskVar)
    {
        // TODO: This function should take a parameter to declare the Task Queue in which to enqueue taskVar.
        //       Currently, this is only called for PromiseTasks.
        // Assert(JavascriptPromiseReactionTaskFunction::Is(taskVar) || JavascriptPromiseResolveThenableTaskFunction::Is(taskVar));
        // UPDATE: Also used by builtin EngineInterfaceObject.Promise.enqueueTask to postError.
        Assert(JavascriptFunction::Is(taskVar));

        if (this->nativeHostPromiseContinuationFunction)
        {
            BEGIN_LEAVE_SCRIPT(scriptContext);
            try
            {
                this->nativeHostPromiseContinuationFunction(taskVar, this->nativeHostPromiseContinuationFunctionState);
            }
            catch (...)
            {
                // Hosts are required not to pass exceptions back across the callback boundary. If
                // this happens, it is a bug in the host, not something that we are expected to
                // handle gracefully.
                Js::Throw::FatalInternalError();
            }
            END_LEAVE_SCRIPT(scriptContext);
        }
        else
        {
            JavascriptFunction* hostPromiseContinuationFunction = this->GetHostPromiseContinuationFunction();

            // TODO: This should EnqueueTask a new PromiseReactionTask in the ES6 PromiseTasks task queue.
            //       Currently, the library is loading WScript.SetTimeout (or window.setTimeout) and we are calling that here.
            //       We should change the layout of this call to match whatever trident ends up building.
            hostPromiseContinuationFunction->GetEntryPoint()(hostPromiseContinuationFunction, Js::CallInfo(Js::CallFlags::CallFlags_Value, 3),
                this->GetUndefined(),
                taskVar,
                JavascriptNumber::ToVar(0, scriptContext));
        }
    }

#ifdef ENABLE_INTL_OBJECT
    void JavascriptLibrary::ResetIntlObject()
    {
        IntlObject = DynamicObject::New(recycler,
            DynamicType::New(scriptContext, TypeIds_Object, objectPrototype, nullptr,
            DeferredTypeHandler<InitializeIntlObject>::GetDefaultInstance()));
    }

    void JavascriptLibrary::EnsureIntlObjectReady()
    {
        Assert(this->IntlObject != nullptr);

        // For Intl builtins, we need to make sure the Intl object has been initialized before fetching the
        // builtins from the EngineInterfaceObject. This is because the builtins are actually created via
        // Intl.js and are registered into the EngineInterfaceObject as part of Intl object initialization.
        try
        {
            this->IntlObject->EnsureObjectReady();
        }
        catch (JavascriptExceptionObject *e)
        {
            // Propogate the OOM and SOE exceptions only
            if(e == ThreadContext::GetContextForCurrentThread()->GetPendingOOMErrorObject() ||
                e == ThreadContext::GetContextForCurrentThread()->GetPendingSOErrorObject())
            {
                e = e->CloneIfStaticExceptionObject(scriptContext);
                throw e;
            }
        }
    }

    void JavascriptLibrary::InitializeIntlObject(DynamicObject* IntlObject, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(IntlObject, mode,  /*initSlotCapacity*/ 2);

        ScriptContext* scriptContext = IntlObject->GetScriptContext();
        if (scriptContext->VerifyAlive()) // Can't initialize if scriptContext closed, will need to run script
        {
            JavascriptLibrary* library = IntlObject->GetLibrary();
            Assert(library->engineInterfaceObject != nullptr);
            library->engineInterfaceObject->InjectIntlLibraryCode(scriptContext, IntlObject);
        }
    }
#endif

    void JavascriptLibrary::SetProfileMode(bool fSet)
    {
        inProfileMode = fSet;
    }

    void JavascriptLibrary::SetDispatchProfile(bool fSet, JavascriptMethod dispatchInvoke)
    {
        if (!fSet)
        {
            this->inDispatchProfileMode = false;
            this->SetDispatchInvoke(dispatchInvoke);
            idMappedFunctionWithPrototypeType->SetEntryPoint(JavascriptExternalFunction::ExternalFunctionThunk);
            externalFunctionWithDeferredPrototypeType->SetEntryPoint(JavascriptExternalFunction::ExternalFunctionThunk);
            stdCallFunctionWithDeferredPrototypeType->SetEntryPoint(JavascriptExternalFunction::StdCallExternalFunctionThunk);
        }
        else
        {
            this->inDispatchProfileMode = true;
            this->SetDispatchInvoke(dispatchInvoke);
            idMappedFunctionWithPrototypeType->SetEntryPoint(ProfileEntryThunk);
            externalFunctionWithDeferredPrototypeType->SetEntryPoint(ProfileEntryThunk);
            stdCallFunctionWithDeferredPrototypeType->SetEntryPoint(ProfileEntryThunk);
        }
    }
    JavascriptString* JavascriptLibrary::CreateEmptyString()
    {
        return LiteralString::CreateEmptyString(stringTypeStatic);
    }

    JavascriptRegExp* JavascriptLibrary::CreateEmptyRegExp()
    {
        UnifiedRegex::RegexPattern* pattern = RegexHelper::CompileDynamic(scriptContext, L"", 0, L"", 0, false);
        return RecyclerNew(scriptContext->GetRecycler(), JavascriptRegExp, pattern,
                           this->GetRegexType());
    }

    void JavascriptLibrary::SetDispatchInvoke(Js::JavascriptMethod dispatchInvoke)
    {
        if (this->dispMemberProxyType != nullptr)
        {
            this->dispMemberProxyType->SetDispatchInvoke(dispatchInvoke);
        }
    }

    void JavascriptLibrary::SetCrossSiteForSharedFunctionType(JavascriptFunction * function, bool useSlotAccessCrossSiteThunk)
    {
        Assert(function->GetDynamicType()->GetIsShared());

        if (useSlotAccessCrossSiteThunk)
        {
            Assert(!ScriptFunction::Is(function));
            Assert(VirtualTableInfo<JavascriptTypedObjectSlotAccessorFunction>::HasVirtualTable(function));
            Assert((function->GetFunctionInfo()->GetAttributes() & Js::FunctionInfo::Attributes::NeedCrossSiteSecurityCheck) != 0);
            Assert(function->GetDynamicType()->GetTypeHandler() == functionTypeHandler);
            function->ChangeType();
            function->SetEntryPoint(scriptContext->GetHostScriptContext()->GetSimpleSlotAccessCrossSiteThunk());
        }
        else if (ScriptFunction::Is(function))
        {
            Assert(!function->GetFunctionInfo()->IsLambda() ?
                function->GetDynamicType()->GetTypeHandler() == JavascriptLibrary::GetDeferredPrototypeFunctionTypeHandler(this->GetScriptContext()) :
                function->GetDynamicType()->GetTypeHandler() == JavascriptLibrary::GetDeferredFunctionTypeHandler());
            // TODO: Theoratically, we can share the cross site thunk here too, but the entry point
            // is a mess :(,   just create a new unshared type and set the cross site thunk
            function->ChangeType();
            function->SetEntryPoint(scriptContext->CurrentCrossSiteThunk);
        }
        else if (BoundFunction::Is(function))
        {
            function->ChangeType();
            function->SetEntryPoint(scriptContext->CurrentCrossSiteThunk);
        }
        else
        {
            DynamicTypeHandler * typeHandler = function->GetDynamicType()->GetTypeHandler();
            if (typeHandler == JavascriptLibrary::GetDeferredPrototypeFunctionTypeHandler(this->GetScriptContext()))
            {
                function->ReplaceType(crossSiteDeferredPrototypeFunctionType);
            }
            else if (typeHandler == Js::DeferredTypeHandler<Js::JavascriptExternalFunction::DeferredInitializer>::GetDefaultInstance())
            {
                function->ReplaceType(crossSiteExternalConstructFunctionWithPrototypeType);
            }
            else
            {
                Assert(typeHandler == &SharedIdMappedFunctionWithPrototypeTypeHandler);
                function->ReplaceType(crossSiteIdMappedFunctionWithPrototypeType);
            }
        }
    }

    JavascriptExternalFunction*
    JavascriptLibrary::CreateExternalFunction(JavascriptMethod entryPoint, PropertyId nameId, Var signature)
    {
        Assert(nameId == 0 || scriptContext->IsTrackedPropertyId(nameId));
        return CreateExternalFunction(entryPoint, TaggedInt::ToVarUnchecked(nameId), signature);
    }

    JavascriptExternalFunction*
    JavascriptLibrary::CreateExternalFunction(JavascriptMethod entryPoint, Var nameId, Var signature)
    {
        JavascriptExternalFunction* function = EnsureReadyIfHybridDebugging(this->CreateIdMappedExternalFunction(entryPoint, externalFunctionWithDeferredPrototypeType));
        function->SetFunctionNameId(nameId);
        function->SetSignature(signature);
#ifdef HEAP_ENUMERATION_VALIDATION
        function->SetHeapEnumValidationCookie(HEAP_ENUMERATION_LIBRARY_OBJECT_COOKIE);
#endif

#ifdef F_JSETW
        JSETW(EventWriteJSCRIPT_BUILD_DIRECT_FUNCTION(scriptContext, function, nameId));
#endif
#if DBG_DUMP
        if (Js::Configuration::Global.flags.Trace.IsEnabled(Js::HostPhase))
        {
            Output::Print(L"Create new external function: methodAddr= %p, propertyRecord= %p, propertyName= %s\n",
                this, nameId,
                TaggedInt::Is(nameId) ? scriptContext->GetThreadContext()->GetPropertyName(TaggedInt::ToInt32(nameId))->GetBuffer() : ((JavascriptString *)nameId)->GetString());
        }
#endif
        return function;
    }

    RuntimeFunction*
    JavascriptLibrary::CloneBuiltinFunction(RuntimeFunction* func)
    {
        FunctionInfo* funcInfo = func->GetFunctionInfo();
        Var length = nullptr;
        Var prototype = nullptr;
        PropertyValueInfo info;

        BOOL builtinFuncHasLength = func->GetProperty(func, PropertyIds::length, &length, &info, scriptContext);
        BOOL builtinFuncHasPrototype = func->GetProperty(func, PropertyIds::prototype, &prototype, &info, scriptContext);

        Assert(builtinFuncHasLength);

        return scriptContext->GetLibrary()->DefaultCreateFunction(funcInfo, TaggedInt::ToInt32(length),
            builtinFuncHasPrototype ? DynamicObject::FromVar(prototype) : nullptr, nullptr, func->functionNameId);
    }

    void JavascriptLibrary::EnsureStringTemplateCallsiteObjectList()
    {
        if (this->stringTemplateCallsiteObjectList == nullptr)
        {
            this->stringTemplateCallsiteObjectList = RecyclerNew(GetRecycler(), StringTemplateCallsiteObjectList, GetRecycler());
        }
    }

    void JavascriptLibrary::AddStringTemplateCallsiteObject(RecyclableObject* callsite)
    {
        this->EnsureStringTemplateCallsiteObjectList();

        RecyclerWeakReference<RecyclableObject>* callsiteRef = this->GetRecycler()->CreateWeakReferenceHandle<RecyclableObject>(callsite);

        this->stringTemplateCallsiteObjectList->Item(callsiteRef);
    }

    RecyclableObject* JavascriptLibrary::TryGetStringTemplateCallsiteObject(ParseNodePtr pnode)
    {
        this->EnsureStringTemplateCallsiteObjectList();

        RecyclerWeakReference<RecyclableObject>* callsiteRef = this->stringTemplateCallsiteObjectList->LookupWithKey(pnode);

        if (callsiteRef)
        {
            RecyclableObject* callsite = callsiteRef->Get();

            if (callsite)
            {
                return callsite;
            }
        }

        return nullptr;
    }

    RecyclableObject* JavascriptLibrary::TryGetStringTemplateCallsiteObject(RecyclableObject* callsite)
    {
        this->EnsureStringTemplateCallsiteObjectList();

        RecyclerWeakReference<RecyclableObject>* callsiteRef = this->GetRecycler()->CreateWeakReferenceHandle<RecyclableObject>(callsite);
        RecyclerWeakReference<RecyclableObject>* existingCallsiteRef = this->stringTemplateCallsiteObjectList->LookupWithKey(callsiteRef);

        if (existingCallsiteRef)
        {
            RecyclableObject* existingCallsite = existingCallsiteRef->Get();

            if (existingCallsite)
            {
                return existingCallsite;
            }
        }

        return nullptr;
    }

#if DBG_DUMP
    const wchar_t* JavascriptLibrary::GetStringTemplateCallsiteObjectKey(Var callsite)
    {
        // Calculate the key for the string template callsite object.
        // Key is combination of the raw string literals delimited by '${}' since string template literals cannot include that symbol.
        // `str1${expr1}str2${expr2}str3` => "str1${}str2${}str3"

        ES5Array* callsiteObj = ES5Array::FromVar(callsite);
        ScriptContext* scriptContext = callsiteObj->GetScriptContext();

        Var var = JavascriptOperators::OP_GetProperty(callsiteObj, Js::PropertyIds::raw, scriptContext);
        ES5Array* rawArray = ES5Array::FromVar(var);
        uint32 arrayLength = rawArray->GetLength();
        uint32 totalStringLength = 0;
        JavascriptString* str;

        Assert(arrayLength != 0);

        // Count the size in characters of the raw strings
        for (uint32 i = 0; i < arrayLength; i++)
        {
            rawArray->DirectGetItemAt(i, &var);
            str = JavascriptString::FromVar(var);
            totalStringLength += str->GetLength();
        }

        uint32 keyLength = totalStringLength + (arrayLength - 1) * 3 + 1;
        wchar_t* key = RecyclerNewArray(scriptContext->GetRecycler(), wchar_t, keyLength);
        wchar_t* ptr = key;
        charcount_t remainingSpace = keyLength;

        // Get first item before loop - there always must be at least one item
        rawArray->DirectGetItemAt(0, &var);
        str = JavascriptString::FromVar(var);

        charcount_t len = str->GetLength();
        js_wmemcpy_s(ptr, remainingSpace, str->GetSz(), len);
        ptr += len;
        remainingSpace -= len;

        // Append a delimiter and the rest of the items
        for (uint32 i = 1; i < arrayLength; i++)
        {
            len = 3; // strlen(L"${}");
            js_wmemcpy_s(ptr, remainingSpace, L"${}", len);
            ptr += len;
            remainingSpace -= len;

            rawArray->DirectGetItemAt(i, &var);
            str = JavascriptString::FromVar(var);

            len = str->GetLength();
            js_wmemcpy_s(ptr, remainingSpace, str->GetSz(), len);
            ptr += len;
            remainingSpace -= len;
        }

        // Ensure string is terminated
        key[keyLength - 1] = L'\0';

        return key;
    }
#endif

    bool StringTemplateCallsiteObjectComparer<ParseNodePtr>::Equals(ParseNodePtr x, RecyclerWeakReference<Js::RecyclableObject>* y)
    {
        Assert(x != nullptr);
        Assert(x->nop == knopStrTemplate);

        Js::RecyclableObject* obj = y->Get();

        // If the weak reference is dead, we can't be equal.
        if (obj == nullptr)
        {
            return false;
        }

        Js::ES5Array* callsite = Js::ES5Array::FromVar(obj);
        uint32 length = callsite->GetLength();
        Js::Var element;
        Js::JavascriptString* str;
        IdentPtr pid;

        // If the length of string literals is different, these callsite objects are not equal.
        if (x->sxStrTemplate.countStringLiterals != length)
        {
            return false;
        }

        element = Js::JavascriptOperators::OP_GetProperty(callsite, Js::PropertyIds::raw, callsite->GetScriptContext());
        Js::ES5Array* rawArray = Js::ES5Array::FromVar(element);

        // Length of the raw strings should be the same as the cooked string literals.
        Assert(length == rawArray->GetLength());

        x = x->sxStrTemplate.pnodeStringRawLiterals;

        Assert(length != 0);

        for (uint32 i = 0; i < length - 1; i++)
        {
            rawArray->DirectGetItemAt(i, &element);
            str = Js::JavascriptString::FromVar(element);

            Assert(x->nop == knopList);
            Assert(x->sxBin.pnode1->nop == knopStr);

            pid = x->sxBin.pnode1->sxPid.pid;

            // If strings have different length, they aren't equal
            if (pid->Cch() != str->GetLength())
            {
                return false;
            }

            // If the strings at this index are not equal, the callsite objects are not equal.
            if (!JsUtil::CharacterBuffer<wchar_t>::StaticEquals(pid->Psz(), str->GetSz(), str->GetLength()))
            {
                return false;
            }

            x = x->sxBin.pnode2;
        }

        // There should be one more string in the callsite array - and the final string in the ParseNode

        rawArray->DirectGetItemAt(length - 1, &element);
        str = Js::JavascriptString::FromVar(element);

        Assert(x->nop == knopStr);
        pid = x->sxPid.pid;

        // If strings have different length, they aren't equal
        if (pid->Cch() != str->GetLength())
        {
            return false;
        }

        // If the strings at this index are not equal, the callsite objects are not equal.
        if (!JsUtil::CharacterBuffer<wchar_t>::StaticEquals(pid->Psz(), str->GetSz(), str->GetLength()))
        {
            return false;
        }

        return true;
    }

    bool StringTemplateCallsiteObjectComparer<ParseNodePtr>::Equals(ParseNodePtr x, ParseNodePtr y)
    {
        Assert(x != nullptr && y != nullptr);
        Assert(x->nop == knopStrTemplate && y->nop == knopStrTemplate);

        // If the ParseNode is the same, they are equal.
        if (x == y)
        {
            return true;
        }

        x = x->sxStrTemplate.pnodeStringRawLiterals;
        y = y->sxStrTemplate.pnodeStringRawLiterals;

        // If one of the templates only includes one string value, the raw literals ParseNode will
        // be a knopStr instead of knopList.
        if (x->nop != y->nop)
        {
            return false;
        }

        const wchar_t* pid_x;
        const wchar_t* pid_y;

        while (x->nop == knopList)
        {
            // If y is knopStr here, that means x has more strings in the list than y does.
            if (y->nop != knopList)
            {
                return false;
            }

            Assert(x->sxBin.pnode1->nop == knopStr);
            Assert(y->sxBin.pnode1->nop == knopStr);

            pid_x = x->sxBin.pnode1->sxPid.pid->Psz();
            pid_y = y->sxBin.pnode1->sxPid.pid->Psz();

            // If the pid values of each raw string don't match each other, these are different.
            if (!DefaultComparer<const wchar_t*>::Equals(pid_x, pid_y))
            {
                return false;
            }

            x = x->sxBin.pnode2;
            y = y->sxBin.pnode2;
        }

        // If y is still knopList here, that means y has more strings in the list than x does.
        if (y->nop != knopStr)
        {
            return false;
        }

        Assert(x->nop == knopStr);

        pid_x = x->sxPid.pid->Psz();
        pid_y = y->sxPid.pid->Psz();

        // This is the final string in the raw literals list. Return true if they are equal.
        return DefaultComparer<const wchar_t*>::Equals(pid_x, pid_y);
    }

    hash_t StringTemplateCallsiteObjectComparer<ParseNodePtr>::GetHashCode(ParseNodePtr i)
    {
        hash_t hash = 0;

        Assert(i != nullptr);
        Assert(i->nop == knopStrTemplate);

        i = i->sxStrTemplate.pnodeStringRawLiterals;

        const wchar_t* pid;

        while (i->nop == knopList)
        {
            Assert(i->sxBin.pnode1->nop == knopStr);

            pid = i->sxBin.pnode1->sxPid.pid->Psz();

            hash ^= DefaultComparer<const wchar_t*>::GetHashCode(pid);
            hash ^= DefaultComparer<const wchar_t*>::GetHashCode(L"${}");

            i = i->sxBin.pnode2;
        }

        Assert(i->nop == knopStr);

        pid = i->sxPid.pid->Psz();

        hash ^= DefaultComparer<const wchar_t*>::GetHashCode(pid);

        return hash;
    }

    bool StringTemplateCallsiteObjectComparer<RecyclerWeakReference<Js::RecyclableObject>*>::Equals(RecyclerWeakReference<Js::RecyclableObject>* x, ParseNodePtr y)
    {
        return StringTemplateCallsiteObjectComparer<ParseNodePtr>::Equals(y, x);
    }

    bool StringTemplateCallsiteObjectComparer<RecyclerWeakReference<Js::RecyclableObject>*>::Equals(RecyclerWeakReference<Js::RecyclableObject>* x, RecyclerWeakReference<Js::RecyclableObject>* y)
    {
        Js::RecyclableObject* objLeft = x->Get();
        Js::RecyclableObject* objRight = y->Get();

        // If either WeakReference is dead, we can't be equal to anything.
        if (objLeft == nullptr || objRight == nullptr)
        {
            return false;
        }

        // If the Var pointers are the same, they are equal.
        if (objLeft == objRight)
        {
            return true;
        }

        Js::ES5Array* arrayLeft = Js::ES5Array::FromVar(objLeft);
        Js::ES5Array* arrayRight = Js::ES5Array::FromVar(objRight);
        uint32 lengthLeft = arrayLeft->GetLength();
        uint32 lengthRight = arrayRight->GetLength();
        Js::Var varLeft;
        Js::Var varRight;

        // If the length of string literals is different, these callsite objects are not equal.
        if (lengthLeft != lengthRight)
        {
            return false;
        }

        Assert(lengthLeft != 0 && lengthRight != 0);

        // Change to the set of raw strings.
        varLeft = Js::JavascriptOperators::OP_GetProperty(arrayLeft, Js::PropertyIds::raw, arrayLeft->GetScriptContext());
        arrayLeft = Js::ES5Array::FromVar(varLeft);

        varRight = Js::JavascriptOperators::OP_GetProperty(arrayRight, Js::PropertyIds::raw, arrayRight->GetScriptContext());
        arrayRight = Js::ES5Array::FromVar(varRight);

        // Length of the raw strings should be the same as the cooked string literals.
        Assert(lengthLeft == arrayLeft->GetLength());
        Assert(lengthRight == arrayRight->GetLength());

        for (uint32 i = 0; i < lengthLeft; i++)
        {
            arrayLeft->DirectGetItemAt(i, &varLeft);
            arrayRight->DirectGetItemAt(i, &varRight);

            // If the strings at this index are not equal, the callsite objects are not equal.
            if (!Js::JavascriptString::Equals(varLeft, varRight))
            {
                return false;
            }
        }

        return true;
    }

    hash_t StringTemplateCallsiteObjectComparer<RecyclerWeakReference<Js::RecyclableObject>*>::GetHashCode(RecyclerWeakReference<Js::RecyclableObject>* o)
    {
        hash_t hash = 0;

        Js::RecyclableObject* obj = o->Get();

        if (obj == nullptr)
        {
            return hash;
        }

        Js::ES5Array* callsite = Js::ES5Array::FromVar(obj);
        Js::Var var = Js::JavascriptOperators::OP_GetProperty(callsite, Js::PropertyIds::raw, callsite->GetScriptContext());
        Js::ES5Array* rawArray = Js::ES5Array::FromVar(var);

        Assert(rawArray->GetLength() > 0);

        rawArray->DirectGetItemAt(0, &var);
        Js::JavascriptString* str = Js::JavascriptString::FromVar(var);
        hash ^= DefaultComparer<const wchar_t*>::GetHashCode(str->GetSz());

        for (uint32 i = 1; i < rawArray->GetLength(); i++)
        {
            hash ^= DefaultComparer<const wchar_t*>::GetHashCode(L"${}");

            rawArray->DirectGetItemAt(i, &var);
            str = Js::JavascriptString::FromVar(var);
            hash ^= DefaultComparer<const wchar_t*>::GetHashCode(str->GetSz());
        }

        return hash;
    }
}
