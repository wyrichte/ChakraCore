//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

    struct props
    {
        uint callCount;
        uint debugModeCallCount;
    };

    struct langFeature
    {
        uint parseCount;
    };

ENTRY_BUILTIN(ArrayisArrayCount)
ENTRY_BUILTIN(ArrayIndexOfCount)
ENTRY_BUILTIN(ArrayEveryCount)
ENTRY_BUILTIN(ArrayFilterCount)
ENTRY_BUILTIN(ArrayForEachCount)
ENTRY_BUILTIN(ArrayLastIndexOfCount)
ENTRY_BUILTIN(ArrayMapCount)
ENTRY_BUILTIN(ArrayReduceCount)
ENTRY_BUILTIN(ArrayReduceRightCount)
ENTRY_BUILTIN(ArraySomeCount)

// Object builtins
ENTRY_BUILTIN(ObjectDefinePropertiesCount)
ENTRY_BUILTIN(ObjectCreateCount)
ENTRY_BUILTIN(ObjectSealCount)
ENTRY_BUILTIN(ObjectFreezeCount)
ENTRY_BUILTIN(ObjectPreventExtensionCount)
ENTRY_BUILTIN(ObjectIsSealedCount)
ENTRY_BUILTIN(ObjectIsFrozenCount)
ENTRY_BUILTIN(ObjectIsExtensibleCount)
ENTRY_BUILTIN(ObjectGetOwnPropertyNamesCount)
ENTRY_BUILTIN(ObjectGetPrototypeOfCount)
ENTRY_BUILTIN(ObjectKeysCount)

ENTRY_BUILTIN(DateToISOStringCount)
ENTRY_BUILTIN(FunctionBindCount)
ENTRY_BUILTIN(StringTrimCount)
ENTRY_BUILTIN(GetOwnPropertySymbolsCount)

//ES6 builtins
//Math builtins
ENTRY_BUILTIN(Log10Count)
ENTRY_BUILTIN(Log1pCount)
ENTRY_BUILTIN(Log2Count)
ENTRY_BUILTIN(Expm1Count)
ENTRY_BUILTIN(SinhCount)
ENTRY_BUILTIN(CoshCount)
ENTRY_BUILTIN(TanhCount)
ENTRY_BUILTIN(AsinhCount)
ENTRY_BUILTIN(AcoshCount)
ENTRY_BUILTIN(AtanhCount)
ENTRY_BUILTIN(HypotCount)
ENTRY_BUILTIN(CbrtCount)
ENTRY_BUILTIN(TruncCount)
ENTRY_BUILTIN(SignCount)
ENTRY_BUILTIN(ImulCount)
ENTRY_BUILTIN(Clz32Count)
ENTRY_BUILTIN(FroundCount)

// Number builtins
ENTRY_BUILTIN(IsNaNCount)
ENTRY_BUILTIN(IsFiniteCount)
ENTRY_BUILTIN(IsIntegerCount)
ENTRY_BUILTIN(IsSafeIntegerCount)

//String builtins
ENTRY_BUILTIN(StartsWithCount)
ENTRY_BUILTIN(EndsWithCount)
ENTRY_BUILTIN(ContainsCount)
ENTRY_BUILTIN(RepeatCount)

//Ctors
ENTRY_BUILTIN(ProxyCount)
ENTRY_BUILTIN(SymbolCount)
ENTRY_BUILTIN(MapCount)
ENTRY_BUILTIN(WeakMapCount)
ENTRY_BUILTIN(WeakSetCount)
ENTRY_BUILTIN(SetCount)
ENTRY_BUILTIN(PromiseCount)

//TypedArray Methods
ENTRY_BUILTIN(TAFromCount)
ENTRY_BUILTIN(TAOfCount)
ENTRY_BUILTIN(TACopyWithinCount)
ENTRY_BUILTIN(TAEntriesCount)
ENTRY_BUILTIN(TAEveryCount)
ENTRY_BUILTIN(TAFilterCount)
ENTRY_BUILTIN(TAFillCount)
ENTRY_BUILTIN(TAFindCount)
ENTRY_BUILTIN(TAFindIndexCount)
ENTRY_BUILTIN(TAForEachCount)
ENTRY_BUILTIN(TAIndexOfCount)
ENTRY_BUILTIN(TAJoinCount)
ENTRY_BUILTIN(TAKeysCount)
ENTRY_BUILTIN(TALastIndexOfCount)
ENTRY_BUILTIN(TAMapCount)
ENTRY_BUILTIN(TAReduceCount)
ENTRY_BUILTIN(TAReduceRightCount)
ENTRY_BUILTIN(TAReverseCount)
ENTRY_BUILTIN(TASomeCount)
ENTRY_BUILTIN(TASortCount)
ENTRY_BUILTIN(TASubArrayCount)
ENTRY_BUILTIN(TAValuesCount)

// Language Features
ENTRY_LANGFEATURE(LetCount)
ENTRY_LANGFEATURE(LambdaCount)
ENTRY_LANGFEATURE(StrictModeFunctionCount)
ENTRY_LANGFEATURE(SuperCount)
ENTRY_LANGFEATURE(ClassCount)
ENTRY_LANGFEATURE(AsmJSFunctionCount)
ENTRY_LANGFEATURE(StringTemplatesCount)
ENTRY_LANGFEATURE(ConstCount)
ENTRY_LANGFEATURE(GeneratorCount)
ENTRY_LANGFEATURE(RestCount)
ENTRY_LANGFEATURE(SpreadFeatureCount)
ENTRY_LANGFEATURE(UnicodeRegexFlagCount)
ENTRY_LANGFEATURE(StickyRegexFlagCount)
ENTRY_LANGFEATURE(DefaultArgFunctionCount)
