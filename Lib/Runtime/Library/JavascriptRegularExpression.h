//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    class JavascriptRegExp : public DynamicObject
    {
        friend class JavascriptRegularExpressionType;
        friend class RegexHelper;

    private:
        static PropertyId specialPropertyIds[];
        static const uint defaultSpecialPropertyIdsCount = 6;

        UnifiedRegex::RegexPattern* pattern;

        Var lastIndexVar;  // null => must build lastIndexVar from current lastIndex

    public:

        // Three states for lastIndex value:
        //  1. lastIndexVar has been updated, we must calculate lastIndex from it when we next need it
        static const CharCount NotCachedValue = (CharCount)-2;
    private:
        //  2. ToNumber(lastIndexVar) yields +inf or -inf or an integer not in range [0, MaxCharCount]
        static const CharCount InvalidValue = CharCountFlag;
        //  3. ToNumber(lastIndexVar) yields NaN, +0, -0 or an integer in range [0, MaxCharCount]
        CharCount lastIndexOrFlag;

        static JavascriptRegExp * GetJavascriptRegExp(Var var, ScriptContext* scriptContext);        

        bool GetPropertyBuiltIns(PropertyId propertyId, Var* value, BOOL* result);
        bool SetPropertyBuiltIns(PropertyId propertyId, Var value, PropertyOperationFlags flags, BOOL* result);
        bool GetSetterBuiltIns(PropertyId propertyId, PropertyValueInfo* info, DescriptorFlags* result);

        // For boxing stack instance
        JavascriptRegExp(JavascriptRegExp * instance);

        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(JavascriptRegExp);
    protected:
        DEFINE_VTABLE_CTOR(JavascriptRegExp, DynamicObject);
    public:
        JavascriptRegExp(UnifiedRegex::RegexPattern* pattern, DynamicType* type);
        JavascriptRegExp(DynamicType * type);

        static uint GetOffsetOfPattern() { return offsetof(JavascriptRegExp, pattern); }
        static uint GetOffsetOfLastIndexVar() { return offsetof(JavascriptRegExp, lastIndexVar); }
        static uint GetOffsetOfLastIndexOrFlag() { return offsetof(JavascriptRegExp, lastIndexOrFlag); }

        inline UnifiedRegex::RegexPattern* GetPattern() const { return pattern; }

        InternalString GetSource() const { return GetPattern()->GetSource(); }
        UnifiedRegex::RegexFlags GetFlags() const { return GetPattern()->GetFlags(); }

        void CacheLastIndex();
        inline CharCountOrFlag GetLastIndex() {
            if (lastIndexOrFlag == NotCachedValue)
                CacheLastIndex();
            return lastIndexOrFlag;
        }

        inline void SetLastIndex(CharCount lastIndex)
        {
            Assert(lastIndex <= MaxCharCount);
            lastIndexVar = 0;
            this->lastIndexOrFlag = lastIndex;
        }

        static bool Is(Var aValue);
        static JavascriptRegExp* FromVar(Var aValue);
        inline void SetRegex(UnifiedRegex::RegexPattern* pattern);

        static JavascriptRegExp* CreateRegEx(const wchar_t* pSource, CharCount sourceLen,
            UnifiedRegex::RegexFlags flags, ScriptContext *scriptContext);
        static JavascriptRegExp* CreateRegEx(Var aValue, Var options, ScriptContext *scriptContext);
        static UnifiedRegex::RegexPattern* CreatePattern(Var aValue, Var options, ScriptContext *scriptContext);
        static Var OP_NewRegEx(Var aCompiledRegex, ScriptContext* scriptContext);

        JavascriptString *ToString(bool sourceOnly = false);

        class EntryInfo
        {
        public:
            static FunctionInfo NewInstance;
            static FunctionInfo Exec;
            static FunctionInfo Test;
            static FunctionInfo ToString;
            static FunctionInfo GetterSymbolSpecies;
            // v5.8 only
            static FunctionInfo Compile;
        };

        static Var NewInstance(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryExec(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryTest(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryToString(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryGetterSymbolSpecies(RecyclableObject* function, CallInfo callInfo, ...);
        // v5.8 only
        static Var EntryCompile(RecyclableObject* function, CallInfo callInfo, ...);

        virtual bool HasReadOnlyPropertiesInvisibleToTypeHandler() override { return true; }

        virtual BOOL HasProperty(PropertyId propertyId) override;
        virtual BOOL GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL InitProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags = PropertyOperation_None, PropertyValueInfo* info = NULL) override;
        virtual BOOL DeleteProperty(PropertyId propertyId, PropertyOperationFlags flags) override;
        virtual DescriptorFlags GetSetter(PropertyId propertyId, Var* setterValue, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual DescriptorFlags GetSetter(JavascriptString* propertyNameString, Var* setterValue, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override;
        virtual BOOL GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override;
        virtual BOOL IsEnumerable(PropertyId propertyId) override;
        virtual BOOL IsConfigurable(PropertyId propertyId) override;
        virtual BOOL IsWritable(PropertyId propertyId) override;
        virtual BOOL GetSpecialPropertyName(uint32 index, Var *propertyName, ScriptContext * requestContext) override;
        virtual uint GetSpecialPropertyCount() const override;
        virtual PropertyId* GetSpecialPropertyIds() const override;

        virtual JavascriptRegExp* MakeCopyOnWriteObject(ScriptContext* scriptContext) override;

        static Js::JavascriptRegExp * BoxStackInstance(Js::JavascriptRegExp * instance);
    };

} // namespace Js
