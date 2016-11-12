#include "TelemetryPch.h"
#include "ESBuiltInsTelemetryProvider.h"

#ifdef TELEMETRY_ESB

#include "..\TelemetryMacros.h"

using namespace Js;

ESBuiltInsTelemetryProvider::ESBuiltInsTelemetryProvider( ScriptContextTelemetry& scriptContextTelemetry ) :
    scriptContextTelemetry( scriptContextTelemetry ),
    opcodeTelemetry( *this ),
    usageMap( ESBuiltInsDatabase::CreateUsageMap( *scriptContextTelemetry.GetScriptContext().TelemetryAllocator() ) )
#ifdef TELEMETRY_ESB_STRINGS
    ,
    nameMath( nullptr ), // JavascriptString::NewCopySzFromArena(_u("Math"), &scriptContextTelemetry.GetScriptContext(), scriptContextTelemetry.GetScriptContext().TelemetryAllocator() ) ),
    nameJson( nullptr ) // JavascriptString::NewCopySzFromArena(_u("JSON"), &scriptContextTelemetry.GetScriptContext(), scriptContextTelemetry.GetScriptContext().TelemetryAllocator() ) )
#endif
{
    // Register the opcodeTelemetry callback handler:
    scriptContextTelemetry.GetOpcodeTelemetry().esBuiltInsOpcodeTelemetry = &this->opcodeTelemetry;
}


ESBuiltInsTelemetryProvider::~ESBuiltInsTelemetryProvider()
{
    ESBuiltInsDatabase::FreeUsageMap( *scriptContextTelemetry.GetScriptContext().TelemetryAllocator(), this->usageMap );
}

void ESBuiltInsTelemetryProvider::OnPropertyEncountered( const Var instance, const PropertyId propertyId, const bool successful )
{
    count_OnPropertyEncountered++;
    
    // 0. If the propertyId is beyond the list of known properties, then abort, we're not interested!
    // 1. Get the ESBuiltInTypeId of `instance`.
    // 1.1. If the Js::TypeId is immediately mappable, get that.
    // 1.2. Otherwise, look up the type name as a string.
    // 1.3. If type name cannot be processed (object has no type name, name not found in database, etc), then abort and return.
    // 2. Get the ESBuiltInPropertyId
    // 2.1. Combine ESBuiltInTypeId and PropertyId together to form the ESBuiltInPropertyIdKey
    // 2.2. Get the ESBuiltInPropertyId from ESBuiltInPropertyIdKey
    // 3. Increment the usage of that member.

    if( propertyId >= Js::PropertyIds::_countJSOnlyProperty ) return;

    ESBuiltInPropertyId esbiPropertyId = ESBuiltInPropertyId::_None;
    bool isConstructorProperty;
    ESBuiltInTypeNameId esbiTypeNameId = this->GetESBuiltInTypeNameId( instance, propertyId, isConstructorProperty, esbiPropertyId );
    
    if( esbiTypeNameId == ESBuiltInTypeNameId::_None )
        return;

    if( esbiPropertyId == ESBuiltInPropertyId::_None ) // `esbiPropertyId` can be found via shortcut.
        esbiPropertyId = ESBuiltInsDatabase::GetESBuiltInPropertyId( esbiTypeNameId, isConstructorProperty, propertyId );

    if( esbiPropertyId != ESBuiltInPropertyId::_None )
        this->IncrementUseCount( esbiPropertyId );
}

void ESBuiltInsTelemetryProvider::OnConstructorCalled( const Js::Var constructorFunction )
{
    count_OnConstructorCalled++;
    
    // Ensure the constructor function is a valid function.
    // Get the name of the function, and get the ESBuiltInPropertyId for its constructor, and increment it.

    if( JavascriptFunction::Is( constructorFunction ) )
    {
        JavascriptFunction* function = JavascriptFunction::FromVar( constructorFunction );

        ESBuiltInTypeNameId esbiTypeNameId;
#ifdef TELEMETRY_ESB_STRINGS
        {
            JavascriptString* functionName = function->GetDisplayName( true );

            esbiTypeNameId = ESBuiltInsDatabase::GetESBuiltInTypeNameId( functionName );
        }
#else
        {
            esbiTypeNameId = ESBuiltInsDatabase::GetESBuiltInTypeNameId_ByPointer( this->scriptContextTelemetry.GetScriptContext(), function );
        }
#endif

        if( esbiTypeNameId != ESBuiltInTypeNameId::_None )
        {
            ESBuiltInPropertyId ctorInvocation = ESBuiltInsDatabase::GetESBuiltInPropertyId( esbiTypeNameId, /*isConstructorProperty:*/ true, PropertyIds::__constructor );
            if( ctorInvocation != ESBuiltInPropertyId::_None )
            {
                this->IncrementUseCount( ctorInvocation );
            }
        }
    }
    
}

