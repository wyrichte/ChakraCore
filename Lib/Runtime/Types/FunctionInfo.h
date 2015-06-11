// Copyright (C) Microsoft. All rights reserved.

namespace Js
{
    class FunctionProxy;
    class FunctionBody;
    class ParseableFunctionInfo;
    class DeferDeserializeFunctionInfo;

    class FunctionInfo: public FinalizableObject
    {
        friend class RemoteFunctionBody;
    protected:
        DEFINE_VTABLE_CTOR_NOBASE(FunctionInfo);
    public:

        enum Attributes : SHORT
        {
            None                       = 0,
            ErrorOnNew                 = 1 << 0,
            SkipDefaultNewObject       = 1 << 1,
            DoNotProfile               = 1 << 2,
            HasNoSideEffect            = 1 << 3, // calling function doesn’t cause an implicit flags to be set,
                                                 // the callee will detect and set implicit flags on its individual operations
            NeedCrossSiteSecurityCheck = 1 << 4,
            DeferredDeserialize        = 1 << 5, // The function represents something that needs to be deserialized on use
            DeferredParse              = 1 << 6, // The function represents something that needs to be parsed on use
            CanBeHoisted               = 1 << 7, // The function return value won't be changed in a loop so the evaluation can be hoisted.
            HasSuperReference          = 1 << 8,
            IsDefaultConstructor       = 1 << 9,
            Lambda                     = 1 << 10,
            CapturesThis               = 1 << 11, // Only lambdas will set this; denotes whether the lambda referred to this, used by debugger
            Generator                  = 1 << 12,
        };
        FunctionInfo(JavascriptMethod entryPoint, Attributes attributes = None, LocalFunctionId functionId = Js::Constants::NoFunctionId, FunctionBody* functionBodyImpl = NULL);

        static DWORD GetFunctionBodyImplOffset() { return offsetof(FunctionInfo, functionBodyImpl); }

        void VerifyOriginalEntryPoint() const;
        JavascriptMethod GetOriginalEntryPoint() const;
        JavascriptMethod GetOriginalEntryPoint_Unchecked() const;
        void SetOriginalEntryPoint(const JavascriptMethod originalEntryPoint);

        BOOL IsDeferred() const { return ((this->attributes & (DeferredDeserialize | DeferredParse)) != 0); }
        BOOL IsLambda() const { return ((this->attributes & Lambda) != 0); }
        BOOL IsConstructor() const { return ((this->attributes & ErrorOnNew) == 0); }
        BOOL IsGenerator() const { return ((this->attributes & Generator) != 0); }

        BOOL HasBody() const { return functionBodyImpl != NULL; }
        BOOL HasParseableInfo() const { return this->HasBody() && !this->IsDeferredDeserializeFunction(); }

        FunctionProxy * GetFunctionProxy() const
        {
            return functionBodyImpl;
        }
        ParseableFunctionInfo* GetParseableFunctionInfo() const
        {
            Assert(functionBodyImpl == NULL || !IsDeferredDeserializeFunction());
            return (ParseableFunctionInfo*) functionBodyImpl;
        }
        ParseableFunctionInfo** GetParseableFunctionInfoRef() const
        {
            Assert(functionBodyImpl == NULL || !IsDeferredDeserializeFunction());
            return (ParseableFunctionInfo**)&functionBodyImpl;
        }
        DeferDeserializeFunctionInfo* GetDeferDeserializeFunctionInfo() const
        {
            Assert(functionBodyImpl == NULL || IsDeferredDeserializeFunction());
            return (DeferDeserializeFunctionInfo*)functionBodyImpl;
        }
        FunctionBody * GetFunctionBody() const;

        Attributes GetAttributes() const { return attributes; }
        Js::LocalFunctionId GetLocalFunctionId() const { return functionId; }
        virtual void Finalize(bool isShutdown)
        {
        }

        virtual void Dispose(bool isShutdown)
        {
        }

        virtual void Mark(Recycler *recycler) override { AssertMsg(false, "Mark called on object that isnt TrackableObject"); }

        BOOL IsDeferredDeserializeFunction() const { return ((this->attributes & DeferredDeserialize) == DeferredDeserialize); }
        BOOL IsDeferredParseFunction() const { return ((this->attributes & DeferredParse) == DeferredParse); }

    protected:
        JavascriptMethod originalEntryPoint;
        LocalFunctionId functionId;        // Per host source context (source file) function Id
        Attributes attributes;
        // WriteBarrier-TODO: Fix this? This is used only by proxies to keep the deserialized version around
        // However, proxies are not allocated as write barrier memory currently so its fine to not set the write barrier for this field
        FunctionProxy * functionBodyImpl;     // Implementation of the function- null if the function doesn't have a body
    };

    // Helper FunctionInfo for builtins that we don't want to profile (script profiler).
    class NoProfileFunctionInfo : public FunctionInfo
    {
    public:
        NoProfileFunctionInfo(JavascriptMethod entryPoint)
            : FunctionInfo(entryPoint, Attributes::DoNotProfile)
        {}
    };
};
