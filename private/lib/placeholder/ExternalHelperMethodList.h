//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#ifdef ENABLE_DOM_FAST_PATH
#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attributeGetter, attributeSetter) HELPERCALL(nameGetter, entryGetter, attributeGetter)
#include "..\engine\DOMFastpathinfolist.h"
    OBJECT_SIMPLESLOT_RECORDS
#undef _ONE_SIMPLESLOT_RECORD

#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attribute, attributeSetter) HELPERCALL(nameSetter, entrySetter, attributeSetter)
#include "..\engine\DOMFastpathinfolist.h"
    OBJECT_SIMPLESLOT_RECORDS
#undef _ONE_SIMPLESLOT_RECORD

#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attributeGetter, attributeSetter) HELPERCALL(nameGetter, entryGetter, attributeGetter)
#include "..\engine\DOMFastpathinfolist.h"
    TYPE_SIMPLESLOT_RECORDS
#undef _ONE_SIMPLESLOT_RECORD

#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attribute, attributeSetter) HELPERCALL(nameSetter, entrySetter, attributeSetter)
#include "..\engine\DOMFastpathinfolist.h"
    TYPE_SIMPLESLOT_RECORDS
#undef _ONE_SIMPLESLOT_RECORD
#endif

