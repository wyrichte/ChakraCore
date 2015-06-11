//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------
#pragma once 
#if DBG

class DbCheckPostLower
{
private:
    Func *func;

    void        Check(IR::Opnd *opnd);
    void        Check(IR::RegOpnd *regOpnd);

public:
    DbCheckPostLower(Func *func) : func(func) { }
    void        Check();
};

#endif  // DBG