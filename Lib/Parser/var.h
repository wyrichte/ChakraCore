//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

/***************************************************************************
Helper functions for dealing with IDispatch
***************************************************************************/

// VTEs are our special variant types.
enum
{
    VTE_MIN = 0x80,    // Start here to avoid collisions with any new VT types
    VTE_DBLBOOL = 0x8b, // [0x8b 139] used by the scanner. boolean stored as 0 or 1 in dblVal.
    VTE_LIM,            // ***** lim of special variant types

    VTE_ARRAY = VT_ARRAY | VT_VARIANT,
};

class VAR : public tagVARIANT
{
public:
    VAR(){}

#if DEBUG
    // Keeping assignment operator, copy constructor in Debug builds to catch issues similar as WOOB 997154.

    VAR(VAR &initVar) : tagVARIANT(initVar)
    {
        //Assert(initVar.vt != VTE_ASTR || initVar.PAStringObj()->GetMasterVariant() == &initVar);
    }

    VAR& operator =(VAR &rtOperand)
    {
        //Assert(rtOperand.vt != VTE_ASTR || rtOperand.PAStringObj()->GetMasterVariant() == &rtOperand);
        tagVARIANT::operator = (rtOperand);
        return *this; 
    }
#endif


private:
};
