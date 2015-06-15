//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

struct ExtractionContext;

void EmitReference(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo);
void EmitAssignment(ParseNode *asgnNode, ParseNode *lhs, Js::RegSlot rhsLocation, ByteCodeGenerator *byteCodeGenerator,FuncInfo *funcInfo);
void EmitLoad(ParseNode *rhs, ByteCodeGenerator *byteCodeGenerator,FuncInfo *funcInfo);
void EmitList(ParseNode *pnode,ByteCodeGenerator *byteCodeGenerator,FuncInfo *funcInfo);
void EmitCall(ParseNode* pnode, Js::RegSlot rhsLocation, ByteCodeGenerator* byteCodeGenerator, FuncInfo* funcInfo, BOOL fReturnValue, BOOL fAssignRegs);
void PropagateFlags(ParseNode *pnodeChild, ParseNode *pnodeParent);

extern const wchar_t *SymbolTypeNames[];

const unsigned int NoIndex=0xffffffff;

#if defined(_M_ARM32_OR_ARM64) || defined(_M_X64)
const long AstBytecodeRatioEstimate = 4;
#else
const long AstBytecodeRatioEstimate = 5;
#endif

class Scope;
class NativeCodeGenerator;

enum SymbolType : byte {
    STFunction,
    STVariable,
    STMemberName,
    STFormal,
    STUnknown
};

typedef JsUtil::CharacterBuffer<WCHAR> SymbolName;

class Symbol
{
private:
    const SymbolName name;
    IdentPtr pid;
    ParseNode *decl;
    Scope *scope;           // scope defining this symbol
    Js::PropertyId position;  // argument position in function declaration
    Js::RegSlot location;  // register in which the symbol resides
    Js::PropertyId scopeSlot;
    Symbol *next;

    SymbolType symbolType;
    BYTE defCount;
    BYTE needDeclaration:1;
    BYTE isBlockVar:1;
    BYTE isGlobal:1;
    BYTE isEval:1;
    BYTE hasNonLocalReference:1;  // if true, then this symbol needs to be heap-allocated
    BYTE funcExpr:1;              // if true, then this symbol is allocated on it's on activation object
    BYTE isCatch:1;               // if true then this a catch identifier
    BYTE hasInit:1;
    BYTE isUsed:1;
    BYTE isGlobalCatch:1;
    BYTE isCommittedToSlot:1;
    BYTE hasNonCommittedReference:1;
    BYTE hasVisitedCapturingFunc:1;
    BYTE isTrackedForDebugger:1;    // Whether the sym is tracked for debugger scope. This is fine because a sym can only be added to (not more than) one scope.
    // These are get and set a lot, don't put it in bit fields, we are exceeding the number of bits anyway
    bool hasFuncAssignment;
    bool hasMaybeEscapedUse;
    bool isNonSimpleParameter;

    AssignmentState assignmentState;

public:
    Symbol(SymbolName const& name, ParseNode *decl,SymbolType symbolType) :
        name(name),
        decl(decl),
        next(null),
        location(Js::Constants::NoRegister),
        needDeclaration(false),
        isBlockVar(false),
        isGlobal(false),
        hasNonLocalReference(false),
        funcExpr(false),
        isCatch(false),
        hasInit(false),
        isUsed(false),
        defCount(0),
        position(Js::Constants::NoProperty),
        scopeSlot(Js::Constants::NoProperty),
        isGlobalCatch(false),
        isCommittedToSlot(false),
        hasNonCommittedReference(false),
        hasVisitedCapturingFunc(false),
        isTrackedForDebugger(false),
        isNonSimpleParameter(false),
        assignmentState(NotAssigned)
    {
        SetSymbolType(symbolType);

        // Set it so we don't have to check it explicitly
        isEval = MatchName(L"eval", 4);

        if (PHASE_TESTTRACE1(Js::StackFuncPhase) && hasFuncAssignment)
        {
            Output::Print(L"HasFuncDecl: %s\n", this->GetName().GetBuffer());
            Output::Flush();
        }
    }

    bool MatchName(const wchar_t *key, int length) {
        return name == SymbolName(key, length);
    }

    void SetScope(Scope *scope) {
        this->scope=scope;
    }
    Scope * GetScope() const { return scope; }

    void SetDecl(ParseNode *pnodeDecl) { decl = pnodeDecl; }
    ParseNode* GetDecl() const { return decl; }

    void SetScopeSlot(Js::PropertyId slot)
    {
        this->scopeSlot = slot;
    }

    Symbol *GetNext() const
    {
        return next;
    }

    void SetNext(Symbol *sym)
    {
        next = sym;
    }

    void SetIsGlobal(bool b) {
        isGlobal=b;
    }

    void SetHasNonLocalReference(bool b, ByteCodeGenerator *byteCodeGenerator);

    bool GetHasNonLocalReference() const {
        return hasNonLocalReference;
    }

    void SetFuncExpr(bool b) {
        funcExpr=b;
    }

    void SetIsBlockVar(bool is) {
        isBlockVar = is;
    }

    bool GetIsBlockVar() const {
        return isBlockVar;
    }

    void SetIsGlobalCatch(bool is) {
        isGlobalCatch = is;
    }

    bool GetIsGlobalCatch() const {
        return isGlobalCatch;
    }

    void SetIsCommittedToSlot()
    {
        this->isCommittedToSlot = true;
    }

    bool GetIsCommittedToSlot() const;

    void SetHasVisitedCapturingFunc()
    {
        this->hasVisitedCapturingFunc = true;
    }

    bool HasVisitedCapturingFunc() const
    {
        return hasVisitedCapturingFunc;
    }

    void SetHasNonCommittedReference(bool has)
    {
        this->hasNonCommittedReference = has;
    }

    bool GetHasNonCommittedReference() const
    {
        return hasNonCommittedReference;
    }

    void SetIsTrackedForDebugger(bool is) {
        isTrackedForDebugger = is;
    }

    bool GetIsTrackedForDebugger() const {
        return isTrackedForDebugger;
    }

    void SetNeedDeclaration(bool need) {
        needDeclaration = need;
    }

    bool GetNeedDeclaration() const {
        return needDeclaration;
    }

    bool GetFuncExpr() const {
        return funcExpr;
    }

    bool GetIsGlobal() const {
        return isGlobal;
    }

    bool GetIsMember() const {
        return symbolType == STMemberName;
    }

    bool GetIsFormal() const {
        return symbolType == STFormal;
    }

    bool GetIsEval() const {
        return isEval;
    }

    bool GetIsCatch() const {
        return isCatch;
    }

    void SetIsCatch(bool b){
        isCatch = b;
    }

    bool GetHasInit() const
    {
        return hasInit;
    }

    void RecordDef() {
      defCount++;
    }

    bool SingleDef() const {
      return defCount==1;
    }

    void SetHasInit(bool has)
    {
        hasInit = has;
    }

    bool GetIsUsed() const
    {
        return isUsed;
    }

    void SetIsUsed(bool is)
    {
        isUsed = is;
    }

    void PromoteAssignmentState()
    {
        if (assignmentState == NotAssigned)
        {
            assignmentState = AssignedOnce;
        }
        else if (assignmentState == AssignedOnce)
        {
            assignmentState = AssignedMultipleTimes;
        }
    }

    bool IsAssignedOnce()
    {
        return assignmentState == AssignedOnce;
    }

    // For stack nested function escape analysis
    bool GetHasMaybeEscapedUse() const { return hasMaybeEscapedUse; }
    void SetHasMaybeEscapedUse(ByteCodeGenerator * byteCodeGenerator);
    bool GetHasFuncAssignment() const { return hasFuncAssignment; }
    void SetHasFuncAssignment(ByteCodeGenerator * byteCodeGenerator);
    void RestoreHasFuncAssignment();

    bool GetIsNonSimpleParameter() const
    {
        return isNonSimpleParameter;
    }

    void SetIsNonSimpleParameter(bool is)
    {
        isNonSimpleParameter = is;
    }

    bool GetIsArguments() const
    {
        return decl != NULL && (decl->grfpn & PNodeFlags::fpnArguments);
    }

    void SetPosition(Js::PropertyId pos) {
        position=pos;
    }

    Js::PropertyId GetPosition() {
        return position;
    }

    Js::PropertyId EnsurePosition(ByteCodeGenerator* byteCodeGenerator);
    Js::PropertyId EnsurePosition(FuncInfo *funcInfo);
    Js::PropertyId EnsurePositionNoCheck(FuncInfo *funcInfo);

    void SetLocation(Js::RegSlot location) {
        this->location=location;
    }

    Js::RegSlot GetLocation() {
        return location;
    }

    Js::PropertyId GetScopeSlot() const { return scopeSlot; }
    bool HasScopeSlot() const { return scopeSlot != Js::Constants::NoProperty; }

    SymbolType GetSymbolType() {
        return symbolType;
    }

    void SetSymbolType(SymbolType symbolType)
    {
        this->symbolType = symbolType;
        this->hasMaybeEscapedUse = GetIsFormal();
        this->hasFuncAssignment = (symbolType == STFunction);
    }

    const wchar_t *GetSymbolTypeName() {
        return SymbolTypeNames[symbolType];
    }

    const JsUtil::CharacterBuffer<WCHAR>& GetName() const {
        return this->name;
    }

