/*
 * ESBuiltinsSCT.h
 * This file is meant to be included by ScriptContextTelemetry.cpp a number of times.
 * Reference that file for more context.
 */

#ifndef SCT_STATE
#error "SCT_STATE is undefined - ESBuiltinsSCT is being used improperly!"
#endif

#if SCT_STATE == 0
            // Happens before anything in the telemetry subsystem

            // grab the data that will be necessary
            Js::LanguageStats* langStats = threadContext->GetLanguageStats();

            // Generate arrays of data.
#define BLOCK_START(blockname, count) UINT32 TOKENJOIN(blockname, _keys) [count+1] = {
#define BLOCK_END() 0}; // handle the trailing commas
#define ENTRY_BUILTIN(ver, type, base, name) crc32(FIELDNAMESTRING(type, base, name)),
#define ENTRY_LANGFEATURE(ver, name)
#define ENTRY_TELPOINT(name)
#include "ESBuiltinFields.h"
#undef ENTRY_TELPOINT
#undef ENTRY_LANGFEATURE
#undef ENTRY_BUILTIN
#undef BLOCK_END
#undef BLOCK_START

#define BLOCK_START(blockname, count) UINT32 TOKENJOIN(blockname, _props) [(count)+1] = {
#define BLOCK_END() 0};
#define Get(propertyId) \
    ( idx = ESBuiltInsDatabase::GetESBuiltInArrayIndex( propertyId ), idx != SIZE_MAX ? this->esBuiltinsTelemetryProvider->usageMap[ idx ] : 0 )
#define ENTRY_BUILTIN(ver, type, base, name) (UINT32)Get(ESBuiltInPropertyId:: ## type ## _ ## base ## _ ## name),
#define ENTRY_LANGFEATURE(ver, name)
#define ENTRY_TELPOINT(name)
#include "ESBuiltinFields.h"
#undef ENTRY_TELPOINT
#undef ENTRY_LANGFEATURE
#undef ENTRY_BUILTIN
#undef Get
#undef BLOCK_END
#undef BLOCK_START

#define BLOCK_START(blockname, count) UINT32 TOKENJOIN(blockname, _callCount) [(count)+1] = {
#define BLOCK_END() 0};
#define Get(propertyId) \
    ( idx = ESBuiltInsDatabase::GetESBuiltInArrayIndex( propertyId ), idx != SIZE_MAX ? this->esBuiltinsTelemetryProvider->usageMap[ idx ] : 0 )
#define ENTRY_BUILTIN(ver, type, base, name) (UINT32)langStats-> type ## _ ## base ## _ ## name .callCount,
#define ENTRY_LANGFEATURE(ver, name)
#define ENTRY_TELPOINT(name)
#include "ESBuiltinFields.h"
#undef ENTRY_TELPOINT
#undef ENTRY_LANGFEATURE
#undef ENTRY_BUILTIN
#undef Get
#undef BLOCK_END
#undef BLOCK_START

#define BLOCK_START(blockname, count) UINT32 TOKENJOIN(blockname, _debugModeCallCount) [(count)+1] = {
#define BLOCK_END() 0};
#define Get(propertyId) \
    ( idx = ESBuiltInsDatabase::GetESBuiltInArrayIndex( propertyId ), idx != SIZE_MAX ? this->esBuiltinsTelemetryProvider->usageMap[ idx ] : 0 )
#define ENTRY_BUILTIN(ver, type, base, name) (UINT32)langStats-> type ## _ ## base ## _ ## name .debugModeCallCount,
#define ENTRY_LANGFEATURE(ver, name)
#define ENTRY_TELPOINT(name)
#include "ESBuiltinFields.h"
#undef ENTRY_TELPOINT
#undef ENTRY_LANGFEATURE
#undef ENTRY_BUILTIN
#undef Get
#undef BLOCK_END
#undef BLOCK_START

            constexpr UINT32 UncategorizedFieldCount = 0
