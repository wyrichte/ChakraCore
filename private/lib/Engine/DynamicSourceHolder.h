//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------


#pragma once
class ISourceHolder;
namespace Js
{
    class DynamicSourceHolder : public ISourceHolder
    {
    private:
        enum MapRequestFor { Source = 1, Length = 2 };

        IActiveScriptByteCodeSource* sourceMapper;
        bool isSourceMapped;
        LPCUTF8 mappedSource;
        DWORD mappedSourceByteLength;

        // Wrapper methods with Asserts to ensure that we aren't trying to access unmapped source
        LPCUTF8 GetMappedSource()
        {
            AssertMsg(isSourceMapped, "Trying to access mapped source, but the source isn't mapped in.");
            AssertMsg(mappedSource != nullptr, "Our mapped source is nullptr, isSourceMapped (Assert above) should be false.");
            AssertMsg(sourceMapper != nullptr, "Source mapper is null, this means that this object has been finalized.");
            return mappedSource;
        };

        DWORD GetMappedSourceLength()
        {
            AssertMsg(isSourceMapped, "Trying to access mapped source length, but the source isn't mapped in.");
            AssertMsg(mappedSource != nullptr, "Our mapped source is nullptr, isSourceMapped (Assert above) should be false.");
            AssertMsg(sourceMapper != nullptr, "Source mapper is null, this means that this object has been finalized.");
            return mappedSourceByteLength;
        };

        // Core Map/Unmap methods
        void TryMapSource();
        void UnMapSource();

    public:
        DynamicSourceHolder(IActiveScriptByteCodeSource* sourceMapper) :
            sourceMapper(sourceMapper),
            isSourceMapped(false),
            mappedSourceByteLength(0),
            mappedSource(nullptr)
        {
            AssertMsg(sourceMapper != nullptr, "Source mapper given is null.");
            sourceMapper->AddRef();
        };

        // Returns false if the source was unable to map succesfully.
        bool EnsureSource(MapRequestFor requestedFor, const char16* reasonString)
        {
            if (!isSourceMapped)
            {
                TryMapSource();
#if ENABLE_DEBUG_CONFIG_OPTIONS
                AssertMsg(reasonString != nullptr, "Reason string for why we are mapping the source was not provided.");
                JS_ETW(EventWriteJSCRIPT_SOURCEMAPPING(reasonString, (ushort)requestedFor));
#endif
            }

            return isSourceMapped;
        }

        virtual bool IsEmpty() override
        {
            return false;
        }

        // Following two methods do not attempt any source mapping
        LPCUTF8 GetSourceUnchecked()
        {
            return this->GetMappedSource();
        }

        DWORD GetByteLengthUnchecked()
        {
            return this->GetMappedSourceLength();
        }

        // Following two methods are calls to EnsureSource before attempting to get the source
        // If EnsureSource returns false, these methods will return nullptr and 0 respectively.
        virtual LPCUTF8 GetSource(const char16* reasonString) override
        {
            if (this->EnsureSource(MapRequestFor::Source, reasonString))
            {
                return this->GetMappedSource();
            }

            return nullptr;
        }
                
        virtual size_t GetByteLength(const char16* reasonString) override
        {
            if (this->EnsureSource(MapRequestFor::Length, reasonString))
            {
                return this->GetMappedSourceLength();
            }

            return 0;
        }

        virtual void Finalize(bool isShutdown) override;

        virtual void Dispose(bool isShutdown) override
        {
        }

        virtual void Mark(Recycler * recycler) override
        {
        }

        virtual ISourceHolder* Clone(ScriptContext *scriptContext) override
        {
            return RecyclerNewFinalized(scriptContext->GetRecycler(), DynamicSourceHolder, this->sourceMapper);
        }

        bool Equals(ISourceHolder* other)
        {
            return this == other || 
                (this->GetByteLength(_u("Equal Comparison")) == other->GetByteLength(_u("Equal Comparison"))
                    && memcmp(this->GetSource(_u("Equal Comparison")), other->GetSource(_u("Equal Comparison")), this->GetByteLength(_u("Equal Comparison"))));
        }
        
        hash_t GetHashCode()
        {
            LPCUTF8 source = GetSource(_u("Hash Code Calculation"));
            size_t byteLength = GetByteLength(_u("Hash Code Calculation"));
            Assert(byteLength < MAXUINT32);
            return JsUtil::CharacterBuffer<utf8char_t>::StaticGetHashCode(source, (charcount_t)byteLength);
        }

        
        virtual bool IsDeferrable() override
        {
            return !PHASE_OFF1(Js::DeferSourceLoadPhase);
        }
    };
}