    Js::PropertyId EnsureScopeSlot(FuncInfo *funcInfo);
    bool IsInSlot(FuncInfo *funcInfo, bool ensureSlotAlloc = false);
    bool NeedsSlotAlloc(FuncInfo *funcInfo);

    static void SaveToPropIdArray(Symbol *sym, Js::PropertyIdArray *propIds, ByteCodeGenerator *byteCodeGenerator, Js::PropertyId *pFirstSlot = null);

    Symbol * GetFuncScopeVarSym() const;

    void SetPid(IdentPtr pid)
    {
        this->pid = pid;
    }
    IdentPtr GetPid() const
    {
        return pid;
    }

private:
    void SetHasMaybeEscapedUseInternal(ByteCodeGenerator * byteCodeGenerator);
    void SetHasFuncAssignmentInternal(ByteCodeGenerator * byteCodeGenerator);
};

// specialize toKey to use the name in the symbol as the key
template <>
SymbolName
JsUtil::ValueToKey<SymbolName, Symbol *>::ToKey(Symbol * const& sym)
{
    return sym->GetName();
}

typedef JsUtil::BaseHashSet<Symbol *, ArenaAllocator, PrimeSizePolicy, SymbolName, DefaultComparer, JsUtil::HashedEntry> SymbolTable;

enum ScopeType
{
    ScopeType_Unknown,
    ScopeType_Global,
    ScopeType_GlobalEvalBlock,
    ScopeType_FunctionBody,
    ScopeType_FuncExpr,
    ScopeType_Block,
    ScopeType_Catch,
    ScopeType_With,
    ScopeType_Parameter
};

class Scope
{
private:
    Scope *enclosingScope;
    Js::RegSlot location;
    FuncInfo *func;
    SymbolTable *symbolTable;
    Symbol *m_symList;
    int m_count;
    ArenaAllocator *alloc;
    uint scopeSlotCount; // count of slots in the local scope
    ScopeType const scopeType;
    BYTE isDynamic:1;
    BYTE isObject:1;
    BYTE canMerge:1;
    BYTE capturesAll:1;
    BYTE mustInstantiate:1;
    BYTE hasCrossScopeFuncAssignment:1;
public:
#if DBG
    BYTE isRestored:1;
#endif
    Scope(ArenaAllocator *alloc, ScopeType scopeType, bool useSymbolTable = false, int capacity = 0) :
        alloc(alloc),
        func(null),
        enclosingScope(null),
        isDynamic(false),
        isObject(false),
        canMerge(true),
        capturesAll(false),
        mustInstantiate(false),
        hasCrossScopeFuncAssignment(false),
        location(Js::Constants::NoRegister),
        symbolTable(null),
        m_symList(null),
        m_count(0),
        scopeSlotCount(0),
        scopeType(scopeType)
#if DBG
        , isRestored(false)
#endif
    {
        if (useSymbolTable)
        {
            symbolTable = Anew(alloc, SymbolTable, alloc, capacity);
        }
    }

    ~Scope()
    {
        if (symbolTable)
        {
            Adelete(alloc, symbolTable);
            symbolTable = null;
        }
    }

    Symbol *FindLocalSymbol(SymbolName const& key)
    {
        Symbol *sym = null;
        if (symbolTable)
        {
            return symbolTable->Lookup(key);
        }
        for (sym = m_symList; sym; sym = sym->GetNext())
        {
            if (sym->GetName() == key)
            {
                break;
            }
        }
        return sym;
    }

    template<class Fn>
    void ForEachSymbol(Fn fn)
    {
        if (symbolTable)
        {
            symbolTable->Map(fn);
        }
        else
        {
            for (Symbol *sym = m_symList; sym;)
            {
                Symbol *next = sym->GetNext();
                fn(sym);
                sym = next;
            }
        }
    }

    template<class Fn>
    void ForEachSymbolUntil(Fn fn)
    {
        if (symbolTable)
        {
            symbolTable->MapUntil(fn);
        }
        else
        {
            for (Symbol *sym = m_symList; sym;)
            {
                Symbol *next = sym->GetNext();
                if (fn(sym))
                {
                    return;
                }
                sym = next;
            }
        }
    }

    // for JScript, this should not return NULL because
    // there is always an enclosing global scope
    Symbol *FindSymbol(SymbolName const& name, SymbolType symbolType, bool fCreate = true)
    {
        Symbol *sym=FindLocalSymbol(name);
        if (sym==NULL)
        {
            if (enclosingScope!=NULL)
            {
                sym = enclosingScope->FindSymbol(name, symbolType);
            }
            else if (fCreate)
            {
                sym = Anew(alloc, Symbol, name, NULL, symbolType);
                AddNewSymbol(sym);
            }
        }
        return sym;
    }

    void AddSymbol(Symbol *sym)
    {
        if (enclosingScope == NULL)
        {
            sym->SetIsGlobal(true);
        }
        sym->SetScope(this);
        if (symbolTable)
        {
            symbolTable->AddNew(sym);
        }
        else
        {
            for (Symbol *symInList = m_symList; symInList; symInList = symInList->GetNext())
            {
                if (symInList->GetName() == sym->GetName())
                {
                    return;
                }
            }
            sym->SetNext(m_symList);
            m_symList = sym;
            m_count++;
        }
    }

    void AddNewSymbol(Symbol *sym)
    {
        if (scopeType == ScopeType_Global)
        {
            sym->SetIsGlobal(true);
        }
        sym->SetScope(this);
        if (symbolTable)
        {
            symbolTable->Add(sym);
        }
        else
        {
            sym->SetNext(m_symList);
            m_symList = sym;
            m_count++;
        }
    }

    bool HasStaticPathToAncestor(Scope const * target) const
    {
        return target == this || (!isDynamic && enclosingScope != nullptr && enclosingScope->HasStaticPathToAncestor(target));
    }

    void SetEnclosingScope(Scope *enclosingScope)
    {
        // Check for scope cycles
        Assert(enclosingScope != this);
        Assert(enclosingScope == nullptr || this != enclosingScope->GetEnclosingScope());
        this->enclosingScope = enclosingScope;
    }

    Scope *GetEnclosingScope() const
    {
        return enclosingScope;
    }

    ScopeType GetScopeType() const
    {
        return this->scopeType;
    }

    int Count() const
    {
        if (symbolTable)
        {
            return symbolTable->Count();
        }
        return m_count;
    }

    void SetFunc(FuncInfo *func)
    {
        this->func = func;
    }

    FuncInfo *GetFunc() const
    {
        return func;
    }

    FuncInfo *GetEnclosingFunc()
    {
        Scope *scope = this;
        while (scope && scope->func == NULL)
        {
            scope = scope->GetEnclosingScope();
        }
        Assert(scope);
        return scope->func;
    }

    void SetLocation(Js::RegSlot loc) { location = loc; }
    Js::RegSlot GetLocation() const { return location; }

    void SetIsDynamic(bool is) { isDynamic = is; }
    bool GetIsDynamic() const { return isDynamic; }

    bool IsEmpty() const;

    bool IsBlockScope(FuncInfo *funcInfo);

    void SetIsObject();
    bool GetIsObject() const { return isObject; }

    void SetCapturesAll(bool does) { capturesAll = does; }
    bool GetCapturesAll() const { return capturesAll; }

    void SetMustInstantiate(bool must) { mustInstantiate = must; }
    bool GetMustInstantiate() const { return mustInstantiate; }

    void SetCanMerge(bool can) { canMerge = can; }
    bool GetCanMerge() const { return canMerge && !mustInstantiate && !isObject; }

    void SetScopeSlotCount(uint i) { scopeSlotCount = i; }
    uint GetScopeSlotCount() const { return scopeSlotCount; }

    void SetHasLocalInClosure(bool has);

    int AddScopeSlot();

    void SetHasCrossScopeFuncAssignment() { hasCrossScopeFuncAssignment = true; }

    void ForceAllSymbolNonLocalReference(ByteCodeGenerator *byteCodeGenerator);

    bool IsGlobalEvalBlockScope() const;

    static void MergeParamAndBodyScopes(ParseNode *pnodeScope, ByteCodeGenerator * byteCodeGenerator);
};

struct SymCheck {
    static const int kMaxInvertedSyms=8;
    Symbol* syms[kMaxInvertedSyms];
    Symbol* permittedSym;
    int symCount;
    bool result;
    bool cond;

    bool AddSymbol(Symbol* sym) {
        if (symCount<kMaxInvertedSyms) {
            syms[symCount++]=sym;
            return true;
        }
        else {
            return false;
        }
    }

    bool MatchSymbol(Symbol* sym) {
        if (sym!=permittedSym) {
            for (int i=0;i<symCount;i++) {
                if (sym==syms[i]) {
                    return true;
                }
            }
        }
        return false;
    }

    void Init() {
        symCount=0;
        result=true;
    }
};

struct InlineCacheUnit {

    InlineCacheUnit() : loadCacheId((uint)-1), loadMethodCacheId((uint)-1), storeCacheId((uint)-1) {}

    union {
        struct {
            uint loadCacheId;
            uint loadMethodCacheId;
            uint storeCacheId;
        };
        struct {
            uint cacheId;
        };
    };
};

