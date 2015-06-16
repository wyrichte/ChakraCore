//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
#if DBG
EXTERN_C Js::JavascriptMethod checkCodeGenThunk;
#endif

namespace Js
{
#if _M_X64
   extern "C" Var amd64_CallFunction(RecyclableObject *function, JavascriptMethod entryPoint, CallInfo callInfo, uint argc, Var *argv);
#endif

    class JavascriptFunction : public DynamicObject
    {
    private:
        static PropertyId specialPropertyIds[];

        // Need a constructor cache on every function (script and native) to avoid extra checks on the fast path, if the function isn't fixed.
        ConstructorCache* constructorCache;

    protected:

        FunctionInfo * functionInfo;  // Underlying function


        DEFINE_VTABLE_CTOR(JavascriptFunction, DynamicObject);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(JavascriptFunction);

    private:
         // noinline, we want to use own stack frame.
        __declspec(noinline) JavascriptFunction* FindCaller(BOOL* foundThis, JavascriptFunction* nullValue, ScriptContext* requestContext);

        BOOL GetCallerProperty(Var originalInstance, Var* value, ScriptContext* requestContext);
        BOOL GetArgumentsProperty(Var originalInstance, Var* value, ScriptContext* requestContext);

        bool GetPropertyBuiltIns(Var originalInstance, PropertyId propertyId, Var* value, ScriptContext* requestContext, BOOL* result);
        bool GetSetterBuiltIns(PropertyId propertyId, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext, DescriptorFlags* descriptorFlags);

        void InvalidateConstructorCacheOnPrototypeChange();

        static const charcount_t DIAG_MAX_FUNCTION_STRING = 256;

    protected:
        static Var NewInstanceHelper(ScriptContext *scriptContext, RecyclableObject* function, CallInfo callInfo, Js::ArgumentReader& args, bool isGenerator = false);

        JavascriptFunction(DynamicType * type);
    public:
        JavascriptFunction(DynamicType * type, FunctionInfo * functionInfo);
        JavascriptFunction(DynamicType * type, FunctionInfo * functionInfo, ConstructorCache* cache);

        class EntryInfo
        {
        public:
            static FunctionInfo NewInstance;
            static FunctionInfo PrototypeEntryPoint;
            static FunctionInfo Apply;
            static FunctionInfo Bind;
            static FunctionInfo Call;
            static FunctionInfo ToString;
            static FunctionInfo ToMethod;
        };

        static const int numberLinesPrependedToAnonymousFunction = 1;

        static DWORD GetFunctionInfoOffset() { return offsetof(JavascriptFunction, functionInfo); }

