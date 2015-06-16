//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------

#pragma once
#define ASMMATH_BUILTIN_SIZE (32)
#define ASMARRAY_BUILTIN_SIZE (16)
#define ASMSIMD_BUILTIN_SIZE (128)
namespace Js {
    // ModuleCompiler encapsulates the compilation of an entire asm.js module. Over
    // the course of an ModuleCompiler object's lifetime, many FunctionCompiler
    // objects will be created and destroyed in sequence, one for each function in
    // the module.
    //
    // *** asm.js FFI calls ***
    //
    // asm.js allows calling out to non-asm.js via "FFI calls". The asm.js type
    // system does not place any constraints on the FFI call. In particular:
    //  - an FFI call's target is not known or speculated at module-compile time;
    //  - a single external function can be called with different signatures.
    //
    // If performance didn't matter, all FFI calls could simply box their arguments
    // and call js::Invoke. However, we'd like to be able to specialize FFI calls
    // to be more efficient in several cases:
    //
    //  - for calls to JS functions which have been jitted, we'd like to call
    //    directly into JIT code without going through C++.
    //
    //  - for calls to certain builtins, we'd like to be call directly into the C++
    //    code for the builtin without going through the general call path.
    //
    // All of this requires dynamic specialization techniques which must happen
    // after module compilation. To support this, at module-compilation time, each
    // FFI call generates a call signature according to the system ABI, as if the
    // callee was a C++ function taking/returning the same types as the caller was
    // passing/expecting. The callee is loaded from a fixed offset in the global
    // data array which allows the callee to change at runtime. Initially, the
    // callee is stub which boxes its arguments and calls js::Invoke.
    //
    // To do this, we need to generate a callee stub for each pairing of FFI callee
    // and signature. We call this pairing an "exit". For example, this code has
    // two external functions and three exits:
    //
    //  function f(global, imports) {
    //    "use asm";
    //    var foo = imports.foo;
    //    var bar = imports.bar;
    //    function g() {
    //      foo(1);      // Exit #1: (int) -> void
    //      foo(1.5);    // Exit #2: (double) -> void
    //      bar(1)|0;    // Exit #3: (int) -> int
    //      bar(2)|0;    // Exit #3: (int) -> int
    //    }
    //  }
    //
    // The ModuleCompiler maintains a hash table (ExitMap) which allows a call site
    // to add a new exit or reuse an existing one. The key is an ExitDescriptor
    // (which holds the exit pairing) and the value is an index into the
    // Vector<Exit> stored in the AsmJSModule.
    //
    // Rooting note: ModuleCompiler is a stack class that contains unrooted
    // PropertyName (JSAtom) pointers.  This is safe because it cannot be
    // constructed without a TokenStream reference.  TokenStream is itself a stack
    // class that cannot be constructed without an AutoKeepAtoms being live on the
    // stack, which prevents collection of atoms.
    //
    // ModuleCompiler is marked as rooted in the rooting analysis.  Don't add
    // non-JSAtom pointers, or this will break!
    typedef Js::Tick AsmJsCompileTime;
    namespace AsmJsLookupSource
    {
        enum Source
        {
            AsmJsModule, AsmJsFunction
        };
    }

    struct AsmJsModuleMemory
    {
        static const int32   MemoryTableBeginOffset = 0;
        // Memory is allocated in this order
        int32 mArrayBufferOffset
            , mDoubleOffset
            , mFuncOffset
            , mFFIOffset
            , mFuncPtrOffset
            , mIntOffset
            , mFloatOffset

#ifdef SIMD_JS_ENABLED
            , mSimdOffset // in SIMDValues
#endif
            ;
        int32   mMemorySize;
    };

    struct AsmJsFunctionMemory
    {
        // Register where module slots are loaded
        static const RegSlot ModuleSlotRegister = 0;
        static const RegSlot ReturnRegister = 0;

