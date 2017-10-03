// This is the list of global static variables exposed to ChakraDiag for 
// hybrid debugging scenarios.
// Entry(staticVariablName, DAC friendly name)

ENTRY(Js::Configuration::Global, Configuration)
ENTRY(ThreadContext::globalListFirst, ThreadContextList)
ENTRY(ThreadContextTLSEntry::s_tlsSlot, TlsSlot)
ENTRY(AutoSystemInfo::Data.dllLoadAddress, DllBaseAddress)
ENTRY(AutoSystemInfo::Data.dllHighAddress, DllHighAddress)

#ifdef _M_AMD64
ENTRY(amd64_ReturnFromCallWithFakeFrame, Amd64FakeEHFrameProcOffset)
#endif

#define INTERNALPROPERTY(n) ENTRY(Js::InternalPropertyRecords::n, InternalProperty_##n)
#include "InternalPropertyList.h"

// Add a dummy entry in chk build, so that chk and fre builds have different DiagGlobals count.
// We can then detect jscript9/jscript9diag chk/fre flavor-mismatch easily.
#ifdef DBG
ENTRY(ThreadContextTLSEntry::s_tlsSlot, __DummyDbgOnlyEntry)
#endif
