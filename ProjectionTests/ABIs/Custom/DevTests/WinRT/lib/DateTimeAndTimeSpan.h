//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include <Windows.Foundation.h>
#include <WindowsDateTimeP.h>

using namespace Windows::Foundation;

#define INT64_MAX       9223372036854775807i64
#define INT64_MIN       (-9223372036854775807i64 - 1)

namespace DevTests
{
    namespace DateTimeAndTimeSpan
    {
        class TestsServer :
            public Microsoft::WRL::RuntimeClass<IDateTimeTests, ITimeSpanTests, IOtherTests>
        {
            InspectableClass(L"DevTests.DateTimeAndTimeSpan.Tests", BaseTrust);

        private:
            DateTime m_dateTime;
            TimeSpan m_timeSpan;

        public:
            TestsServer() 
            { 
                m_dateTime.UniversalTime = 0; 
                m_timeSpan.Duration = 0;
            }

            // IDateTime Tests
            IFACEMETHOD(get_DateTime)(__out DateTime * value) { *value = m_dateTime; return S_OK; }
            IFACEMETHOD(put_DateTime)(__in DateTime value) { m_dateTime = value; return S_OK; }

            IFACEMETHOD(VerifyDateTimeInt)(__in DateTime date, __in __int64 expected, __out boolean * equal)
            {
                *equal = (date.UniversalTime == expected);
                return S_OK;
            }
            IFACEMETHOD(VerifyDateTimeSystemTime)(__in DateTime date, __in t_UInt16 year, __in t_UInt16 month, __in t_UInt16 day, 
                __in t_UInt16 hour, __in t_UInt16 minute, __in t_UInt16 second, __in t_UInt16 milliseconds, 
                __in t_UInt16 offset, __out boolean * equal)
            {
                DateTime result;
                HRESULT hr = ProduceDateTimeSystemTime(year, month, day, hour, minute, second, milliseconds, offset, &result);
                if (SUCCEEDED(hr))
                {
                    *equal = (result.UniversalTime == date.UniversalTime);
                }
                return hr;
            }
            IFACEMETHOD(ProduceDateTimeInt)(__in __int64 value, __out DateTime * date)
            {
                DateTime retVal = { value };
                *date = retVal;
                return S_OK;
            }
            IFACEMETHOD(ProduceDateTimeSystemTime)(__in t_UInt16 year, __in t_UInt16 month, __in t_UInt16 day, 
                __in t_UInt16 hour, __in t_UInt16 minute, __in t_UInt16 second, __in t_UInt16 milliseconds, 
                __in t_UInt16 offset, __out DateTime * date)
            {
                SYSTEMTIME time = { year, month, NULL, day, hour, minute, second, milliseconds };
                DateTime result;
                HRESULT hr = RoSystemTimeToDateTime(time, &result);
                if (SUCCEEDED(hr))
                {
                    result.UniversalTime = result.UniversalTime + offset;
                    *date = result;
                }
                return hr;
            }
            IFACEMETHOD(DateTimeCmp)(__in DateTime a, __in DateTime b, __out __int64 * difference)
            {
                *difference = a.UniversalTime - b.UniversalTime;
                return S_OK;
            }
            IFACEMETHOD(CmpDateTimeToStored)(__in DateTime date, __out __int64 * difference)
            {
                *difference = m_dateTime.UniversalTime - date.UniversalTime;
                return S_OK;
            }
            IFACEMETHOD(RoundTripDateTime)(__in DateTime _in, __out DateTime * _out) { *_out = _in; return S_OK; }
            IFACEMETHOD(MarshalDummyDateTime)(__in DummyDateTime _in, __out DummyDateTime * _out) { *_out = _in; return S_OK; }
            IFACEMETHOD(ResetDateTime)(__in DateTime _in, __out DateTime * _out) { _in.UniversalTime = 0; *_out = _in; return S_OK; }

            //ITimeSpanTests
            IFACEMETHOD(get_TimeSpan)(__out TimeSpan * value) { *value = m_timeSpan; return S_OK; }
            IFACEMETHOD(put_TimeSpan)(__in TimeSpan value) { m_timeSpan = value; return S_OK; }

            IFACEMETHOD(VerifyTimeSpan)(__in TimeSpan span, __in __int64 expected, __out boolean * equal)
            {
                *equal = (span.Duration == expected);
                return S_OK;
            }
            IFACEMETHOD(ProduceTimeSpan)(__in __int64 value, __out TimeSpan * span)
            {
                TimeSpan retVal = { value };
                *span = retVal;
                return S_OK;
            }
            IFACEMETHOD(TimeSpanCmp)(__in TimeSpan a, __in TimeSpan b, __out __int64 * difference)
            {
                *difference = a.Duration - b.Duration;
                return S_OK;
            }
            IFACEMETHOD(CmpTimeSpanToStored)(__in TimeSpan span, __out __int64 * difference)
            {
                *difference = m_timeSpan.Duration - span.Duration;
                return S_OK;
            }
            IFACEMETHOD(RoundTripTimeSpan)(__in TimeSpan _in, __out TimeSpan * _out) { *_out = _in; return S_OK; }
            IFACEMETHOD(MarshalDummyTimeSpan)(__in DummyTimeSpan _in, __out DummyTimeSpan * _out) { *_out = _in; return S_OK; }
            IFACEMETHOD(ResetTimeSpan)(__in TimeSpan _in, __out TimeSpan * _out) { _in.Duration = 0; *_out = _in; return S_OK; }

            //IOtherTests
            IFACEMETHOD(MarshalEventLog)(__in EventLog _in, __out EventLog * _out)
            {
                *_out = _in;
                WindowsDuplicateString(_in.EventName,&_out->EventName);
                return S_OK;
            }

            IFACEMETHOD(CreateInt64)(__in t_UInt32 high, __in t_UInt32 low, __out __int64 * result)
            {
                *result = (ULONGLONG)low |
                              (((ULONGLONG)high) << 32);
                return S_OK;
            }

            IFACEMETHOD(GetInt64Max)(__out __int64 * max)
            {
                *max = INT64_MAX;
                return S_OK;
            }

            IFACEMETHOD(GetInt64Min)(__out __int64 * min)
            {
                *min = INT64_MIN;
                return S_OK;
            }

            IFACEMETHOD(VerifyInt64Max)(__in __int64 value, __out boolean * equal)
            {
                *equal = (value == INT64_MAX);
                return S_OK;
            }

            IFACEMETHOD(Int64Cmp)(__in __int64 a, __in __int64 b, __out __int64 * difference)
            {
                *difference = a - b;
                return S_OK;
            }

            IFACEMETHOD(DateInStringOut)(__in DateTime date, __out HSTRING * result)
            {
                SYSTEMTIME sysTime;
                HRESULT hr = RoDateTimeToSystemTime(date, &sysTime);
                if (SUCCEEDED(hr))
                {
                    LPCWSTR message;
                    if((sysTime.wMonth == 1) && (sysTime.wDay == 1))
                    {
                        message = L"Happy New Year!";
                    }
                    else
                    {
                        message = L"Not New Year's";
                    }
                    WindowsCreateString(message, (UINT32)wcslen(message), result);
                }
                return hr;
            }
        };
    }
}
