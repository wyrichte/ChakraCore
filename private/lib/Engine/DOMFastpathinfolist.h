//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------
#ifndef _ONE_SIMPLESLOT_RECORD
#error _ONE_SIMPLESLOT_RECORD must be defined before including this file
#endif

#undef ONE_SIMPLESLOT_RECORD
#define ONE_SIMPLESLOT_RECORD(kind, x) _ONE_SIMPLESLOT_RECORD(Simple##kind##SlotGetter##x, Simple##kind##SlotSetter##x, DOMFastPath<x>::EntryInfo::Simple##kind##SlotGetter, DOMFastPath<x>::EntryInfo::Simple##kind##SlotSetter, DOMFastPath<x>::EntrySimple##kind##SlotGetter, DOMFastPath<x>::EntrySimple##kind##SlotSetter, (HelperMethodAttribute)(AttrCanThrow | AttrInVariant), AttrCanThrow)

#define TEN_SIMPLESLOT_RECORD(kind, x) \
    ONE_SIMPLESLOT_RECORD(kind, x##0) \
    ONE_SIMPLESLOT_RECORD(kind, x##1) \
    ONE_SIMPLESLOT_RECORD(kind, x##2) \
    ONE_SIMPLESLOT_RECORD(kind, x##3) \
    ONE_SIMPLESLOT_RECORD(kind, x##4) \
    ONE_SIMPLESLOT_RECORD(kind, x##5) \
    ONE_SIMPLESLOT_RECORD(kind, x##6) \
    ONE_SIMPLESLOT_RECORD(kind, x##7) \
    ONE_SIMPLESLOT_RECORD(kind, x##8) \
    ONE_SIMPLESLOT_RECORD(kind, x##9) \

#define OBJECT_SIMPLESLOT_RECORDS \
    ONE_SIMPLESLOT_RECORD(Object, 0) \
    ONE_SIMPLESLOT_RECORD(Object, 1) \
    ONE_SIMPLESLOT_RECORD(Object, 2) \
    ONE_SIMPLESLOT_RECORD(Object, 3) \
    ONE_SIMPLESLOT_RECORD(Object, 4) \
    ONE_SIMPLESLOT_RECORD(Object, 5) \
    ONE_SIMPLESLOT_RECORD(Object, 6) \
    ONE_SIMPLESLOT_RECORD(Object, 7) \
    ONE_SIMPLESLOT_RECORD(Object, 8) \
    ONE_SIMPLESLOT_RECORD(Object, 9) \
    TEN_SIMPLESLOT_RECORD(Object, 1)

#define TYPE_SIMPLESLOT_RECORDS \
    ONE_SIMPLESLOT_RECORD(Type, 0) \
    ONE_SIMPLESLOT_RECORD(Type, 1) \
    ONE_SIMPLESLOT_RECORD(Type, 2) \
    ONE_SIMPLESLOT_RECORD(Type, 3)