//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    class OpCodeUtilAsmJs
    {
    public:
        static wchar_t const * GetOpCodeName( OpCodeAsmJs op );

        static OpLayoutTypeAsmJs GetOpCodeLayout( OpCodeAsmJs op );
        static bool IsValidByteCodeOpcode( OpCodeAsmJs op );
        static bool IsValidOpcode( OpCodeAsmJs op );
    private:
#if DBG_DUMP || ENABLE_DEBUG_CONFIG_OPTIONS
        static wchar_t const * const OpCodeAsmJsNames[(int)Js::OpCodeAsmJs::MaxByteSizedOpcodes + 1];
        static wchar_t const * const ExtendedOpCodeAsmJsNames[];
#endif
        static OpLayoutTypeAsmJs const OpCodeAsmJsLayouts[];
        static OpLayoutTypeAsmJs const ExtendedOpCodeAsmJsLayouts[];
    };
};