typedef JsUtil::BaseDictionary<ParseNode*, SList<Symbol*>*, ArenaAllocator, PowerOf2SizePolicy> CapturedSymMap;

class FuncInfo
{
private:
    struct SlotKey
    {
        Scope* scope;
        Js::PropertyId slot;
    };

    template<class TSlotKey>
    class SlotKeyComparer
    {
    public:
        static bool Equals(TSlotKey key1, TSlotKey key2)
        {
            return (key1.scope == key2.scope && key1.slot == key2.slot);
        }

        static int GetHashCode(TSlotKey key)
        {
            return ((int)key.scope) | key.slot & ArenaAllocator::ObjectAlignmentMask;
        }
    };


    uint inlineCacheCount;
    uint rootObjectLoadInlineCacheCount;
    uint rootObjectLoadMethodInlineCacheCount;
    uint rootObjectStoreInlineCacheCount;
    uint isInstInlineCacheCount;
    uint referencedPropertyIdCount;
    uint NewInlineCache()
    {
        AssertMsg(this->inlineCacheCount < (uint)-2, "Inline cache index wrapped around?");
        return inlineCacheCount++;
    }
    uint NewRootObjectLoadInlineCache()
    {
        AssertMsg(this->rootObjectLoadInlineCacheCount < (uint)-2, "Inline cache index wrapped around?");
        return rootObjectLoadInlineCacheCount++;
    }
    uint NewRootObjectLoadMethodInlineCache()
    {
        AssertMsg(this->rootObjectLoadMethodInlineCacheCount < (uint)-2, "Inline cache index wrapped around?");
        return rootObjectLoadMethodInlineCacheCount++;
    }
    uint NewRootObjectStoreInlineCache()
    {
        AssertMsg(this->rootObjectStoreInlineCacheCount < (uint)-2, "Inline cache index wrapped around?");
        return rootObjectStoreInlineCacheCount++;
    }
    uint NewReferencedPropertyId()
    {
        AssertMsg(this->referencedPropertyIdCount < (uint)-2, "Referenced Property Id index wrapped around?");
        return referencedPropertyIdCount++;
    }

    FuncInfo *currentChildFunction;
    Scope *currentChildScope;
    SymbolTable *capturedSyms;
    CapturedSymMap *capturedSymMap;

public:
    ArenaAllocator *alloc;
    // set in Bind/Assign pass
    Js::RegSlot varRegsCount;    // number of registers used for non-constants
    Js::RegSlot constRegsCount;  // number of registers used for constants
    Js::ArgSlot inArgsCount; // number of in args (including 'this')
    Js::RegSlot outArgsMaxDepth; // max depth of out args stack
    Js::RegSlot outArgsCurrentExpr; // max number of out args accumlated in the current nested expression
#if DBG
    uint32 outArgsDepth;  // number of calls nested in a expression
#endif
    const wchar_t *name;      // name of the function
    Js::RegSlot nullConstantRegister; // location, if any, of enregistered null constant
    Js::RegSlot undefinedConstantRegister; // location, if any, of enregistered undefined constant
    Js::RegSlot trueConstantRegister; // location, if any, of enregistered true constant
    Js::RegSlot falseConstantRegister; // location, if any, of enregistered false constant
    Js::RegSlot thisPointerRegister;  // location, if any, of this pointer
    Js::RegSlot superRegister; // location, if any, of the super reference
private:
    Js::RegSlot envRegister; // location, if any, of the closure environment
public:
    Js::RegSlot frameObjRegister; // location, if any, of the heap-allocated local frame
    Js::RegSlot frameSlotsRegister; // location, if any, of the heap-allocated local frame
    Js::RegSlot frameDisplayRegister; // location, if any, of the display of nested frames
    Js::RegSlot funcObjRegister;
    Js::RegSlot stackClosureReg;
    Js::RegSlot yieldRegister;
    Js::RegSlot firstTmpReg;
    Js::RegSlot curTmpReg;
    int sameNameArgsPlaceHolderSlotCount; // count of place holder slots for same name args
    int localPropIdOffset;
    Js::RegSlot firstThunkArgReg;
    short thunkArgCount;
    short staticFuncId;

    uint callsEval : 1;
    uint childCallsEval : 1;
    uint hasArguments : 1;
    uint hasHeapArguments : 1;
    uint isEventHandler : 1;
    uint hasLocalInClosure : 1;
    uint hasClosureReference : 1;
    uint hasGlobalReference : 1;
    uint hasCachedScope : 1;
    uint funcExprNameReference : 1;
    uint applyEnclosesArgs:1;
    uint escapes:1;
    uint hasDeferredChild : 1;  // switch for DeferNested to persist outer scopes
    uint childHasWith : 1;      // deferNested needs to know if child has with
    uint hasLoop : 1;
    uint hasEscapedUseNestedFunc : 1;
    uint needEnvRegister : 1;
    uint hasCapturedThis : 1;

    typedef ResizableUIntHashTable<Js::RegSlot, ArenaAllocator, PrimePolicy> ConstantRegisterMap;
    ConstantRegisterMap constantToRegister; // maps uint constant to register
    typedef SimpleHashTable<IdentPtr, Js::RegSlot, ArenaAllocator, DefaultComparer, true, PowerOf2Policy> PidRegisterMap;
    PidRegisterMap stringToRegister; // maps string constant to register
    typedef JsUtil::BaseDictionary<double,Js::RegSlot, ArenaAllocator, PrimeSizePolicy> DoubleRegisterMap;
    DoubleRegisterMap doubleConstantToRegister; // maps double constant to register

    typedef JsUtil::BaseDictionary<ParseNodePtr, Js::RegSlot, ArenaAllocator, PowerOf2SizePolicy, Js::StringTemplateCallsiteObjectComparer> StringTemplateCallsiteRegisterMap;
    StringTemplateCallsiteRegisterMap stringTemplateCallsiteRegisterMap; // maps string template callsite constant to register

    Scope *paramScope; // top level scope for parameter default values
    Scope *bodyScope; // top level scope of the function body
    Scope *funcExprScope;
    ParseNode *root;          // top-level AST for function
    Js::ParseableFunctionInfo* byteCodeFunction; // reference to generated bytecode function (could be defer parsed or actually parsed)
    SList<ParseNode*> targetStatements; // statements that are targets of jumps (break or continue)
    Js::ByteCodeLabel singleExit;
    typedef SList<InlineCacheUnit> InlineCacheList;
    typedef JsUtil::BaseDictionary<Js::PropertyId, InlineCacheList*, ArenaAllocator, PowerOf2SizePolicy> InlineCacheIdMap;
    typedef JsUtil::BaseDictionary<Js::RegSlot, InlineCacheIdMap*, ArenaAllocator, PowerOf2SizePolicy> InlineCacheMap;
    typedef JsUtil::BaseDictionary<Js::PropertyId, uint, ArenaAllocator, PowerOf2SizePolicy> RootObjectInlineCacheIdMap;
    typedef JsUtil::BaseDictionary<Js::PropertyId, uint, ArenaAllocator, PowerOf2SizePolicy> ReferencedPropertyIdMap;
    RootObjectInlineCacheIdMap * rootObjectLoadInlineCacheMap;
    RootObjectInlineCacheIdMap * rootObjectLoadMethodInlineCacheMap;
    RootObjectInlineCacheIdMap * rootObjectStoreInlineCacheMap;
    InlineCacheMap * inlineCacheMap;
    ReferencedPropertyIdMap * referencedPropertyIdToMapIndex;
    SListBase<uint> valueOfStoreCacheIds;
    SListBase<uint> toStringStoreCacheIds;
    typedef JsUtil::BaseDictionary<SlotKey, Js::ProfileId, ArenaAllocator, PowerOf2SizePolicy, SlotKeyComparer> SlotProfileIdMap;
    SlotProfileIdMap slotProfileIdMap;
    Js::PropertyId thisScopeSlot;
    Js::PropertyId superScopeSlot;
    bool isThisLexicallyCaptured;
    bool isSuperLexicallyCaptured;
    Symbol *argumentsSymbol;
    JsUtil::List<Js::RegSlot, ArenaAllocator> nonUserNonTempRegistersToInitialize;

