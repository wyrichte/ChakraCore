//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

/*****************************************************************************/
#ifndef PTNODE
#error  Define PTNODE before including this file.
#endif
/*****************************************************************************/
//
//     Node oper
//                    , "Node name"
//                                          , pcode
//                                                    , parse node kind
//                                                                  , flags
//                                                                                          , JSON Name
//                                                                                                                              , Authoring Node Type

PTNODE(knopNone       , "<none>"           , Nop      , None        , fnopNone              , ""                                , apnkNone       )

/***************************************************************************
    Leaf nodes.
***************************************************************************/
PTNODE(knopName       , "name"             , Nop      , Pid         , fnopLeaf              , "NameExpr"                        , apnkName       )
PTNODE(knopInt        , "int const"        , Nop      , Int         , fnopLeaf|fnopConst    , "NumberLit"                       , apnkInt        )
PTNODE(knopFlt        , "flt const"        , Nop      , Flt         , fnopLeaf|fnopConst    , "NumberLit"                       , apnkFlt        )
PTNODE(knopStr        , "str const"        , Nop      , Pid         , fnopLeaf|fnopConst    , "StringLit"                       , apnkStr        )
PTNODE(knopRegExp     , "reg expr"         , Nop      , Pid         , fnopLeaf|fnopConst    , "RegExprLit"                      , apnkRegExp     )
PTNODE(knopThis       , "this"             , Nop      , None        , fnopLeaf              , "ThisExpr"                        , apnkThis       )
PTNODE(knopSuper      , "super"            , Nop      , None        , fnopLeaf              , "SuperExpr"                       , apnkSuper      )
PTNODE(knopNull       , "null"             , Nop      , None        , fnopLeaf              , "NullLit"                         , apnkNull       )
PTNODE(knopFalse      , "false"            , Nop      , None        , fnopLeaf              , "FalseLit"                        , apnkFalse      )
PTNODE(knopTrue       , "true"             , Nop      , None        , fnopLeaf              , "TrueLit"                         , apnkTrue       )
PTNODE(knopEmpty      , "empty"            , Nop      , None        , fnopLeaf              , "EmptStmt"                        , apnkEmpty      )
PTNODE(knopYieldLeaf  , "yield leaf"       , Nop      , None        , fnopLeaf              , "YieldLeafExpr"                   , apnkYieldLeaf  )

/***************************************************************************
Unary operators.
***************************************************************************/
PTNODE(knopNot        , "~"                , Nop      , Uni         , fnopUni               , "BitNotOper"                      , apnkNot        )
PTNODE(knopNeg        , "unary -"          , Nop      , Uni         , fnopUni               , "NegOper"                         , apnkNeg        )
PTNODE(knopPos        , "unary +"          , Nop      , Uni         , fnopUni               , "PosOper"                         , apnkPos        )
PTNODE(knopLogNot     , "!"                , Nop      , Uni         , fnopUni               , "LogNotOper"                      , apnkLogNot     )
PTNODE(knopEllipsis   , "..."              , Nop      , Uni         , fnopUni               , "Spread"                          , apnkEllipsis   )
// ___compact range : do not add or remove in this range.
//    Gen code of  OP_LclIncPost,.. depends on parallel tables with this range
PTNODE(knopIncPost    , "++ post"          , Nop      , Uni         , fnopUni|fnopAsg       , "PostIncExpr"                     , apnkIncPost    )
PTNODE(knopDecPost    , "-- post"          , Nop      , Uni         , fnopUni|fnopAsg       , "PostDecExpr"                     , apnkDecPost    )
PTNODE(knopIncPre     , "++ pre"           , Nop      , Uni         , fnopUni|fnopAsg       , "PreIncExpr"                      , apnkIncPre     )
PTNODE(knopDecPre     , "-- pre"           , Nop      , Uni         , fnopUni|fnopAsg       , "PreDecExpr"                      , apnkDecPre     )
//___end range
PTNODE(knopTypeof     , "typeof"           , Nop      , Uni         , fnopUni               , "TypeOfExpr"                      , apnkTypeof     )
PTNODE(knopVoid       , "void"             , Nop      , Uni         , fnopUni               , "VoidExpr"                        , apnkVoid       )
PTNODE(knopDelete     , "delete"           , Nop      , Uni         , fnopUni               , "DeleteStmt"                      , apnkDelete     )
PTNODE(knopArray      , "arr cnst"         , Nop      , ArrLit      , fnopUni               , "ArrayExpr"                       , apnkArray      )
PTNODE(knopObject     , "obj cnst"         , Nop      , Uni         , fnopUni               , "ObjectExpr"                      , apnkObject     )
PTNODE(knopTempRef    , "temp ref"         , Nop      , Uni         , fnopUni               , "TempRef"                         , apnkTempRef    )
PTNODE(knopComputedName,"[name]"           , Nop      , Uni         , fnopUni               , "ComputedNameExpr"                , apnkComputedName)
PTNODE(knopYield      , "yield"            , Nop      , Uni         , fnopUni|fnopAsg       , "YieldExpr"                       , apnkYield      )
PTNODE(knopYieldStar  , "yield *"          , Nop      , Uni         , fnopUni|fnopAsg       , "YieldStarExpr"                   , apnkYieldStar  )