/// <remarks>Even if <param ref="esBuiltInPropertyId" /> is `_None` it should be logged to ensure the telemetry code is operational.</remarks>
void ESBuiltInsTelemetryProvider::IncrementUseCount(const ESBuiltInPropertyId esBuiltInPropertyId)
{
    size_t idx = ESBuiltInsDatabase::GetESBuiltInArrayIndex( esBuiltInPropertyId );
    if( idx == SIZE_MAX ) return;
    byte* countPtr = &this->usageMap[ idx ];
    byte  count    = *countPtr;
    if( count++ == BYTE_MAX ) return;
    *countPtr = count;
}

bool ESBuiltInsTelemetryProvider::IsBuiltInMath(const Var& instance) const
{
    DynamicObject* math = this->scriptContextTelemetry.GetScriptContext().GetLibrary()->GetMathObject();

    return instance == math;
}

bool ESBuiltInsTelemetryProvider::IsBuiltInJson(const Var& instance) const
{
    DynamicObject* json = this->scriptContextTelemetry.GetScriptContext().GetLibrary()->GetJSONObject();

    return instance == json;
}

ESBuiltInTypeNameId ESBuiltInsTelemetryProvider::GetESBuiltInTypeNameId_Function( const Var instance, const PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ ESBuiltInPropertyId& shortcutPropertyFound )
{
    count_GetBuiltInTypeNameFunction++;
    
    // BUG: the property-accession `String.prototype` causes this method to return `(isConstructor: false, typeId: ESBuiltInTypeNameId::Function)` because `prototype` is a member of Function.
    // However it should return `(isConstructor: true, typeId: ESBuiltInTypeNameId::String)`

//#ifdef NEVER
    // `instance` could be accessing a property of Function (e.g. Function.bind), or it could be accessing a Constructor Property (e.g. Array.isArray).
    // So see if the PropertyId belongs to Function, and if so, return immediately before doing a name-check.

    ESBuiltInPropertyId definedOnFunction;
    definedOnFunction = ESBuiltInsDatabase::GetESBuiltInPropertyId( ESBuiltInTypeNameId::Function, /*isConstructorProperty:*/ false, propertyId );
    if( definedOnFunction != ESBuiltInPropertyId::_None )
    {
        isConstructorProperty = false;
        shortcutPropertyFound = definedOnFunction;
        return ESBuiltInTypeNameId::Function;
    }

    definedOnFunction = ESBuiltInsDatabase::GetESBuiltInPropertyId( ESBuiltInTypeNameId::Function, /*isConstructorProperty:*/ true, propertyId );
    if( definedOnFunction != ESBuiltInPropertyId::_None ) {
        // There is a bug here in that `Function_Constructor_length` or `Function_Constructor_prototype` will never be recorded as a Constructor property, as the names are also used by instances, and this code has no way of knowing if this is *a* function, or *the* Function function.
        isConstructorProperty = true;
        shortcutPropertyFound = definedOnFunction;
        return ESBuiltInTypeNameId::Function;
    }
//#endif

    // PropertyId does not belong to Function, therefore this is a Constructor property. But which Constructor is it?
    // There are two ways to solve this:
    // 1. Look up the TypeNameId by string (i.e. the name of the function), "Array.isArray" -> "Array" -> the global Array constructor function.
    // 2. Compare the pointer to the function to known pointers, `if( function == library.ArrayConstructor )`

    // Method 1 has the advantage of working to detect polyfills, but is slow as string comparisons, even when using a Trie, require lots of steps, repeatedly.
    // Method 2 only detects usage of implemented and non-overrriden built-ins, but is a lot faster.

    const JavascriptFunction* instanceAsFunction = JavascriptFunction::FromVar( instance );

    isConstructorProperty = true;

    ESBuiltInTypeNameId ret = ESBuiltInsDatabase::GetESBuiltInTypeNameId_ByPointer( this->scriptContextTelemetry.GetScriptContext(), instanceAsFunction );
    return ret;
}