#define BLOCK_START(blockname, count)
#define BLOCK_END()
#define ENTRY_BUILTIN(ver, type, base, name)
#define ENTRY_LANGFEATURE(ver, name) +1
#define ENTRY_TELPOINT(name) +1
#include "ESBuiltinFields.h"
#undef ENTRY_TELPOINT
#undef ENTRY_LANGFEATURE
#undef ENTRY_BUILTIN
#undef BLOCK_END
#undef BLOCK_START
                ;

            UINT32 UncategorizedNameTags[UncategorizedFieldCount+1] = {
#define BLOCK_START(blockname, count)
#define BLOCK_END()
#define ENTRY_BUILTIN(ver, type, base, name)
#define ENTRY_LANGFEATURE(ver, name) crc32(FIELDNAMESTRING(type, base, name)),
#define ENTRY_TELPOINT(name) crc32(FIELDNAMESTRING(type, base, name)),
#include "ESBuiltinFields.h"
#undef ENTRY_TELPOINT
#undef ENTRY_LANGFEATURE
#undef ENTRY_BUILTIN
#undef BLOCK_END
#undef BLOCK_START
            0};

            UINT32 UncategorizedData[UncategorizedFieldCount+1] = {
#define BLOCK_START(blockname, count)
#define BLOCK_END()
#define ENTRY_BUILTIN(ver, type, base, name)
#define ENTRY_LANGFEATURE(ver, name) (langStats->name.parseCount),
#define ENTRY_TELPOINT(name) (langStats->name.count),
#include "ESBuiltinFields.h"
#undef ENTRY_TELPOINT
#undef ENTRY_LANGFEATURE
#undef ENTRY_BUILTIN
#undef BLOCK_END
#undef BLOCK_START
            0};

// end of #ifdef SCT_STATE == 0


#elif SCT_STATE == 1
        // allocate space for field names
        char _TlgNameUncatNT[sizeof("UncategorizedNameTags")]; UINT8 _TlgInUncatNT;
        char _TlgNameUncatD[sizeof("UncategorizedData")]; UINT8 _TlgInUncatD;
#define BLOCK_END()
#define BLOCK_START(blockname, count) char UNIQNAME(_TlgName) [sizeof(FIELDNAMESTRING(blockname,arr,keys))]; UINT8 UNIQNAME(_TlgIn); \
                                      char UNIQNAME(_TlgNameo) [sizeof(FIELDNAMESTRING(blockname,arr,props))]; UINT8 UNIQNAME(_TlgIno); \
                                      char UNIQNAME(_TlgNamec) [sizeof(FIELDNAMESTRING(blockname,arr,callCount))]; UINT8 UNIQNAME(_TlgInc); \
                                      char UNIQNAME(_TlgNamed) [sizeof(FIELDNAMESTRING(blockname,arr,dMCallCount))]; UINT8 UNIQNAME(_TlgInd);
#define ENTRY_BUILTIN(ver, type, base, name) //char UNIQNAME(_TlgName) [sizeof(FIELDNAMESTRING(type, base, name))]; UINT8 UNIQNAME(_TlgIn);
#define ENTRY_LANGFEATURE(ver, name)
#define ENTRY_TELPOINT(name)
#include "ESBuiltinFields.h"
#undef ENTRY_TELPOINT
#undef ENTRY_LANGFEATURE
#undef ENTRY_BUILTIN
#undef BLOCK_START
#undef BLOCK_END
// end of #elif SCT_STATE == 1


#elif SCT_STATE == 2
        // store the field names and types
        , ( "UncategorizedNameTags" ), TlgInUINT32|64
        , ( "UncategorizedData" ), TlgInUINT32|64