        static const RegSlot FunctionRegister = 0;
        static const RegSlot CallReturnRegister = 0;
        static const RegSlot ModuleEnvRegister = 1;
        static const RegSlot ArrayBufferRegister = 2;
        static const RegSlot ArraySizeRegister = 3;
        static const RegSlot ScriptContextBufferRegister = 4;
        //Var Return register and Module Environnement and Array Buffer
        static const int32 RequiredVarConstants = 5;
    };
    namespace AsmJsCompilation
    {
        enum Phases
        {
            Module,
            ByteCode,
            TemplateJIT,

            Phases_COUNT
        };
    };

    class AsmJsModuleCompiler
    {
        struct AsmJsModuleExport
        {
            PropertyId id;
            RegSlot location;
        };
    private:
        typedef JsUtil::BaseDictionary<PropertyId, MathBuiltin, ArenaAllocator> MathNameMap;
        typedef JsUtil::BaseDictionary<PropertyId, TypedArrayBuiltin, ArenaAllocator> ArrayNameMap;
        typedef JsUtil::BaseDictionary<PropertyId, AsmJsSymbol*, ArenaAllocator> ModuleEnvironment;
        typedef JsUtil::List<AsmJsFunc*, ArenaAllocator> ModuleFunctionArray;
        typedef JsUtil::List<AsmJsFunctionTable*, ArenaAllocator> ModuleFunctionTableArray;
        typedef JsUtil::List<AsmJsModuleExport, ArenaAllocator> ModuleExportArray;
        typedef JsUtil::Queue<AsmJsArrayView *, ArenaAllocator> ModuleArrayViewList;
        typedef AsmJsRegisterSpaceGeneric<int, 0> ModuleIntVars;
        typedef AsmJsRegisterSpaceGeneric<double, 0> ModuleDoubleVars;
        typedef AsmJsRegisterSpaceGeneric<float, 0> ModuleFloatVars;
        typedef AsmJsRegisterSpaceGeneric<AsmJsImportFunction, 0> ModuleImportFunctions;

#ifdef SIMD_JS_ENABLED
        typedef AsmJsRegisterSpaceGeneric<AsmJsSIMDValue, 0> ModuleSIMDVars;
        typedef JsUtil::BaseDictionary<PropertyId, AsmJsSIMDFunction*, ArenaAllocator> SIMDNameMap;

        inline bool LookupStdLibSIMDNameInMap   (PropertyName name, AsmJsSIMDFunction **simdFunc, SIMDNameMap* map) const;
        bool AddStandardLibrarySIMDNameInMap    (PropertyId id, AsmJsSIMDFunction* simdFunc, SIMDNameMap* map);
        
#endif
        // Keep allocator first to free Dictionnary before deleting the allocator
        ArenaAllocator                  mAllocator;
        ExclusiveContext *              mCx;
        AsmJSParser &                   mCurrentParserNode;
        PropertyName                    mModuleFunctionName;
        ParseNode *                     mModuleFunctionNode;
        MathNameMap                     mStandardLibraryMathNames;
        ArrayNameMap                    mStandardLibraryArrayNames;
        ModuleEnvironment               mModuleEnvironment;
        PropertyName                    mStdLibArgName, mForeignArgName, mBufferArgName;
        ModuleFunctionArray             mFunctionArray;
        ModuleIntVars                   mIntVarSpace;
        ModuleDoubleVars                mDoubleVarSpace;
        ModuleFloatVars                 mFloatVarSpace;
        ModuleImportFunctions           mImportFunctions;
#ifdef SIMD_JS_ENABLED
        // Maps functions names to func symbols. Three maps since names are not unique across SIMD types (e.g. SIMD.{float32x4|int32x4}.add)
        // Also used to find if an operation is supported on a SIMD type. 
        SIMDNameMap                         mStdLibSIMDInt32x4Map;
        SIMDNameMap                         mStdLibSIMDFloat32x4Map;
        SIMDNameMap                         mStdLibSIMDFloat64x2Map;
        // global simd values space. 
        ModuleSIMDVars                  mSimdVarSpace;
        BVStatic<ASMSIMD_BUILTIN_SIZE>  mAsmSimdBuiltinUsedBV;
#endif
        ModuleExportArray               mExports;
        RegSlot                         mExportFuncIndex; // valid only if export object is empty
        ModuleFunctionTableArray        mFunctionTableArray;
        int                             mVarImportCount;
        int                             mVarCount;
        int32                           mFuncPtrTableCount;
        AsmJsModuleMemory               mModuleMemory;
        AsmJsCompileTime                mCompileTime;
        AsmJsCompileTime                mCompileTimeLastTick;
        long                            mMaxAstSize;
        BVStatic<ASMMATH_BUILTIN_SIZE>  mAsmMathBuiltinUsedBV;
        BVStatic<ASMARRAY_BUILTIN_SIZE> mAsmArrayBuiltinUsedBV;
        AsmJsCompileTime                mPhaseCompileTime[AsmJsCompilation::Phases_COUNT];
        ModuleArrayViewList             mArrayViews;
        uint                            mMaxHeapAccess;
#if DBG
        bool mStdLibArgNameInit : 1;
        bool mForeignArgNameInit : 1;
        bool mBufferArgNameInit : 1;
#endif
        bool mInitialised : 1;
        bool mUsesChangeHeap : 1;
        bool mUsesHeapBuffer : 1;
    public:
        AsmJsModuleCompiler( ExclusiveContext *cx, AsmJSParser &parser );
        bool Init();
#ifdef SIMD_JS_ENABLED
        bool InitSIMDBuiltins();

