//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "..\..\..\lib\common\core\BinaryFeatureControl.cpp"
#ifdef LANGUAGE_SERVICE
EXTERN_C  
BOOLEAN IsMessageBoxWPresent()  
{  
    return FALSE;  
}  
  
#endif
