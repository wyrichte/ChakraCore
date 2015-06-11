//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    IAuthorStructure *NewStructure(FileAuthoring *fileAuthoring);
    void AddStaticStructure(IAuthorStructure *authorStructure, ArenaAllocator *alloc, ParseNode *rootNode);
    void AddDynamicStructure(IAuthorStructure *authorStructure, ArenaAllocator *alloc, Js::ScriptContext *scriptContext, AuthoringFileHandle *file, ParseNodeTree *tree);
}