    // constRegsCount is set to 2 because R0 is the return register, and R1 is the root object.
    FuncInfo(
        const wchar_t *name,
        ArenaAllocator *alloc,
        Scope *paramScope,
        Scope *bodyScope,
        ParseNode *pnode,
        Js::ParseableFunctionInfo* byteCodeFunction)
        : alloc(alloc),
        varRegsCount(0),
        constRegsCount(2),
        inArgsCount(0),
        firstTmpReg(Js::Constants::NoRegister),
        curTmpReg(Js::Constants::NoRegister),
        outArgsMaxDepth(0),
        outArgsCurrentExpr(0),
#if DBG
        outArgsDepth(0),
#endif
        name(name),
        nullConstantRegister(Js::Constants::NoRegister),
        undefinedConstantRegister(Js::Constants::NoRegister),
        trueConstantRegister(Js::Constants::NoRegister),
        falseConstantRegister(Js::Constants::NoRegister),
        thisPointerRegister(Js::Constants::NoRegister),
        superRegister(Js::Constants::NoRegister),
        envRegister(Js::Constants::NoRegister),
        frameObjRegister(Js::Constants::NoRegister),
        frameSlotsRegister(Js::Constants::NoRegister),
        frameDisplayRegister(Js::Constants::NoRegister),
        funcObjRegister(Js::Constants::NoRegister),
        stackClosureReg(Js::Constants::NoRegister),
        yieldRegister(Js::Constants::NoRegister),
        paramScope(paramScope),
        bodyScope(bodyScope),
        funcExprScope(NULL),
        root(pnode),
        capturedSyms(nullptr),
        capturedSymMap(nullptr),
        currentChildFunction(nullptr),
        currentChildScope(nullptr),
        callsEval(false),
        childCallsEval(false),
        hasArguments(false),
        hasHeapArguments(false),
        isEventHandler(false),
        hasLocalInClosure(false),
        hasClosureReference(false),
        hasGlobalReference(false),
        hasCachedScope(false),
        funcExprNameReference(false),
        applyEnclosesArgs(false),
        escapes(false),
        hasDeferredChild(false),
        childHasWith(false),
        hasLoop(false),
        hasEscapedUseNestedFunc(false),
        needEnvRegister(false),
        hasCapturedThis(false),
        staticFuncId(-1),
        inlineCacheMap(null),
        slotProfileIdMap(alloc),
        localPropIdOffset(-1),
        sameNameArgsPlaceHolderSlotCount(0),
        thisScopeSlot(Js::Constants::NoProperty),
        superScopeSlot(Js::Constants::NoProperty),
        isThisLexicallyCaptured(false),
        isSuperLexicallyCaptured(false),
        inlineCacheCount(0),
        rootObjectLoadInlineCacheCount(0),
        rootObjectLoadMethodInlineCacheCount(0),
        rootObjectStoreInlineCacheCount(0),
        isInstInlineCacheCount(0),
        referencedPropertyIdCount(0),
        argumentsSymbol(NULL),
        nonUserNonTempRegistersToInitialize(alloc),
        constantToRegister(/*size=*/ 17, alloc),
        stringToRegister(/*size=*/ 17, alloc),
        doubleConstantToRegister(alloc, /*size=*/ 17),
        stringTemplateCallsiteRegisterMap(alloc, 17),
        targetStatements(alloc)
    {
        this->byteCodeFunction = byteCodeFunction;        
        bodyScope->SetFunc(this);
        if (paramScope != nullptr)
        {
            paramScope->SetFunc(this);
        }
    }

    uint NewIsInstInlineCache() { return isInstInlineCacheCount++; }
    uint GetInlineCacheCount() const { return this->inlineCacheCount; }
    uint GetRootObjectLoadInlineCacheCount() const { return this->rootObjectLoadInlineCacheCount; }
    uint GetRootObjectLoadMethodInlineCacheCount() const { return this->rootObjectLoadMethodInlineCacheCount; }
    uint GetRootObjectStoreInlineCacheCount() const { return this->rootObjectStoreInlineCacheCount; }
    uint GetIsInstInlineCacheCount() const { return this->isInstInlineCacheCount; }
    uint GetReferencedPropertyIdCount() const { return this->referencedPropertyIdCount; }
    void SetFirstTmpReg(Js::RegSlot tmpReg)
    {
        Assert(this->firstTmpReg == Js::Constants::NoRegister);
        Assert(this->curTmpReg == Js::Constants::NoRegister);
        this->firstTmpReg = tmpReg;
        this->curTmpReg = tmpReg;
    }

    bool IsTmpReg(Js::RegSlot tmpReg)
    {
        Assert(this->firstTmpReg != Js::Constants::NoRegister);
        return !RegIsConst(tmpReg) && tmpReg >= firstTmpReg;
    }

    bool RegIsConst(Js::RegSlot reg)
    {
        // varRegsCount includes the tmp regs. so if reg number is larger than that,
        // then it must be in the negative range for const
        return reg >= varRegsCount;
    }

    Js::RegSlot NextVarRegister()
    {
        AssertMsg(this->firstTmpReg == Js::Constants::NoRegister, "Shouldn't assign var register after we start allocating temp reg");
        Js::RegSlot reg = varRegsCount;
        UInt32Math::Inc(varRegsCount);
        return REGSLOT_TO_VARREG(reg);
    }

    Js::RegSlot NextConstRegister()
    {
        AssertMsg(this->firstTmpReg == Js::Constants::NoRegister, "Shouldn't assign var register after we start allocating temp reg");
        Js::RegSlot reg = constRegsCount;
        UInt32Math::Inc(constRegsCount);
        return REGSLOT_TO_CONSTREG(reg);
    }

    Js::RegSlot RegCount() const
    {
        return constRegsCount + varRegsCount;
    }

    bool GetApplyEnclosesArgs() const { return applyEnclosesArgs; }
    void SetApplyEnclosesArgs(bool b) { applyEnclosesArgs=b; }

    bool IsGlobalFunction() const {
        return root && root->nop == knopProg;
    }

    // Fake global ->
    //    1) new Function code's global code
    //    2) global code generated from the reparsing deferred parse function

    bool IsFakeGlobalFunction(ulong flags) const {
        return IsGlobalFunction() && !(flags & fscrGlobalCode);
    }

    Scope *GetBodyScope() const {
        return bodyScope;
    }

    Scope *GetParamScope() const {
        return paramScope;
    }

    Scope *GetTopLevelScope() const {
        // Top level scope will be the same for knopProg and knopFncDecl.
        return paramScope;
    }

    Scope* GetFuncExprScope() const {
        return funcExprScope;
    }

    void SetFuncExprScope(Scope* funcExprScope) {
        this->funcExprScope = funcExprScope;
    }

    Symbol *GetArgumentsSymbol() const
    {
        return argumentsSymbol;
    }

    void SetArgumentsSymbol(Symbol *sym)
    {
        Assert(argumentsSymbol == null || argumentsSymbol == sym);
        argumentsSymbol = sym;
    }

    bool GetCallsEval() const {
        return callsEval;
    }

    void SetCallsEval(bool does) {
        callsEval = does;
    }

    bool GetHasArguments() const {
        return hasArguments;
    }

    void SetHasArguments(bool has) {
        hasArguments = has;
    }

    bool GetHasHeapArguments() const
    {
        return hasHeapArguments;
    }

    void SetHasHeapArguments(bool has, bool optArgInBackend = false)
    {
        hasHeapArguments = has;
        byteCodeFunction->SetDoBackendArgumentsOptimization(optArgInBackend);
    }

    bool GetIsEventHandler() const {
        return isEventHandler;
    }

    void SetIsEventHandler(bool is) {
        isEventHandler = is;
    }

    bool GetChildCallsEval() const {
        return childCallsEval;
    }

    void SetChildCallsEval(bool does) {
        childCallsEval = does;
    }

    bool GetHasLocalInClosure() const {
        return hasLocalInClosure;
    }

    void SetHasLocalInClosure(bool has) {
        hasLocalInClosure = has;
    }

    bool GetHasClosureReference() const {
        return hasClosureReference;
    }

    void SetHasCachedScope(bool has) {
        hasCachedScope = has;
    }

    bool GetHasCachedScope() const {
        return hasCachedScope;
    }

    void SetFuncExprNameReference(bool has) {
        funcExprNameReference = has;
    }

    bool GetFuncExprNameReference() const {
        return funcExprNameReference;
    }

    void SetHasClosureReference(bool has) {
        hasClosureReference = has;
    }

    bool GetHasGlobalRef() const {
        return hasGlobalReference;
    }

    void SetHasGlobalRef(bool has) {
        hasGlobalReference = has;
    }

    bool GetIsStrictMode() const {
        return this->byteCodeFunction->GetIsStrictMode();
    }

    bool Escapes() const {
        return escapes;
    }

    void SetEscapes(bool does) {
        escapes = does;
    }

    bool HasMaybeEscapedNestedFunc() const {
        return hasEscapedUseNestedFunc;
    }

    void SetHasMaybeEscapedNestedFunc(DebugOnly(wchar_t const * reason));

    bool IsDeferred() const {
        return root && root->sxFnc.pnodeBody == NULL;
    }

    bool IsRestored()
    {
        // FuncInfo are from RestoredScopeInfo
        return root == null;
    }

    bool HasDeferredChild() const {
        return hasDeferredChild;
    }

    void SetHasDeferredChild() {
        hasDeferredChild = true;
    }

    Js::FunctionBody* GetParsedFunctionBody()
    {
        AssertMsg(this->byteCodeFunction->IsFunctionParsed(), "Function must be parsed in order to call this method");
        Assert(!IsDeferred());

        return this->byteCodeFunction->GetFunctionBody();
    }

    bool ChildHasWith() const {
        return childHasWith;
    }

    void SetChildHasWith() {
        childHasWith = true;
    }

    bool HasCapturedThis() const {
        return hasCapturedThis;
    }

    void SetHasCapturedThis() {
        hasCapturedThis = true;
    }

    BOOL HasSuperReference() const
    {
        return root->sxFnc.HasSuperReference();
    }

    BOOL IsClassMember() const
    {
        return root->sxFnc.IsClassMember();
    }

    BOOL IsLambda() const {
        return root->sxFnc.IsLambda();
    }

