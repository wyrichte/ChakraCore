//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------


namespace OpCodeAttrAsmJs
{
    // False if the opcode results in jump to end of the function and there cannot be fallthrough. 
    bool HasFallThrough(Js::OpCodeAsmJs opcode);
    // True if the opcode has a small/large layout
    bool HasMultiSizeLayout(Js::OpCodeAsmJs opcode);
};