/***************************************************************************
Binary and ternary operators.
***************************************************************************/
PTNODE(knopAdd        , "+"                , Add_A    , Bin         , fnopBin               , "AddOper"                        , apnkAdd        )
PTNODE(knopSub        , "-"                , Sub_A    , Bin         , fnopBin               , "SubOper"                        , apnkSub        )
PTNODE(knopMul        , "*"                , Mul_A    , Bin         , fnopBin               , "MulOper"                        , apnkMul        )
PTNODE(knopDiv        , "/"                , Div_A    , Bin         , fnopBin               , "DivOper"                        , apnkDiv        )
PTNODE(knopMod        , "%"                , Rem_A    , Bin         , fnopBin               , "ModOper"                        , apnkMod        )
PTNODE(knopOr         , "|"                , Or_A     , Bin         , fnopBin               , "BitOrOper"                      , apnkOr         )
PTNODE(knopXor        , "^"                , Xor_A    , Bin         , fnopBin               , "BitXorOper"                     , apnkXor        )
PTNODE(knopAnd        , "&"                , And_A    , Bin         , fnopBin               , "BitAndOper"                     , apnkAnd        )
PTNODE(knopEq         , "=="               , OP(Eq)   , Bin         , fnopBin|fnopRel       , "EqualOper"                      , apnkEq         )
PTNODE(knopNe         , "!="               , OP(Neq)  , Bin         , fnopBin|fnopRel       , "NotEqualOper"                   , apnkNe         )
PTNODE(knopLt         , "<"                , OP(Lt)   , Bin         , fnopBin|fnopRel       , "LessThanOper"                   , apnkLt         )
PTNODE(knopLe         , "<="               , OP(Le)   , Bin         , fnopBin|fnopRel       , "LessThanEqualOper"              , apnkLe         )
PTNODE(knopGe         , ">="               , OP(Ge)   , Bin         , fnopBin|fnopRel       , "GreaterThanEqualOper"           , apnkGe         )
PTNODE(knopGt         , ">"                , OP(Gt)   , Bin         , fnopBin|fnopRel       , "GreaterThanOper"                , apnkGt         )
PTNODE(knopCall       , "()"               , Nop      , Call        , fnopBin               , "CallExpr"                       , apnkCall       )
PTNODE(knopDot        , "."                , Nop      , Bin         , fnopBin               , "DotOper"                        , apnkDot        )
PTNODE(knopAsg        , "="                , Nop      , Bin         , fnopBin|fnopAsg       , "AssignmentOper"                 , apnkAsg        )
PTNODE(knopInstOf     , "instanceof"       , IsInst   , Bin         , fnopBin|fnopRel       , "InstanceOfExpr"                 , apnkInstOf     )
PTNODE(knopIn         , "in"               , IsIn     , Bin         , fnopBin|fnopRel       , "InOper"                         , apnkIn         )
PTNODE(knopEqv        , "==="              , OP(SrEq) , Bin         , fnopBin|fnopRel       , "StrictEqualOper"                , apnkEqv        )
PTNODE(knopNEqv       , "!=="              , OP(SrNeq), Bin         , fnopBin|fnopRel       , "NotStrictEqualOper"             , apnkNEqv       )
PTNODE(knopComma      , ","                , Nop      , Bin         , fnopBin               , "CommaOper"                      , apnkComma      )
PTNODE(knopLogOr      , "||"               , Nop      , Bin         , fnopBin               , "LogOrOper"                      , apnkLogOr      )
PTNODE(knopLogAnd     , "&&"               , Nop      , Bin         , fnopBin               , "LogAndOper"                     , apnkLogAnd     )
PTNODE(knopLsh        , "<<"               , Shl_A    , Bin         , fnopBin               , "LeftShiftOper"                  , apnkLsh        )
PTNODE(knopRsh        , ">>"               , Shr_A    , Bin         , fnopBin               , "RightShiftOper"                 , apnkRsh        )
PTNODE(knopRs2        , ">>>"              , ShrU_A   , Bin         , fnopBin               , "UnsignedRightShiftOper"         , apnkRs2        )
PTNODE(knopNew        , "new"              , Nop      , Call        , fnopBin               , "NewExpr"                        , apnkNew        )
PTNODE(knopIndex      , "[]"               , Nop      , Bin         , fnopBin               , "IndexOper"                      , apnkIndex      )
PTNODE(knopQmark      , "?"                , Nop      , Tri         , fnopBin               , "IfExpr"                         , apnkQmark      )