    void RemoveTargetStmt(ParseNode* pnodeStmt) {
        targetStatements.Remove(pnodeStmt);
    }

    void AddTargetStmt(ParseNode *pnodeStmt) {
        targetStatements.Prepend(pnodeStmt);
    }

    Js::RegSlot LookupDouble(double d) {
        return doubleConstantToRegister.Lookup(d,Js::Constants::NoRegister);
    }

    bool TryGetDoubleLoc(double d, Js::RegSlot *loc) {
        Js::RegSlot ret=LookupDouble(d);
        *loc=ret;
        return(ret!=Js::Constants::NoRegister);
    }

    void AddDoubleConstant(double d, Js::RegSlot location) {
        doubleConstantToRegister.Item(d,location);
    }

    bool NeedEnvRegister() const { return this->needEnvRegister; }
    void SetNeedEnvRegister() { this->needEnvRegister = true; };
    Js::RegSlot GetEnvRegister() const
    {
        Assert(this->envRegister != Js::Constants::NoRegister);
        return this->envRegister;
    }
    Js::RegSlot AssignEnvRegister(bool constReg)
    {
        Assert(needEnvRegister);
        Assert(this->envRegister == Js::Constants::NoRegister);
        Js::RegSlot reg = constReg? NextConstRegister() : NextVarRegister();
        this->envRegister = reg;
        return reg;
    }

    Js::RegSlot AssignThisRegister()
    {
        if (this->thisPointerRegister == Js::Constants::NoRegister)
        {
            this->thisPointerRegister = NextVarRegister();
        }
        return this->thisPointerRegister;
    }

    Js::RegSlot AssignSuperRegister()
    {
        if (this->superRegister == Js::Constants::NoRegister)
        {
            this->superRegister = NextVarRegister();
        }
        return this->superRegister;
    }

    Js::RegSlot AssignNullConstRegister()
    {
        if (this->nullConstantRegister == Js::Constants::NoRegister)
        {
            this->nullConstantRegister = NextConstRegister();
        }
        return this->nullConstantRegister;
    }

    Js::RegSlot AssignUndefinedConstRegister()
    {
        if (this->undefinedConstantRegister == Js::Constants::NoRegister)
        {
            this->undefinedConstantRegister = NextConstRegister();
        }
        return this->undefinedConstantRegister;
    }

    Js::RegSlot AssignTrueConstRegister()
    {
        if (this->trueConstantRegister == Js::Constants::NoRegister)
        {
            this->trueConstantRegister = NextConstRegister();
        }
        return this->trueConstantRegister;
    }

    Js::RegSlot AssignFalseConstRegister()
    {
        if (this->falseConstantRegister == Js::Constants::NoRegister)
        {
            this->falseConstantRegister = NextConstRegister();
        }
        return this->falseConstantRegister;
    }

    Js::RegSlot AssignYieldRegister()
    {
        AssertMsg(this->yieldRegister == Js::Constants::NoRegister, "yield register should only be assigned once by FinalizeRegisters()");
        this->yieldRegister = NextVarRegister();
        return this->yieldRegister;
    }

    Js::RegSlot AssignStackClosureRegs()
    {
        if (this->stackClosureReg == Js::Constants::NoRegister)
        {
            this->stackClosureReg = NextVarRegister();
            this->GetParsedFunctionBody()->SetStackClosureRegister(this->stackClosureReg);
            NextVarRegister();
        }

        return this->stackClosureReg;
    }

    Js::RegSlot GetStackScopeSlotsReg()
    {
        return this->stackClosureReg;
    }

    Js::RegSlot GetStackFrameDisplayReg()
    {
        return this->stackClosureReg + 1;
    }

    void StartRecordingOutArgs(unsigned int argCount)
    {
#if DBG
        outArgsDepth++;
#endif
        // We should have checked for argCount overflow already
        Assert(argCount == (Js::ArgSlot)argCount);

        // Add one for the space to save the m_outParams pointer in InterpreterStackFrame::PushOut
        unsigned int outArgsCount = argCount + 1;
        outArgsCurrentExpr += (Js::ArgSlot)outArgsCount;

        // Check for overflow
        if ((Js::ArgSlot)outArgsCount != outArgsCount || outArgsCurrentExpr < outArgsCount)
        {
            Js::Throw::OutOfMemory();
        }
        outArgsMaxDepth = max(outArgsMaxDepth, outArgsCurrentExpr);
    }

    void EndRecordingOutArgs(Js::ArgSlot argCount)
    {
        AssertMsg(outArgsDepth > 0, "mismatched Start and End");
        Assert(outArgsCurrentExpr >= argCount);
#if DBG
        outArgsDepth--;
#endif
        // Add one to pop the space to save the m_outParams pointer
        outArgsCurrentExpr -= (argCount + 1);

        Assert(outArgsDepth != 0 || outArgsCurrentExpr == 0);
    }

    Js::RegSlot AcquireLoc(ParseNode *pnode);
    Js::RegSlot AcquireTmpRegister();
    void ReleaseLoc(ParseNode *pnode);
    void ReleaseReference(ParseNode *pnode);
    void ReleaseLoad(ParseNode *pnode);
    void ReleaseTmpRegister(Js::RegSlot tmpReg);

    uint FindOrAddReferencedPropertyId(Js::PropertyId propertyId);

    uint FindOrAddRootObjectInlineCacheId(Js::PropertyId propertyId, bool isLoadMethod, bool isStore);

    uint FindOrAddInlineCacheId(Js::RegSlot instanceSlot, Js::PropertyId propertySlot, bool isLoadMethod, bool isStore)
    {
        Assert(instanceSlot != Js::Constants::NoRegister);
        Assert(propertySlot != Js::Constants::NoProperty);
        Assert(!isLoadMethod || !isStore);

        InlineCacheIdMap *properties;
        uint cacheId;

        if (isStore)
        {
            //      ... = foo.toString;
            //      foo.toString = ...;
            // We need a new cache here to ensure SetProperty() is called, which will set the side-effect bit
            // on the scriptContext.
            switch (propertySlot)
            {
            case Js::PropertyIds::valueOf:
                cacheId = this->NewInlineCache();
                valueOfStoreCacheIds.Prepend(alloc, cacheId);
                return cacheId;

            case Js::PropertyIds::toString:
                cacheId = this->NewInlineCache();
                toStringStoreCacheIds.Prepend(alloc, cacheId);
                return cacheId;
            };
        }

        if (!inlineCacheMap->TryGetValue(instanceSlot, &properties))
        {
            properties = Anew(alloc, InlineCacheIdMap, alloc, 17);
            inlineCacheMap->Add(instanceSlot, properties);
        }

        InlineCacheList* cacheList;
        if (!properties->TryGetValue(propertySlot, &cacheList))
        {
            cacheList = Anew(alloc, InlineCacheList, alloc);
            properties->Add(propertySlot, cacheList);
        }

        // If we share inline caches we should never have more than one entry in the list.
        Assert(Js::FunctionBody::ShouldShareInlineCaches() || cacheList->Count() <= 1);

        InlineCacheUnit cacheIdUnit;

        if (Js::FunctionBody::ShouldShareInlineCaches() && !cacheList->Empty())
        {
            cacheIdUnit = cacheList->Head();
            if (isLoadMethod)
            {
                if (cacheIdUnit.loadMethodCacheId == (uint)-1)
                {
                    cacheIdUnit.loadMethodCacheId = this->NewInlineCache();
                }
                cacheId = cacheIdUnit.loadMethodCacheId;
            }
            else if (isStore)
            {
                if (cacheIdUnit.storeCacheId == (uint)-1)
                {
                    cacheIdUnit.storeCacheId = this->NewInlineCache();
                }
                cacheId = cacheIdUnit.storeCacheId;
            }
            else
            {
                if (cacheIdUnit.loadCacheId == (uint)-1)
                {
                    cacheIdUnit.loadCacheId = this->NewInlineCache();
                }
                cacheId = cacheIdUnit.loadCacheId;
            }
            cacheList->Head() = cacheIdUnit;
        }
        else
        {
            cacheId = this->NewInlineCache();
            if (Js::FunctionBody::ShouldShareInlineCaches())
            {
                if (isLoadMethod)
                {
                    cacheIdUnit.loadCacheId = (uint)-1;
                    cacheIdUnit.loadMethodCacheId = cacheId;
                    cacheIdUnit.storeCacheId = (uint)-1;
                }
                else if (isStore)
                {
                    cacheIdUnit.loadCacheId = (uint)-1;
                    cacheIdUnit.loadMethodCacheId = (uint)-1;
                    cacheIdUnit.storeCacheId = cacheId;
                }
                else
                {
                    cacheIdUnit.loadCacheId = cacheId;
                    cacheIdUnit.loadMethodCacheId = (uint)-1;
                    cacheIdUnit.storeCacheId = (uint)-1;
                }
            }
            else
            {
                cacheIdUnit.cacheId = cacheId;
            }
            cacheList->Prepend(cacheIdUnit);
        }

        return cacheId;
    }