        // Resolves a SIMD function name to its symbol
        bool LookupStdLibSIMDName(PropertyId baseId, PropertyName fieldName, AsmJsSIMDFunction **simdFunc);
        bool LookupStdLibSIMDName(AsmJsSIMDBuiltinFunction baseId, PropertyName fieldName, AsmJsSIMDFunction **simdFunc);

        // Resolves a symbol name to SIMD constructor/operation and perform checks
        AsmJsSIMDFunction *LookupSimdConstructor(PropertyName name);
        AsmJsSIMDFunction *LookupSimdOperation(PropertyName name);

        void AddSimdBuiltinUse(int index){ mAsmSimdBuiltinUsedBV.Set(index); }
        // adds simd constant var to module 
        bool AddSimdValueVar(PropertyName name, ParseNode* pnode, AsmJsSIMDFunction* simdFunc);
        
#endif
        AsmJsCompileTime GetTick();
        void AccumulateCompileTime();
        void AccumulateCompileTime(AsmJsCompilation::Phases phase);
        // Return compile time in ms
        uint64 GetCompileTime() const;
        void PrintCompileTrace() const;

        // A valid module may have a NULL name
        inline PropertyName GetModuleFunctionName() const{return mModuleFunctionName;}
        inline ParseNode *GetModuleFunctionNode() const{return mModuleFunctionNode;}

        inline ArenaAllocator* GetAllocator() {return &mAllocator;}
        inline long GetMaxAstSize() const{return mMaxAstSize;}
        inline void UpdateMaxAstSize( long val ){mMaxAstSize = val>mMaxAstSize?val:mMaxAstSize;}

        //Mutable interface
        inline void InitModuleName( PropertyName name ){mModuleFunctionName = name;}
        inline void InitModuleNode( AsmJSParser &parser ){mModuleFunctionNode = parser;}
        inline AsmJSParser& GetCurrentParserNode(){return mCurrentParserNode;}
        inline void SetCurrentParseNode( AsmJSParser & val ){mCurrentParserNode = val;}

        void InitStdLibArgName( PropertyName n );
        void InitForeignArgName( PropertyName n );
        void InitBufferArgName( PropertyName n );
        PropertyName GetBufferArgName()  const;
        PropertyName GetForeignArgName() const;
        PropertyName GetStdLibArgName()  const;
        BVStatic<ASMMATH_BUILTIN_SIZE> GetAsmMathBuiltinUsedBV();
        void AddMathBuiltinUse(int index){ mAsmMathBuiltinUsedBV.Set(index); }
        BVStatic<ASMARRAY_BUILTIN_SIZE> GetAsmArrayBuiltinUsedBV();
        void AddArrayBuiltinUse(int index){ mAsmArrayBuiltinUsedBV.Set(index); }
        bool LookupStandardLibraryMathName(PropertyName name, MathBuiltin *mathBuiltin) const;
        bool LookupStandardLibraryArrayName(PropertyName name, TypedArrayBuiltin *builtin) const;