#define BLOCK_END()
#define BLOCK_START(blockname, count)  , ( FIELDNAMESTRING(blockname,arr,keys) ), TlgInUINT32|64 \
                                       , ( FIELDNAMESTRING(blockname,arr,props) ), TlgInUINT32|64 \
                                       , ( FIELDNAMESTRING(blockname,arr,callCount) ), TlgInUINT32|64 \
                                       , ( FIELDNAMESTRING(blockname,arr,dMCallCount) ), TlgInUINT32|64
#define ENTRY_BUILTIN(ver, type, base, name)
#define ENTRY_LANGFEATURE(ver, name)
#define ENTRY_TELPOINT(name)
#include "ESBuiltinFields.h"
#undef ENTRY_TELPOINT
#undef ENTRY_LANGFEATURE
#undef ENTRY_BUILTIN
#undef BLOCK_START
#undef BLOCK_END
// end of #elif SCT_STATE == 2


#elif SCT_STATE == 3
        // add the number of bytes for the fielddef
        // Uncategorized stuff:
        +(2+2)
        // Categorized stuff:
#define BLOCK_END()
#define BLOCK_START(blockname, count) +(2+2+2+2)
#define ENTRY_BUILTIN(ver, type, base, name)
#define ENTRY_LANGFEATURE(ver, name)
#define ENTRY_TELPOINT(name)
#include "ESBuiltinFields.h"
#undef ENTRY_TELPOINT
#undef ENTRY_LANGFEATURE
#undef ENTRY_BUILTIN
#undef BLOCK_START
#undef BLOCK_END
// end of #elif SCT_STATE == 3


#elif SCT_STATE == 4
        // reference the data
        _TlgCreateArray( &_TlgData[_TlgIdx], UncategorizedNameTags, UncategorizedFieldCount, sizeof(UINT32)), _TlgIdx += 2,
        _TlgCreateArray( &_TlgData[_TlgIdx], UncategorizedData, UncategorizedFieldCount, sizeof(UINT32)), _TlgIdx += 2,
#define BLOCK_END()
#define BLOCK_START(blockname, count) _TlgCreateArray( &_TlgData[_TlgIdx], TOKENJOIN(blockname,_keys), count, sizeof(UINT32)), _TlgIdx += 2, \
                                      _TlgCreateArray( &_TlgData[_TlgIdx], TOKENJOIN(blockname,_props), count, sizeof(UINT32)), _TlgIdx += 2, \
                                      _TlgCreateArray( &_TlgData[_TlgIdx], TOKENJOIN(blockname,_callCount), count, sizeof(UINT32)), _TlgIdx += 2, \
                                      _TlgCreateArray( &_TlgData[_TlgIdx], TOKENJOIN(blockname,_debugModeCallCount), count, sizeof(UINT32)), _TlgIdx += 2,
#define ENTRY_BUILTIN(ver, type, base, name) 
#define ENTRY_LANGFEATURE(ver, name)
#define ENTRY_TELPOINT(name)
#include "ESBuiltinFields.h"
#undef ENTRY_TELPOINT
#undef ENTRY_LANGFEATURE
#undef ENTRY_BUILTIN
#undef BLOCK_START
#undef BLOCK_END
// end of #elif SCT_STATE == 4


#elif SCT_STATE == 5
        // do any necessary clean-up
        // (none needed here)
// end of #elif SCT_STATE == 5


#elif SCT_STATE == -1
        // run only at compile-time to generate integrity checks
// end of #elif SCT_STATE == -1


#elif SCT_STATE == -2
        // register the telemetry provider with the ScriptContextTelemetry
        this->esBuiltinsTelemetryProvider = Anew(scriptContext.TelemetryAllocator(), ESBuiltInsTelemetryProvider, *this);
        this->telemetryProviders.Add( this->esBuiltinsTelemetryProvider );
// end of #elif SCT_STATE == -2


#elif SCT_STATE == -3
        // header definition of local property
        ESBuiltInsTelemetryProvider* esBuiltinsTelemetryProvider;
// end of #elif SCT_STATE == -3


#else
#error "SCT_STATE value not supported by ESBuiltinsSCT!"
#endif