    Js::ProfileId FindOrAddSlotProfileId(Scope* scope, Js::PropertyId propertyId)
    {
        SlotKey key;

        key.scope = scope;
        key.slot = propertyId;
        Js::ProfileId profileId = Js::Constants::NoProfileId;

        if (!this->slotProfileIdMap.TryGetValue(key, &profileId))
        {
            Assert(this->byteCodeFunction->IsFunctionParsed());
            if (this->byteCodeFunction->GetFunctionBody()->AllocProfiledSlotId(&profileId))
            {
                this->slotProfileIdMap.Add(key, profileId);
            }
        }

        return profileId;
    }

    void EnsureThisScopeSlot()
    {
        if (this->thisScopeSlot == Js::Constants::NoRegister)
        {
            Scope* scope = this->bodyScope->IsGlobalEvalBlockScope() ? this->GetGlobalEvalBlockScope() : this->bodyScope;
            this->thisScopeSlot = scope->AddScopeSlot();
        }
    }

    void EnsureSuperScopeSlot()
    {
        if (this->superScopeSlot == Js::Constants::NoRegister)
        {
            this->superScopeSlot = this->bodyScope->AddScopeSlot();
        }
    }

    void SetIsThisLexicallyCaptured()
    {
        this->isThisLexicallyCaptured = true;
    }

    void SetIsSuperLexicallyCaptured()
    {
        this->isSuperLexicallyCaptured = true;
    }

    Scope * GetGlobalBlockScope() const;
    Scope * GetGlobalEvalBlockScope() const;

    FuncInfo *GetCurrentChildFunction() const
    {
        return this->currentChildFunction;
    }

    void SetCurrentChildFunction(FuncInfo *funcInfo)
    {
        this->currentChildFunction = funcInfo;
    }

    Scope *GetCurrentChildScope() const
    {
        return this->currentChildScope;
    }

    void SetCurrentChildScope(Scope *scope)
    {
        this->currentChildScope = scope;
    }

    SymbolTable *GetCapturedSyms() const { return capturedSyms; }

    void OnStartVisitFunction(ParseNode *pnodeFnc);
    void OnEndVisitFunction(ParseNode *pnodeFnc);
    void OnStartVisitScope(Scope *scope);
    void OnEndVisitScope(Scope *scope);
    void AddCapturedSym(Symbol *sym);
    CapturedSymMap *EnsureCapturedSymMap();

#if DBG_DUMP
    void Dump();
#endif
};

class ByteCodeGenerator
{
private:
    static SymbolName const argumentsName; // arguments

    Js::ScriptContext* scriptContext;
    ArenaAllocator *alloc;
    ulong flags;
    Js::PropertyRecordList* propertyRecords;
    SList<FuncInfo*> *funcInfoStack;
    ParseNode *currentBlock;
    ParseNode *currentTopStatement;
    Scope *currentScope;
    Scope *globalScope; // the global members will be in this scope
    Js::ScopeInfo* parentScopeInfo;
    Js::ByteCodeWriter  m_writer;

#if DBG
    bool executingGenerate;
#endif

    // pointer to the root function wrapper that will be invoked by the caller
    Js::ParseableFunctionInfo * pRootFunc;

    long maxAstSize;
    uint16 envDepth;
    uint sourceIndex;
    uint dynamicScopeCount;
    uint loopDepth;
    uint visitIndirectDepth;
    uint16 m_callSiteId;
    NativeCodeGenerator * nativeCodeGen;
    bool NoNative;
    bool deadLoopPossible;
    bool callsConstructor;
    bool isBinding;
    bool trackEnvDepth;
    bool funcEscapes;
    bool inPrologue;
    bool inDestructuredArray;
    ParseNode* outerLoop;
    Parser* parser; // currently active parser (used for AST transformation)

    Js::Utf8SourceInfo *m_utf8SourceInfo;

    // The stack walker won't be able to find the current function being defer parse, pass in
    // The address so we can patch it up if it is a stack function and we need to box it.
    Js::ScriptFunction ** functionRef;
public:
    // This points to the current function body which can be reused at the time parsing (called due to deffered parsing logic) a subtree.
    Js::FunctionBody * pCurrentFunction;

    bool InDestructuredArray() const { return inDestructuredArray; }
    void SetInDestructuredArray(bool in) { inDestructuredArray = in; }

    bool InPrologue() const { return inPrologue; }
    void SetInPrologue(bool val) { inPrologue = val; }
    Parser* GetParser() { return parser; }
    Js::ParseableFunctionInfo * GetRootFunc(){return pRootFunc;}
    void SetRootFuncInfo(FuncInfo* funcInfo);
    // Treat the return value register like a constant register so that the byte code writer maps it to the bottom
    // of the register range.
    static const Js::RegSlot ReturnRegister = REGSLOT_TO_CONSTREG(Js::FunctionBody::ReturnValueRegSlot);
    static const Js::RegSlot RootObjectRegister = REGSLOT_TO_CONSTREG(Js::FunctionBody::RootObjectRegSlot);
    static const unsigned int DefaultArraySize=0;  // This __must__ be '0' so that "(new Array()).length == 0"
    static const unsigned int MinArgumentsForCallOptimization = 16;
    bool forceNoNative;    

    ByteCodeGenerator(Js::ScriptContext* scriptContext, Js::ScopeInfo* parentScopeInfo);

#if DBG_DUMP
    bool Trace() const {
        return Js::Configuration::Global.flags.Trace.IsEnabled(Js::ByteCodePhase);
    }
#else
    bool Trace() const
    {
        return false;
    }
#endif

    Js::ScriptContext* GetScriptContext() { return scriptContext; }

    Scope *GetCurrentScope() const { return currentScope; }

    void SetCurrentBlock(ParseNode *pnode) { currentBlock = pnode; }
    ParseNode *GetCurrentBlock() const { return currentBlock; }

    void SetCurrentTopStatement(ParseNode *pnode) { currentTopStatement = pnode; }
    ParseNode *GetCurrentTopStatement() const { return currentTopStatement; }

    void SetCallsConstructor(bool b) {
        callsConstructor=b;
    }

    void SetDeadLoopPossible(bool b) {
        deadLoopPossible=b;
    }

    ParseNode* GetOuterLoop() { return outerLoop; }

    bool GetDeadLoopPossible() {
        return deadLoopPossible;
    }

    Js::ModuleID GetModuleID() const
    {
        return m_utf8SourceInfo->GetSrcInfo()->moduleID;
    }

    void SetFlags(ulong grfscr)
    {
        flags = grfscr;
    }

    ulong GetFlags(void)
    {
        return flags;
    }

    bool IsBinding() const {
        return isBinding;
    }

    Js::ByteCodeWriter *Writer() {
        return &m_writer;
    }

    ArenaAllocator *GetAllocator() {
        return alloc;
    }

    SymbolName const& GetArgumentsName()
    {
        return argumentsName;
    }

    Js::PropertyRecordList* EnsurePropertyRecordList()
    {
        if (this->propertyRecords == null)
        {
            Recycler* recycler = this->scriptContext->GetRecycler();
            this->propertyRecords = RecyclerNew(recycler, Js::PropertyRecordList, recycler);
        }

        return this->propertyRecords;
    }

    bool IsEvalWithBlockScopingNoParentScopeInfo()
    {
        return (flags & fscrEvalCode) && !HasParentScopeInfo() && scriptContext->GetConfig()->IsBlockScopeEnabled();
    }

    Js::ProfileId GetNextCallSiteId(Js::OpCode op)
    {
        if (m_writer.ShouldIncrementCallSiteId(op))
        {
            if (m_callSiteId != Js::Constants::NoProfileId)
            {
                return m_callSiteId++;
            }
        }
        return m_callSiteId;
    }

    Js::RegSlot NextVarRegister();
    Js::RegSlot NextConstRegister();
    FuncInfo *TopFuncInfo() const;

    void EnterLoop() {  if (this->TopFuncInfo()) { this->TopFuncInfo()->hasLoop = true; } loopDepth++; }
    void ExitLoop() { loopDepth--; }
    BOOL IsInLoop() const { return loopDepth > 0; }
    // TODO: per-function register assignment for env and global symbols
    void AssignRegister(Symbol *sym);
    void AddTargetStmt(ParseNode *pnodeStmt);
    Js::RegSlot AssignNullConstRegister();
    Js::RegSlot AssignUndefinedConstRegister();
    Js::RegSlot AssignTrueConstRegister();
    Js::RegSlot AssignFalseConstRegister();
    Js::RegSlot AssignThisRegister();
    void SetNeedEnvRegister();
    void AssignFrameObjRegister();
    void AssignFrameSlotsRegister();
    void AssignFrameDisplayRegister();

    void InitScopeSlotArray(FuncInfo * funcInfo);
    void FinalizeRegisters(FuncInfo * funcInfo, Js::FunctionBody * byteCodeFunction);
    void SetHasTry(bool has);
    void SetHasFinally(bool has);
    void SetNumberOfInArgs(Js::ArgSlot argCount);
    Js::RegSlot EnregisterConstant(unsigned int constant);
    Js::RegSlot EnregisterStringConstant(IdentPtr pid);
    Js::RegSlot EnregisterDoubleConstant(double d);
    Js::RegSlot EnregisterStringTemplateCallsiteConstant(ParseNode* pnode);

    static Js::JavascriptArray* BuildArrayFromStringList(ParseNode* stringNodeList, uint arrayLength, Js::ScriptContext* scriptContext);

