#pragma once

namespace Authoring
{
    // A class that contains the callbacks used to handling type information tracking undefined values.
    //
    // If an expression has meta-data that declares the expression has a type but JavaScript semantics
    // dictate the expression must be undefined (such as accessing property that isn't defined, or has
    // been deleted, but has a <field ..> meta-data comment exists for the property), a tracking undefined 
    // object can be returned instead that allows the declared type of the expression to be retrieved 
    // later. This class handles this process.
    // 
    // The runtime has hooks that will call these callbacks when it determines that an undefined value
    // is the correct result of an expression evaluation. These callbacks are called to let the langauge
    // service encode information to be tracked by that undefined value.
    class MissingValueHandler: public AuthoringCallbacks
    {
    private:
        // The maximum number of parameters we will produce a tracking value for.
        static const int PARAMETER_LIMIT = 5;

        typedef JsUtil::BaseDictionary<Js::RecyclableObject *, Js::Var, ArenaAllocator, PrimeSizePolicy, RecyclerPointerComparer> InstanceMap;
        typedef JsUtil::BaseDictionary<Js::JavascriptFunction *, Js::RecyclableObject **, ArenaAllocator, PrimeSizePolicy, RecyclerPointerComparer> FunctionMap;
        
#if DEBUG
        Js::ScriptContext *scriptContext;
#endif
        InstanceMap *missingValueMap;
        FunctionMap *functionParameterMap;
        Js::RecyclableObject **defaultParameters;
        ScriptContextPath *contextPath;
        ScriptContextManager *manager;
        FileAuthoring *fileAuthoring;
        charcount_t lastEvalLength;
        bool sendParsePhaseChange;
        bool inMissingValueHandler;
        bool isCallGraphEnabled;
        uint indent;
    public:
        MissingValueHandler(Js::ScriptContext *scriptContext, ScriptContextPath *contextPath, PhaseReporter *phaseReporter, FileAuthoring *fileAuthoring);
        virtual void Parsing() override;
        virtual void GeneratingByteCode() override;
        virtual void Executing() override;
        virtual void Progress() override;
        virtual void PreparingEval(charcount_t length) override;
        virtual Js::RecyclableObject *GetMissingPropertyResult(Js::ScriptContext *scriptContext, Js::RecyclableObject *instance, Js::PropertyId id, Js::TypeId typeId) override;
        virtual Js::RecyclableObject *GetMissingItemResult(Js::ScriptContext *scritpContext, Js::RecyclableObject *instance, uint32 index, Js::TypeId typeId) override;
        virtual Js::RecyclableObject *GetMissingParameterValue(Js::ScriptContext *scriptContext, Js::JavascriptFunction *function, uint32 paramIndex) override;
        virtual Js::RecyclableObject *GetTrackingKey(Js::ScriptContext *scriptContext, Js::Var value, Js::TypeId typeId) override;
        virtual Js::Var GetCallerName(Js::ScriptContext *scriptContext, int fileId, int offset) override;
        virtual Js::Var GetTrackingValue(Js::ScriptContext *scriptContext, Js::RecyclableObject *value) override;
        virtual bool HasThisStmt(Js::ScriptContext *scriptContext, Js::JavascriptFunction *function) override;
        virtual Js::Var GetExecutingScriptFileName(Js::ScriptContext *scriptContext) override;
        virtual bool CopyOnGet() override { return fileAuthoring && fileAuthoring->ShouldCopyOnGet(); }
        virtual int GetFileIdOfSourceIndex(Js::ScriptContext *scriptContext, int sourceIndex) override;
        virtual DWORD_PTR GetAuthorSource(int sourceIndex, Js::ScriptContext *scriptContext) override;
        void SetScriptContextPath(ScriptContextPath *scriptContextPath);
        void SetFileAuthoring(FileAuthoring *fileAuthoring);
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS

        virtual void LogFunctionStart(Js::ScriptContext *scriptContext, Js::FunctionBody * functionBody) override;
        virtual void LogFunctionEnd(Js::ScriptContext *scriptContext) override;

        virtual bool IsCallGraphEnabled() const override
        {
            return CONFIG_FLAG(LSCallGraph) || this->isCallGraphEnabled;
        }

        virtual void SetCallGraph(bool enable) override
        {
            this->isCallGraphEnabled = enable;
        }
#endif

    private:
        void ReportPhase(AuthorFileAuthoringPhase phase);
        Js::RecyclableObject *RecordTrackingValue(Js::Var value, Js::ScriptContext *scriptContext, Js::TypeId typeId);
        Js::RecyclableObject **NewParameterValueArray(Js::ScriptContext *scriptContext);
        PhaseReporter* GetActivePhaseReporter();
    };
}