// ___compact range : do not add or remove in this range.
//    Gen code of  OP_LclAsg*,.. depends on parallel tables with this range
PTNODE(knopAsgAdd     , "+="               , Add_A    , Bin         , fnopBin|fnopAsg       , "AddAssignExpr"                  , apnkAsgAdd     )
PTNODE(knopAsgSub     , "-="               , Sub_A    , Bin         , fnopBin|fnopAsg       , "SubAssignExpr"                  , apnkAsgSub     )
PTNODE(knopAsgMul     , "*="               , Mul_A    , Bin         , fnopBin|fnopAsg       , "MulAssignExpr"                  , apnkAsgMul     )
PTNODE(knopAsgDiv     , "/="               , Div_A    , Bin         , fnopBin|fnopAsg       , "DivAssignExpr"                  , apnkAsgDiv     )
PTNODE(knopAsgMod     , "%="               , Rem_A    , Bin         , fnopBin|fnopAsg       , "ModAssignExpr"                  , apnkAsgMod     )
PTNODE(knopAsgAnd     , "&="               , And_A    , Bin         , fnopBin|fnopAsg       , "BitAndAssignExpr"               , apnkAsgAnd     )
PTNODE(knopAsgXor     , "^="               , Xor_A    , Bin         , fnopBin|fnopAsg       , "BitXorAssignExpr"               , apnkAsgXor     )
PTNODE(knopAsgOr      , "|="               , Or_A     , Bin         , fnopBin|fnopAsg       , "BitOrAssignExpr"                , apnkAsgOr      )
PTNODE(knopAsgLsh     , "<<="              , Shl_A    , Bin         , fnopBin|fnopAsg       , "LeftShiftAssignExpr"            , apnkAsgLsh     )
PTNODE(knopAsgRsh     , ">>="              , Shr_A    , Bin         , fnopBin|fnopAsg       , "RightShiftAssignExpr"           , apnkAsgRsh     )
PTNODE(knopAsgRs2     , ">>>="             , ShrU_A   , Bin         , fnopBin|fnopAsg       , "UnsignedRightShiftAssignExpr"   , apnkAsgRs2     )
//___end range

