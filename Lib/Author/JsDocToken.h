//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Authoring
{
    enum JsDocToken
    {
        none,               /* Intentionally used to mean this token is not a tag */
        star,
        atreturns,
        atproperty,
        atparam,
        atdeprecated,
        attype,
        atdescription,
        atsummary,
        attypedef,
        atunknown,
        openbrace,
        closebrace,
        openbracket,
        closebracket,
        eof,
        endline,
        name,
        type_name,
        description,
        value,
        pipe,
        hyphen,
        equals,
        unknown
    };
}