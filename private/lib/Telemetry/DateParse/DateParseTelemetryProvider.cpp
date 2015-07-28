#include "TelemetryPch.h"
#include "DateParseTelemetryProvider.h"

#ifdef TELEMETRY_DateParse

#include "..\TelemetryMacros.h"

#ifdef TELEMETRY_OUTPUTPRINT
#define TELEMETRY_DateParse_StringCollection
#endif

using namespace Js;

DateParseTelemetryProvider::DateParseTelemetryProvider( ScriptContextTelemetry& scriptContextTelemetry ) :
    scriptContextTelemetry( scriptContextTelemetry ),
    allocator( L"Telemetry-DateParse", scriptContextTelemetry.GetScriptContext().GetThreadContext()->GetPageAllocator(), Throw::OutOfMemory ),
    hitCount( &allocator ),
    successCount( 0 ),
    failCount( 0 )
{
    // Register the knownMethodTelemetry callback handler:
    scriptContextTelemetry.GetKnownMethodTelemetry().dateParseTelemetryProvider = this;
}


DateParseTelemetryProvider::~DateParseTelemetryProvider()
{
}

/// <remarks>This is a separate function because I had difficulties with the VS debugger when this was a lambda.</remarks>
static void MapCallback(const wchar_t* const& key, uint32 const& value)
{
    if( key == nullptr ) // This null-check shouldn't be necessary, but just-in-case.
    {
        Output::Print( L"String: null, Count: %d\r\n", value );
    }
    else
    {
        Output::Print( L"String: \"%s\", Count: %d\r\n", key, value );
    }
}

void DateParseTelemetryProvider::OutputPrint()
{
    if( CONFIG_ISENABLED(Js::DateParseTelFlag) )
    {
        Output::Print( L"----------\r\n");
        Output::Print( L"-- Date.parse Telemetry.\r\n");
        Output::Print( L"Success: %d\r\n", this->successCount );
        Output::Print( L"Failures: %d\r\n", this->failCount );
        Output::Print( L"Failed input strings:\r\n" );
        hitCount.Map( MapCallback );
        Output::Print( L"----------\r\n");
    }
}

void DateParseTelemetryProvider::OutputTraceLogging()
{
    TraceLogChakra("Date.parse Telemetry",
        TraceLoggingUInt32( this->successCount, "Success" ),
        TraceLoggingUInt32( this->failCount, "Failures" )
    );
}

void DateParseTelemetryProvider::JavascriptDate_ParseHelper( ScriptContext* scriptContext, JavascriptString* str, double returnValue, bool exceptionRaised )
{
    bool success = !exceptionRaised && !JavascriptNumber::IsNan( returnValue );
    if     ( success && this->successCount < MAXUINT32 ) this->successCount++;
    else if(            this->failCount    < MAXUINT32 ) this->failCount++;

#ifdef TELEMETRY_DateParse_StringCollection
    if( !success && str != nullptr )
    {
        const wchar_t* copy = str->GetSzCopy( /*scriptContext->TelemetryAllocator()*/ &this->allocator );
        uint32 value;
        if( hitCount.TryGetValue( copy, &value ) )
        {
            hitCount.Item( copy, value + 1 );
        }
        else
        {
            hitCount.AddNew( copy, 1 );
        }
    }
#endif
}

#endif