ESBuiltInTypeNameId ESBuiltInsTelemetryProvider::GetESBuiltInTypeNameId_Object( const Var instance, const PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ ESBuiltInPropertyId& shortcutPropertyFound )
{
    count_GetBuiltInTypeNameObject++;
    
    if( this->IsBuiltInMath( instance ) ) return ESBuiltInTypeNameId::Math;
    if( this->IsBuiltInJson( instance ) ) return ESBuiltInTypeNameId::JSON;
    
    // `instance` could be a Polyfill Object. Inspect the name of its Constructor to see if it matches (e.g. `function DataView() { this.someDataViewProperty = 'abc'; };` ).
    // However, an optimization: don't name-check if the propertyId already belongs to Object (e.g. hasOwnProperty).

    ESBuiltInPropertyId definedOnObject = ESBuiltInsDatabase::GetESBuiltInPropertyId( ESBuiltInTypeNameId::Object, /*isConstructorProperty:*/ false, propertyId ); // `isConstructorProperty` will always be false because `instance` is not a function and so cannot be a constructor.
    if( definedOnObject != ESBuiltInPropertyId::_None )
    {
        isConstructorProperty = false;
        shortcutPropertyFound = definedOnObject;
        return ESBuiltInTypeNameId::Object;
    }

#ifdef TELEMETRY_ESB_GetConstructorPropertyPolyfillDetection
    {
        // PropertyId does not belong to Object, so get the object's constructor name and see if it's a built-in. This happens in the case of polyfilled constructors.

        // Get the Constructor property, as per DiagObjectModel.cpp, RecyclableObjectDisplay::Type() (line 1669)
        RecyclableObject*  instanceAsPropertyObject = RecyclableObject::FromVar( instance ); // I'm not sure why this is necessary.
        Var                value;
        PropertyValueInfo* info = nullptr;

        ScriptContext* scriptContextPtr = const_cast<ScriptContext*>( &this->scriptContextTelemetry.GetScriptContext() );

        bool ok = JavascriptOperators::GetProperty( instance, instanceAsPropertyObject, PropertyIds::constructor, &value, scriptContextPtr, info ) == TRUE;
        if( ok && JavascriptFunction::Is( value ) )
        {
            JavascriptFunction* constructorFunction = JavascriptFunction::FromVar( value );
            {
                ESBuiltInTypeNameId ret = ESBuiltInsDatabase::GetESBuiltInTypeNameId_ByPointer( this->scriptContextTelemetry.GetScriptContext(), constructorFunction );
                return ret;
            }
        }
        else
        {
            return ESBuiltInTypeNameId::_None;
        }
    }
#else
    {
        // Skip checking for polyfills, abort further processing.
        return ESBuiltInTypeNameId::_None;
    }
#endif
}

ESBuiltInTypeNameId ESBuiltInsTelemetryProvider::GetESBuiltInTypeNameId_Other( const Var instance, const PropertyId propertyId, _Out_ bool& isConstructorProperty, TypeId typeId, _Out_ ESBuiltInPropertyId& shortcutPropertyFound )
{
    count_GetBuiltInTypeNameOther++;
    
    ESBuiltInTypeNameId esbiTypeNameId = ESBuiltInsDatabase::GetESBuiltInTypeNameId_ByTypeId( typeId );
    if( esbiTypeNameId == ESBuiltInTypeNameId::_None ) return esbiTypeNameId;

    // See if the specified property is defined on the identified type, if not, see if it's defined for Object (and so, is inherited):
    ESBuiltInPropertyId definedOnType = ESBuiltInsDatabase::GetESBuiltInPropertyId( esbiTypeNameId, /*isConstructorProperty:*/ false, propertyId );
    if( definedOnType == ESBuiltInPropertyId::_None )
    {
        ESBuiltInPropertyId definedOnObject = ESBuiltInsDatabase::GetESBuiltInPropertyId( ESBuiltInTypeNameId::Object, /*isConstructorProperty:*/ false, propertyId ); // `isConstructorProperty` will always be false because `instance` is not a function and so cannot be a constructor.
        if( definedOnObject != ESBuiltInPropertyId::_None )
        {
            shortcutPropertyFound = definedOnObject;
            return ESBuiltInTypeNameId::Object;
        }
    }
    else
    {
        shortcutPropertyFound = definedOnType;
    }

    return esbiTypeNameId;
}