        // Lookup the name in the function environment if provided, then the module environment
        // indicate the origin of the symbol if specified
        AsmJsSymbol* LookupIdentifier( PropertyName name, AsmJsFunc* func = nullptr, AsmJsLookupSource::Source* lookupSource = nullptr );
        AsmJsFunctionDeclaration* LookupFunction( PropertyName name );
        bool DefineIdentifier( PropertyName name, AsmJsSymbol* symbol );
        bool AddNumericVar( PropertyName name, ParseNode* pnode, bool isFloat, bool isMutable = true);
        bool AddGlobalVarImport( PropertyName name, PropertyName field, AsmJSCoercion coercion );
        bool AddModuleFunctionImport( PropertyName name, PropertyName field );
        bool AddNumericConst( PropertyName name, const double* cst );
        bool AddArrayView( PropertyName name, ArrayBufferView::ViewType type );
        bool AddExport( PropertyName name, RegSlot location );
        bool SetExportFunc( AsmJsFunc* func );
        bool AddFunctionTable( PropertyName name, const int size );
        void AddMathLibName(PropertyId pid);
        //Immutable interface
        Parser *GetParser() const;
        ByteCodeGenerator* GetByteCodeGenerator() const;
        ScriptContext  *GetScriptContext() const;

        bool FailName( ParseNode *usepn, const wchar *fmt, PropertyName name );
        bool Fail( ParseNode* usepn, const wchar *error );

        bool AreAllFuncTableDefined();
        bool UsesChangeHeap() { return mUsesChangeHeap; }
        bool UsesHeapBuffer() { return mUsesHeapBuffer; }
        void SetUsesHeapBuffer(bool val) { mUsesHeapBuffer = val; }
        void UpdateMaxHeapAccess(uint index);
        uint GetMaxHeapAccess() { return mMaxHeapAccess; }
        // Compile/Validate function name and arguments (define their types)
        bool CompileFunction(AsmJsFunc * func, int funcIndex);
        bool CompileAllFunctions();
        void RevertAllFunctions();
        bool CommitFunctions();
        bool CommitModule();
        bool FinalizeModule();
        AsmJsFunc* CreateNewFunctionEntry(ParseNode* pnodeFnc);
        bool CheckChangeHeap(AsmJsFunc * func);


        void InitMemoryOffsets           ();
        inline int32 GetIntOffset        () const{return mModuleMemory.mIntOffset;}
        inline int32 GetFloatOffset        () const{return mModuleMemory.mFloatOffset;}
        inline int32 GetFuncPtrOffset    () const{return mModuleMemory.mFuncPtrOffset;}
        inline int32 GetFFIOffset        () const{return mModuleMemory.mFFIOffset;}
        inline int32 GetFuncOffset       () const{return mModuleMemory.mFuncOffset;}
        inline int32 GetDoubleOffset     () const{return mModuleMemory.mDoubleOffset; }
#ifdef SIMD_JS_ENABLED
        inline int32 GetSimdOffset       () const{ return mModuleMemory.mSimdOffset;  }
#endif
        inline int32 GetFuncPtrTableCount() const{return mFuncPtrTableCount;}
        inline void SetFuncPtrTableCount ( int32 val ){mFuncPtrTableCount = val;}