    bool HasParentScopeInfo() const
    {
        return this->parentScopeInfo != null;
    }
    void RestoreScopeInfo(Js::FunctionBody* funcInfo);
    FuncInfo *StartBindGlobalStatements(ParseNode *pnode);
    void AssignPropertyId(Symbol *sym, Js::ParseableFunctionInfo* functionInfo);
    void AssignPropertyId(IdentPtr pid);

    void ProcessCapturedSyms(ParseNode *pnodeFnc);

    void EnterVisitIndirect() { if (++this->visitIndirectDepth == 0) Js::Throw::OutOfMemory(); }
    void LeaveVisitIndirect() { --this->visitIndirectDepth; Assert(this->visitIndirectDepth != 0xffffffff); }
    bool InVisitIndirect() const { return this->visitIndirectDepth != 0; }

    void RecordAllIntConstants(FuncInfo * funcInfo);
    void RecordAllStrConstants(FuncInfo * funcInfo);
    void RecordAllStringTemplateCallsiteConstants(FuncInfo* funcInfo);

    // for now, this just assigns field ids for the current script
    // later, we will combine this information with the global field id map
    // this temporary code will not work if a global member is accessed both with and without a lhs
    void AssignPropertyIds(Js::ParseableFunctionInfo* functionInfo);
    void MapCacheIdsToPropertyIds(FuncInfo *funcInfo);
    void MapReferencedPropertyIds(FuncInfo *funcInfo);
    FuncInfo *StartBindFunction(const wchar_t *name, int nameLength, bool* pfuncExprWithName, ParseNode *pnode);
    void EndBindFunction(bool funcExprWithName);
    void StartBindProg(ParseNode *pnode);
    void EndBindProg();
    void StartBindCatch(ParseNode *pnode);

    // Block scopes related functions
    template<class Fn> void IterateBlockScopedVariables(ParseNode *pnodeBlock, Fn fn);
    void InitBlockScopedContent(ParseNode *pnodeBlock, Js::DebuggerScope *debuggerScope, FuncInfo *funcInfo);

    Js::DebuggerScope* RecordStartScopeObject(ParseNode *pnodeBlock, Js::DiagExtraScopesType scopeType, Js::RegSlot scopeLocation = Js::Constants::NoRegister, int* index = nullptr);
    void RecordEndScopeObject(ParseNode *pnodeBlock);

    bool IsDeadLoop(ParseNode* pnode,FuncInfo* funcInfo);
    void EndBindCatch();
    void StartEmitFunction(ParseNode *pnodeFnc);
    void EndEmitFunction(ParseNode *pnodeFnc);
    void StartEmitBlock(ParseNode *pnodeBlock);
    void EndEmitBlock(ParseNode *pnodeBlock);
    void StartEmitCatch(ParseNode *pnodeCatch);
    void EndEmitCatch(ParseNode *pnodeCatch);
    void StartEmitWith(ParseNode *pnodeWith);
    void EndEmitWith(ParseNode *pnodeWith);
    void EnsureFncScopeSlots(ParseNode *pnode, FuncInfo *funcInfo);
    void EnsureLetConstScopeSlots(ParseNode *pnodeBlock, FuncInfo *funcInfo);
    void PushScope(Scope *innerScope);
    void PopScope();
    void PushBlock(ParseNode *pnode);
    void PopBlock();

    __inline void PushFuncInfo(wchar_t const * location, FuncInfo* funcInfo)
    {
        // We might have multiple global scope for deferparse
        //Assert(!funcInfo->IsGlobalFunction() || this->TopFuncInfo() == null || this->TopFuncInfo()->IsGlobalFunction());
        if (PHASE_TRACE1(Js::ByteCodePhase))
        {
            Output::Print(L"%s: PushFuncInfo: %s", location, funcInfo->name);
            if (this->TopFuncInfo())
            {
                Output::Print(L" Top: %s", this->TopFuncInfo()->name);
            }
            Output::Print(L"\n");
            Output::Flush();
        }
        funcInfoStack->Push(funcInfo);
    }

    __inline void PopFuncInfo(wchar_t const * location)
    {
        FuncInfo * funcInfo = funcInfoStack->Pop();
        //Assert(!funcInfo->IsGlobalFunction() || this->TopFuncInfo() == null || this->TopFuncInfo()->IsGlobalFunction());
        if (PHASE_TRACE1(Js::ByteCodePhase))
        {
            Output::Print(L"%s: PopFuncInfo: %s", location, funcInfo->name);
            if (this->TopFuncInfo())
            {
                Output::Print(L" Top: %s", this->TopFuncInfo()->name);
            }
            Output::Print(L"\n");
            Output::Flush();
        }
    }

    Js::RegSlot PrependLocalScopes(Js::RegSlot evalEnv, Js::RegSlot tempLoc, FuncInfo *funcInfo);
    Symbol *FindSymbol(Symbol **symRef, IdentPtr pid, bool forReference = false);
    Symbol *FindLocalSymbol(SymbolName const& key);
    Symbol *AddSymbolToScope(Scope *scope, const wchar_t *key, int keyLength, ParseNode *varDecl, SymbolType symbolType);
    Symbol *AddSymbolToFunctionScope(const wchar_t *key, int keyLength, ParseNode *varDecl, SymbolType symbolType);
    void FuncEscapes(Scope *scope);
//    void PrintFuncInfo();
    void EmitTopLevelStatement(ParseNode *stmt, FuncInfo *funcInfo, BOOL fReturnValue);
    void EmitInvertedLoop(ParseNode* outerLoop,ParseNode* invertedLoop,FuncInfo* funcInfo);
    bool DefineFunctions(FuncInfo *funcInfoParent, bool userVarsDefined);
    Js::RegSlot DefineOneFunction(ParseNode *pnodeFnc, FuncInfo *funcInfoParent, bool generateAssignment=true, Js::RegSlot regEnv = Js::Constants::NoRegister, Js::RegSlot frameDisplayTemp = Js::Constants::NoRegister);
    Js::RegSlot DefineOneFunctionHandleBoxedFD(ParseNode *pnodeFnc, FuncInfo *funcInfo, bool generateAssignment, Js::RegSlot regEnv = Js::Constants::NoRegister);
    void DefineCachedFunctions(FuncInfo *funcInfoParent);
    bool DefineUncachedFunctions(FuncInfo *funcInfoParent, bool userVarsDefined);
    void DefineUserVars(FuncInfo *funcInfo);
    void InitBlockScopedNonTemps(ParseNode *pnode, FuncInfo *funcInfo);
    // temporarily load all constants and special registers in a single block
    void LoadAllConstants(FuncInfo *funcInfo);
    void LoadFrameDisplay(FuncInfo *funcInfo);
    void LoadHeapArguments(FuncInfo *funcInfo);
    void LoadUncachedHeapArguments(FuncInfo *funcInfo);
    void LoadCachedHeapArguments(FuncInfo *funcInfo);
    void LoadThisObject(FuncInfo *funcInfo, bool thisLoadedFromParams = false);
    void EmitThis(FuncInfo *funcInfo, Js::RegSlot fromRegister);
    void GetEnclosingNonLambdaScope(FuncInfo *funcInfo, Scope * &scope, Js::PropertyId &envIndex);
    void EmitInternalScopedSlotLoad(FuncInfo *funcInfo, Scope *scope, Js::PropertyId envIndex, Js::RegSlot slot, Js::RegSlot symbolRegister);

    // TODO: home the 'this' argument
    void EmitLoadFormalIntoRegister(ParseNode *pnodeFormal, Js::ArgSlot pos, FuncInfo *funcInfo);
    void HomeArguments(FuncInfo *funcInfo);

    void EnsureNoRedeclarations(ParseNode *pnodeBlock, FuncInfo *funcInfo);

    void DefineLabels(FuncInfo *funcInfo);
    void EmitProgram(ParseNode *pnodeProg);
    void EmitScopeList(ParseNode *pnode);
    void EmitDefaultArgs(FuncInfo *funcInfo, ParseNode *pnode);
    void EmitOneFunction(ParseNode *pnode);
    void EmitGlobalFncDeclInit(Js::RegSlot rhsLocation, Js::PropertyId propertyId, FuncInfo * funcInfo);
    void EmitLocalPropInit(Js::RegSlot rhsLocation, Symbol *sym, FuncInfo *funcInfo);
    void EmitPropStore(Js::RegSlot rhsLocation, Symbol *sym, IdentPtr pid, FuncInfo *funcInfo, bool isLet = false, bool isConst = false, bool isFncDeclVar = false);
    void EmitPropLoad(Js::RegSlot lhsLocation, Symbol *sym, IdentPtr pid, FuncInfo *funcInfo);
    void EmitPropDelete(Js::RegSlot lhsLocation, Symbol *sym, IdentPtr pid, FuncInfo *funcInfo);
    void EmitPropTypeof(Js::RegSlot lhsLocation, Symbol *sym, IdentPtr pid, FuncInfo *funcInfo);
    void EmitTypeOfFld(FuncInfo * funcInfo, Js::PropertyId propertyId, Js::RegSlot value, Js::RegSlot instance, Js::OpCode op1);

