//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#ifdef ENABLE_DOM_FAST_PATH
#define SIMPLESLOT_RECORD_KIND Object
#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attributeGetter, attributeSetter) HELPERCALL(nameGetter, entryGetter, attributeGetter)
#include "..\engine\DOMFastpathinfolist.h"
#undef _ONE_SIMPLESLOT_RECORD
#undef SIMPLESLOT_RECORD_KIND

#define SIMPLESLOT_RECORD_KIND Object
#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attribute, attributeSetter) HELPERCALL(nameSetter, entrySetter, attributeSetter)
#include "..\engine\DOMFastpathinfolist.h"
#undef _ONE_SIMPLESLOT_RECORD
#undef SIMPLESLOT_RECORD_KIND

#define SIMPLESLOT_RECORD_KIND Type
#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attributeGetter, attributeSetter) HELPERCALL(nameGetter, entryGetter, attributeGetter)
#include "..\engine\DOMFastpathinfolist.h"
#undef _ONE_SIMPLESLOT_RECORD
#undef SIMPLESLOT_RECORD_KIND

#define SIMPLESLOT_RECORD_KIND Type
#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attribute, attributeSetter) HELPERCALL(nameSetter, entrySetter, attributeSetter)
#include "..\engine\DOMFastpathinfolist.h"
#undef _ONE_SIMPLESLOT_RECORD
#undef SIMPLESLOT_RECORD_KIND
#endif