        static Var NewInstance(RecyclableObject* function, CallInfo callInfo, ...);
        static Var NewInstanceRestrictedMode(RecyclableObject* function, CallInfo callInfo, ...);
        static Var PrototypeEntryPoint(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryApply(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryBind(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryToMethod(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryCall(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryToString(RecyclableObject* function, CallInfo callInfo, ...);

        static bool Is(Var aValue);
        static JavascriptFunction* FromVar(Var aValue);
        Var CallFunction(Arguments args);
        Var CallRootFunction(Arguments args, ScriptContext * scriptContext);
        Var CallRootFunctionInternal(Arguments args, ScriptContext * scriptContext);
        template <typename T>
        static T CallAsmJsFunction(RecyclableObject * function, void* entryPoint, uint argc, Var * argv);
        template <bool isConstruct>
        static Var CalloutHelper(RecyclableObject* function, Var thisArg, Var argArray, ScriptContext* scriptContext);

        static Var ApplyHelper(RecyclableObject* function, Var thisArg, Var argArray, ScriptContext* scriptContext);
        static Var ConstructHelper(RecyclableObject* function, Var thisArg, Var argArray, ScriptContext* scriptContext);
        static Var CallRootFunctionInScript(JavascriptFunction* func, Arguments args);

        static Var CallAsConstructor(Var v, Arguments args, ScriptContext* scriptContext, const Js::AuxArray<uint32> *spreadIndices = nullptr);
        static Var FinishConstructor(const Var constructorReturnValue, Var newObject, JavascriptFunction *const function);

        static BOOL IsConstructor(Var obj);
#if DBG
        static void CheckValidDebugThunk(ScriptContext* scriptContext, RecyclableObject *function);
#endif
        template <bool doStackProbe>
        static Var CallFunction(RecyclableObject* obj, JavascriptMethod entryPoint, Arguments args);
        static Var CallSpreadFunction(RecyclableObject* obj, JavascriptMethod entryPoint, Arguments args, const Js::AuxArray<uint32> *spreadIndices);
        static uint32 GetSpreadSize(const Arguments args, const Js::AuxArray<uint32> *spreadIndices, ScriptContext *scriptContext);
        static void SpreadArgs(const Arguments args, Arguments& destArgs, const Js::AuxArray<uint32> *spreadIndices, ScriptContext *scriptContext);
        static Var EntrySpreadCall(const Js::AuxArray<uint32> *spreadIndices, RecyclableObject* function, CallInfo callInfo, ...);
        static void CheckAlignment();
#ifdef ENABLE_NATIVE_CODEGEN
        static BOOL IsNativeAddress(ScriptContext * scriptContext, void * codeAddr);
#endif
        // TODO: Move these to ScriptFunction
        static Var DeferredParsingThunk(RecyclableObject* function, CallInfo callInfo, ...);
        static JavascriptMethod DeferredParse(ScriptFunction** function);
        static JavascriptMethod DeferredParseCore(ScriptFunction** function, BOOL &fParsed);
        static void ReparseAsmJsModule(ScriptFunction ** function);
        static Var DeferredDeserializeThunk(RecyclableObject* function, CallInfo callInfo, ...);
        static JavascriptMethod DeferredDeserialize(ScriptFunction* function);

        static BYTE GetOffsetOfFunctionInfo()
        {
            CompileAssert(offsetof(JavascriptFunction, functionInfo) <= UCHAR_MAX);
            return offsetof(JavascriptFunction, functionInfo);
        }
        static uint32 GetOffsetOfConstructorCache() { return offsetof(JavascriptFunction, constructorCache); };

        static JavascriptString* GetNativeFunctionDisplayString(ScriptContext *scriptContext, JavascriptString *name);
        static JavascriptString* GetLibraryCodeDisplayString(ScriptContext* scriptContext, PCWSTR displayName);

        template <class StringHelper, class String, class ScriptContext>
        static String GetNativeFunctionDisplayStringCommon(ScriptContext* scriptContext, String name);
        template <class StringHelper, class String, class ScriptContext>
        static String GetLibraryCodeDisplayStringCommon(ScriptContext* scriptContext, PCWSTR displayName);

        FunctionInfo * GetFunctionInfo() const { return functionInfo; }
        void SetFunctionInfo(FunctionInfo *info) { functionInfo = info; }
        FunctionProxy * GetFunctionProxy() const;
        ParseableFunctionInfo * GetParseableFunctionInfo() const;
        DeferDeserializeFunctionInfo * GetDeferDeserializeFunctionInfo() const;
        FunctionBody * GetFunctionBody() const;
        virtual JavascriptString* GetDisplayNameImpl() const;
        JavascriptString* DisplayNameHelper(const wchar_t* name, charcount_t length) const;
        JavascriptString* GetDisplayName(bool isFunctionName = false) const;
        bool IsLibraryCode() const;

        inline BOOL IsScriptFunction() const;
        virtual Var GetSourceString() const { return null; }
        virtual Var EnsureSourceString();
        virtual BOOL IsExternalFunction() { return FALSE; }
        virtual BOOL IsWinRTFunction() { return FALSE; }
        inline BOOL IsStrictMode();
        inline BOOL IsLambda() const;
        virtual inline BOOL IsConstructor() const;

        ConstructorCache* GetConstructorCache() { Assert(this->constructorCache != null); return this->constructorCache; }
        ConstructorCache* EnsureValidConstructorCache();

        void ResetConstructorCacheToDefault();

#if DBG_DUMP
        void Dump();
#endif

        virtual bool HasReadOnlyPropertiesInvisibleToTypeHandler() override { return true; }

        virtual BOOL HasProperty(PropertyId propertyId) override;
        virtual BOOL GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL SetPropertyWithAttributes(PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags = PropertyOperation_None, SideEffects possibleSideEffects = SideEffects_Any) override;
        virtual BOOL GetAccessors(PropertyId propertyId, Var *getter, Var *setter, ScriptContext * requestContext) override;
        virtual DescriptorFlags GetSetter(PropertyId propertyId, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual DescriptorFlags GetSetter(JavascriptString* propertyNameString, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL IsConfigurable(PropertyId propertyId) override;
        virtual BOOL IsEnumerable(PropertyId propertyId) override;
        virtual BOOL IsWritable(PropertyId propertyId) override;
        virtual BOOL GetSpecialPropertyName(uint32 index, Var *propertyName, ScriptContext * requestContext) override;
        virtual uint GetSpecialPropertyCount() const override;
        virtual PropertyId* GetSpecialPropertyIds() const override;
        virtual BOOL DeleteProperty(PropertyId propertyId, PropertyOperationFlags flags) override;
        virtual BOOL GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override;
        virtual BOOL GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override;
        virtual Var GetTypeOfString(ScriptContext * requestContext) override;

        virtual BOOL HasInstance(Var instance, ScriptContext* scriptContext, IsInstInlineCache* inlineCache = NULL);
        static BOOL HasInstance(Var funcPrototype, Var instance, ScriptContext* scriptContext, IsInstInlineCache* inlineCache, JavascriptFunction* function);

        virtual JavascriptFunction* MakeCopyOnWriteObject(ScriptContext* scriptContext) override;

        // This will be overriden for the BoundFunction
        virtual bool IsBoundFunction() { return false; }

        virtual bool CloneMethod(JavascriptFunction** pnewMethod, const Var newHome) { return false; }

        static BOOL IsThrowTypeErrorFunction(JavascriptFunction* function, ScriptContext* scriptContext);

        void SetEntryPoint(JavascriptMethod method);
#if DBG
        void VerifyEntryPoint();

        inline static bool JavascriptFunction::IsBuiltinProperty(Var objectWithProperty, PropertyIds propertyId);
#endif
        private:
            int  ResumeForOutOfBoundsAsmJSArrayRefs(int exceptionCode, PEXCEPTION_POINTERS exceptionInfo);
    };
    class AsmJSInstructionDecoder
    {
    public:
        struct InstructionData
        {
        public:
            bool isLoad : 1;
            bool isFloat32 : 1;
            bool isFloat64 : 1;
            bool isInvalidInstr : 1;
            BYTE bufferReg = 0;
            BYTE dstReg = 0;
            uint instrSizeInByte = 0;
            uint64 bufferValue = 0;
            InstructionData() :isLoad(0), isFloat32(0), isFloat64(0), isInvalidInstr(0){}
        };
        struct RexByteValue
        {
        public:
            bool isR : 1;
            bool isX : 1;
            bool isW : 1;
            bool isB : 1;
            uint rexValue;
            RexByteValue() :isR(0), isX(0), isW(0), isB(0), rexValue(0){}
        };
        static InstructionData CheckValidInstr(BYTE* &pc, PEXCEPTION_POINTERS exceptionInfo, FunctionBody* funcBody);
    };
    class JavascriptFunctionSpecialProperties
    {
    public:
        inline static bool IsSpecialProperty(PropertyId id)
        {
            return id == PropertyIds::caller || id == PropertyIds::arguments;
        }
    };

    //
    // ---- implementation shared with diagnostics ----
    //
    template <class StringHelper, class String, class ScriptContext>
    String JavascriptFunction::GetNativeFunctionDisplayStringCommon(ScriptContext* scriptContext, String name)
    {
        auto library = scriptContext->GetLibrary();
        String sourceString;
        sourceString = library->CreateStringFromCppLiteral(JS_DISPLAY_STRING_FUNCTION_HEADER); //L"function "
        sourceString = StringHelper::Concat(sourceString, name);
        sourceString = StringHelper::Concat(sourceString, library->CreateStringFromCppLiteral(JS_DISPLAY_STRING_FUNCTION_BODY)); //L"() { [native code] }"
        return sourceString;
    }

    template <class StringHelper, class String, class ScriptContext>
    String JavascriptFunction::GetLibraryCodeDisplayStringCommon(ScriptContext* scriptContext, PCWSTR displayName)
    {
        String sourceString;
        if(wcscmp(displayName, Js::Constants::AnonymousFunction) == 0)
        {
            sourceString = scriptContext->GetLibrary()->GetFunctionDisplayString();
        }
        else
        {
            sourceString = GetNativeFunctionDisplayStringCommon<StringHelper>(scriptContext, StringHelper::NewCopySz(displayName, scriptContext));
        }
        return sourceString;
    }

} // namespace Js
