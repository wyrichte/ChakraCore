// Copyright (c) Microsoft Corporation. All rights reserved.

DEFINE_TEST_DIR(Functional, L"");
DEFINE_TEST_DIR(Configurable, L"");
DEFINE_TEST_DIR(HeapDump, L"");
DEFINE_TEST_DIR(Generated, L"");
DEFINE_TEST_DIR(ScriptErrorTests, L"");
DEFINE_TEST_DIR(RecyclerStress, L"exclude_nostress,exclude_snap");
DEFINE_TEST_DIR(MemoryTracing, L"exclude_fre");
DEFINE_TEST_DIR(Versioning, L"");
DEFINE_TEST_DIR(Intl, L"");

#undef DEFINE_TEST_DIR
#undef DEFINE_TEST_DIR_