//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

// Base class for TempTrackers.  Contain the basic data and merge logic 
class TempTrackerBase
{
public:    
#if DBG
    bool HasTempTransferDependencies() const { return tempTransferDependencies != null; }
#endif
protected:
    TempTrackerBase(JitArenaAllocator * alloc, bool inLoop);
    ~TempTrackerBase();
    void MergeData(TempTrackerBase * fromData, bool deleteData);
    void MergeDependencies(HashTable<BVSparse<JitArenaAllocator> *> * toData, HashTable<BVSparse<JitArenaAllocator> *> *& fromData, bool deleteData);
    void AddTransferDependencies(int sourceId, SymID dstSymId, HashTable<BVSparse<JitArenaAllocator> *> * dependencies);
    void AddTransferDependencies(BVSparse<JitArenaAllocator> * bv, SymID dstSymID);
    void OrHashTableOfBitVector(HashTable<BVSparse<JitArenaAllocator> *> * toData, HashTable<BVSparse<JitArenaAllocator> *> *& fromData, bool deleteData);

    JitArenaAllocator * GetAllocator() const;
    BVSparse<JitArenaAllocator>                nonTempSyms;
    BVSparse<JitArenaAllocator>                tempTransferredSyms;
    HashTable<BVSparse<JitArenaAllocator> *> * tempTransferDependencies;

#if DBG
    void Dump(wchar_t const * traceName);
#endif
};

// Actual temp tracker class, with a template plug-in model to determine what kind of temp we want to track
// (Number or Object)
template <typename T>
class TempTracker : public T
{
#if DBG
    friend class ObjectTempVerify;    
#endif
public:
    TempTracker(JitArenaAllocator * alloc, bool inLoop);
    void MergeData(TempTracker<T> * fromData, bool deleteData);

    // Actual mark temp algorithm that are shared, but have different condition based 
    // on the type of tracker as the tmeplate parameter    
    void ProcessUse(StackSym * sym, BackwardPass * backwardPass);        
    void MarkTemp(StackSym * sym, BackwardPass * backwardPass); 

#if DBG    
    void Dump() { __super::Dump(T::GetTraceName()); }
#endif
};

class NumberTemp : public TempTrackerBase
{
public:
    void ProcessInstr(IR::Instr * instr, BackwardPass * backwardPass);
    void ProcessPropertySymUse(IR::SymOpnd * symOpnd, IR::Instr * instr, BackwardPass * backwardPass);
    void ProcessIndirUse(IR::IndirOpnd * indirOpnd, IR::Instr * instr, BackwardPass * backwardPass);

protected:
    NumberTemp(JitArenaAllocator * alloc, bool inLoop);

    // Overrides of the base class for extra data merging
    void MergeData(NumberTemp * fromData, bool deleteData);

    // Function used by the TempTracker
    static bool IsTempUse(IR::Instr * instr, Sym * sym, BackwardPass * backwardPass);
    static bool IsTempTransfer(IR::Instr * instr);
    bool CanMarkTemp(IR::Instr * instr, BackwardPass * backwardPass);
    static void SetDstIsTemp(bool dstIsTemp, bool dstIsTempTransferred, IR::Instr * instr, BackwardPass * backwardPass);
   
    static bool IsTempProducing(IR::Instr * instr);
    bool HasExposedFieldDependencies(BVSparse<JitArenaAllocator> * bvTempTransferDependencies, BackwardPass * backwardPass);

    // Support for property transfer, so we can stack allocate number if it is assigned to another stack allocated object
    bool IsTempPropertyTransferLoad(IR::Instr * instr, BackwardPass * backwardPass);
    bool IsTempPropertyTransferStore(IR::Instr * instr, BackwardPass * backwardPass);
    void PropagateTempPropertyTransferStoreDependencies(SymID usedSymID, PropertySym * propertySym);    

    bool IsTempIndirTransferLoad(IR::Instr * instr, BackwardPass * backwardPass);

    bool IsInLoop() const { return propertyIdsTempTransferDependencies != null; }
    bool DoMarkTempNumbersOnTempObjects(BackwardPass * backwardPass) const;

#if DBG_DUMP    
    static bool DoTrace(BackwardPass * backwardPass);    
    static wchar_t const * GetTraceName() { return L"MarkTempNumber"; }
    void Dump(wchar_t const * traceName);
#endif

    // true if we have a LdElem_A from stack object that has non temp uses.
    bool nonTempElemLoad;

    // all the uses of values coming from LdElem_A, needed to detect dependencies on value set on stack objects
    BVSparse<JitArenaAllocator> elemLoadDependencies;

    // PropertyIds that has Ld*Fld  from stack object that has non temp uses.
    BVSparse<JitArenaAllocator> nonTempPropertyIds;

    // Per properties dependencies of Ld*Fld
    HashTable<BVSparse<JitArenaAllocator> *> * propertyIdsTempTransferDependencies;

    // Trace upward exposed mark tmep object fields
    HashTable<BVSparse<JitArenaAllocator> * > * upwardExposedMarkTempObjectSymsProperties;
    BVSparse<JitArenaAllocator> upwardExposedMarkTempObjectLiveFields;
};