    private:
        void RevertFunction(int funcIndex);
        bool SetupFunctionArguments(AsmJsFunc * func, ParseNodePtr pnode);
        bool SetupLocalVariables(AsmJsFunc * func);
        void ASTPrepass(ParseNodePtr pnode, AsmJsFunc * func);
        void BindArguments(ParseNode* argList);
        bool AddStandardLibraryMathName(PropertyId id, AsmJsMathFunction* func, AsmJSMathBuiltinFunction mathLibFunctionName);
        bool AddStandardLibraryMathName(PropertyId id, const double* cstAddr, AsmJSMathBuiltinFunction mathLibFunctionName);
        bool AddStandardLibraryArrayName(PropertyId id, AsmJsTypedArrayFunction * func, AsmJSTypedArrayBuiltinFunction mathLibFunctionName);
        bool CheckByteLengthCall(ParseNode * node, ParseNode * newBufferDecl);
#ifdef SIMD_JS_ENABLED
        bool ValidateSimdConstructor(ParseNode* pnode, AsmJsSIMDFunction* simdFunc, AsmJsSIMDValue& value);
#endif
    };


    class AsmJsModuleInfo
    {
        /// proxy of asmjs module
        struct ModuleVar
        {
            RegSlot location;
            AsmJsVarType type;
            union
            {
                int intInit;
                float floatInit;
                double doubleInit;
#ifdef SIMD_JS_ENABLED
                AsmJsSIMDValue simdInit;
#endif
            } initialiser;
            bool isMutable;
        };
        struct ModuleVarImport
        {
            RegSlot location;
            AsmJsVarType type;
            PropertyId field;
        };
        struct ModuleFunctionImport
        {
            RegSlot location;
            PropertyId field;
        };
        struct ModuleFunction
        {
            RegSlot location;
        };
        struct ModuleExport
        {
            PropertyId* id;
            RegSlot* location;
        };
        struct ModuleFunctionTable
        {
            int size;
            RegSlot* moduleFunctionIndex;
        };

        Recycler* mRecycler;
        int mArgInCount; // for runtime validation of arguments in
        int mVarCount, mVarImportCount, mFunctionImportCount, mFunctionCount, mFunctionTableCount, mExportsCount;
#ifdef SIMD_JS_ENABLED
        int mSimdRegCount; // part of mVarCount
#endif

        PropertyIdArray*             mExports;
        RegSlot*                     mExportsFunctionLocation;
        RegSlot                      mExportFunctionIndex; // valid only if export object is empty
        ModuleVar*                   mVars;
        ModuleVarImport*             mVarImports;
        ModuleFunctionImport*        mFunctionImports;
        ModuleFunction*              mFunctions;
        ModuleFunctionTable*         mFunctionTables;
        AsmJsModuleMemory            mModuleMemory;
        BVStatic<ASMMATH_BUILTIN_SIZE>  mAsmMathBuiltinUsed;
        BVStatic<ASMARRAY_BUILTIN_SIZE> mAsmArrayBuiltinUsed;
#ifdef SIMD_JS_ENABLED
        BVStatic<ASMSIMD_BUILTIN_SIZE>  mAsmSimdBuiltinUsed;
#endif
        uint                         mMaxHeapAccess;
        bool                         mUsesChangeHeap;
        bool                         mIsProcessed;
    public:
        AsmJsModuleInfo( Recycler* recycler ) :
            mRecycler( recycler )
            , mArgInCount( 0 )
            , mVarCount( 0 )
            , mVarImportCount( 0 )
            , mFunctionImportCount( 0 )
            , mFunctionCount( 0 )
            , mFunctionTableCount( 0 )
#ifdef SIMD_JS_ENABLED
            , mSimdRegCount(0)
#endif
            , mVars( nullptr )
            , mVarImports( nullptr )
            , mFunctionImports( nullptr )
            , mFunctions( nullptr )
            , mMaxHeapAccess(0)
            , mUsesChangeHeap(false)
            , mIsProcessed(false)
        {

        }

