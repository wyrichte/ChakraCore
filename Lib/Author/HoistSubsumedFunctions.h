//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    struct RewriteContext;
    class ParseNodeTree;

    struct HoistSubsumedFunctions
    {
        static void Apply(ArenaAllocator* alloc, ParseNodeTree* tree);
    };
}