    void EmitLoadInstance(Symbol *sym, IdentPtr pid, Js::RegSlot *pThisLocation, Js::RegSlot *pTargetLocation, FuncInfo *funcInfo);
    void EmitGlobalBody(FuncInfo *funcInfo);
    void EmitFunctionBody( FuncInfo *funcInfo );
    void EmitAsmFunctionBody( FuncInfo *funcInfo );
    void EmitScopeObjectInit(FuncInfo *funcInfo);

    void EmitPatchableRootProperty(Js::OpCode opcode, Js::RegSlot regSlot, Js::PropertyId propertyId, bool isLoadMethod, bool isStore, FuncInfo *funcInfo);

    struct TryScopeRecord;
    JsUtil::DoublyLinkedList<TryScopeRecord> tryScopeRecordsList;
    void EmitLeaveOpCodesBeforeYield();
    void EmitTryBlockHeadersAfterYield();

    void InvalidateCachedOuterScopes(FuncInfo *funcInfo);

    bool InDynamicScope() const { return dynamicScopeCount != 0; }

    Scope * FindScopeForSym(Scope *symScope, Scope *scope, Js::PropertyId *envIndex, FuncInfo *funcInfo) const;

    static Js::OpCode GetStFldOpCode(bool isStrictMode, bool isRoot, bool isLetDecl, bool isConstDecl)
    {
        return isConstDecl ? (isRoot ? Js::OpCode::InitRootConstFld : Js::OpCode::InitConstFld) :
            isLetDecl ? (isRoot ? Js::OpCode::InitRootLetFld : Js::OpCode::InitLetFld) :
            isStrictMode ? (isRoot ? Js::OpCode::StRootFldStrict : Js::OpCode::StFldStrict) :
            isRoot ? Js::OpCode::StRootFld : Js::OpCode::StFld;
    }
    static Js::OpCode GetStFldOpCode(FuncInfo* funcInfo, bool isRoot, bool isLetDecl, bool isConstDecl)
    {
        return GetStFldOpCode(funcInfo->GetIsStrictMode(), isRoot, isLetDecl, isConstDecl);
    }
    static Js::OpCode GetScopedStFldOpCode(bool isStrictMode)
    {
        return isStrictMode ? Js::OpCode::ScopedStFldStrict : Js::OpCode::ScopedStFld;
    }
    static Js::OpCode GetScopedStFldOpCode(FuncInfo* funcInfo)
    {
        return GetScopedStFldOpCode(funcInfo->GetIsStrictMode());
    }
    static Js::OpCode GetStElemIOpCode(bool isStrictMode)
    {
        return isStrictMode ? Js::OpCode::StElemI_A_Strict : Js::OpCode::StElemI_A;
    }
    static Js::OpCode GetStElemIOpCode(FuncInfo* funcInfo)
    {
        return GetStElemIOpCode(funcInfo->GetIsStrictMode());
    }

    bool DoJitLoopBodies(FuncInfo *funcInfo) const
    {
        // Never jit loop bodies in a function with a try.
        // Otherwise, always jit loop bodies under /forcejitloopbody.
        // Otherwise, jit loop bodies unless we're in eval/"new Function" or feature is disabled.

        Assert(funcInfo->byteCodeFunction->IsFunctionParsed());
        Js::FunctionBody* functionBody = funcInfo->byteCodeFunction->GetFunctionBody();

        return functionBody->ForceJITLoopBody() || funcInfo->byteCodeFunction->IsJitLoopBodyPhaseEnabled();
    }

    static void Generate(__in ParseNode *pnode, ulong grfscr, __in ByteCodeGenerator* byteCodeGenerator, __inout Js::ParseableFunctionInfo ** ppRootFunc, __in uint sourceIndex, __in bool forceNoNative, __in Parser* parser, Js::ScriptFunction ** functionRef);
    void Begin(
        __in ArenaAllocator *alloc,
        __in ulong grfscr,
        __in Js::ParseableFunctionInfo* pRootFunc);

    void SetCurrentSourceIndex(uint sourceIndex) { this->sourceIndex = sourceIndex; }
    uint GetCurrentSourceIndex(){return sourceIndex;}   

    static inline bool IsFalse(ParseNode* node)
    {
        return (node->nop == knopInt && node->sxInt.lw == 0) || node->nop == knopFalse;
    }

    void StartStatement(ParseNode* node);
    void EndStatement(ParseNode* node);
    void StartSubexpression(ParseNode* node);
    void EndSubexpression(ParseNode* node);

    bool IsLanguageServiceEnabled() const;
    bool UseParserBindings() const;

    // Debugger methods.
    bool IsInDebugMode() const;
    bool IsInNonDebugMode() const;
    bool ShouldTrackDebuggerMetadata() const;
    void TrackRegisterPropertyForDebugger(Js::DebuggerScope *debuggerScope, Symbol *symbol, FuncInfo *funcInfo, Js::DebuggerScopePropertyFlags flags = Js::DebuggerScopePropertyFlags_None, bool isFunctionDeclaration = false);
    void TrackActivationObjectPropertyForDebugger(Js::DebuggerScope *debuggerScope, Symbol *symbol, Js::DebuggerScopePropertyFlags flags = Js::DebuggerScopePropertyFlags_None, bool isFunctionDeclaration = false);
    void TrackSlotArrayPropertyForDebugger(Js::DebuggerScope *debuggerScope, Symbol* symbol, Js::PropertyId propertyId, Js::DebuggerScopePropertyFlags flags = Js::DebuggerScopePropertyFlags_None, bool isFunctionDeclaration = false);
    void TrackFunctionDeclarationPropertyForDebugger(Symbol *functionDeclarationSymbol, FuncInfo *funcInfoParent);
    void UpdateDebuggerPropertyInitializationOffset(Js::RegSlot location, Js::PropertyId propertyId, bool shouldConsumeRegister = true);

    FuncInfo *FindEnclosingNonLambda()
    {
        for (Scope *scope = TopFuncInfo()->GetBodyScope(); scope; scope = scope->GetEnclosingScope())
        {
            if (!scope->GetFunc()->IsLambda())
            {
                return scope->GetFunc();
            }
        }
        Assert(0);
        return nullptr;
    }

    bool CanStackNestedFunc(FuncInfo * funcInfo, bool trace = false);
    void CheckDeferParseHasMaybeEscapedNestedFunc();
    bool NeedObjectAsFunctionScope(FuncInfo * funcInfo, ParseNode * pnodeFnc) const;
    bool HasInterleavingDynamicScope(Symbol * sym) const;

    void MarkThisUsedInLambda();

    void EmitInitCapturedThis(FuncInfo* funcInfo, Scope* scope);

    static bool NeedScopeObjectForArguments(FuncInfo *funcInfo, ParseNode *pnodeFnc)
    {
        // We can avoid creating a scope object with arguments present if:
        bool dontNeedScopeObject =
            // We have arguments, and
            funcInfo->GetHasHeapArguments()
            // Either we are in strict mode, or have strict mode formal semantics from a non-simple parameter list, and
            && (funcInfo->GetIsStrictMode()
                || !pnodeFnc->sxFnc.IsSimpleParameterList())
            // Neither of the scopes are objects (they may still be in the language service despite the conditions above.)
            && !funcInfo->paramScope->GetIsObject()
            && !funcInfo->bodyScope->GetIsObject();

        return funcInfo->GetHasHeapArguments()
            // Regardless of the conditions above, we won't need a scope object if there aren't any formals.
            && (pnodeFnc->sxFnc.pnodeArgs != nullptr || pnodeFnc->sxFnc.pnodeRest != nullptr)
            && !dontNeedScopeObject;
    }

private:
    bool NeedCheckBlockVar(Symbol* sym, Scope* scope, FuncInfo* funcInfo) const;
};

HRESULT GenerateByteCode(__in ParseNode *pnode, __in ulong grfscr, __in Js::ScriptContext* scriptContext, __inout Js::ParseableFunctionInfo ** ppRootFunc, __in uint sourceIndex, __in bool forceNoNative, __in Parser* parser, __in CompileScriptException *pse, Js::ScopeInfo* parentScopeInfo = nullptr, Js::ScriptFunction ** functionRef = nullptr);

//
// Output for -Trace:ByteCode
//
#if DBG_DUMP
#define TRACE_BYTECODE(fmt, ...) \
    if (Js::Configuration::Global.flags.Trace.IsEnabled(Js::ByteCodePhase)) \
    { \
        Output::Print(fmt, __VA_ARGS__); \
    }
#else
#define TRACE_BYTECODE(fmt, ...)
#endif

template <class Fn, bool mapRest>
void MapFormalsImpl(ParseNode *pnodeFunc, Fn fn)
{
    for (ParseNode *pnode = pnodeFunc->sxFnc.pnodeArgs; pnode != nullptr; pnode = pnode->sxVar.pnodeNext)
    {
        fn(pnode);
    }
    if (mapRest && pnodeFunc->sxFnc.pnodeRest != nullptr)
    {
        fn(pnodeFunc->sxFnc.pnodeRest);
    }
}

template <class Fn>
void MapFormalsWithoutRest(ParseNode *pnodeFunc, Fn fn)
{
    return MapFormalsImpl<Fn, false>(pnodeFunc, fn);
}

template <class Fn>
void MapFormals(ParseNode *pnodeFunc, Fn fn)
{
    return MapFormalsImpl<Fn, true>(pnodeFunc, fn);
}