        ModuleVar& GetVar( int i )
        {
            Assert( i < mVarCount );
            return mVars[i];
        }
        ModuleVarImport& GetVarImport( int i )
        {
            Assert( i < mVarImportCount );
            return mVarImports[i];
        }
        ModuleFunctionImport& GetFunctionImport( int i )
        {
            Assert( i < mFunctionImportCount );
            return mFunctionImports[i];
        }
        ModuleFunction& GetFunction( int i )
        {
            Assert( i < mFunctionCount );
            return mFunctions[i];
        }
        ModuleFunctionTable& GetFunctionTable( int i )
        {
            Assert( i < mFunctionTableCount );
            return mFunctionTables[i];
        }
        void SetFunctionTableSize( int index, int size );
        ModuleExport GetExport( int i )
        {
            ModuleExport ex;
            ex.id = &mExports->elements[i];
            ex.location = &mExportsFunctionLocation[i];
            return ex;
        }
        PropertyIdArray* GetExportsIdArray() const
        {
            return mExports;
        }


        // Accessors
    public:
        inline Js::RegSlot GetExportFunctionIndex() const{return mExportFunctionIndex;}
        inline void SetExportFunctionIndex( Js::RegSlot val ){mExportFunctionIndex = val;}
        void SetExportsCount(int count);
        inline int GetExportsCount()const
        {
            return mExportsCount;
        }
        inline int GetArgInCount() const
        {
            return mArgInCount;
        }
        inline void SetArgInCount( int val )
        {
            mArgInCount = val;
        }
        inline int GetFunctionCount() const
        {
            return mFunctionCount;
        }
        void SetFunctionCount( int val );
        inline int GetFunctionTableCount() const
        {
            return mFunctionTableCount;
        }
        void SetFunctionTableCount( int val );
        inline int GetFunctionImportCount() const
        {
            return mFunctionImportCount;
        }
        void SetFunctionImportCount( int val );
        inline int GetVarImportCount() const
        {
            return mVarImportCount;
        }
        void SetVarImportCount( int val );
        inline int GetVarCount() const
        {
            return mVarCount;
        }
        void SetVarCount( int val );
        inline bool IsRuntimeProcessed() const
        {
            return mIsProcessed;
        }
        void SetIsRuntimeProcessed(bool val)
        {
            mIsProcessed = val;
        }
        inline const AsmJsModuleMemory& GetModuleMemory() const
        {
            return mModuleMemory;
        }
        inline void SetModuleMemory( const AsmJsModuleMemory& val )
        {
            mModuleMemory = val;
        }
        inline void SetAsmMathBuiltinUsed(const BVStatic<ASMMATH_BUILTIN_SIZE> val)
        {
            mAsmMathBuiltinUsed = val;
        }
        inline BVStatic<ASMMATH_BUILTIN_SIZE> const GetAsmMathBuiltinUsed()const
        {
            return mAsmMathBuiltinUsed;
        }
        inline void SetAsmArrayBuiltinUsed(const BVStatic<ASMARRAY_BUILTIN_SIZE> val)
        {
            mAsmArrayBuiltinUsed = val;
        }
        inline BVStatic<ASMARRAY_BUILTIN_SIZE> const GetAsmArrayBuiltinUsed()const
        {
            return mAsmArrayBuiltinUsed;
        }
        void SetUsesChangeHeap(bool val)
        {
            mUsesChangeHeap = val;
        }
        inline bool GetUsesChangeHeap() const
        {
            return mUsesChangeHeap;
        }
        void SetMaxHeapAccess(uint val)
        {
            mMaxHeapAccess = val;
        }
        inline uint GetMaxHeapAccess() const
        {
            return mMaxHeapAccess;
        }
#ifdef SIMD_JS_ENABLED
        inline void SetSimdRegCount(int val) { mSimdRegCount = val;  }
        inline int GetSimdRegCount() const   { return mSimdRegCount; }
        inline void SetAsmSimdBuiltinUsed(const BVStatic<ASMSIMD_BUILTIN_SIZE> val)
        {
            mAsmSimdBuiltinUsed = val;
        }
        inline BVStatic<ASMSIMD_BUILTIN_SIZE> const GetAsmSimdBuiltinUsed()const
        {
            return mAsmSimdBuiltinUsed;
        }
#endif

        static void EnsureHeapAttached(ScriptFunction * func);
    };


};