ESBuiltInTypeNameId ESBuiltInsTelemetryProvider::GetESBuiltInTypeNameId( const Var instance, const PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ ESBuiltInPropertyId& shortcutPropertyFound )
{
    isConstructorProperty = false;
    
    if( instance == nullptr) return ESBuiltInTypeNameId::_None;

    Js::TypeId typeId = JavascriptOperators::GetTypeId( instance );
    
    // Special-cases are needed for Functions and Objects because we don't know what their real type is.
    if( typeId == TypeId::TypeIds_Function )
    {
        return this->GetESBuiltInTypeNameId_Function( instance, propertyId, isConstructorProperty, shortcutPropertyFound );
    }
    else if( typeId == TypeId::TypeIds_Object )
    {
        return this->GetESBuiltInTypeNameId_Object( instance, propertyId, isConstructorProperty, shortcutPropertyFound );
    }
    else
    {
        return this->GetESBuiltInTypeNameId_Other( instance, propertyId, isConstructorProperty, typeId, shortcutPropertyFound );
    }
}

static constexpr UINT32 crc_table[] = {
   0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
   0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
   0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
   0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
   0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
   0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
   0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
   0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
   0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
   0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
   0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
   0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
   0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
   0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
   0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
   0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

UINT32 crc32(const char* in) {
    UINT32 crc = (UINT32)-1;
    while (*in != '\0') {
        crc = (crc >> 8) ^ crc_table[(crc ^ *in) & 0xFF];
        in++;
    }
    return crc ^ (UINT32)-1;
}

void ESBuiltInsTelemetryProvider::OutputTraceLogging(GUID activityId, DWORD hostType, bool isJSRT)
{
    if (!this->throttle.isThrottled())
    {
        ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
        Js::LanguageStats* langStats = threadContext->GetLanguageStats();
        size_t idx;

#define STRINGOF(n) #n
#define FIELDNAMESTRING(type, base, name) STRINGOF(type ## _ ## base ## _ ## name)
#define TOKENJOIN2(x, y) x ## y
#define TOKENJOIN(x, y) TOKENJOIN2(x, y)
        
        // Generate arrays of data.
#define BLOCK_START(blockname, count) UINT32 TOKENJOIN(blockname, _keys) [count+1] = {
#define BLOCK_END() 0};
#define ENTRY_BUILTIN(ver, type, base, name) crc32(FIELDNAMESTRING(type, base, name)),
#define ENTRY_LANGFEATURE(ver, name)
#define ENTRY_TELPOINT(name)
#include "../LangTelFields.h"
#undef ENTRY_TELPOINT
#undef ENTRY_LANGFEATURE
#undef ENTRY_BUILTIN
#undef BLOCK_END
#undef BLOCK_START

#define BLOCK_START(blockname, count) UINT32 TOKENJOIN(blockname, _props) [(count)+1] = {
#define BLOCK_END() 0};
#define Get(propertyId) \
    ( idx = ESBuiltInsDatabase::GetESBuiltInArrayIndex( propertyId ), idx != SIZE_MAX ? this->usageMap[ idx ] : 0 )
#define ENTRY_BUILTIN(ver, type, base, name) (UINT32)Get(ESBuiltInPropertyId:: ## type ## _ ## base ## _ ## name),
#define ENTRY_LANGFEATURE(ver, name)
#define ENTRY_TELPOINT(name)
#include "../LangTelFields.h"
#undef ENTRY_TELPOINT
#undef ENTRY_LANGFEATURE
#undef ENTRY_BUILTIN
#undef Get
#undef BLOCK_END
#undef BLOCK_START

#define BLOCK_START(blockname, count) UINT32 TOKENJOIN(blockname, _callCount) [(count)+1] = {
#define BLOCK_END() 0};
#define Get(propertyId) \
    ( idx = ESBuiltInsDatabase::GetESBuiltInArrayIndex( propertyId ), idx != SIZE_MAX ? this->usageMap[ idx ] : 0 )
#define ENTRY_BUILTIN(ver, type, base, name) (UINT32)langStats-> type ## _ ## base ## _ ## name .callCount,
#define ENTRY_LANGFEATURE(ver, name)
#define ENTRY_TELPOINT(name)
#include "../LangTelFields.h"
#undef ENTRY_TELPOINT
#undef ENTRY_LANGFEATURE
#undef ENTRY_BUILTIN
#undef Get
#undef BLOCK_END
#undef BLOCK_START

#define BLOCK_START(blockname, count) UINT32 TOKENJOIN(blockname, _debugModeCallCount) [(count)+1] = {
#define BLOCK_END() 0};
#define Get(propertyId) \
    ( idx = ESBuiltInsDatabase::GetESBuiltInArrayIndex( propertyId ), idx != SIZE_MAX ? this->usageMap[ idx ] : 0 )
#define ENTRY_BUILTIN(ver, type, base, name) (UINT32)langStats-> type ## _ ## base ## _ ## name .debugModeCallCount,
#define ENTRY_LANGFEATURE(ver, name)
#define ENTRY_TELPOINT(name)
#include "../LangTelFields.h"
#undef ENTRY_TELPOINT
#undef ENTRY_LANGFEATURE
#undef ENTRY_BUILTIN
#undef Get
#undef BLOCK_END
#undef BLOCK_START

        // The following is a bunch of preprocessor shenanigans to get around a few limits,
        // among the telemetry framework, the compiler, and the language. It's not the most
        // readable code that you'll ever see, but I'll try to give a quick explanation:
        //
        // We have a bunch of telemetry data points described in LangTelFields. Since these
        // are all things we'd want to send back in one go when a user finishes with a page
        // or scriptcontext, we should send them back in one event (reducing overhead). The
        // telemetry framework requires that we define and fill a constant struct in a very
        // special page used by the telemetry framework to determine what events any binary
        // can produce, which saves the overhead of sending the event tag names every time.
        // Once we produce that from our data with a few macros and includes, sending event
        // data is just a matter of two last passes over the data definitions; one to count
        // up fields, and one to get the data from them correctly and put it in the buffer.

#define BLOCK_END() 
        TELPROLOGUE("ESBuiltins")
#define UNIQNAME(prefix) TOKENJOIN(prefix, __LINE__)
#define BLOCK_START(blockname, count) char UNIQNAME(_TlgName) [sizeof(FIELDNAMESTRING(blockname,arr,keys))]; UINT8 UNIQNAME(_TlgIn); \
                                      char UNIQNAME(_TlgNameo) [sizeof(FIELDNAMESTRING(blockname,arr,props))]; UINT8 UNIQNAME(_TlgIno); \
                                      char UNIQNAME(_TlgNamec) [sizeof(FIELDNAMESTRING(blockname,arr,callCount))]; UINT8 UNIQNAME(_TlgInc); \
                                      char UNIQNAME(_TlgNamed) [sizeof(FIELDNAMESTRING(blockname,arr,dMCallCount))]; UINT8 UNIQNAME(_TlgInd);
#define ENTRY_BUILTIN(ver, type, base, name) //char UNIQNAME(_TlgName) [sizeof(FIELDNAMESTRING(type, base, name))]; UINT8 UNIQNAME(_TlgIn);
#define ENTRY_LANGFEATURE(ver, name) char UNIQNAME(_TlgName) [sizeof(#name)]; UINT8 UNIQNAME(_TlgIn);
#define ENTRY_TELPOINT(name) char UNIQNAME(_TlgName) [sizeof(#name)]; UINT8 UNIQNAME(_TlgIn);
#include "../LangTelFields.h"
#undef ENTRY_TELPOINT
#undef ENTRY_LANGFEATURE
#undef ENTRY_BUILTIN
#undef BLOCK_START
#undef UNIQNAME
        TELMIDLOGUEPART1("ESBuiltins")
#define BLOCK_START(blockname, count)  , ( FIELDNAMESTRING(blockname,arr,keys) ), TlgInUINT32|64 \
                                       , ( FIELDNAMESTRING(blockname,arr,props) ), TlgInUINT32|64 \
                                       , ( FIELDNAMESTRING(blockname,arr,callCount) ), TlgInUINT32|64 \
                                       , ( FIELDNAMESTRING(blockname,arr,dMCallCount) ), TlgInUINT32|64
#define ENTRY_BUILTIN(ver, type, base, name) //, (FIELDNAMESTRING(type, base, name)), TlgInUINT32
#define ENTRY_LANGFEATURE(ver, name) , ( # name ), TlgInUINT32
#define ENTRY_TELPOINT(name) , ( # name ), TlgInUINT32
#include "../LangTelFields.h"
#undef ENTRY_TELPOINT
#undef ENTRY_LANGFEATURE
#undef ENTRY_BUILTIN
#undef BLOCK_START
        TELMIDLOGUEPART2()
#define BLOCK_START(blockname, count) +(2+2+2+2)
#define ENTRY_BUILTIN(ver, type, base, name) +0
#define ENTRY_LANGFEATURE(ver, name) +1
#define ENTRY_TELPOINT(name) +1
#include "../LangTelFields.h"
#undef ENTRY_TELPOINT
#undef ENTRY_LANGFEATURE
#undef ENTRY_BUILTIN
#undef BLOCK_START
        TELMIDLOGUEPART3()
#define Get(propertyId) \
    ( idx = ESBuiltInsDatabase::GetESBuiltInArrayIndex( propertyId ), idx != SIZE_MAX ? this->usageMap[ idx ] : 0 )
#define BLOCK_START(blockname, count) _TlgCreateArray( &_TlgData[_TlgIdx], TOKENJOIN(blockname,_keys), count, sizeof(UINT32)), _TlgIdx += 2, \
                                      _TlgCreateArray( &_TlgData[_TlgIdx], TOKENJOIN(blockname,_props), count, sizeof(UINT32)), _TlgIdx += 2, \
                                      _TlgCreateArray( &_TlgData[_TlgIdx], TOKENJOIN(blockname,_callCount), count, sizeof(UINT32)), _TlgIdx += 2, \
                                      _TlgCreateArray( &_TlgData[_TlgIdx], TOKENJOIN(blockname,_debugModeCallCount), count, sizeof(UINT32)), _TlgIdx += 2,
#define ENTRY_BUILTIN(ver, type, base, name) 
#define ENTRY_LANGFEATURE(ver, name) _TlgCreateDesc<UINT32>(&_TlgData[_TlgIdx], (langStats->name.parseCount)), _TlgIdx += 1,
#define ENTRY_TELPOINT(name) _TlgCreateDesc<UINT32>(&_TlgData[_TlgIdx], (langStats->name.count)), _TlgIdx += 1,
#include "../LangTelFields.h"
#undef ENTRY_TELPOINT
#undef ENTRY_LANGFEATURE
#undef ENTRY_BUILTIN
#undef BLOCK_START
#undef Get
        TELEPILOGUE()
#undef FIELDNAMESTRING
#undef STRINGOF
#undef TOKENJOIN
#undef TOKENJOIN2
    }
}

void ESBuiltInsTelemetryProvider::OutputPrint()
{
#ifdef TELEMETRY_OUTPUTPRINT
    if( CONFIG_ISENABLED(Js::ESBLangTelFlag) )
    {
        Output::Print( _u("----------\r\n") );
        Output::Print( _u("-- ECMAScript 5-7 Built-Ins Telemetry.\r\n") );
        Output::Print( _u("Date.parse Telemetry.\r\n"));
        Output::Print( _u("OnPropertyEncountered     : %d \r\n"), count_OnPropertyEncountered      );
        Output::Print( _u("OnConstructorCalled       : %d \r\n"), count_OnConstructorCalled        );
        Output::Print( _u("GetBuiltInTypeNameFunction: %d \r\n"), count_GetBuiltInTypeNameFunction );
        Output::Print( _u("GetBuiltInTypeNameObject  : %d \r\n"), count_GetBuiltInTypeNameObject   );
        Output::Print( _u("GetBuiltInTypeNameOther   : %d \r\n"), count_GetBuiltInTypeNameOther    );

        Output::Print( _u("ESBuiltInPropertyId \tObject               \tFunction            \tCallCount \r\n") );

        bool atLeast1 = false;
        // For each ESBuiltInProperty in ESBuiltInsDatabase::ESBuiltInPropertyList
        //  Get ESBuiltInPropertyId, convert to ESBuiltInPropertyIdIdx
        //      Get count from usageMap

        ESBuiltInPropertyList& properties = ESBuiltInsDatabase::GetESBuiltInPropertyIdIdxList();
        for( int i = 0; i < properties.Count(); i++ )
        {
            ESBuiltInProperty& prop = properties.Item( i );

            AssertMsg( i < ESBuiltInsDatabase::ESBuiltInPropertyIdCount, "i is not out of bounds." );

            byte count = this->usageMap[ i ];
            if( count == BYTE_MAX )
            {
                Output::Print( _u("%20d\t%-20s\t%-20s\t255+\r\n"), prop.esbiPropertyId, prop.constructorName, prop.propertyName );
                atLeast1 = true;
            }
            else if( count > 0 )
            {
                Output::Print( _u("%20d\t%-20s\t%-20s\t%9d\r\n"), prop.esbiPropertyId, prop.constructorName, prop.propertyName, count );
                atLeast1 = true;
            }
        }

        if( !atLeast1 )
        {
            Output::Print( _u("No built-ins called.\r\n") );
        }
        Output::Print( _u("----------\r\n"));
    }
#endif
}

#endif