PTNODE(knopScope      , "::"               , Nop      , Bin         , fnopNotExprStmt|fnopBin, "ScopeOper"                      , apnkScope     )
PTNODE(knopMember     , ":"                , Nop      , Bin         , fnopNotExprStmt|fnopBin, "MemberOper"                     , apnkMember    )
PTNODE(knopMemberShort, "membShort"        , Nop      , Bin         , fnopNotExprStmt|fnopBin, "ShorthandMember"                , apnkMember    )
PTNODE(knopSetMember  , "set"              , Nop      , Bin         , fnopBin                , "SetDecl"                        , apnkSetMember )
PTNODE(knopGetMember  , "get"              , Nop      , Bin         , fnopBin                , "GetDecl"                        , apnkGetMember )
/***************************************************************************
General nodes.
***************************************************************************/
PTNODE(knopList       , "<list>"           , Nop      , Bin         , fnopBinList|fnopNotExprStmt, ""                          , apnkList       )
PTNODE(knopVarDecl    , "varDcl"           , Nop      , Var         , fnopNotExprStmt        , "VarDecl"                       , apnkVarDecl    )
PTNODE(knopConstDecl  , "constDcl"         , Nop      , Var         , fnopNotExprStmt        , "ConstDecl"                     , apnkConstDecl  )
PTNODE(knopLetDecl    , "letDcl"           , Nop      , Var         , fnopNotExprStmt        , "LetDecl"                       , apnkLetDecl    )
PTNODE(knopTemp       , "temp"             , Nop      , Var         , fnopNone               , "Temp"                          , apnkTemp       )
PTNODE(knopFncDecl    , "fncDcl"           , Nop      , Fnc         , fnopLeaf               , "FuncDecl"                      , apnkFncDecl    )
PTNODE(knopClassDecl  , "classDecl"        , Nop      , Class       , fnopLeaf               , "ClassDecl"                     , apnkClassDecl  )
PTNODE(knopProg       , "program"          , Nop      , Prog        , fnopNotExprStmt        , "Unit"                          , apnkProg       )
PTNODE(knopEndCode    , "<endcode>"        , Nop      , None        , fnopNotExprStmt        , ""                              , apnkEndCode    )
PTNODE(knopDebugger   , "debugger"         , Nop      , None        , fnopNotExprStmt        , "DebuggerStmt"                  , apnkDebugger   )
PTNODE(knopFor        , "for"              , Nop      , For         , fnopNotExprStmt|fnopCleanup|fnopBreak|fnopContinue , "ForStmtm"                      , apnkFor        )
PTNODE(knopIf         , "if"               , Nop      , If          , fnopNotExprStmt        , "IfStmt"                        , apnkIf         )
PTNODE(knopWhile      , "while"            , Nop      , While       , fnopNotExprStmt|fnopCleanup|fnopBreak|fnopContinue , "WhileStmt"                     , apnkWhile      )
PTNODE(knopDoWhile    , "do-while"         , Nop      , While       , fnopNotExprStmt|fnopCleanup|fnopBreak|fnopContinue , "DoWhileStmt"                   , apnkDoWhile    )
PTNODE(knopForIn      , "for in"           , Nop      , ForIn       , fnopNotExprStmt|fnopCleanup|fnopBreak|fnopContinue , "ForInStmt"                     , apnkForIn      )
PTNODE(knopForOf      , "for of"           , Nop      , ForOf       , fnopNotExprStmt|fnopCleanup|fnopBreak|fnopContinue , "ForOfStmt"                     , apnkForOf      )
PTNODE(knopBlock      , "{}"               , Nop      , Block       , fnopNotExprStmt        , "Block"                         , apnkBlock      )
PTNODE(knopStrTemplate, "``"               , Nop      , StrTemplate , fnopNone               , "StringTemplateDecl"            , apnkStrTemplate )
PTNODE(knopWith       , "with"             , Nop      , With        , fnopNotExprStmt        , "WithStmt"                      , apnkWith       )
PTNODE(knopBreak      , "break"            , Nop      , Jump        , fnopNotExprStmt        , "BreakStmt"                     , apnkBreak      )
PTNODE(knopContinue   , "continue"         , Nop      , Jump        , fnopNotExprStmt        , "ContinueStmt"                  , apnkContinue   )
PTNODE(knopLabel      , "label"            , Nop      , Label       , fnopNotExprStmt        , "LabelDecl"                     , apnkLabel      )
PTNODE(knopSwitch     , "switch"           , Nop      , Switch      , fnopNotExprStmt|fnopBreak, "SwitchStmt"                    , apnkSwitch     )
PTNODE(knopCase       , "case"             , Nop      , Case        , fnopNotExprStmt        , "CaseStmt"                      , apnkCase       )
PTNODE(knopTryCatch   , "try-catch"        , Nop      , TryCatch    , fnopNotExprStmt        , "TryCatchStmt"                  , apnkTryCatch   )
PTNODE(knopCatch      , "catch"            , Nop      , Catch       , fnopNotExprStmt|fnopCleanup, "CatchClause"                   , apnkCatch      )
PTNODE(knopReturn     , "return"           , Nop      , Return      , fnopNotExprStmt        , "ReturnStmt"                    , apnkReturn     )
PTNODE(knopTry        , "try"              , Nop      , Try         , fnopNotExprStmt|fnopCleanup, "TryStmt"                       , apnkTry        )
PTNODE(knopThrow      , "throw"            , Nop      , Uni         , fnopNotExprStmt        , "ThrowStmt"                     , apnkThrow      )
PTNODE(knopFinally    , "finally"          , Nop      , Finally     , fnopNotExprStmt|fnopCleanup, "FinallyStmt"                   , apnkFinally    )
PTNODE(knopTryFinally , "try-finally"      , Nop      , TryFinally  , fnopNotExprStmt        , "TryFinallyStmt"                , apnkTryFinally )

/***************************************************************************
Extra node kinds used by the LanguageService
***************************************************************************/
PTNODE(knopVarDeclList,   "<varlist>"      , Nop      , Bin         , fnopBinList|fnopNotExprStmt, "VarDeclList"               , apnkVarDeclList)
PTNODE(knopConstDeclList, "<varlist>"      , Nop      , Bin         , fnopBinList|fnopNotExprStmt, "ConstDeclList"             , apnkConstDeclList)
PTNODE(knopLetDeclList,   "<varlist>"      , Nop      , Bin         , fnopBinList|fnopNotExprStmt, "LetDeclList"               , apnkLetDeclList)
PTNODE(knopDefaultCase,   "default"        , Nop      , Case        , fnopNone              , "DefaultCaseStmt"                , apnkDefaultCase)
#undef PTNODE
#undef OP
