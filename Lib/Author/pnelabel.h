//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

/*****************************************************************************/
#ifndef PNELABEL
#error  Define PNELABEL before including this file.
#endif
/*****************************************************************************/

    PNELABEL(pneNone             , ""            , apneNone           )
    PNELABEL(pneOperand          , "operand"     , apneOperand        )
    PNELABEL(pneLeft             , "left"        , apneLeft           )
    PNELABEL(pneRight            , "right"       , apneRight          )
    PNELABEL(pneCondition        , "condition"   , apneCondition      )
    PNELABEL(pneThen             , "then"        , apneThen           )
    PNELABEL(pneElse             , "else"        , apneElse           )
    PNELABEL(pneInitialization   , "init"        , apneInitialization )
    PNELABEL(pneIncrement        , "increment"   , apneIncrement      )
    PNELABEL(pneBody             , "body"        , apneBody           )
    PNELABEL(pneBlockBody        , "body"        , apneBlockBody      )
    PNELABEL(pneValue            , "value"       , apneValue          )
    PNELABEL(pneTarget           , "target"      , apneTarget         )
    PNELABEL(pneArguments        , "args"        , apneArguments      )
    PNELABEL(pneArgument         , ""            , apneArgument       )
    PNELABEL(pneMembers          , "members"     , apneMembers        )
    PNELABEL(pneVariable         , "variable"    , apneVariable       )
    PNELABEL(pneObject           , "object"      , apneObject         )
    PNELABEL(pneTry              , "try"         , apneTry            )
    PNELABEL(pneCatch            , "catch"       , apneCatch          )
    PNELABEL(pneFinally          , "finally"     , apneFinally        )
    PNELABEL(pneCase             , ""            , apneCase           )
    PNELABEL(pneDefaultCase      , ""            , apneDefaultCase    )
    PNELABEL(pneElements         , "elements"    , apneElements       )
    PNELABEL(pneListItem         , ""            , apneListItem       )
    PNELABEL(pneMember           , "member"      , apneMember         )
    PNELABEL(pneType             , "type"        , apneType           )
    PNELABEL(pneExtends          , "extends"     , apneExtends        )
    PNELABEL(pneCtor             , "ctor"        , apneCtor           )
    PNELABEL(pneStaticMembers    , "staticMembers", apneStaticMembers )
    PNELABEL(pneStringLiterals   , "stringLiterals", apneStringLiterals)
    PNELABEL(pneSubstitutionExpression, "substitutionExpression", apneSubstitutionExpression)
    PNELABEL(pneStringRawLiterals, "stringRawLiterals", apneStringRawLiterals)

#undef  PNELABEL