typedef TempTracker<NumberTemp> TempNumberTracker;

class ObjectTemp : public TempTrackerBase
{
public:    
    static bool CanStoreTemp(IR::Instr * instr);
    static void ProcessInstr(IR::Instr * instr);
    void ProcessBailOnNoProfile(IR::Instr * instr); 

    // Used internally and by Globopt::GenerateBailOutMarkTempObjectIfNeeded
    static StackSym * GetStackSym(IR::Opnd * opnd, IR::PropertySymOpnd ** pPropertySymOpnd);
protected:
    // Place holder functions, only implemented for ObjectTempVerify
    ObjectTemp(JitArenaAllocator * alloc, bool inLoop) : TempTrackerBase(alloc, inLoop) { /* Do nothing */ }    

    // Function used by the TempTracker
    static bool IsTempUse(IR::Instr * instr, Sym * sym, BackwardPass * backwardPass);
    static bool IsTempTransfer(IR::Instr * instr);
    static bool CanMarkTemp(IR::Instr * instr, BackwardPass * backwardPass);
    static void SetDstIsTemp(bool dstIsTemp, bool dstIsTempTransferred, IR::Instr * instr, BackwardPass * backwardPass);
    

    // Object tracker doesn't support property transfer 
    // (So we don't stack allocate object if it is assigned to another stack allocated object)
    bool IsTempPropertyTransferLoad(IR::Instr * instr, BackwardPass * backwardPass) { return false; }
    bool IsTempPropertyTransferStore(IR::Instr * instr, BackwardPass * backwardPass) { return false; }
    void PropagateTempPropertyTransferStoreDependencies(SymID usedSymID, PropertySym * propertySym) { Assert(false); }
    bool IsTempIndirTransferLoad(IR::Instr * instr, BackwardPass * backwardPass) { return false; }
    bool HasExposedFieldDependencies(BVSparse<JitArenaAllocator> * bvTempTransferDependencies, BackwardPass * backwardPass) { return false; }

#if DBG_DUMP
    static bool DoTrace(BackwardPass * backwardPass);
    static wchar_t const * GetTraceName() { return L"MarkTempObject"; }
#endif
private:
    static bool IsTempProducing(IR::Instr * instr);
    static bool IsTempUseOpCodeSym(IR::Instr * instr, Js::OpCode opcode, Sym * sym);

    friend class NumberTemp;
#if DBG
    friend class ObjectTempVerify;
#endif
};

typedef TempTracker<ObjectTemp> TempObjectTracker;

#if DBG
class ObjectTempVerify : public TempTrackerBase
{
    friend class GlobOpt;    
public:
    void ProcessInstr(IR::Instr * instr, BackwardPass * backwardPass);    
    void NotifyBailOutRemoval(IR:: Instr * instr, BackwardPass * backwardPass);
    void NotifyDeadStore(IR::Instr * instr, BackwardPass * backwardPass);
    void NotifyDeadByteCodeUses(IR::Instr * instr);
    void NotifyReverseCopyProp(IR::Instr * instr);

    void MergeDeadData(BasicBlock * block);    
    static bool DependencyCheck(IR::Instr *instr, BVSparse<JitArenaAllocator> * bvTempTransferDependencies, BackwardPass * backwardPass);
protected:
    ObjectTempVerify(JitArenaAllocator * alloc, bool inLoop);
    void MergeData(ObjectTempVerify * fromData, bool deleteData);

    // Function used by the TempTracker
    static bool IsTempUse(IR::Instr * instr, Sym * sym, BackwardPass * backwardPass);
    static bool IsTempTransfer(IR::Instr * instr);
    static bool CanMarkTemp(IR::Instr * instr, BackwardPass * backwardPass);
    void SetDstIsTemp(bool dstIsTemp, bool dstIsTempTransferred, IR::Instr * instr, BackwardPass * backwardPass);    

    // Object tracker doesn't support property transfer 
    // (So we don't stack allocate object if it is assigned to another stack allocated object)
    bool IsTempPropertyTransferLoad(IR::Instr * instr, BackwardPass * backwardPass) { return false; }
    bool IsTempPropertyTransferStore(IR::Instr * instr, BackwardPass * backwardPass) { return false; }
    void PropagateTempPropertyTransferStoreDependencies(SymID usedSymID, PropertySym * propertySym)  { Assert(false); }
    bool IsTempIndirTransferLoad(IR::Instr * instr, BackwardPass * backwardPass) { return false; }
    bool HasExposedFieldDependencies(BVSparse<JitArenaAllocator> * bvTempTransferDependencies, BackwardPass * backwardPass) { return false; }

    static bool DoTrace(BackwardPass * backwardPass);
    static wchar_t const * GetTraceName() { return L"MarkTempObjectVerify"; }
private:
    BVSparse<JitArenaAllocator> removedUpwardExposedUse;
};

template <typename T> bool IsObjectTempVerify() { return false; }
template <> inline bool IsObjectTempVerify<ObjectTempVerify>() { return true; }

typedef TempTracker<ObjectTempVerify> TempObjectVerifyTracker;
#endif