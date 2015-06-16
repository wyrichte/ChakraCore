//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    enum EBinaryMathOpCodes
    {
        BMO_ADD,
        BMO_SUB,
        BMO_MUL,
        BMO_DIV,
        BMO_REM,

        BMO_MAX,
    };

    enum EBinaryMathOpCodesTypes
    {
        BMOT_Int,
        BMOT_UInt,
        BMOT_Float,
        BMOT_Double,
        BMOT_MAX
    };

    const OpCodeAsmJs BinaryMathOpCodes[BMO_MAX][BMOT_MAX] = {
        /*BMO_ADD*/{ OpCodeAsmJs::Add_Int, OpCodeAsmJs::Add_Int, OpCodeAsmJs::Add_Flt, OpCodeAsmJs::Add_Db },
        /*BMO_SUB*/{ OpCodeAsmJs::Sub_Int, OpCodeAsmJs::Sub_Int, OpCodeAsmJs::Sub_Flt, OpCodeAsmJs::Sub_Db },
        /*BMO_MUL*/{ OpCodeAsmJs::Mul_Int, OpCodeAsmJs::Mul_Int, OpCodeAsmJs::Mul_Flt, OpCodeAsmJs::Mul_Db },
        /*BMO_DIV*/{ OpCodeAsmJs::Div_Int, OpCodeAsmJs::Div_UInt,OpCodeAsmJs::Div_Flt, OpCodeAsmJs::Div_Db },
        /*BMO_REM*/{ OpCodeAsmJs::Rem_Int, OpCodeAsmJs::Rem_UInt,OpCodeAsmJs::Nop,     OpCodeAsmJs::Rem_Db }
    };

    enum EBinaryComparatorOpCodes
    {
        /*<, <=, >, >=, ==, !=*/
        BCO_LT,
        BCO_LE,
        BCO_GT,
        BCO_GE,
        BCO_EQ,
        BCO_NE,

        BCO_MAX,
    };

    enum EBinaryComparatorOpCodesTypes
    {
        BCOT_Int,
        BCOT_UInt,
        BCOT_Float,
        BCOT_Double,
        BCOT_MAX
    };

    const OpCodeAsmJs BinaryComparatorOpCodes[BCO_MAX][BCOT_MAX] = {
                    //  int            unsigned int     double
        /*BCO_LT*/{ OpCodeAsmJs::CmLt_Int, OpCodeAsmJs::CmLt_UnInt, OpCodeAsmJs::CmLt_Flt, OpCodeAsmJs::CmLt_Db },
        /*BCO_LE*/{ OpCodeAsmJs::CmLe_Int, OpCodeAsmJs::CmLe_UnInt, OpCodeAsmJs::CmLe_Flt, OpCodeAsmJs::CmLe_Db },
        /*BCO_GT*/{ OpCodeAsmJs::CmGt_Int, OpCodeAsmJs::CmGt_UnInt, OpCodeAsmJs::CmGt_Flt, OpCodeAsmJs::CmGt_Db },
        /*BCO_GE*/{ OpCodeAsmJs::CmGe_Int, OpCodeAsmJs::CmGe_UnInt, OpCodeAsmJs::CmGe_Flt, OpCodeAsmJs::CmGe_Db },
        /*BCO_EQ*/{ OpCodeAsmJs::CmEq_Int, OpCodeAsmJs::CmEq_Int, OpCodeAsmJs::CmEq_Flt, OpCodeAsmJs::CmEq_Db },
        /*BCO_NE*/{ OpCodeAsmJs::CmNe_Int, OpCodeAsmJs::CmNe_Int, OpCodeAsmJs::CmNe_Flt, OpCodeAsmJs::CmNe_Db },
    };


#define CheckNodeLocation(info,type) if(!mFunction->IsValidLocation<type>(&info)){\
    throw AsmJsCompilationException( L"Invalid Node location[%d] ", info.location ); }

    
    AsmJSByteCodeGenerator::AsmJSByteCodeGenerator( AsmJsFunc* func, AsmJsModuleCompiler* compiler ) :
        mFunction( func )
        , mAllocator(L"AsmjsByteCode", compiler->GetScriptContext()->GetThreadContext()->GetPageAllocator(), Throw::OutOfMemory)
        , mInfo( mFunction->GetFuncInfo() )
        , mCompiler( compiler )
        , mByteCodeGenerator(mCompiler->GetByteCodeGenerator())
        , mNestedCallCount(0)
        , mIsCallLegal(true)
    {
        mWriter.Create();

        const long astSize = func->GetFncNode()->sxFnc.astSize/AstBytecodeRatioEstimate;
        // Use the temp allocator in bytecode write temp buffer.
        mWriter.InitData(&mAllocator, astSize);

#ifdef LOG_BYTECODE_AST_RATIO
        // log the max Ast size
        Output::Print(L"Max Ast size: %d", astSize);
#endif
    }

    bool AsmJSByteCodeGenerator::BlockHasOwnScope( ParseNode* pnodeBlock )
    {
        Assert( pnodeBlock->nop == knopBlock );
        return pnodeBlock->sxBlock.scope != nullptr && ( !( pnodeBlock->grfpn & fpnSyntheticNode ) );
    }

    void AsmJSByteCodeGenerator::LoadAllConstants()
    {        
    
        FunctionBody *funcBody = mFunction->GetFuncBody();
        funcBody->CreateConstantTable();
        Var* table = (Var*)funcBody->GetConstTable();
        table += AsmJsFunctionMemory::RequiredVarConstants - 1; // we do -1 here as the VarConstant count is erobased calculation 

        int* intTable = (int*)table;
        // int Return Register
        *intTable = 0;
        intTable++;

        JsUtil::BaseDictionary<int, RegSlot, ArenaAllocator, PowerOf2SizePolicy, AsmJsComparer> intMap = mFunction->GetRegisterSpace<int>().GetConstMap();

        for (auto it = intMap.GetIterator(); it.IsValid(); it.MoveNext())
        {
            JsUtil::BaseDictionary<int, RegSlot, ArenaAllocator, PowerOf2SizePolicy, AsmJsComparer>::EntryType &entry = it.Current();
            *intTable = entry.Key();
            intTable++;
        }

        float* floatTable = (float*)intTable;
        // float Return Register
        *floatTable = 0;
        floatTable++;

        JsUtil::BaseDictionary<float, RegSlot, ArenaAllocator, PowerOf2SizePolicy, AsmJsComparer> floatMap = mFunction->GetRegisterSpace<float>().GetConstMap();

        for (auto it = floatMap.GetIterator(); it.IsValid(); it.MoveNext())
        {
            JsUtil::BaseDictionary<float, RegSlot, ArenaAllocator, PowerOf2SizePolicy, AsmJsComparer>::EntryType &entry = it.Current();
            *floatTable = entry.Key();
            floatTable++;
        }

        double* doubleTable = (double*)floatTable;
        // double Return Register
        *doubleTable = 0;
        doubleTable++;

        JsUtil::BaseDictionary<double, RegSlot, ArenaAllocator, PowerOf2SizePolicy, AsmJsComparer> doubleMap = mFunction->GetRegisterSpace<double>().GetConstMap();

        for (auto it = doubleMap.GetIterator(); it.IsValid(); it.MoveNext())
        {
            JsUtil::BaseDictionary<double, RegSlot, ArenaAllocator, PowerOf2SizePolicy, AsmJsComparer>::EntryType &entry = it.Current();
            *doubleTable = entry.Key();
            doubleTable++;
        }

#ifdef SIMD_JS_ENABLED
        if (SIMD_JS_FLAG)
        {
            AsmJsSIMDValue* simdTable = (AsmJsSIMDValue*)doubleTable;
            // SIMD return register
            simdTable->f64[0] = 0; simdTable->f64[1] = 0;

            JsUtil::BaseDictionary<AsmJsSIMDValue, RegSlot, ArenaAllocator, PowerOf2SizePolicy, AsmJsComparer> simdMap = mFunction->GetRegisterSpace<AsmJsSIMDValue>().GetConstMap();
            for (auto it = simdMap.GetIterator(); it.IsValid(); it.MoveNext())
            {
                JsUtil::BaseDictionary<AsmJsSIMDValue, RegSlot, ArenaAllocator, PowerOf2SizePolicy, AsmJsComparer>::EntryType &entry = it.Current();
                RegSlot regSlot = entry.Value();
                Assert((Var*)simdTable + regSlot < (Var*)funcBody->GetConstTable() + funcBody->GetConstantCount());
                // we cannot do sequential copy since register are assigned to constants in the order they appear in the code, not per dictionary order.
                simdTable[entry.Value()] = entry.Key();
            }
        }
#endif
    }

    void AsmJSByteCodeGenerator::FinalizeRegisters( FunctionBody* byteCodeFunction )
    {
        // REVIEW: check that this calculation makes sense
        // this value is the number of Var slots needed to allocate all the const
        int nbConst =
            ((mFunction->GetRegisterSpace<double>().GetConstCount() + 1) * DOUBLE_SLOTS_SPACE) // space required for all double constants + 1 return register reserved
            + (int)((mFunction->GetRegisterSpace<float>().GetConstCount() + 1)* FLOAT_SLOTS_SPACE + 0.5 /*ceil*/) // space required for all float constants + 1 return register reserved
            + (int)((mFunction->GetRegisterSpace<int>().GetConstCount() + 1) * INT_SLOTS_SPACE + 0.5/*ceil*/) // space required for all int constants + 1 return register reserved
            + AsmJsFunctionMemory::RequiredVarConstants;
#ifdef SIMD_JS_ENABLED
        if (SIMD_JS_FLAG)
        {
            nbConst += (int)((mFunction->GetRegisterSpace<AsmJsSIMDValue>().GetConstCount() + 1) * SIMD_SLOTS_SPACE); // Return register is already reserved in the regspace.
        }
#endif
        byteCodeFunction->SetConstantCount(nbConst);
        
        // add 3 for each of I0, F0, and D0
        RegSlot regCount = mInfo->RegCount() + 3 + AsmJsFunctionMemory::RequiredVarConstants;
#ifdef SIMD_JS_ENABLED
        if (SIMD_JS_FLAG)
        {
            // 1 return reg for SIMD
            regCount++;
        }
#endif
        
        byteCodeFunction->SetFirstTmpReg(regCount);
    }

    bool AsmJSByteCodeGenerator::EmitOneFunction()
    {
        Assert(mFunction->GetFncNode());
        Assert(mFunction->GetBodyNode());
        AsmJsFunctionCompilation autoCleanup( this );
        try
        {
            ParseNode* pnode = mFunction->GetFncNode();
            Assert( pnode && pnode->nop == knopFncDecl );
            Assert( mInfo != nullptr );

            ByteCodeGenerator* byteCodeGen = GetOldByteCodeGenerator();
            MaybeTodo( mInfo->IsFakeGlobalFunction( byteCodeGen->GetFlags() ) );

            // Support default arguments ?
            MaybeTodo( pnode->sxFnc.HasDefaultArguments() );

            FunctionBody* functionBody = mFunction->GetFuncBody();
            functionBody->SetStackNestedFunc( false );
            
            FinalizeRegisters(functionBody);            

            ArenaAllocator* alloc = byteCodeGen->GetAllocator();
            mInfo->inlineCacheMap = Anew( alloc, FuncInfo::InlineCacheMap,
                                          alloc,
                                          mInfo->RegCount()   // Pass the actual register count. TODO: Check if we can reduce this count
                                          );
            mInfo->rootObjectLoadInlineCacheMap = Anew( alloc, FuncInfo::RootObjectInlineCacheIdMap,
                                                        alloc,
                                                        10 );
            mInfo->rootObjectStoreInlineCacheMap = Anew( alloc, FuncInfo::RootObjectInlineCacheIdMap,
                                                         alloc,
                                                         10 );
            mInfo->referencedPropertyIdToMapIndex = Anew( alloc, FuncInfo::RootObjectInlineCacheIdMap,
                                                          alloc,
                                                          10 );
            functionBody->AllocateLiteralRegexArray();


            mWriter.Begin(byteCodeGen, functionBody, alloc, true /* byteCodeGen->DoJitLoopBodies( funcInfo )*/, mInfo->hasLoop);

            // for now, emit all constant loads at top of function (should instead put in
            // closest dominator of uses)
            LoadAllConstants();
            DefineLabels( );
            EmitAsmJsFunctionBody();
            
            //Set that the function is asmjsFuntion in functionBody here so that Intitalize ExecutionMode call later will check for that and not profile in asmjsMode 
            functionBody->SetIsAsmJsFunction(true);
            functionBody->SetIsAsmjsMode(true);

            // Do a uint32 add just to verify that we haven't overflowed the reg slot type.
            UInt32Math::Add( mFunction->GetRegisterSpace<int>().GetTotalVarCount(), mFunction->GetRegisterSpace<int>().GetConstCount());
            UInt32Math::Add( mFunction->GetRegisterSpace<double>().GetTotalVarCount(), mFunction->GetRegisterSpace<double>().GetConstCount());
            UInt32Math::Add( mFunction->GetRegisterSpace<float>().GetTotalVarCount(), mFunction->GetRegisterSpace<float>().GetConstCount());

            byteCodeGen->MapCacheIdsToPropertyIds( mInfo );
            byteCodeGen->MapReferencedPropertyIds( mInfo );

            mWriter.End();
            autoCleanup.FinishCompilation();

            functionBody->SetInitialDefaultEntryPoint();
            functionBody->SetIsByteCodeDebugMode( byteCodeGen->IsInDebugMode() );

#if DBG_DUMP
            if( PHASE_DUMP( ByteCodePhase, mInfo->byteCodeFunction ) && Configuration::Global.flags.Verbose )
            {
                pnode->Dump();
            }
            if( byteCodeGen->Trace() || PHASE_DUMP( ByteCodePhase, mInfo->byteCodeFunction ) )
            {
                AsmJsByteCodeDumper::Dump( mFunction, functionBody );
            }
#endif
        }
        catch( AsmJsCompilationException& e )
        {
            PrintAsmJsCompilationError( e.msg() );
            return false;
        }
        return true;
    }


    void AsmJSByteCodeGenerator::PrintAsmJsCompilationError(__out_ecount(256)  wchar_t* msg)
    {
        uint offset = mWriter.GetCurrentOffset();
        ULONG line = 0;
        LONG col = 0;
        if (!mFunction->GetFuncBody()->GetLineCharOffset(offset, &line, &col))
        {
            line = 0;
            col = 0;
        }

        wchar_t filename[_MAX_FNAME];
        wchar_t ext[_MAX_EXT];
        _wsplitpath_s( Configuration::Global.flags.Filename, NULL, 0, NULL, 0, filename, _MAX_FNAME, ext, _MAX_EXT );

        LPCOLESTR NoneName = L"None";
        LPCOLESTR moduleName = NoneName;
        if(mCompiler->GetModuleFunctionName())
        {
            moduleName = mCompiler->GetModuleFunctionName()->Psz();
        }

        AsmJSCompiler::OutputError(mCompiler->GetScriptContext(),
            L"\n%s%s(%d, %d)\n\tAsm.js Compilation Error function : %s::%s\n\t%s\n",
            filename, ext, line + 1, col + 1, moduleName, mFunction->GetName()->Psz(), msg);
    }

    void AsmJSByteCodeGenerator::DefineLabels()
    {
        mInfo->singleExit=mWriter.DefineLabel();
        SList<ParseNode *>::Iterator iter(&mInfo->targetStatements);
        while (iter.Next())
        {
            ParseNode * node = iter.Data();
            node->sxStmt.breakLabel=mWriter.DefineLabel();
            node->sxStmt.continueLabel=mWriter.DefineLabel();
            node->emitLabels=true;
        }
    }

    void AsmJSByteCodeGenerator::EmitAsmJsFunctionBody()
    {
        ParseNode *pnodeBody = mFunction->GetBodyNode();
        ParseNode *varStmts = pnodeBody;

        while (varStmts->nop == knopList)
        {
            ParseNode * pnode = ParserWrapper::GetBinaryLeft(varStmts);
            while (pnode && pnode->nop != knopEndCode)
            {
                ParseNode * decl;
                if (pnode->nop == knopConstDeclList || pnode->nop == knopVarDeclList || pnode->nop == knopList)
                {
                    decl = ParserWrapper::GetBinaryLeft(pnode);
                    pnode = ParserWrapper::GetBinaryRight(pnode);
                }
                else
                {  
                    decl = pnode;
                    pnode = nullptr;
                }

                if (decl->nop != knopVarDecl)
                {
                    goto varDeclEnd;
                }

                Assert(decl->nop == knopVarDecl);

                // since we are parsing the same way we created variables the same time, it is safe to assume these are AsmJsVar*
                AsmJsVar* var = (AsmJsVar*)mFunction->FindVar(ParserWrapper::VariableName(decl));

                if (var->GetType().isInt())
                {
                    mWriter.AsmInt1Const1(Js::OpCodeAsmJs::Ld_IntConst, var->GetLocation(), var->GetIntInitialiser());
                }
                else
                {
                    AsmJsVar * initSource = nullptr;
                    if (decl->sxVar.pnodeInit->nop == knopName)
                    {
                        AsmJsSymbol * initSym = mCompiler->LookupIdentifier(decl->sxVar.pnodeInit->name(), mFunction);
                        if (initSym->GetSymbolType() == AsmJsSymbol::Variable)
                        {
                            // in this case we are initializing with value of a constant var
                            initSource = initSym->Cast<AsmJsVar>();
                        }
                        else
                        {
                            Assert(initSym->GetSymbolType() == AsmJsSymbol::MathConstant);
                            Assert(initSym->GetType() == AsmJsType::Double);
                            AsmJsMathConst* initConst = initSym->Cast<AsmJsMathConst>();
                            mWriter.AsmDouble1Addr1(Js::OpCodeAsmJs::Ld_DbAddr, var->GetLocation(), initConst->GetVal());
                        }
                    }
                    else
                    {
                        initSource = var;
                    }
                    if (initSource)
                    {
                        if (var->GetType().isDouble())
                        {
                            mWriter.AsmReg2(Js::OpCodeAsmJs::Ld_Db, var->GetLocation(), mFunction->GetConstRegister<double>(initSource->GetDoubleInitialiser()));
                        }
                        else if (var->GetType().isFloat())
                        {
                            mWriter.AsmReg2(Js::OpCodeAsmJs::Ld_Flt, var->GetLocation(), mFunction->GetConstRegister<float>(initSource->GetFloatInitialiser()));
                        }
#ifdef SIMD_JS_ENABLED
                        else if (SIMD_JS_FLAG)
                        {
                            Assert(var->GetType().isSIMDType());
                            switch (var->GetType().GetWhich())
                            {
                                case AsmJsType::Float32x4:
                                    mWriter.AsmReg2(Js::OpCodeAsmJs::Simd128_Ld_F4, var->GetLocation(), mFunction->GetConstRegister<AsmJsSIMDValue>(var->GetSimdConstInitialiser()));
                                    break;
                                case AsmJsType::Float64x2:
                                    mWriter.AsmReg2(Js::OpCodeAsmJs::Simd128_Ld_D2, var->GetLocation(), mFunction->GetConstRegister<AsmJsSIMDValue>(var->GetSimdConstInitialiser()));
                                    break;
                                case AsmJsType::Int32x4:
                                    mWriter.AsmReg2(Js::OpCodeAsmJs::Simd128_Ld_I4, var->GetLocation(), mFunction->GetConstRegister<AsmJsSIMDValue>(var->GetSimdConstInitialiser()));
                                    break;
                                default:
                                    Assert(UNREACHED);

                            }
                        }
#endif
                        else
                        {
                            Assert(UNREACHED);
                        }
                    }
                }
            }
                varStmts = ParserWrapper::GetBinaryRight(varStmts);
        }
    varDeclEnd:

        // Emit a function body. Only explicit returns and the implicit "undef" at the bottom
        // get copied to the return register.

        while (varStmts->nop == knopList)
        {
            ParseNode *stmt = ParserWrapper::GetBinaryLeft(varStmts);
            EmitTopLevelStatement( stmt );
            varStmts = ParserWrapper::GetBinaryRight(varStmts);
        }
        Assert(!varStmts->CapturesSyms());
        EmitTopLevelStatement(varStmts);
    }

    void AsmJSByteCodeGenerator::EmitTopLevelStatement( ParseNode *stmt )
    {
        if( stmt->nop == knopFncDecl && stmt->sxFnc.IsDeclaration() )
        {
            throw AsmJsCompilationException( L"Cannot declare functions inside asm.js functions" );
        }
        const EmitExpressionInfo& info = Emit( stmt );
        // free tmp register here
        mFunction->ReleaseLocationGeneric( &info );
    }

    EmitExpressionInfo AsmJSByteCodeGenerator::Emit( ParseNode *pnode )
    {
        if( !pnode )
        {
            return EmitExpressionInfo( AsmJsType::Void );
        }
        switch( pnode->nop )
        {
        case knopReturn:
            return EmitReturn( pnode );
        case knopList:{
            while( pnode && pnode->nop == knopList )
            {
                const EmitExpressionInfo& info = Emit( ParserWrapper::GetBinaryLeft( pnode ) );
                mFunction->ReleaseLocationGeneric( &info );
                pnode = ParserWrapper::GetBinaryRight( pnode );
            }
            return Emit( pnode );
        }
        case knopComma:{
            const EmitExpressionInfo& info = Emit( ParserWrapper::GetBinaryLeft( pnode ) );
            mFunction->ReleaseLocationGeneric( &info );
            return Emit( ParserWrapper::GetBinaryRight( pnode ) );
        }
        case knopBlock:
        {
            EmitExpressionInfo info = Emit(pnode->sxBlock.pnodeStmt);
            if (pnode->emitLabels)
            {
                mWriter.MarkAsmJsLabel(pnode->sxStmt.breakLabel);
            }
            return info;
        }
        case knopCall:
            return EmitCall( pnode );
        case knopPos:
            return EmitUnaryPos( pnode );
        case knopNeg:
            return EmitUnaryNeg( pnode );
        case knopNot:
            return EmitUnaryNot( pnode );
        case knopLogNot:
            return EmitUnaryLogNot( pnode );
        case knopEq:
            return EmitBinaryComparator( pnode, BCO_EQ );
        case knopNe:
            return EmitBinaryComparator( pnode, BCO_NE );
        case knopLt:
            return EmitBinaryComparator( pnode, BCO_LT );
        case knopLe:
            return EmitBinaryComparator( pnode, BCO_LE );
        case knopGe:
            return EmitBinaryComparator( pnode, BCO_GE );
        case knopGt:
            return EmitBinaryComparator( pnode, BCO_GT );
        case knopOr:
            return EmitBinaryInt( pnode, OpCodeAsmJs::Or_Int );
        case knopXor:
            return EmitBinaryInt( pnode, OpCodeAsmJs::Xor_Int );
        case knopAnd:
            return EmitBinaryInt( pnode, OpCodeAsmJs::And_Int );
        case knopLsh:
            return EmitBinaryInt( pnode, OpCodeAsmJs::Shl_Int );
        case knopRsh:
            return EmitBinaryInt( pnode, OpCodeAsmJs::Shr_Int );
        case knopRs2:
            return EmitBinaryInt( pnode, OpCodeAsmJs::ShrU_Int );
        case knopMod:
            return EmitBinaryMultiType( pnode, BMO_REM );
        case knopDiv:
            return EmitBinaryMultiType( pnode, BMO_DIV );
        case knopMul:
            return EmitBinaryMultiType( pnode, BMO_MUL );
        case knopSub:
            return EmitBinaryMultiType( pnode, BMO_SUB );
        case knopAdd:
            return EmitBinaryMultiType( pnode, BMO_ADD );
        case knopName:
        case knopStr:
            return EmitIdentifier( pnode );
        case knopIndex:
            return EmitLdArrayBuffer( pnode );
        case knopEndCode:
            if( mFunction->GetReturnType() == AsmJsRetType::Void )
            {
                mWriter.AsmReg1( Js::OpCodeAsmJs::LdUndef, AsmJsFunctionMemory::ReturnRegister );
            }
            mWriter.MarkAsmJsLabel( mFunction->GetFuncInfo()->singleExit );
            mWriter.EmptyAsm( OpCodeAsmJs::Ret );
            break;
        case knopAsg:
            return EmitAssignment( pnode );
        case knopFlt:
            if (ParserWrapper::IsMinInt(pnode))
            {
                return EmitExpressionInfo(mFunction->GetConstRegister<int>(MININT32), AsmJsType::Signed);
            }
            else if (ParserWrapper::IsUnsigned(pnode))
            {
                return EmitExpressionInfo(mFunction->GetConstRegister<int>((uint32)pnode->sxFlt.dbl), AsmJsType::Unsigned);
            }
            else
            {
                return EmitExpressionInfo(mFunction->GetConstRegister<double>(pnode->sxFlt.dbl), AsmJsType::DoubleLit);
            }
        case knopInt:
            if (pnode->sxInt.lw < 0)
            {
                return EmitExpressionInfo(mFunction->GetConstRegister<int>(pnode->sxInt.lw), AsmJsType::Signed);
            }
            else
            {
                return EmitExpressionInfo(mFunction->GetConstRegister<int>(pnode->sxInt.lw), AsmJsType::Fixnum);
            }
        case knopIf:
            return EmitIf( pnode );
        case knopQmark:
            return EmitQMark( pnode );
        case knopSwitch:
            return EmitSwitch( pnode );
        case knopFor:
            MaybeTodo( pnode->sxFor.pnodeInverted != NULL );
//             if( pnode->sxFor.pnodeInverted != NULL )
//             {
//                 byteCodeGenerator->EmitInvertedLoop( pnode, pnode->sxFor.pnodeInverted, funcInfo );
//             }
//             else
            {
                const EmitExpressionInfo& initInfo = Emit( pnode->sxFor.pnodeInit );
                mFunction->ReleaseLocationGeneric( &initInfo );
                return EmitLoop( pnode,
                          pnode->sxFor.pnodeCond,
                          pnode->sxFor.pnodeBody,
                          pnode->sxFor.pnodeIncr);
            }
            break;
        case knopWhile:
            return EmitLoop( pnode,
                      pnode->sxWhile.pnodeCond,
                      pnode->sxWhile.pnodeBody,
                      nullptr);
        case knopDoWhile:
            return EmitLoop( pnode,
                      pnode->sxWhile.pnodeCond,
                      pnode->sxWhile.pnodeBody,
                      NULL,
                      true );
        case knopBreak:
            Assert( pnode->sxJump.pnodeTarget->emitLabels );
            StartStatement(pnode);
            mWriter.AsmBr( pnode->sxJump.pnodeTarget->sxStmt.breakLabel );
            if( pnode->emitLabels )
            {
                mWriter.MarkAsmJsLabel( pnode->sxStmt.breakLabel );
            }
            EndStatement(pnode);
            break;
        case knopContinue:
            Assert( pnode->sxJump.pnodeTarget->emitLabels );
            StartStatement(pnode);
            mWriter.AsmBr( pnode->sxJump.pnodeTarget->sxStmt.continueLabel );
            EndStatement(pnode);
            break;
        case knopLabel:
            break;
        case knopVarDecl:
            throw AsmJsCompilationException( L"Variable declaration must happen at the top of the function" );
            break;
#ifdef SIMD_JS_ENABLED
        case knopDot:
            if (SIMD_JS_FLAG)
            {
                // expr.(x|y|z|w) or expr.signMask
                return EmitDotExpr(pnode);
            }
            // fall-through
#endif
        default:
            throw AsmJsCompilationException( L"Unhandled parse opcode for asm.js" );
            break;
        }

        return EmitExpressionInfo(AsmJsType::Void);
    }

    EmitExpressionInfo AsmJSByteCodeGenerator::EmitBinaryMultiType( ParseNode * pnode, EBinaryMathOpCodes op )
    {
        ParseNode* lhs = ParserWrapper::GetBinaryLeft(pnode);
        ParseNode* rhs = ParserWrapper::GetBinaryRight(pnode);
        EmitExpressionInfo lhsEmit = Emit( lhs );
        EmitExpressionInfo rhsEmit = Emit( rhs );
        const AsmJsType& lType = lhsEmit.type;
        const AsmJsType& rType = rhsEmit.type;

        if( !lType.isVarAsmJsType() || !rType.isVarAsmJsType() )
        {
            throw AsmJsCompilationException( L"Type of expression unknown" );
        }

        EmitExpressionInfo emitInfo( AsmJsType::Double );
        StartStatement(pnode);
        if( (lType.GetWhich() == AsmJsType::Unsigned && rType.isUnsigned()) ||
            (rType.GetWhich() == AsmJsType::Unsigned && lType.isInt())
            )
        {
            CheckNodeLocation( lhsEmit, int );
            CheckNodeLocation( rhsEmit, int );

            if( BinaryMathOpCodes[op][BMOT_UInt] == OpCodeAsmJs::Nop )
            {
                throw AsmJsCompilationException( L"invalid Binary unsigned operation" );
            }

            // try to reuse tmp register
            RegSlot intReg = GetAndReleaseBinaryLocations<int>( &lhsEmit, &rhsEmit );
            mWriter.AsmReg3( BinaryMathOpCodes[op][BMOT_UInt], intReg, lhsEmit.location, rhsEmit.location );
            emitInfo.location = intReg;
            emitInfo.type = AsmJsType::Unsigned;
        }
        else if( lType.isInt() && rType.isInt() )
        {
            CheckNodeLocation( lhsEmit, int );
            CheckNodeLocation( rhsEmit, int );

            if( op == BMO_REM || op == BMO_DIV )
            {
                if( !lType.isSigned() || !rType.isSigned() )
                {
                    throw AsmJsCompilationException( L"arguments to / or %% must both be double?, float?, signed, or unsigned; %s and %s given", lType.toChars(), rType.toChars() );
                }
            }

            // try to reuse tmp register
            RegSlot intReg = GetAndReleaseBinaryLocations<int>( &lhsEmit, &rhsEmit );
            mWriter.AsmReg3( BinaryMathOpCodes[op][BMOT_Int], intReg, lhsEmit.location, rhsEmit.location );
            emitInfo.location = intReg;
            emitInfo.type = AsmJsType::Int;
        }
        else if (lType.isMaybeDouble() && rType.isMaybeDouble())
        {
            CheckNodeLocation( lhsEmit, double );
            CheckNodeLocation( rhsEmit, double );

            RegSlot dbReg = GetAndReleaseBinaryLocations<double>( &lhsEmit, &rhsEmit );
            mWriter.AsmReg3( BinaryMathOpCodes[op][BMOT_Double], dbReg, lhsEmit.location, rhsEmit.location );
            emitInfo.location = dbReg;
        }
        else if (lType.isMaybeFloat() && rType.isMaybeFloat())
        {
            if (BinaryMathOpCodes[op][BMOT_Float] == OpCodeAsmJs::Nop)
            {
                throw AsmJsCompilationException(L"invalid Binary float operation");
            }

            CheckNodeLocation(lhsEmit, float);
            CheckNodeLocation(rhsEmit, float);

            RegSlot floatReg = GetAndReleaseBinaryLocations<float>(&lhsEmit, &rhsEmit);
            mWriter.AsmReg3(BinaryMathOpCodes[op][BMOT_Float], floatReg, lhsEmit.location, rhsEmit.location);
            emitInfo.location = floatReg;
            emitInfo.type = AsmJsType::Floatish;
        }
        else
        {
            throw AsmJsCompilationException( L"Unsuported math operation" );
        }   
        EndStatement(pnode);
        return emitInfo;
    }

    EmitExpressionInfo AsmJSByteCodeGenerator::EmitBinaryInt( ParseNode * pnode, OpCodeAsmJs op )
    {
        ParseNode* lhs = ParserWrapper::GetBinaryLeft( pnode );
        ParseNode* rhs = ParserWrapper::GetBinaryRight( pnode );
        const bool isRhs0 = rhs->nop == knopInt && rhs->sxInt.lw == 0;
        const bool isOr0Operation = op == OpCodeAsmJs::Or_Int && isRhs0;
        if( isOr0Operation && lhs->nop == knopCall )
        {
            EmitExpressionInfo info = EmitCall(lhs, AsmJsRetType::Signed);
            if (!info.type.isIntish())
            {
                throw AsmJsCompilationException(L"Invalid type for [| & ^ >> << >>>] left and right operand must be of type intish");
            }
            info.type = AsmJsType::Signed;
            return info;
        }
        const EmitExpressionInfo& lhsEmit = Emit( lhs );
        const EmitExpressionInfo& rhsEmit = Emit( rhs );
        const AsmJsType& lType = lhsEmit.type;
        const AsmJsType& rType = rhsEmit.type;
        if( !lType.isIntish() || !rType.isIntish() )
        {
            throw AsmJsCompilationException( L"Invalid type for [| & ^ >> << >>>] left and right operand must be of type intish" );
        }
        CheckNodeLocation( lhsEmit, int );
        CheckNodeLocation( rhsEmit, int );
        StartStatement(pnode);
        EmitExpressionInfo emitInfo( AsmJsType::Signed );
        if( op == OpCodeAsmJs::ShrU_Int )
        {
            emitInfo.type = AsmJsType::Unsigned;
        }
        // ignore this specific operation, useful for non asm.js
        if( !isRhs0 || op == OpCodeAsmJs::And_Int )
        {
            RegSlot dstReg = GetAndReleaseBinaryLocations<int>( &lhsEmit, &rhsEmit );
            mWriter.AsmReg3( op, dstReg, lhsEmit.location, rhsEmit.location );
            emitInfo.location = dstReg;
        }
        else
        {
            mFunction->ReleaseLocation<int>( &rhsEmit );
            emitInfo.location = lhsEmit.location;
        }
        EndStatement(pnode);
        return emitInfo;
    }

    EmitExpressionInfo AsmJSByteCodeGenerator::EmitReturn( ParseNode * pnode )
    {
        ParseNode* expr = pnode->sxReturn.pnodeExpr;
        // return is always the beggining of a statement
        AsmJsRetType retType;
        EmitExpressionInfo emitInfo( Constants::NoRegister, AsmJsType::Void );
        if( !expr )
        {
            if( !mFunction->CheckAndSetReturnType( AsmJsRetType::Void ) )
            {
                throw AsmJsCompilationException( L"Different return type for the function" );
            }
            retType = AsmJsRetType::Void;
            // Make sure we return something 
            mWriter.AsmReg1(Js::OpCodeAsmJs::LdUndef, AsmJsFunctionMemory::ReturnRegister);
        }
        else
        {
            EmitExpressionInfo info = Emit(expr);
            StartStatement(pnode);
            if (info.type.isSubType(AsmJsType::Double))
            {
                CheckNodeLocation(info, double);
                // get return value from tmp register
                mWriter.Conv(OpCodeAsmJs::Return_Db, 0, info.location);
                mFunction->ReleaseLocation<double>(&info);
                emitInfo.type = AsmJsType::Double;
                retType = AsmJsRetType::Double;
            }
            else if (info.type.isSubType(AsmJsType::Signed))
            {
                CheckNodeLocation(info, int);
                // get return value from tmp register
                mWriter.Conv(OpCodeAsmJs::Return_Int, 0, info.location);
                mFunction->ReleaseLocation<int>(&info);
                emitInfo.type = AsmJsType::Signed;
                retType = AsmJsRetType::Signed;
            }
            else if (info.type.isSubType(AsmJsType::Float))
            {
                CheckNodeLocation(info, float);
                // get return value from tmp register
                mWriter.Conv(OpCodeAsmJs::Return_Flt, 0, info.location);
                mFunction->ReleaseLocation<float>(&info);
                emitInfo.type = AsmJsType::Float;
                retType = AsmJsRetType::Float;
            }
#ifdef SIMD_JS_ENABLED
            else if (SIMD_JS_FLAG && info.type.isSubType(AsmJsType::Float32x4))
            {
                CheckNodeLocation(info, AsmJsSIMDValue);
                mWriter.Conv(OpCodeAsmJs::Simd128_Return_F4, 0, info.location);
                mFunction->ReleaseLocation<AsmJsSIMDValue>(&info);
                emitInfo.type = AsmJsType::Float32x4;
                retType = AsmJsRetType::Float32x4;
            }
            else if (SIMD_JS_FLAG && info.type.isSubType(AsmJsType::Int32x4))
            {
                CheckNodeLocation(info, AsmJsSIMDValue);
                mWriter.Conv(OpCodeAsmJs::Simd128_Return_I4, 0, info.location);
                mFunction->ReleaseLocation<AsmJsSIMDValue>(&info);
                emitInfo.type = AsmJsType::Int32x4;
                retType = AsmJsRetType::Int32x4;
            }
            else if (SIMD_JS_FLAG && info.type.isSubType(AsmJsType::Float64x2))
            {
                CheckNodeLocation(info, AsmJsSIMDValue);
                mWriter.Conv(OpCodeAsmJs::Simd128_Return_D2, 0, info.location);
                mFunction->ReleaseLocation<AsmJsSIMDValue>(&info);
                emitInfo.type = AsmJsType::Float64x2;
                retType = AsmJsRetType::Float64x2;
            }
#endif
            else
            {
                throw AsmJsCompilationException(L"Expression for return must be subtype of Signed, Double, or Float");
            }
            EndStatement(pnode);
        }
        // check if we saw another return already with a dirrent type
        if (!mFunction->CheckAndSetReturnType(retType))
        {
            throw AsmJsCompilationException(L"Different return type for the function %s", mFunction->GetName()->Psz());
        }
        mWriter.AsmBr( mFunction->GetFuncInfo()->singleExit );
        return emitInfo;
    }
    bool AsmJSByteCodeGenerator::IsFRound(AsmJsMathFunction* sym)
    {
        return (sym && sym->GetMathBuiltInFunction() == AsmJSMathBuiltin_fround);
    }

    // First set of opcode are for External calls, second set is for internal calls
    static const OpCodeAsmJs callOpCode[2][7] =
    {
        {
            OpCodeAsmJs::StartCall
            , OpCodeAsmJs::Call
            , OpCodeAsmJs::ArgOut_Db
            , OpCodeAsmJs::ArgOut_Int
            , OpCodeAsmJs::Conv_VTD
            , OpCodeAsmJs::Conv_VTI
            , OpCodeAsmJs::Conv_VTF
        },
        {
            OpCodeAsmJs::I_StartCall
            , OpCodeAsmJs::I_Call
            , OpCodeAsmJs::I_ArgOut_Db
            , OpCodeAsmJs::I_ArgOut_Int
            , OpCodeAsmJs::I_Conv_VTD
            , OpCodeAsmJs::I_Conv_VTI
            , OpCodeAsmJs::I_Conv_VTF
        }
    };

    Js::EmitExpressionInfo AsmJSByteCodeGenerator::EmitCall(ParseNode * pnode, AsmJsRetType expectedType /*= AsmJsType::Void*/)
    {
        Assert( pnode->nop == knopCall );
        
        ParseNode* identifierNode = pnode->sxCall.pnodeTarget;
        RegSlot funcTableIndexRegister = Constants::NoRegister;

        // Function table
        if( pnode->sxCall.pnodeTarget->nop == knopIndex )
        {
            identifierNode = ParserWrapper::GetBinaryLeft( pnode->sxCall.pnodeTarget );
            ParseNode* indexNode = ParserWrapper::GetBinaryRight( pnode->sxCall.pnodeTarget );

            // check for table size annotation
            if( indexNode->nop != knopAnd )
            {
                throw AsmJsCompilationException( L"Function table call must be of format identifier[expr & NumericLiteral](...)" );
            }

            ParseNode* tableSizeNode = ParserWrapper::GetBinaryRight( indexNode );
            if( tableSizeNode->nop != knopInt )
            {
                throw AsmJsCompilationException( L"Function table call must be of format identifier[expr & NumericLiteral](...)" );
            }
            if (tableSizeNode->sxInt.lw < 0)
            {
                throw AsmJsCompilationException(L"Function table size must be positive");
            }
            const uint tableSize = tableSizeNode->sxInt.lw+1;
            if( !::Math::IsPow2(tableSize) )
            {
                throw AsmJsCompilationException( L"Function table size must be a power of 2" );
            }

            // Check for function table identifier
            if( !ParserWrapper::IsNameDeclaration( identifierNode ) )
            {
                throw AsmJsCompilationException( L"Function call must be of format identifier(...) or identifier[expr & size](...)" );
            }
            PropertyName funcName = identifierNode->name();
            AsmJsFunctionDeclaration* sym = mCompiler->LookupFunction( funcName );
            if( !sym )
            {
                throw AsmJsCompilationException( L"Unable to find function table %s", funcName->Psz() );
            }
            else 
            {
                if( sym->GetSymbolType() != AsmJsSymbol::FuncPtrTable )
                {
                    throw AsmJsCompilationException( L"Identifier %s is not a function table", funcName->Psz() );
                }
                AsmJsFunctionTable* funcTable = sym->Cast<AsmJsFunctionTable>();
                if( funcTable->GetSize() != tableSize )
                {
                    throw AsmJsCompilationException( L"Trying to load from Function table %s of size [%s] with size [%d]", funcName->Psz(), funcTable->GetSize(), tableSize );
                }
            }

            const EmitExpressionInfo& indexInfo = Emit( indexNode );
            if( !indexInfo.type.isInt() )
            {
                throw AsmJsCompilationException( L"Array Buffer View index must be type int" );
            }
            CheckNodeLocation( indexInfo, int );
            funcTableIndexRegister = indexInfo.location;
        }

        if( !ParserWrapper::IsNameDeclaration( identifierNode ) )
        {
            throw AsmJsCompilationException( L"Function call must be of format identifier(...) or identifier[expr & size](...)" );
        }
        PropertyName funcName = identifierNode->name();
        AsmJsFunctionDeclaration* sym = mCompiler->LookupFunction(funcName);
        if( !sym )
        {
            throw AsmJsCompilationException( L"Undefined function %s", funcName );
        }

#ifdef SIMD_JS_ENABLED
        if (SIMD_JS_FLAG)
        {
            if (sym->GetSymbolType() == AsmJsSymbol::SIMDBuiltinFunction)
            {
                return EmitSimdBuiltin(pnode, sym->Cast<AsmJsSIMDFunction>(), expectedType);
            }
        }
#endif
        
        if (IsFRound((AsmJsMathFunction*)sym))
        {
            expectedType = AsmJsRetType::Float;
        }

        const bool isFFI = sym->GetSymbolType() == AsmJsSymbol::ImportFunction;
        const bool isMathBuiltin = sym->GetSymbolType() == AsmJsSymbol::MathBuiltinFunction;
        if( isMathBuiltin )
        {
            return EmitMathBuiltin( pnode, sym->Cast<AsmJsMathFunction>(), expectedType );
        }

        // math builtins have different requirements for call-site coercion
        if (!sym->CheckAndSetReturnType(expectedType))
        {
            throw AsmJsCompilationException(L"Different return type found for function %s", funcName->Psz());
        }
        if (!mIsCallLegal)
        {
            Assert(!isMathBuiltin); // math builtins cannot change heap, so they are specifically excluded from this rule
            throw AsmJsCompilationException(L"Call is not legal at this location");
        }
        const int StartCallIndex = 0;
        const int CallIndex = 1;
        const int ArgOut_DbIndex = 2;
        const int ArgOut_IntIndex = 3;
        const int Conv_VTDIndex = 4;
        const int Conv_VTIIndex =5;
        const int Conv_VTFIndex = 6;
        const int funcOpCode = isFFI ? 0 : 1;

        // StartCall        
        const uint16 argCount = pnode->sxCall.argCount;
        StartStatement(pnode);
        ++mNestedCallCount;
        
        uint startCallOffset = mWriter.GetCurrentOffset();
        auto startCallChunk = mWriter.GetCurrentChunk();
        size_t startCallChunkOffset = startCallChunk->GetCurrentOffset();

        bool patchStartCall = sym->GetArgCount() == Constants::UninitializedValue;
        if (patchStartCall)
        {
            // we will not know the types of the arguments for the first call to a defered function,
            // so we put a placeholder instr in the bytecode and then patch it with correct arg size
            // once we evaluate the arguments
            mWriter.AsmStartCall(callOpCode[funcOpCode][StartCallIndex], Constants::InvalidArgSlot);
        }
        else
        {
            // args size + 1 pointer
            const ArgSlot argByteSize = sym->GetArgByteSize(argCount) + sizeof(Var);
            mWriter.AsmStartCall(callOpCode[funcOpCode][StartCallIndex], argByteSize);
        }
        AutoArrayPtr<AsmJsType> types(nullptr, 0);
        int maxDepthForLevel = 0;
        if( argCount > 0 )
        {
            ParseNode* argNode = pnode->sxCall.pnodeArgs;
            uint16 regSlotLocation = 1;
            types.Set(HeapNewArray( AsmJsType, argCount ), argCount);
            
            for( int i = 0; i < argCount; i++ )
            {
                // Get i arg node
                ParseNode* arg = argNode;
                if( argNode->nop == knopList )
                {
                    arg = ParserWrapper::GetBinaryLeft( argNode );
                    argNode = ParserWrapper::GetBinaryRight( argNode );
                }

                // Emit argument
                const EmitExpressionInfo& argInfo = Emit( arg );
                types[i] = argInfo.type;
                // OutParams i
                if( argInfo.type.isDouble() )
                {
                    CheckNodeLocation( argInfo, double );
                    if (callOpCode[funcOpCode][ArgOut_DbIndex] == OpCodeAsmJs::ArgOut_Db)
                    {
                        mWriter.AsmReg2(callOpCode[funcOpCode][ArgOut_DbIndex], regSlotLocation, argInfo.location);
                        regSlotLocation++; // in case of external calls this is boxed and converted to a Var                         
                    }
                    else
                    {
                        mWriter.AsmReg2(callOpCode[funcOpCode][ArgOut_DbIndex], regSlotLocation, argInfo.location);
                        regSlotLocation += sizeof(double) / sizeof(Var);// in case of internal calls we will pass this arg as double
                    }
                    mFunction->ReleaseLocation<double>( &argInfo );
                }
                else if (argInfo.type.isFloat())
                {
                    CheckNodeLocation(argInfo, float);
                    Assert(!isFFI);
                    mWriter.AsmReg2(OpCodeAsmJs::I_ArgOut_Flt, regSlotLocation, argInfo.location);
                    regSlotLocation++;
                    mFunction->ReleaseLocation<float>(&argInfo);
                }
                else if (argInfo.type.isInt())
                {
                    CheckNodeLocation( argInfo, int );
                    mWriter.AsmReg2(callOpCode[funcOpCode][ArgOut_IntIndex], regSlotLocation, argInfo.location);
                    regSlotLocation++;
                    mFunction->ReleaseLocation<int>( &argInfo );
                }
#ifdef SIMD_JS_ENABLED
                else if (SIMD_JS_FLAG && argInfo.type.isSIMDType())
                {
                    if (isFFI)
                    {
                        throw AsmJsCompilationException(L"FFI function %s doesn't support SIMD arguments", funcName->Psz());
                    }

                    CheckNodeLocation(argInfo, AsmJsSIMDValue);
                    switch (argInfo.type.GetWhich())
                    {
                    case AsmJsType::Int32x4:
                        mWriter.AsmReg2(OpCodeAsmJs::Simd128_I_ArgOut_I4, regSlotLocation, argInfo.location);
                        break;
                    case    AsmJsType::Float32x4:
                        mWriter.AsmReg2(OpCodeAsmJs::Simd128_I_ArgOut_F4, regSlotLocation, argInfo.location);
                        break;
                    case    AsmJsType::Float64x2:
                        mWriter.AsmReg2(OpCodeAsmJs::Simd128_I_ArgOut_D2, regSlotLocation, argInfo.location);
                        break;
                    }
                    regSlotLocation += sizeof(AsmJsSIMDValue) / sizeof(Var);
                    mFunction->ReleaseLocation<AsmJsSIMDValue>(&argInfo);
                }
#endif
                else
                {
                    Assert(UNREACHED);
                }
                // if there are nested calls, track whichever is the deepest
                if (maxDepthForLevel < mFunction->GetArgOutDepth())
                {
                    maxDepthForLevel = mFunction->GetArgOutDepth();
                }
            }
        }
        // Check if this function supports the type of these arguments
        AsmJsRetType retType;
        const bool supported = sym->SupportsArgCall( argCount, types, retType );
        if( !supported )
        {
            throw AsmJsCompilationException( L"Function %s doesn't support arguments", funcName->Psz() );
        }
        if (sym->GetSymbolType() == AsmJsSymbol::FuncPtrTable)
        {
            AsmJsFunctionTable* funcTable = sym->Cast<AsmJsFunctionTable>();
            if (!funcTable->EnsureSignatureIsKnown(mCompiler))
            {
                throw AsmJsCompilationException(L"Error trying to define function table %s's signature", funcName->Psz());
            }
        }
        // need to validate return type again because function might support arguments,
        // but return a different type ie:abs(int) -> int, but expecting double
        // don't validate the return type for foreign import functions
        if( !isFFI && retType != expectedType )
        {
            throw AsmJsCompilationException( L"Function %s returns different type", funcName->Psz() );
        }

        const ArgSlot argByteSize = sym->GetArgByteSize(argCount) + sizeof(Var);
        // +1 is for function object
        ArgSlot runtimeArg = argCount + 1;
        if (funcOpCode == 1) // for non import functions runtimeArg is calculated from argByteSize
        {
            runtimeArg = (ArgSlot)(::ceil((double)(argByteSize / sizeof(Var)))) + 1;
        }
        // +1 is for return address
        maxDepthForLevel += runtimeArg + 1;

        // Make sure we have enough memory allocated for OutParameters
        if (mNestedCallCount > 1)
        {
            mFunction->SetArgOutDepth(maxDepthForLevel);
        }
        else
        {
            mFunction->SetArgOutDepth(0);
        }
        mFunction->UpdateMaxArgOutDepth(maxDepthForLevel);

        if (patchStartCall)
        {
            uint latestOffset = mWriter.GetCurrentOffset();
            auto latestChunk = mWriter.GetCurrentChunk();
            size_t latestChunkOffset = latestChunk->GetCurrentOffset();

            // now that we know the types, we can patch the StartCall instr
            startCallChunk->SetCurrentOffset(startCallChunkOffset);
            mWriter.SetCurrent(startCallOffset, startCallChunk);

            // args size + 1 pointer
            mWriter.AsmStartCall(callOpCode[funcOpCode][StartCallIndex], argByteSize, true /* isPatching */);

            // ... and return to where we left off in the buffer like nothing ever happened
            latestChunk->SetCurrentOffset(latestChunkOffset);
            mWriter.SetCurrent(latestOffset, latestChunk);
        }

        // Load function from env
        switch( sym->GetSymbolType() )
        {
        case AsmJsSymbol::ModuleFunction:
            LoadModuleFunction( AsmJsFunctionMemory::FunctionRegister, sym->GetFunctionIndex() );
            break;
        case AsmJsSymbol::ImportFunction:
            LoadModuleFFI( AsmJsFunctionMemory::FunctionRegister, sym->GetFunctionIndex() );
            break;
        case AsmJsSymbol::FuncPtrTable:
            LoadModuleFunctionTable( AsmJsFunctionMemory::FunctionRegister, sym->GetFunctionIndex(), funcTableIndexRegister );
            mFunction->ReleaseTmpRegister<int>( funcTableIndexRegister );
            break;
        default:
            Assert( false );
        }
        
        // Call
        mWriter.AsmCall( callOpCode[funcOpCode][CallIndex], AsmJsFunctionMemory::CallReturnRegister, AsmJsFunctionMemory::FunctionRegister, runtimeArg, expectedType );
        // use expected type because return type could be invalid if the function is a FFI
        EmitExpressionInfo info( expectedType.toType() );
        switch( expectedType.which() )
        {
        case AsmJsRetType::Void  :
            // do nothing
            break;
        case AsmJsRetType::Signed:
        {
            RegSlot intReg = mFunction->AcquireTmpRegister<int>();
            mWriter.AsmReg2( callOpCode[funcOpCode][Conv_VTIIndex], intReg, AsmJsFunctionMemory::CallReturnRegister );
            info.location = intReg;
            break;
        }
        case AsmJsRetType::Double:
        {
            RegSlot dbReg = mFunction->AcquireTmpRegister<double>();
            mWriter.AsmReg2( callOpCode[funcOpCode][Conv_VTDIndex], dbReg, AsmJsFunctionMemory::CallReturnRegister );
            info.location = dbReg;
            break;
        }
        case AsmJsRetType::Float:
        {
            Assert(!isFFI); //check spec
            RegSlot fltReg = mFunction->AcquireTmpRegister<float>();
            mWriter.AsmReg2(callOpCode[funcOpCode][Conv_VTFIndex], fltReg, AsmJsFunctionMemory::CallReturnRegister);
            info.location = fltReg;
            break;
        }
#ifdef SIMD_JS_ENABLED
        case AsmJsRetType::Float32x4:
            if (SIMD_JS_FLAG)
            {
                Assert(!isFFI);
                RegSlot simdReg = mFunction->AcquireTmpRegister<AsmJsSIMDValue>();
                mWriter.AsmReg2(OpCodeAsmJs::Simd128_I_Conv_VTF4, simdReg, AsmJsFunctionMemory::CallReturnRegister);
                info.location = simdReg;
                break;
            }
            Assert(UNREACHED);
        case AsmJsRetType::Int32x4:
            if (SIMD_JS_FLAG)
            {
                Assert(!isFFI);
                RegSlot simdReg = mFunction->AcquireTmpRegister<AsmJsSIMDValue>();
                mWriter.AsmReg2(OpCodeAsmJs::Simd128_I_Conv_VTI4, simdReg, AsmJsFunctionMemory::CallReturnRegister);
                info.location = simdReg;
                break;
            }
            Assert(UNREACHED);
        case AsmJsRetType::Float64x2:
            if (SIMD_JS_FLAG)
            {
                Assert(!isFFI);
                RegSlot simdReg = mFunction->AcquireTmpRegister<AsmJsSIMDValue>();
                mWriter.AsmReg2(OpCodeAsmJs::Simd128_I_Conv_VTD2, simdReg, AsmJsFunctionMemory::CallReturnRegister);
                info.location = simdReg;
                break;
            }
            Assert(UNREACHED);
#endif
        default:
            break;
        }
        EndStatement(pnode);
        --mNestedCallCount;
        Assert(mNestedCallCount >= 0);

        return info;
    }

#ifdef SIMD_JS_ENABLED
    EmitExpressionInfo* AsmJSByteCodeGenerator::EmitSimdBuiltinArguments(ParseNode* pnode, AsmJsFunctionDeclaration* func, __out_ecount(pnode->sxCall.argCount) AsmJsType *argsTypes, EmitExpressionInfo *argsInfo)
    {
        const uint16 argCount = pnode->sxCall.argCount;
        Assert(argsTypes);
        Assert(argsInfo);
        
        if (argCount > 0)
        {
            ParseNode* argNode = pnode->sxCall.pnodeArgs;
            
            for (int i = 0; i < argCount; i++)
            {
                // Get i arg node
                ParseNode* arg = argNode;
                
                if (argNode->nop == knopList)
                {
                    arg = ParserWrapper::GetBinaryLeft(argNode);
                    argNode = ParserWrapper::GetBinaryRight(argNode);
                }
                if (SIMD_JS_FLAG && (func->GetSymbolType() == AsmJsSymbol::SIMDBuiltinFunction))
                {
                    AsmJsSIMDFunction *simdFunc = func->Cast<AsmJsSIMDFunction>();

                    if (arg->nop == knopCall)
                    {
                        // REVIEW: Is this exactly according to spec ?
                        // This enforces Asm.js rule that all arg calls to user-functions have to be coerced.
                        // Generic calls have to be coerced unless used in a SIMD coercion.
                        // For example, we cannot do f4add(foo(), bar()), but we can do f4add(f4(foo()), f4(bar()))
                        //
                        // We are only allowed calls as args in similar cases:
                        //      Float32x4: 
                        //          f4(foo());                      call coercion, any call is allowed
                        //          f4(fround(), fround(), ...);    constructor, only fround is allowed
                        //          f4add(f4sub(..),f4(..));        operation, only other SIMD functions are allowed (including coercion)
                        //
                        //      Int32x4: 
                        //          i4(foo());                      call coercion, any call is allowed
                        //          i4add(i4sub(), i4());           operation, only other SIMD functions are allowed (including coercion)
                        //      
                        //      Float64x2: 
                        //          similar to Int32x4

                        PropertyName argCallTarget = ParserWrapper::VariableName(arg->sxCall.pnodeTarget);
                        AsmJsFunctionDeclaration* argCall = mCompiler->LookupFunction(argCallTarget);

                        if (!argCall)
                        {
                            throw AsmJsCompilationException(L"Undefined function %s.", argCallTarget->Psz());
                        }

                        EmitExpressionInfo argInfo;

                        if (simdFunc->IsTypeCheck(argCount))
                        {
                            // type check. Any call is allowed as argument.
                            Assert(i == 0);
                            argInfo = EmitCall(arg, simdFunc->GetReturnType());
                        }
                        // special case for fround inside some float32x4 operations
                        // f4(fround(), ...) , f4splat(fround()), f4.withX/Y/Z/W(fround())
                        else if ((simdFunc->IsConstructor(4) && simdFunc->GetSimdBuiltInFunction() == AsmJsSIMDBuiltinFunction::AsmJsSIMDBuiltin_float32x4) ||  /*float32x4 all args*/
                            simdFunc->GetSimdBuiltInFunction() == AsmJsSIMDBuiltinFunction::AsmJsSIMDBuiltin_float32x4_splat ||                                 /*splat all args*/
                            (i == 1 &&                                                                                                                          /*second arg to withX/Y/Z/W*/
                            (simdFunc->GetSimdBuiltInFunction() == AsmJsSIMDBuiltinFunction::AsmJsSIMDBuiltin_float32x4_withX ||
                            simdFunc->GetSimdBuiltInFunction() == AsmJsSIMDBuiltinFunction::AsmJsSIMDBuiltin_float32x4_withY ||
                            simdFunc->GetSimdBuiltInFunction() == AsmJsSIMDBuiltinFunction::AsmJsSIMDBuiltin_float32x4_withZ ||
                            simdFunc->GetSimdBuiltInFunction() == AsmJsSIMDBuiltinFunction::AsmJsSIMDBuiltin_float32x4_withW)))
                        {

                            if (argCall && argCall->GetSymbolType() == AsmJsSymbol::MathBuiltinFunction && IsFRound(argCall->Cast<AsmJsMathFunction>()))
                            {
                                argInfo = EmitCall(arg, AsmJsRetType::Float);
                            }
                            else
                            {
                                throw AsmJsCompilationException(L"Invalid call as SIMD argument. Expecting fround.");
                            }
                        }
                        else if (AsmJsSIMDFunction::SameTypeOperations(simdFunc, argCall->Cast<AsmJsSIMDFunction>()))
                        {
                            // any other simd operation. call arguments have to be SIMD operations of same type.
                            argInfo = EmitCall(arg, simdFunc->GetArgType(i).toRetType());
                        }
                        else
                        {
                            throw AsmJsCompilationException(L"Invalid call as SIMD argument");
                        }

                        argsTypes[i] = argInfo.type;
                        argsInfo[i].type = argInfo.type;
                        argsInfo[i].location = argInfo.location;
                        // arg already emitted
                        continue;
                    }
                    else if (arg->nop == knopFlt && simdFunc->IsFloat32x4Func())
                    {
                        // Any floating point constant as float32x4 op arg is considered DoubleLit
                        // For all float32x4 operations, if the arg type is DoubleLit, regSlot should be in Float reg space.
                        argsTypes[i] = AsmJsType::DoubleLit;
                        argsInfo[i].type = AsmJsType::DoubleLit;
                        argsInfo[i].location = mFunction->GetConstRegister<float>((float)arg->sxFlt.dbl);
                        // no need to emit
                        continue;
                    }
                    
                }
                // Emit argument
                const EmitExpressionInfo& argInfo = Emit(arg);
                argsTypes[i] = argInfo.type;
                argsInfo[i].type = argInfo.type;
                argsInfo[i].location = argInfo.location;
            }
        }
        return argsInfo;
    }

    bool    AsmJSByteCodeGenerator::ValidateSimdFieldAccess(PropertyName field, const AsmJsType& receiverType, OpCodeAsmJs &op, int &laneIndex, AsmJsType &laneType)
    {
        AssertMsg(PropertyIds::x == PropertyIds::y - 1 && PropertyIds::x == PropertyIds::z - 2 && PropertyIds::x == PropertyIds::w - 3, "Expecting contiguous SIMD lanes property IDs");
        
        PropertyId fieldId = field->GetPropertyId();
        // Bind propertyId if not already.
        if (fieldId == Js::Constants::NoProperty)
        {
            mByteCodeGenerator->AssignPropertyId(field);
            fieldId = field->GetPropertyId();
        }

        if (receiverType.isSIMDType())
        {
            if (fieldId >= PropertyIds::x && fieldId <= PropertyIds::w)
            {
                switch (receiverType.GetWhich())
                {
                    case AsmJsType::Int32x4:
                        op = OpCodeAsmJs::Simd128_LdLane_I4;
                        laneType = AsmJsType::Signed;
                        break;
                    case AsmJsType::Float32x4:
                        op = OpCodeAsmJs::Simd128_LdLane_F4;
                        laneType = AsmJsType::Float;
                        break;
                    case AsmJsType::Float64x2:
                        if (fieldId >= PropertyIds::z)
                        {
                            return false;
                        }
                        op = OpCodeAsmJs::Simd128_LdLane_D2;
                        laneType = AsmJsType::Double;
                        break;
                    default:
                        Assert(UNREACHED);
                }
                laneIndex = fieldId - PropertyIds::x;
                return true;
            }
            
            if (fieldId == PropertyIds::signMask)
            {
                switch (receiverType.GetWhich())
                {
                case AsmJsType::Int32x4:
                    op = OpCodeAsmJs::Simd128_LdSignMask_I4;
                    break;
                case AsmJsType::Float32x4:
                    op = OpCodeAsmJs::Simd128_LdSignMask_F4;
                    break;
                case AsmJsType::Float64x2:
                    op = OpCodeAsmJs::Simd128_LdSignMask_D2;
                    break;
                default:
                    Assert(UNREACHED);
                }
                return true;
            }
        }
        return false;
    }

    EmitExpressionInfo AsmJSByteCodeGenerator::EmitDotExpr(ParseNode* pnode)
    {
        Assert(ParserWrapper::IsDotMember(pnode));
        EmitExpressionInfo exprInfo(Constants::NoRegister, AsmJsType::Void);
        OpCodeAsmJs opcode;
        int laneIndex = -1;
        AsmJsType laneType = AsmJsType::Void;
        RegSlot dst = Constants::NoRegister;
        ParseNode* base = ParserWrapper::DotBase(pnode);
        PropertyName field = ParserWrapper::DotMember(pnode);
        EmitExpressionInfo baseInfo = Emit(base);
       
        
        
        if (!ValidateSimdFieldAccess(field, baseInfo.type, opcode, laneIndex, laneType))
        {
            throw AsmJsCompilationException(L"Expression does not support field access or invalid field name");
        }
        
        AssertMsg(baseInfo.type.isSIMDType(), "Expecting SIMD value");
        mFunction->ReleaseLocation<AsmJsSIMDValue>(&baseInfo);
        
        if (laneType != AsmJsType::Void)
        {
            // lane access
            AssertMsg((laneIndex >= SIMD_X && laneIndex <= SIMD_Y) || (laneIndex >= SIMD_Z && laneIndex <= SIMD_W && (baseInfo.type.isSIMDFloat32x4() || baseInfo.type.isSIMDInt32x4())), "Invalid SIMD lane index");
            
            switch (laneType.GetWhich())
            {
            case AsmJsType::Signed:
                dst = mFunction->AcquireTmpRegister<int>();
                break;
            case AsmJsType::Float:
                dst = mFunction->AcquireTmpRegister<float>();
                break;
            case AsmJsType::Double:
                dst = mFunction->AcquireTmpRegister<double>();
                break;
            default:
                Assert(UNREACHED);
            }
            mWriter.AsmReg2IntConst1(opcode, dst, baseInfo.location, laneIndex);
            exprInfo.type = laneType;
            exprInfo.location = dst;
        }
        else
        {
            // sign mask
            dst = mFunction->AcquireTmpRegister<int>();
            mWriter.AsmReg2(opcode, dst, baseInfo.location);
            exprInfo.type = AsmJsType::Signed;
            exprInfo.location = dst;
        }
        
        return exprInfo;
    }

    EmitExpressionInfo AsmJSByteCodeGenerator::EmitSimdBuiltin(ParseNode* pnode, AsmJsSIMDFunction* simdFunction, AsmJsRetType expectedType)
    {
        // StartCall
        const uint16 argCount = pnode->sxCall.argCount;
        
        AutoArrayPtr<AsmJsType> types(nullptr, 0);
        AutoArrayPtr<EmitExpressionInfo> argsInfo(nullptr, 0);

        if (argCount > 0)
        {
            types.Set(HeapNewArray(AsmJsType, argCount), argCount);
            argsInfo.Set(HeapNewArray(EmitExpressionInfo, argCount), argCount);

            EmitSimdBuiltinArguments(pnode, simdFunction, types, argsInfo);
        }

        AsmJsRetType retType;
        OpCodeAsmJs op;
        const bool supported = simdFunction->SupportsSIMDCall(argCount, types, op, retType);

        if (!supported)
        {
            throw AsmJsCompilationException(L"SIMD builtin function doesn't support arguments");
        }

        Assert(retType.toVarType().isSIMD());

        // If a simd built-in is used without coercion, then expectedType is Void
        // e.g. x = f4add(a, b);

        if (expectedType != AsmJsRetType::Void && retType != expectedType)
        {
            throw AsmJsCompilationException(L"SIMD builtin function returns wrong type");
        }

        // Release all used location before acquiring a new tmp register
        for (int i = argCount - 1; i >= 0; i--)
        {
            mFunction->ReleaseLocationGeneric(&argsInfo[i]);
        }

        RegSlot dst = mFunction->AcquireTmpRegister<AsmJsSIMDValue>();

        EmitExpressionInfo emitInfo(dst, retType.toType());

        switch (argCount){
        case 1:
            mWriter.AsmReg2(op, dst, argsInfo[0].location);
            break;
        case 2:
            mWriter.AsmReg3(op, dst, argsInfo[0].location, argsInfo[1].location);
            break;
        case 3: 
            mWriter.AsmReg4(op, dst, argsInfo[0].location, argsInfo[1].location, argsInfo[2].location);
            break;
        case 4:
            mWriter.AsmReg5(op, dst, argsInfo[0].location, argsInfo[1].location, argsInfo[2].location, argsInfo[3].location);
            break;
        case 5:
            mWriter.AsmReg6(op, dst, argsInfo[0].location, argsInfo[1].location, argsInfo[2].location, argsInfo[3].location, argsInfo[4].location);
            break;

        default:
            AssertMsg(UNREACHED, "Wrong argument count to SIMD function");
        }
        
        return emitInfo;
        
    }
#endif
   
    EmitExpressionInfo AsmJSByteCodeGenerator::EmitMathBuiltin(ParseNode* pnode, AsmJsMathFunction* mathFunction, AsmJsRetType expectedType)
    {
        if (mathFunction->GetMathBuiltInFunction() == AsmJSMathBuiltinFunction::AsmJSMathBuiltin_max || mathFunction->GetMathBuiltInFunction() == AsmJSMathBuiltinFunction::AsmJSMathBuiltin_min)
        {
            return EmitMinMax(pnode, mathFunction, expectedType);
        }

        ++mNestedCallCount;

        const uint16 argCount = pnode->sxCall.argCount;
        ParseNode* argNode = pnode->sxCall.pnodeArgs;

        // for fround, if we have a fround(NumericLiteral), we want to just emit Ld_Flt NumericLiteral
        if (argCount == 1 && IsFRound(mathFunction) && ParserWrapper::IsFroundNumericLiteral(argNode))
        {
            Assert(expectedType == AsmJsRetType::Float);
            StartStatement(pnode);
            RegSlot dst = mFunction->AcquireTmpRegister<float>();
            EmitExpressionInfo emitInfo(dst, expectedType.toType());
            if (argNode->nop == knopFlt)
            {
                mWriter.AsmReg2(OpCodeAsmJs::Ld_Flt, dst, mFunction->GetConstRegister<float>((float)argNode->sxFlt.dbl));
            }
            else if (argNode->nop == knopInt)
            {
                mWriter.AsmReg2(OpCodeAsmJs::Ld_Flt, dst, mFunction->GetConstRegister<float>((float)argNode->sxInt.lw));
            }
            else
            {
                Assert(ParserWrapper::IsNegativeZero(argNode));
                mWriter.AsmReg2(OpCodeAsmJs::Ld_Flt, dst, mFunction->GetConstRegister<float>(-0.0f));
            }
            EndStatement(pnode);
            return emitInfo;
        }
        
        // The logic here is similar to EmitSimdBuiltinArguments()
        // TODO: Maybe outline this to EmitArguments() after RI. Currently it is causing frequent conflicts upon FI.

        AutoArrayPtr<AsmJsType> types(nullptr, 0);
        AutoArrayPtr<EmitExpressionInfo> argsInfo(nullptr, 0);
        int maxDepthForLevel = 0;
        if( argCount > 0 )
        {
            types.Set(HeapNewArray(AsmJsType, argCount), argCount);
            argsInfo.Set(HeapNewArray(EmitExpressionInfo, argCount), argCount);

            for( int i = 0; i < argCount; i++ )
            {
                // Get i arg node
                ParseNode* arg = argNode;
                // Special case for fround(abs()) call 
                if (argNode->nop == knopCall && mathFunction->GetMathBuiltInFunction() == AsmJSMathBuiltinFunction::AsmJSMathBuiltin_fround)
                {
                    // Emit argument
                    const EmitExpressionInfo& argInfo = EmitCall(arg, AsmJsRetType::Float);
                    types[i] = argInfo.type;
                    argsInfo[i].type = argInfo.type;
                    argsInfo[i].location = argInfo.location;
                }
                else
                {
                    if (argNode->nop == knopList)
                    {
                        arg = ParserWrapper::GetBinaryLeft(argNode);
                        argNode = ParserWrapper::GetBinaryRight(argNode);
                    }
                    // Emit argument
                    const EmitExpressionInfo& argInfo = Emit(arg);
                    types[i] = argInfo.type;
                    argsInfo[i].type = argInfo.type;
                    argsInfo[i].location = argInfo.location;
                }
                // if there are nested calls, track whichever is the deepest
                if (maxDepthForLevel < mFunction->GetArgOutDepth())
                {
                    maxDepthForLevel = mFunction->GetArgOutDepth();
                }
            }
        }
        StartStatement(pnode);
        // Check if this function supports the type of these arguments
        AsmJsRetType retType;
        OpCodeAsmJs op;
        const bool supported = mathFunction->SupportsMathCall( argCount, types, op, retType );
        if( !supported )
        {
            throw AsmJsCompilationException( L"Math builtin function doesn't support arguments" );
        }

        // Release all used location before acquiring a new tmp register
        for (int i = argCount - 1; i >= 0 ; i--)
        {
            mFunction->ReleaseLocationGeneric( &argsInfo[i] );
        }

        const ArgSlot argByteSize = mathFunction->GetArgByteSize(argCount) + sizeof(Var);
        // +1 is for function object
        ArgSlot runtimeArg = (ArgSlot)(::ceil((double)(argByteSize / sizeof(Var)))) + 1;
        // +1 is for return address
        maxDepthForLevel += runtimeArg + 1;

        // Make sure we have enough memory allocated for OutParameters
        if (mNestedCallCount > 1)
        {
            mFunction->SetArgOutDepth(maxDepthForLevel);
        }
        else
        {
            mFunction->SetArgOutDepth(0);
        }
        mFunction->UpdateMaxArgOutDepth(maxDepthForLevel);

        const bool isInt = retType.toType().isInt();
        const bool isFloatish = retType.toType().isFloatish();
        Assert(isInt || isFloatish || retType.toType().isDouble());

        RegSlot dst;
        if( isInt )
        {
            dst = mFunction->AcquireTmpRegister<int>();
        }
        else if (isFloatish)
        {
            dst = mFunction->AcquireTmpRegister<float>();
        }
        else
        {
            dst = mFunction->AcquireTmpRegister<double>();
        }

        EmitExpressionInfo emitInfo(dst, retType.toType());

        switch( argCount )
        {
        case 1:
            mWriter.AsmReg2( op, dst, argsInfo[0].location );
            break;
        case 2:
            mWriter.AsmReg3( op, dst, argsInfo[0].location, argsInfo[1].location );
            break;
        default:
            Assume(UNREACHED);
        }
#if DBG
        for (int i = 0; i < argCount; i++)
        {
            if (argsInfo[i].type.isSubType(AsmJsType::Floatish))
            {
                CheckNodeLocation(argsInfo[i], float);
            }
            else if (argsInfo[i].type.isSubType(AsmJsType::MaybeDouble))
            {
                CheckNodeLocation(argsInfo[i], double);
            }
            else if (argsInfo[i].type.isSubType(AsmJsType::Intish))
            {
                CheckNodeLocation(argsInfo[i], int);
            }
        }
#endif
        EndStatement(pnode);
        --mNestedCallCount;
        return emitInfo;
    }

    EmitExpressionInfo AsmJSByteCodeGenerator::EmitMinMax(ParseNode* pnode, AsmJsMathFunction* mathFunction, AsmJsRetType expectedType)
    {
        Assert(mathFunction->GetArgCount() == 2);
        ++mNestedCallCount;
        
        uint16 argCount = pnode->sxCall.argCount;
        ParseNode* argNode = pnode->sxCall.pnodeArgs;

        if (argCount < 2)
        {
            throw AsmJsCompilationException(L"Math builtin function doesn't support arguments");
        }

        AutoArrayPtr<AsmJsType> types(nullptr, 0);
        AutoArrayPtr<EmitExpressionInfo> argsInfo(nullptr, 0);
        types.Set(HeapNewArray(AsmJsType, mathFunction->GetArgCount()), mathFunction->GetArgCount());
        argsInfo.Set(HeapNewArray(EmitExpressionInfo, mathFunction->GetArgCount()), mathFunction->GetArgCount());

        ParseNode * arg = ParserWrapper::GetBinaryLeft(argNode);
        argNode = ParserWrapper::GetBinaryRight(argNode);
        // Emit first arg as arg0
        argsInfo[0] = Emit(arg);
        int maxDepthForLevel = mFunction->GetArgOutDepth();
        types[0] = argsInfo[0].type;

        EmitExpressionInfo dstInfo;
        for (int i = 1; i < argCount; i++)
        {
            if (argNode->nop == knopList)
            {
                arg = ParserWrapper::GetBinaryLeft(argNode);
                argNode = ParserWrapper::GetBinaryRight(argNode);
            }
            else
            {
                arg = argNode;
            }
            // arg1 will always be the next arg in the argList
            argsInfo[1] = Emit(arg);
            types[1] = argsInfo[1].type;

            // if there are nested calls, track whichever is the deepest
            if (maxDepthForLevel < mFunction->GetArgOutDepth())
            {
                maxDepthForLevel = mFunction->GetArgOutDepth();
            }

            // Check if this function supports the type of these arguments
            AsmJsRetType retType;
            OpCodeAsmJs op;
            const bool supported = mathFunction->SupportsMathCall(mathFunction->GetArgCount(), types, op, retType);
            if (!supported)
            {
                throw AsmJsCompilationException(L"Math builtin function doesn't support arguments");
            }

            const ArgSlot argByteSize = mathFunction->GetArgByteSize(argCount) + sizeof(Var);
            // +1 is for function object
            ArgSlot runtimeArg = (ArgSlot)(::ceil((double)(argByteSize / sizeof(Var)))) + 1;
            // +1 is for return address
            maxDepthForLevel += runtimeArg + 1;

            // Make sure we have enough memory allocated for OutParameters
            if (mNestedCallCount > 1)
            {
                mFunction->SetArgOutDepth(maxDepthForLevel);
            }
            else
            {
                mFunction->SetArgOutDepth(0);
            }
            mFunction->UpdateMaxArgOutDepth(maxDepthForLevel);
            maxDepthForLevel = 0;
            mFunction->ReleaseLocationGeneric(&argsInfo[1]);
            mFunction->ReleaseLocationGeneric(&argsInfo[0]);

            dstInfo.type = retType.toType();
            if (retType.toType().isSigned())
            {
                dstInfo.location = mFunction->AcquireTmpRegister<int>();
            }
            else
            {
                Assert(retType.toType().isDouble());
                dstInfo.location = mFunction->AcquireTmpRegister<double>();
            }

            mWriter.AsmReg3(op, dstInfo.location, argsInfo[0].location, argsInfo[1].location);
            // for max/min calls with more than 2 arguments, we use the result of previous call for arg0 
            argsInfo[0] = dstInfo;
#if DBG
            for (uint j = 0; j < mathFunction->GetArgCount(); j++)
            {
                if (argsInfo[j].type.isSubType(AsmJsType::MaybeDouble))
                {
                    CheckNodeLocation(argsInfo[j], double);
                }
                else if (argsInfo[j].type.isSubType(AsmJsType::Intish))
                {
                    CheckNodeLocation(argsInfo[j], int);
                }
                else
                {
                    Assert(UNREACHED);
                }
            }
#endif
        }
        --mNestedCallCount;
        return dstInfo;
    }

    Js::EmitExpressionInfo AsmJSByteCodeGenerator::EmitIdentifier( ParseNode * pnode )
    {
        Assert( ParserWrapper::IsNameDeclaration( pnode ) );
        PropertyName name = pnode->name();
        AsmJsLookupSource::Source source;
        AsmJsSymbol* sym = mCompiler->LookupIdentifier( name, mFunction, &source );
        if( !sym )
        {
            throw AsmJsCompilationException( L"Undefined identifier %s", name->Psz() );
        }

        switch( sym->GetSymbolType() )
        {
        case AsmJsSymbol::Variable:{
            AsmJsVar * var = sym->Cast<AsmJsVar>();
            if (!var->isMutable())
            {
                // currently const is only allowed for variables at module scope
                Assert(source == AsmJsLookupSource::AsmJsModule);

                EmitExpressionInfo emitInfo(var->GetType());
                if (var->GetVarType().isInt())
                {
                    emitInfo.location = mFunction->AcquireTmpRegister<int>();
                    mWriter.AsmInt1Const1(Js::OpCodeAsmJs::Ld_IntConst, emitInfo.location, var->GetIntInitialiser());
                }
                else if (var->GetVarType().isFloat())
                {
                    emitInfo.location = mFunction->AcquireTmpRegister<float>();
                    mWriter.AsmReg2(Js::OpCodeAsmJs::Ld_Flt, emitInfo.location, mFunction->GetConstRegister<float>(var->GetFloatInitialiser()));
                }
                else
                {
                    Assert(var->GetVarType().isDouble());
                    emitInfo.location = mFunction->AcquireTmpRegister<double>();
                    mWriter.AsmReg2(Js::OpCodeAsmJs::Ld_Db, emitInfo.location, mFunction->GetConstRegister<double>(var->GetDoubleInitialiser()));
                }
                return emitInfo;
            }
            // else fall through
        }
        case AsmJsSymbol::Argument:
        case AsmJsSymbol::ConstantImport:{
            AsmJsVarBase* var = sym->Cast<AsmJsVarBase>();
            if( source == AsmJsLookupSource::AsmJsFunction )
            {
                return EmitExpressionInfo( var->GetLocation(), var->GetType() );
            }
            else
            {
                Assert( source == AsmJsLookupSource::AsmJsModule );
                EmitExpressionInfo emitInfo(var->GetType());
                if (var->GetVarType().isInt())
                {
                    emitInfo.location = mFunction->AcquireTmpRegister<int>();
                    LoadModuleInt(emitInfo.location, var->GetLocation());
                }
                else if (var->GetVarType().isFloat())
                {
                    emitInfo.location = mFunction->AcquireTmpRegister<float>();
                    LoadModuleFloat(emitInfo.location, var->GetLocation());
                }
                else if (var->GetVarType().isDouble())
                {
                    emitInfo.location = mFunction->AcquireTmpRegister<double>();
                    LoadModuleDouble(emitInfo.location, var->GetLocation());
                }
#ifdef SIMD_JS_ENABLED
                else if (SIMD_JS_FLAG && var->GetVarType().isSIMD())
                {
                    emitInfo.location = mFunction->AcquireTmpRegister<AsmJsSIMDValue>();
                    LoadModuleSimd( emitInfo.location, var->GetLocation(), var->GetVarType());
                }
#endif
                else
                {
                    Assert(UNREACHED);
                }
                return emitInfo;
            }
            break;
        }
        case AsmJsSymbol::MathConstant:{
            AsmJsMathConst* mathConst = sym->Cast<AsmJsMathConst>();
            
            if( mathConst->GetType().isDouble() )
            {
                RegSlot loc = mFunction->AcquireTmpRegister<double>();
                mWriter.AsmDouble1Addr1( OpCodeAsmJs::Ld_DbAddr, loc, mathConst->GetVal() );
                return EmitExpressionInfo( loc, AsmJsType::Double );
            }
            else
            {
                Assert( false ); // Currently all math const are doubles
            }
            break;
        }

#ifdef SIMD_JS_ENABLED
        case AsmJsSymbol::SIMDBuiltinFunction:
#endif
        case AsmJsSymbol::ImportFunction     :
        case AsmJsSymbol::FuncPtrTable       :
        case AsmJsSymbol::ModuleFunction     :
        case AsmJsSymbol::ArrayView          :
        case AsmJsSymbol::MathBuiltinFunction:
        default:
            throw AsmJsCompilationException( L"Cannot use identifier %s in this context", name->Psz() );
            break;
        }

        Assert( false ); // all cases should be handled
        return EmitExpressionInfo();
    }

    static const OpCodeAsmJs typedArrayOp[2][2] =
    {
        { OpCodeAsmJs::LdArrConst, OpCodeAsmJs::LdArr },//LoadTypedArray
        { OpCodeAsmJs::StArrConst, OpCodeAsmJs::StArr },//StoreTypedArray
    };

    EmitExpressionInfo AsmJSByteCodeGenerator::EmitTypedArrayIndex(ParseNode* indexNode, OpCodeAsmJs &op, uint32 &indexSlot, ArrayBufferView::ViewType viewType, TypedArrayEmitType emitType)
    {
        mCompiler->SetUsesHeapBuffer(true);
        bool isConst = false;
        uint32 slot = 0;
        if(indexNode->nop == knopName)
        {
            AsmJsSymbol * declSym = mCompiler->LookupIdentifier(indexNode->name(), mFunction);
            if (declSym && !declSym->isMutable() && declSym->GetSymbolType() == AsmJsSymbol::Variable)
            {
                AsmJsVar * definition = declSym->Cast<AsmJsVar>();
                if(definition->GetVarType().isInt())
                {
                    slot = (uint32)definition->GetIntInitialiser();
                    isConst = true;
                }
            }
        }
        if (indexNode->nop == knopInt || indexNode->nop == knopFlt || isConst)
        {
            // Emit a different opcode for numerical literral
            if (!isConst)
            {
                if (indexNode->nop == knopInt)
                {
                    slot = (uint32)indexNode->sxInt.lw;
                }
                else if (ParserWrapper::IsMinInt(indexNode))
                {
                    // this is going to be an error, but we can do this to allow it to get same error message as invalid int
                    slot = (uint32)MININT32;
                }
                else if (ParserWrapper::IsUnsigned(indexNode))
                {
                    slot = (uint32)indexNode->sxFlt.dbl;
                }
                else
                {
                    EmitExpressionInfo indexInfo = Emit(indexNode);
                    throw AsmJsCompilationException(L"Array Index must be intish; %s given", indexInfo.type.toChars());
                }
            }
            // do the right shift now
            switch( viewType )
            {
            case Js::ArrayBufferView::TYPE_INT16:
            case Js::ArrayBufferView::TYPE_UINT16:
                if (slot & 0x80000000)
                {
                    throw AsmJsCompilationException(L"Numeric literal for heap16 must be within 0 <= n < 2^31; %d given", slot);
                }
                slot <<= 1;
                break;
            case Js::ArrayBufferView::TYPE_INT32:
            case Js::ArrayBufferView::TYPE_UINT32:
            case Js::ArrayBufferView::TYPE_FLOAT32:
                if (slot & 0xC0000000)
                {
                    throw AsmJsCompilationException(L"Numeric literal for heap32 must be within 0 <= n < 2^30; %d given", slot);
                }
                slot <<= 2;
                break;
            case Js::ArrayBufferView::TYPE_FLOAT64:
                if (slot & 0xE0000000)
                {
                    throw AsmJsCompilationException(L"Numeric literal for heap64 must be within 0 <= n < 2^29; %d given", slot);
                }
                slot <<= 3;
                break;
            default:
                break;
            }
            mCompiler->UpdateMaxHeapAccess(slot);
            op = typedArrayOp[emitType][0];
        }
        else
        {
            EmitExpressionInfo indexInfo;
            if (indexNode->nop != knopRsh && viewType != Js::ArrayBufferView::TYPE_INT8 && viewType != Js::ArrayBufferView::TYPE_UINT8)
            {
                throw AsmJsCompilationException( L"index expression isn't shifted; must be an Int8/Uint8 access" );
            }
            int val = 0;
            uint32 mask = (uint32)~0;
            ParseNode* index;
            if (indexNode->nop == knopRsh)
            {
                ParseNode* rhsNode = ParserWrapper::GetBinaryRight(indexNode);
                if (!rhsNode || rhsNode->nop != knopInt)
                {
                    throw AsmJsCompilationException(L"shift amount must be constant");
                }
                switch (viewType)
                {
                case Js::ArrayBufferView::TYPE_INT8:
                case Js::ArrayBufferView::TYPE_UINT8:
                    val = 0;
                    mask = (uint32)~0;
                    break;
                case Js::ArrayBufferView::TYPE_INT16:
                case Js::ArrayBufferView::TYPE_UINT16:
                    val = 1;
                    mask = (uint32)~1;
                    break;
                case Js::ArrayBufferView::TYPE_INT32:
                case Js::ArrayBufferView::TYPE_UINT32:
                case Js::ArrayBufferView::TYPE_FLOAT32:
                    val = 2;
                    mask = (uint32)~3;
                    break;
                case Js::ArrayBufferView::TYPE_FLOAT64:
                    val = 3;
                    mask = (uint32)~7;
                    break;
                default:
                    Assume(UNREACHED);
                }
                if (rhsNode->sxInt.lw != val)
                {
                    throw AsmJsCompilationException(L"shift amount must be %d", val);
                }
                index = ParserWrapper::GetBinaryLeft(indexNode);
            }
            else
            {
                index = indexNode;
            }

            bool isConst = false;
            if (index->nop == knopName)
            {
                AsmJsSymbol * declSym = mCompiler->LookupIdentifier(index->name(), mFunction);
                if (declSym && !declSym->isMutable() && declSym->GetSymbolType() == AsmJsSymbol::Variable)
                {
                    AsmJsVar * definition = declSym->Cast<AsmJsVar>();
                    if (definition->GetVarType().isInt())
                    {
                        slot = (uint32)definition->GetIntInitialiser();
                        slot &= mask;
                        op = typedArrayOp[emitType][0];
                        isConst = true;
                        mCompiler->UpdateMaxHeapAccess(slot);
                    }
                }
            }
            if( ParserWrapper::IsUInt( index) )
            {
                slot = ParserWrapper::GetUInt(index);
                slot &= mask;
                op = typedArrayOp[emitType][0];

                mCompiler->UpdateMaxHeapAccess(slot);
            }
            else if (!isConst)
            {
                indexInfo = Emit( index );
                if( !indexInfo.type.isIntish() )
                {
                    throw AsmJsCompilationException( L"Left operand of >> must be intish; %s given", indexInfo.type.toChars() );
                }
                indexSlot = indexInfo.location;
                op = typedArrayOp[emitType][1];
                return indexInfo;
            }
        }
        indexSlot = slot;
        return EmitExpressionInfo();
    }

    Js::EmitExpressionInfo AsmJSByteCodeGenerator::EmitLdArrayBuffer( ParseNode * pnode )
    {
        ParseNode* arrayNameNode = ParserWrapper::GetBinaryLeft( pnode );
        ParseNode* indexNode = ParserWrapper::GetBinaryRight( pnode );
        if( !ParserWrapper::IsNameDeclaration( arrayNameNode ) )
        {
            throw AsmJsCompilationException( L"Invalid symbol " );
        }

        PropertyName name = arrayNameNode->name();
        AsmJsSymbol* sym = mCompiler->LookupIdentifier(name, mFunction);
        if( !sym || sym->GetSymbolType() != AsmJsSymbol::ArrayView )
        {
            throw AsmJsCompilationException( L"Invalid identifier %s", name->Psz() );
        }
        AsmJsArrayView* arrayView = sym->Cast<AsmJsArrayView>();
        ArrayBufferView::ViewType viewType = arrayView->GetViewType();

        OpCodeAsmJs op;
        uint32 indexSlot = 0;
        // if changeHeap is implemented, calls are illegal in index expression
        bool wasCallLegal = mIsCallLegal;
        mIsCallLegal = !mCompiler->UsesChangeHeap();
        EmitExpressionInfo indexInfo = EmitTypedArrayIndex(indexNode, op, indexSlot, viewType, LoadTypedArray);
        mIsCallLegal = wasCallLegal;
        mFunction->ReleaseLocationGeneric(&indexInfo);

        EmitExpressionInfo info( arrayView->GetType() );
        if( info.type.isIntish() )
        {
            info.location = mFunction->AcquireTmpRegister<int>();
        }
        else if (info.type.isMaybeFloat())
        {            
            info.location = mFunction->AcquireTmpRegister<float>();
        }
        else
        {
            Assert(info.type.isMaybeDouble());
            info.location = mFunction->AcquireTmpRegister<double>();
        }
        mWriter.AsmTypedArr( op, info.location, indexSlot, viewType );

        return info;
    }

    EmitExpressionInfo AsmJSByteCodeGenerator::EmitAssignment( ParseNode * pnode )
    {
        StartStatement(pnode);
        ParseNode* lhs = ParserWrapper::GetBinaryLeft( pnode );
        ParseNode* rhs = ParserWrapper::GetBinaryRight(pnode);
        EmitExpressionInfo rhsEmit;
        if( ParserWrapper::IsNameDeclaration( lhs ) )
        {
            rhsEmit = Emit(rhs);
            const AsmJsType& rType = rhsEmit.type;

            PropertyName name = lhs->name();
            AsmJsLookupSource::Source source;
            AsmJsSymbol* sym = mCompiler->LookupIdentifier( name, mFunction, &source );
            if( !sym )
            {
                throw AsmJsCompilationException( L"Undefined identifier %s", name->Psz() );
            }

            if( !sym->isMutable() )
            {
                throw AsmJsCompilationException( L"Cannot assign to identifier %s", name->Psz() );
            }

            AsmJsVarBase* var = sym->Cast<AsmJsVarBase>();
            if( !var->GetType().isSuperType( rType ) )
            {
                throw AsmJsCompilationException( L"Cannot assign this type to identifier %s", name->Psz() );
            }

            switch( source )
            {
            case Js::AsmJsLookupSource::AsmJsModule:
                if( var->GetVarType().isInt() )
                {
                    CheckNodeLocation( rhsEmit, int );
                    SetModuleInt( var->GetLocation(), rhsEmit.location );
                }
                else if (var->GetVarType().isFloat())
                {
                    CheckNodeLocation(rhsEmit, float);
                    SetModuleFloat(var->GetLocation(), rhsEmit.location);
                }
                else if (var->GetVarType().isDouble())
                {
                    CheckNodeLocation( rhsEmit, double );
                    SetModuleDouble( var->GetLocation(), rhsEmit.location );
                }
#ifdef SIMD_JS_ENABLED
                else if (SIMD_JS_FLAG && var->GetVarType().isSIMD())
                {
                    
                    CheckNodeLocation(rhsEmit, AsmJsSIMDValue);
                    SetModuleSimd(var->GetLocation(), rhsEmit.location, var->GetVarType());
                }
#endif
                else
                {
                    Assert(UNREACHED);
                }
                break;
            case Js::AsmJsLookupSource::AsmJsFunction:
                if( var->GetVarType().isInt() )
                {
                    CheckNodeLocation( rhsEmit, int );
                    mWriter.AsmReg2( Js::OpCodeAsmJs::Ld_Int, var->GetLocation(), rhsEmit.location );
                }
                else if (var->GetVarType().isFloat())
                {
                    CheckNodeLocation(rhsEmit, float);
                    Assert(var->GetVarType().isFloat());
                    mWriter.AsmReg2(Js::OpCodeAsmJs::Ld_Flt, var->GetLocation(), rhsEmit.location);
                }
                else if (var->GetVarType().isDouble())
                {
                    CheckNodeLocation( rhsEmit, double );
                    mWriter.AsmReg2( Js::OpCodeAsmJs::Ld_Db, var->GetLocation(), rhsEmit.location );
                }
#ifdef SIMD_JS_ENABLED
                else if (SIMD_JS_FLAG && var->GetVarType().isSIMD())
                {
                    CheckNodeLocation(rhsEmit, AsmJsSIMDValue);
                    LoadSimd(var->GetLocation(), rhsEmit.location, var->GetVarType());
                }
#endif
                else
                {
                    Assert(UNREACHED);
                }
                break;
            default:
                break;
            }

        }
        else if( lhs->nop == knopIndex )
        {
            ParseNode* arrayNameNode = ParserWrapper::GetBinaryLeft( lhs );
            ParseNode* indexNode = ParserWrapper::GetBinaryRight( lhs );
            if( !ParserWrapper::IsNameDeclaration( arrayNameNode ) )
            {
                throw AsmJsCompilationException( L"Invalid symbol " );
            }

            PropertyName name = arrayNameNode->name();
            AsmJsSymbol* sym = mCompiler->LookupIdentifier(name, mFunction);
            if( !sym || sym->GetSymbolType() != AsmJsSymbol::ArrayView )
            {
                throw AsmJsCompilationException( L"Invalid identifier %s", name->Psz() );
            }
            // must emit index expr first in case it has side effects
            AsmJsArrayView* arrayView = sym->Cast<AsmJsArrayView>();
            ArrayBufferView::ViewType viewType = arrayView->GetViewType();

            OpCodeAsmJs op;
            uint32 indexSlot = 0;
            // if changeHeap is implemented, calls are illegal in index expression and on right hand side of assignments
            bool wasCallLegal = mIsCallLegal;
            mIsCallLegal = !mCompiler->UsesChangeHeap();
            EmitExpressionInfo indexInfo = EmitTypedArrayIndex(indexNode, op, indexSlot, viewType, StoreTypedArray);
            rhsEmit = Emit(rhs);
            mIsCallLegal = wasCallLegal;

            if (viewType == ArrayBufferView::TYPE_FLOAT32)
            {
                if (!rhsEmit.type.isFloatish() && !rhsEmit.type.isMaybeDouble())
                {
                    throw AsmJsCompilationException(L"Cannot assign value to TYPE_FLOAT32 ArrayBuffer");
                }
                // do the conversion to float only for double
                if (rhsEmit.type.isMaybeDouble())
                {
                    CheckNodeLocation(rhsEmit, double);
                    RegSlot dst = mFunction->AcquireTmpRegister<float>();
                    mWriter.AsmReg2(OpCodeAsmJs::Fround_Db, dst, rhsEmit.location);
                    mFunction->ReleaseLocation<double>(&rhsEmit);
                    rhsEmit.location = dst;
                    rhsEmit.type = AsmJsType::Float;
                }                
            }
            else if (viewType == ArrayBufferView::TYPE_FLOAT64)
            {
                if (!rhsEmit.type.isMaybeFloat() && !rhsEmit.type.isMaybeDouble())
                {
                    throw AsmJsCompilationException(L"Cannot assign value to TYPE_FLOAT64 ArrayBuffer");
                }
                // do the conversion to double only for float 
                if (rhsEmit.type.isMaybeFloat())
                {
                    CheckNodeLocation(rhsEmit, float);
                    RegSlot dst = mFunction->AcquireTmpRegister<double>();
                    mWriter.AsmReg2(OpCodeAsmJs::Conv_FTD, dst, rhsEmit.location);
                    mFunction->ReleaseLocation<float>(&rhsEmit);
                    rhsEmit.location = dst;
                    rhsEmit.type = AsmJsType::Double;
                }
            }
            else if (!rhsEmit.type.isSubType(arrayView->GetType()))
            {
                throw AsmJsCompilationException( L"Cannot assign value ArrayBuffer" );
            }

            // REVIEW: this seems hacky, but to keep tmp registers in order, I need to release rhsEmit.local before indexInfo.location
            mWriter.AsmTypedArr(op, rhsEmit.location, indexSlot, viewType);
            RegSlot rhsReg = rhsEmit.location;
            mFunction->ReleaseLocationGeneric(&rhsEmit);
            mFunction->ReleaseLocationGeneric(&indexInfo);
            RegSlot newRhsReg;
            if (rhsEmit.type.isMaybeDouble())
            {
                newRhsReg = mFunction->AcquireTmpRegister<double>();
                mWriter.AsmReg2(OpCodeAsmJs::Ld_Db, newRhsReg, rhsReg);
            }
            else if (rhsEmit.type.isFloatish())
            {
                newRhsReg = mFunction->AcquireTmpRegister<float>();
                mWriter.AsmReg2(OpCodeAsmJs::Ld_Flt, newRhsReg, rhsReg);
            }
            else
            {
                newRhsReg = mFunction->AcquireTmpRegister<int>();
                mWriter.AsmReg2(OpCodeAsmJs::Ld_Int, newRhsReg, rhsReg);
            }
            rhsEmit.location = newRhsReg;


        }
        else
        {
            throw AsmJsCompilationException( L"Can only assign to an identifier or an ArrayBufferView" );
        }
        EndStatement(pnode);
        return rhsEmit;
    }

    EmitExpressionInfo AsmJSByteCodeGenerator::EmitBinaryComparator( ParseNode * pnode, EBinaryComparatorOpCodes op )
    {
        ParseNode* lhs = ParserWrapper::GetBinaryLeft( pnode );
        ParseNode* rhs = ParserWrapper::GetBinaryRight( pnode );
        const EmitExpressionInfo& lhsEmit = Emit( lhs );
        EmitExpressionInfo rhsEmit = Emit( rhs );
        const AsmJsType& lType = lhsEmit.type;
        const AsmJsType& rType = rhsEmit.type;
        StartStatement(pnode);
        EmitExpressionInfo emitInfo(AsmJsType::Int);
        OpCodeAsmJs compOp;
        // Must check for Unsigned first, because isInt() checks for isUnsigned()
        if( lType.isUnsigned() && ( rType.isUnsigned() || rhs->nop == knopFlt ) )
        {
            if( rhs->nop == knopFlt )
            {
                // try to see if that value is actually an unsigned int
                rhsEmit.location = mFunction->GetConstRegister<int>( (int)( (uint32)rhs->sxFlt.dbl ) );
            }
            CheckNodeLocation( lhsEmit, int );
            CheckNodeLocation( rhsEmit, int );
            emitInfo.location = GetAndReleaseBinaryLocations<int>( &lhsEmit, &rhsEmit );
            compOp = BinaryComparatorOpCodes[op][BCOT_UInt];
        }
        else if( lType.isInt() && rType.isInt() )
        {
            CheckNodeLocation( lhsEmit, int );
            CheckNodeLocation( rhsEmit, int );
            emitInfo.location = GetAndReleaseBinaryLocations<int>( &lhsEmit, &rhsEmit );
            compOp = BinaryComparatorOpCodes[op][BCOT_Int];
        }
        else if( lType.isDouble() && rType.isDouble() )
        {
            CheckNodeLocation( lhsEmit, double );
            CheckNodeLocation( rhsEmit, double );
            emitInfo.location = mFunction->AcquireTmpRegister<int>();
            mFunction->ReleaseLocation<double>( &rhsEmit );
            mFunction->ReleaseLocation<double>( &lhsEmit );
            compOp = BinaryComparatorOpCodes[op][BCOT_Double];
        }
        else if (lType.isFloat() && rType.isFloat())
        {
            CheckNodeLocation(lhsEmit, float);
            CheckNodeLocation(rhsEmit, float);
            emitInfo.location = mFunction->AcquireTmpRegister<int>();
            mFunction->ReleaseLocation<float>(&rhsEmit);
            mFunction->ReleaseLocation<float>(&lhsEmit);
            compOp = BinaryComparatorOpCodes[op][BCOT_Float];
        }
        else
        {
            throw AsmJsCompilationException( L"Type not supported for comparison" );
        }
        mWriter.AsmReg3( compOp, emitInfo.location, lhsEmit.location, rhsEmit.location );
        EndStatement(pnode);
        return emitInfo;
    }

    EmitExpressionInfo AsmJSByteCodeGenerator::EmitUnaryPos( ParseNode * pnode )
    {
        ParseNode* rhs = ParserWrapper::GetUnaryNode( pnode );
        EmitExpressionInfo rhsEmit ;
        if (rhs->nop == knopCall)
        {
            rhsEmit = EmitCall(rhs, AsmJsRetType::Double);
        }
        else
        {
            rhsEmit = Emit(rhs);
        }
        const AsmJsType& rType = rhsEmit.type;
        StartStatement(pnode);
        EmitExpressionInfo emitInfo( AsmJsType::Double );
        RegSlot dst;
        if( rType.isUnsigned() )
        {
            CheckNodeLocation( rhsEmit, int );
            dst = mFunction->AcquireTmpRegister<double>();
            mWriter.AsmReg2( OpCodeAsmJs::Conv_UTD, dst, rhsEmit.location );
            mFunction->ReleaseLocation<int>( &rhsEmit );
        }
        else if( rType.isSigned() )
        {
            CheckNodeLocation( rhsEmit, int );
            dst = mFunction->AcquireTmpRegister<double>();
            mWriter.AsmReg2( OpCodeAsmJs::Conv_ITD, dst, rhsEmit.location );
            mFunction->ReleaseLocation<int>( &rhsEmit );
        }
        else if (rType.isMaybeDouble())
        {
            CheckNodeLocation( rhsEmit, double );
            dst = rhsEmit.location;
        }
        else if (rType.isMaybeFloat())
        {
            CheckNodeLocation(rhsEmit, float);
            dst = mFunction->AcquireTmpRegister<double>();
            mWriter.AsmReg2(OpCodeAsmJs::Conv_FTD, dst, rhsEmit.location);
            mFunction->ReleaseLocation<float>(&rhsEmit);
        }
        else
        {
            throw AsmJsCompilationException( L"Type not supported for unary +" );
        }
        emitInfo.location = dst;
        EndStatement(pnode);
        return emitInfo;
    }

    Js::EmitExpressionInfo AsmJSByteCodeGenerator::EmitUnaryNeg( ParseNode * pnode )
    {
        ParseNode* rhs = ParserWrapper::GetUnaryNode( pnode );
        const EmitExpressionInfo& rhsEmit = Emit( rhs );
        const AsmJsType& rType = rhsEmit.type;
        StartStatement(pnode);
        EmitExpressionInfo emitInfo;
        if( rType.isInt() )
        {
            CheckNodeLocation( rhsEmit, int );
            RegSlot dst = GetAndReleaseUnaryLocations<int>( &rhsEmit );
            emitInfo.type = AsmJsType::Intish;
            mWriter.AsmReg2( OpCodeAsmJs::Neg_Int, dst, rhsEmit.location );
            emitInfo.location = dst;
        }
        else if (rType.isMaybeDouble())
        {
            CheckNodeLocation( rhsEmit, double );
            RegSlot dst = GetAndReleaseUnaryLocations<double>( &rhsEmit );
            emitInfo.type = AsmJsType::Double;
            mWriter.AsmReg2( OpCodeAsmJs::Neg_Db, dst, rhsEmit.location );
            emitInfo.location = dst;
        }
        else if (rType.isMaybeFloat())
        {
            CheckNodeLocation(rhsEmit, float);
            RegSlot dst = GetAndReleaseUnaryLocations<float>(&rhsEmit);
            emitInfo.type = AsmJsType::Floatish;
            mWriter.AsmReg2(OpCodeAsmJs::Neg_Flt, dst, rhsEmit.location);
            emitInfo.location = dst;
        }
        else
        {
            throw AsmJsCompilationException( L"Type not supported for unary -" );
        }
        EndStatement(pnode);
        return emitInfo;
    }

    Js::EmitExpressionInfo AsmJSByteCodeGenerator::EmitUnaryNot( ParseNode * pnode )
    {
        ParseNode* rhs = ParserWrapper::GetUnaryNode( pnode );
        int count = 1;
        while( rhs->nop == knopNot )
        {
            ++count;
            rhs = ParserWrapper::GetUnaryNode( rhs );
        }
        EmitExpressionInfo rhsEmit = Emit( rhs );
        AsmJsType rType = rhsEmit.type;
        StartStatement(pnode);
        if( count >= 2 && rType.isMaybeDouble() )
        {
            CheckNodeLocation( rhsEmit, double );
            count -= 2;
            RegSlot dst = mFunction->AcquireTmpRegister<int>();
            mWriter.AsmReg2( OpCodeAsmJs::Conv_DTI, dst, rhsEmit.location );
            mFunction->ReleaseLocation<double>( &rhsEmit );
            
            // allow the converted value to be negated (useful for   ~(~~(+x)) )
            rType = AsmJsType::Signed;
            rhsEmit.location = dst;
        }
        if (count >= 2 && rType.isMaybeFloat())
        {
            CheckNodeLocation(rhsEmit, float);
            count -= 2;
            RegSlot dst = mFunction->AcquireTmpRegister<int>();
            mWriter.AsmReg2(OpCodeAsmJs::Conv_FTI, dst, rhsEmit.location);
            mFunction->ReleaseLocation<float>(&rhsEmit);

            // allow the converted value to be negated (useful for   ~(~~(fround(x))) )
            rType = AsmJsType::Signed;
            rhsEmit.location = dst;
        }
        if( rType.isIntish() )
        {
            if( count & 1 )
            {
                CheckNodeLocation( rhsEmit, int );
                RegSlot dst = GetAndReleaseUnaryLocations<int>( &rhsEmit );
                // do the conversion only if we have an odd number of the operator
                mWriter.AsmReg2( OpCodeAsmJs::Not_Int, dst, rhsEmit.location );
                rhsEmit.location = dst;
            }
            rhsEmit.type = AsmJsType::Signed;
        }
        else
        {
            throw AsmJsCompilationException( L"Type not supported for unary ~" );
        }
        EndStatement(pnode);
        return rhsEmit;
    }

    Js::EmitExpressionInfo AsmJSByteCodeGenerator::EmitUnaryLogNot( ParseNode * pnode )
    {
        ParseNode* rhs = ParserWrapper::GetUnaryNode( pnode );
        int count = 1;
        while( rhs->nop == knopLogNot )
        {
            ++count;
            rhs = ParserWrapper::GetUnaryNode( rhs );
        }

        const EmitExpressionInfo& rhsEmit = Emit( rhs );
        const AsmJsType& rType = rhsEmit.type;
        StartStatement(pnode);
        EmitExpressionInfo emitInfo( AsmJsType::Signed );
        if( rType.isInt() )
        {
            CheckNodeLocation( rhsEmit, int );
            RegSlot dst = GetAndReleaseUnaryLocations<int>( &rhsEmit );
            if( count & 1 )
            {
                // do the conversion only if we have an odd number of the operator
                mWriter.AsmReg2( OpCodeAsmJs::LogNot_Int, dst, rhsEmit.location );
            }
            else
            {
                // otherwise, make sure the result is 0|1
                mWriter.AsmReg2( OpCodeAsmJs::Conv_ITB, dst, rhsEmit.location );
            }
            emitInfo.location = dst;
        }
        else
        {
            throw AsmJsCompilationException( L"Type not supported for unary !" );
        }
        EndStatement(pnode);
        return emitInfo;
    }


    EmitExpressionInfo AsmJSByteCodeGenerator::EmitBooleanExpression( ParseNode* expr, Js::ByteCodeLabel trueLabel, Js::ByteCodeLabel falseLabel )
    {
        switch( expr->nop )
        {
        case knopLogNot:{
            const EmitExpressionInfo& info = EmitBooleanExpression( expr->sxUni.pnode1, falseLabel, trueLabel );
            return info;
            break;
        }
//         case knopEq:
//         case knopNe:
//         case knopLt:
//         case knopLe:
//         case knopGe:
//         case knopGt:
//             byteCodeGenerator->StartStatement( expr );
//             EmitBinaryOpnds( expr->sxBin.pnode1, expr->sxBin.pnode2, byteCodeGenerator, funcInfo );
//             funcInfo->ReleaseLoc( expr->sxBin.pnode2 );
//             funcInfo->ReleaseLoc( expr->sxBin.pnode1 );
//             mWriter.BrReg2( nopToOp[expr->nop], trueLabel, expr->sxBin.pnode1->location,
//                                                  expr->sxBin.pnode2->location );
//             mWriter.AsmBr( falseLabel );
//             byteCodeGenerator->EndStatement( expr );
//             break;
//         case knopName:
//             byteCodeGenerator->StartStatement( expr );
//             Emit( expr, byteCodeGenerator, funcInfo, false );
//             mWriter.BrReg1( Js::OpCode::BrTrue_A, trueLabel, expr->location );
//             mWriter.AsmBr( falseLabel );
//             byteCodeGenerator->EndStatement( expr );
//             break;
        default:{
            const EmitExpressionInfo& info = Emit( expr );
            if( !info.type.isInt() )
            {
                throw AsmJsCompilationException( L"Comparison expressions must be type signed" );
            }
            mWriter.AsmBrReg1( Js::OpCodeAsmJs::BrTrue_Int, trueLabel, info.location );
            mWriter.AsmBr( falseLabel );
            return info;
            break;
            }
        }
    }

    EmitExpressionInfo AsmJSByteCodeGenerator::EmitIf( ParseNode * pnode )
    {
        Js::ByteCodeLabel trueLabel = mWriter.DefineLabel();
        Js::ByteCodeLabel falseLabel = mWriter.DefineLabel();
        const EmitExpressionInfo& boolInfo = EmitBooleanExpression( pnode->sxIf.pnodeCond, trueLabel, falseLabel );
        mFunction->ReleaseLocation<int>( &boolInfo );


        mWriter.MarkAsmJsLabel( trueLabel );

        const EmitExpressionInfo& trueInfo = Emit( pnode->sxIf.pnodeTrue );
        mFunction->ReleaseLocationGeneric( &trueInfo );

        if( pnode->sxIf.pnodeFalse != nullptr )
        {
            // has else clause
            Js::ByteCodeLabel skipLabel = mWriter.DefineLabel();

            // Record the branch bytecode offset
            mWriter.RecordStatementAdjustment( Js::FunctionBody::SAT_FromCurrentToNext );

            // then clause skips else clause
            mWriter.AsmBr( skipLabel );
            // generate code for else clause
            mWriter.MarkAsmJsLabel( falseLabel );

            const EmitExpressionInfo& falseInfo = Emit( pnode->sxIf.pnodeFalse );
            mFunction->ReleaseLocationGeneric( &falseInfo );

            mWriter.MarkAsmJsLabel( skipLabel );

        }
        else
        {
            mWriter.MarkAsmJsLabel( falseLabel );
        }
        if( pnode->emitLabels )
        {
            mWriter.MarkAsmJsLabel( pnode->sxStmt.breakLabel );
        }
        return EmitExpressionInfo( AsmJsType::Void );
    }

    Js::EmitExpressionInfo AsmJSByteCodeGenerator::EmitLoop( ParseNode *loopNode, ParseNode *cond, ParseNode *body, ParseNode *incr, BOOL doWhile /*= false */ )
    {
        // Need to increment loop count whether we are going to profile or not for HasLoop()
        StartStatement(loopNode);
        Js::ByteCodeLabel loopEntrance = mWriter.DefineLabel();
        Js::ByteCodeLabel continuePastLoop = mWriter.DefineLabel();

        uint loopId = mWriter.EnterLoop( loopEntrance );
        loopNode->sxLoop.loopId = loopId;
        EndStatement(loopNode);
        if( doWhile )
        {
            const EmitExpressionInfo& bodyInfo = Emit( body );
            mFunction->ReleaseLocationGeneric( &bodyInfo );

            if( loopNode->emitLabels )
            {
                mWriter.MarkAsmJsLabel( loopNode->sxStmt.continueLabel );
            }
            if( !ByteCodeGenerator::IsFalse( cond ) )
            {
                const EmitExpressionInfo& condInfo = EmitBooleanExpression( cond, loopEntrance, continuePastLoop );
                mFunction->ReleaseLocationGeneric( &condInfo );
            }
        }
        else
        {
            if( cond )
            {
                Js::ByteCodeLabel trueLabel = mWriter.DefineLabel();
                const EmitExpressionInfo& condInfo = EmitBooleanExpression( cond, trueLabel, continuePastLoop );
                mFunction->ReleaseLocationGeneric( &condInfo );
                mWriter.MarkAsmJsLabel( trueLabel );
            }
            const EmitExpressionInfo& bodyInfo = Emit( body );
            mFunction->ReleaseLocationGeneric( &bodyInfo );

            if( loopNode->emitLabels )
            {
                mWriter.MarkAsmJsLabel( loopNode->sxStmt.continueLabel );
            }
            if( incr != NULL )
            {
                const EmitExpressionInfo& incrInfo = Emit( incr );
                mFunction->ReleaseLocationGeneric( &incrInfo );
            }
            mWriter.AsmBr( loopEntrance );
        }
        mWriter.MarkAsmJsLabel( continuePastLoop );
        if( loopNode->emitLabels )
        {
            mWriter.MarkAsmJsLabel( loopNode->sxStmt.breakLabel );
        }

        mWriter.ExitLoop( loopId );



        return EmitExpressionInfo( AsmJsType::Void );
    }

    EmitExpressionInfo AsmJSByteCodeGenerator::EmitQMark( ParseNode * pnode )
    {
        StartStatement(pnode->sxTri.pnode1);
        Js::ByteCodeLabel trueLabel = mWriter.DefineLabel();
        Js::ByteCodeLabel falseLabel = mWriter.DefineLabel();
        Js::ByteCodeLabel skipLabel = mWriter.DefineLabel();
        EndStatement(pnode->sxTri.pnode1);
        const EmitExpressionInfo& boolInfo = EmitBooleanExpression( pnode->sxTri.pnode1, trueLabel, falseLabel );
        mFunction->ReleaseLocationGeneric( &boolInfo );

        RegSlot intReg = mFunction->AcquireTmpRegister<int>();
        RegSlot doubleReg = mFunction->AcquireTmpRegister<double>();
        RegSlot floatReg = mFunction->AcquireTmpRegister<float>();
        EmitExpressionInfo emitInfo( AsmJsType::Void );


        mWriter.MarkAsmJsLabel( trueLabel );
        const EmitExpressionInfo& trueInfo = Emit( pnode->sxTri.pnode2 );
        StartStatement(pnode->sxTri.pnode2);
        if( trueInfo.type.isInt() )
        {
            mWriter.AsmReg2( Js::OpCodeAsmJs::Ld_Int, intReg, trueInfo.location );
            mFunction->ReleaseLocation<int>( &trueInfo );
            mFunction->ReleaseTmpRegister<double>(doubleReg);
            mFunction->ReleaseTmpRegister<float>(floatReg);
            emitInfo.location = intReg;
            emitInfo.type = AsmJsType::Int;
        }
        else if( trueInfo.type.isDouble() )
        {
            mWriter.AsmReg2( Js::OpCodeAsmJs::Ld_Db, doubleReg, trueInfo.location );
            mFunction->ReleaseLocation<double>( &trueInfo );
            mFunction->ReleaseTmpRegister<int>( intReg );
            mFunction->ReleaseTmpRegister<float>(floatReg);
            emitInfo.location = doubleReg;
            emitInfo.type = AsmJsType::Double;
        }
        else
        {
            Assert(trueInfo.type.isFloat());
            mWriter.AsmReg2(Js::OpCodeAsmJs::Ld_Flt, floatReg, trueInfo.location);
            mFunction->ReleaseLocation<float>(&trueInfo);
            mFunction->ReleaseTmpRegister<int>(intReg);
            mFunction->ReleaseTmpRegister<double>(doubleReg);
            emitInfo.location = floatReg;
            emitInfo.type = AsmJsType::Float;
        }
        mWriter.AsmBr( skipLabel );
        EndStatement(pnode->sxTri.pnode2);
        mWriter.MarkAsmJsLabel( falseLabel );
        const EmitExpressionInfo& falseInfo = Emit( pnode->sxTri.pnode3 );
        StartStatement(pnode->sxTri.pnode3);
        if( falseInfo.type.isInt() )
        {
            if( !trueInfo.type.isInt() )
            {
                throw AsmJsCompilationException( L"Conditionnal expressions results must be the same type" );
            }
            mWriter.AsmReg2( Js::OpCodeAsmJs::Ld_Int, intReg, falseInfo.location );
            mFunction->ReleaseLocation<int>( &falseInfo );
        }
        else if( falseInfo.type.isDouble() )
        {
            if( !trueInfo.type.isDouble() )
            {
                throw AsmJsCompilationException( L"Conditionnal expressions results must be the same type" );
            }
            mWriter.AsmReg2( Js::OpCodeAsmJs::Ld_Db, doubleReg, falseInfo.location );
            mFunction->ReleaseLocation<double>( &falseInfo );
        }
        else
        {
            Assert(falseInfo.type.isFloat());
            if (!trueInfo.type.isFloat())
            {
                throw AsmJsCompilationException(L"Conditionnal expressions results must be the same type");
            }
            mWriter.AsmReg2(Js::OpCodeAsmJs::Ld_Flt, floatReg, falseInfo.location);
            mFunction->ReleaseLocation<float>(&falseInfo);
        }
        mWriter.MarkAsmJsLabel( skipLabel );
        EndStatement(pnode->sxTri.pnode3);
        return emitInfo;
    }

    EmitExpressionInfo AsmJSByteCodeGenerator::EmitSwitch( ParseNode * pnode )
    {
        BOOL fHasDefault = false;
        Assert( pnode->sxSwitch.pnodeVal != NULL );
        const EmitExpressionInfo& valInfo = Emit( pnode->sxSwitch.pnodeVal );

        if( !valInfo.type.isSigned() )
        {
            throw AsmJsCompilationException( L"Switch value must be type Signed, FixNum" );
        }

        RegSlot regVal = GetAndReleaseUnaryLocations<int>( &valInfo );
        StartStatement(pnode);
        mWriter.AsmReg2( OpCodeAsmJs::Ld_Int, regVal, valInfo.location );
        EndStatement(pnode);

        // TODO: if all cases are compile-time constants, emit a switch statement in the byte
        // code so the BE can optimize it.

        ParseNode *pnodeCase;
        for( pnodeCase = pnode->sxSwitch.pnodeCases; pnodeCase; pnodeCase = pnodeCase->sxCase.pnodeNext )
        {
            // Jump to the first case body if this one doesn't match. Make sure any side-effects of the case
            // expression take place regardless.
            pnodeCase->sxCase.labelCase = mWriter.DefineLabel();
            if( pnodeCase == pnode->sxSwitch.pnodeDefault )
            {
                fHasDefault = true;
                continue;
            }
            ParseNode* caseExpr = pnodeCase->sxCase.pnodeExpr;
            if ((caseExpr->nop != knopInt || (caseExpr->sxInt.lw >> 31) > 1) && !ParserWrapper::IsMinInt(caseExpr))
            {
                throw AsmJsCompilationException( L"Switch case value must be int in the range [-2^31, 2^31)" );
            }

            const EmitExpressionInfo& caseExprInfo = Emit( pnodeCase->sxCase.pnodeExpr );
            mWriter.AsmBrReg2( OpCodeAsmJs::BrEq_Int, pnodeCase->sxCase.labelCase, regVal, caseExprInfo.location );
            // do not need to release location because int constants cannot be released
        }

        // No explicit case value matches. Jump to the default arm (if any) or break out altogether.
        if( fHasDefault )
        {
            mWriter.AsmBr( pnode->sxSwitch.pnodeDefault->sxCase.labelCase );
        }
        else
        {
            if( !pnode->emitLabels )
            {
                pnode->sxStmt.breakLabel = mWriter.DefineLabel();
            }
            mWriter.AsmBr( pnode->sxStmt.breakLabel );
        }
        // Now emit the case arms to which we jump on matching a case value.
        for( pnodeCase = pnode->sxSwitch.pnodeCases; pnodeCase; pnodeCase = pnodeCase->sxCase.pnodeNext )
        {
            mWriter.MarkAsmJsLabel( pnodeCase->sxCase.labelCase );
            const EmitExpressionInfo& caseBodyInfo = Emit( pnodeCase->sxCase.pnodeBody );
            mFunction->ReleaseLocationGeneric( &caseBodyInfo );
        }

        mFunction->ReleaseTmpRegister<int>( regVal );

        if( !fHasDefault || pnode->emitLabels )
        {
            mWriter.MarkAsmJsLabel( pnode->sxStmt.breakLabel );
        }

        return EmitExpressionInfo( AsmJsType::Void );
    }

    void AsmJSByteCodeGenerator::EmitEmptyByteCode(FuncInfo * funcInfo, ByteCodeGenerator * byteCodeGen)
    {
        funcInfo->byteCodeFunction->SetGrfscr(byteCodeGen->GetFlags());
        funcInfo->byteCodeFunction->SetSourceInfo(byteCodeGen->GetCurrentSourceIndex(),
            funcInfo->root,
            !!(byteCodeGen->GetFlags() & fscrEvalCode),
            ((byteCodeGen->GetFlags() & fscrDynamicCode) && !(byteCodeGen->GetFlags() & fscrEvalCode)));

        FunctionBody * functionBody = funcInfo->byteCodeFunction->GetFunctionBody();

        class AutoCleanup
        {
        private:
            FunctionBody * mFunctionBody;
            ByteCodeGenerator * mByteCodeGen;
        public:
            AutoCleanup(FunctionBody * functionBody, ByteCodeGenerator * byteCodeGen) : mFunctionBody(functionBody), mByteCodeGen(byteCodeGen)
            {
            }

            void Done()
            {
                mFunctionBody = nullptr;
            }
            ~AutoCleanup()
            {
                if (mFunctionBody)
                {
                    mFunctionBody->ResetByteCodeGenState();
                    mByteCodeGen->Writer()->Reset();
                }
            }
        } autoCleanup(functionBody, byteCodeGen);

        byteCodeGen->Writer()->Begin(byteCodeGen, functionBody, byteCodeGen->GetAllocator(), false, false);
        byteCodeGen->Writer()->End();

        autoCleanup.Done();
    }

    void AsmJSByteCodeGenerator::StartStatement(ParseNode* pnode)
    {
        mWriter.StartStatement(pnode, 0);
        //         Output::Print( L"%*s+%d\n",tab, " ", pnode->ichMin ); 
        //         ++tab;
    }

    void AsmJSByteCodeGenerator::EndStatement(ParseNode* pnode)
    {
        mWriter.EndStatement(pnode);
        //         Output::Print( L"%*s-%d\n",tab, " ", pnode->ichMin );
        //         --tab;
    }



   // int tab = 0;
    void AsmJSByteCodeGenerator::LoadModuleInt( RegSlot dst, RegSlot index )
    {
        mWriter.AsmSlot(OpCodeAsmJs::LdSlot_Int, dst, AsmJsFunctionMemory::ModuleEnvRegister, index + (int32)(mCompiler->GetIntOffset() / INT_SLOTS_SPACE + 0.5));
    }
    void AsmJSByteCodeGenerator::LoadModuleFloat(RegSlot dst, RegSlot index)
    {
        mWriter.AsmSlot(OpCodeAsmJs::LdSlot_Flt, dst, AsmJsFunctionMemory::ModuleEnvRegister, index + (int32)(mCompiler->GetFloatOffset() / FLOAT_SLOTS_SPACE + 0.5));
    }
    void AsmJSByteCodeGenerator::LoadModuleDouble( RegSlot dst, RegSlot index )
    {
        mWriter.AsmSlot(OpCodeAsmJs::LdSlot_Db, dst, AsmJsFunctionMemory::ModuleEnvRegister, index + mCompiler->GetDoubleOffset() / DOUBLE_SLOTS_SPACE);
    }

    void AsmJSByteCodeGenerator::LoadModuleFFI( RegSlot dst, RegSlot index )
    {
        mWriter.AsmSlot(OpCodeAsmJs::LdSlot, dst, AsmJsFunctionMemory::ModuleEnvRegister, index + mCompiler->GetFFIOffset());
    }

    void AsmJSByteCodeGenerator::LoadModuleFunction( RegSlot dst, RegSlot index )
    {
        mWriter.AsmSlot(OpCodeAsmJs::LdSlot, dst, AsmJsFunctionMemory::ModuleEnvRegister, index + mCompiler->GetFuncOffset());
    }

    void AsmJSByteCodeGenerator::LoadModuleFunctionTable( RegSlot dst, RegSlot FuncTableIndex, RegSlot FuncIndexLocation )
    {
        mWriter.AsmSlot( OpCodeAsmJs::LdSlotArr, AsmJsFunctionMemory::ModuleSlotRegister, AsmJsFunctionMemory::ModuleEnvRegister, FuncTableIndex+mCompiler->GetFuncPtrOffset() );
        mWriter.AsmSlot( OpCodeAsmJs::LdArr_Func, dst, AsmJsFunctionMemory::ModuleSlotRegister, FuncIndexLocation );
    }

    void AsmJSByteCodeGenerator::SetModuleInt( Js::RegSlot dst, RegSlot src )
    {
        mWriter.AsmSlot(OpCodeAsmJs::StSlot_Int, src, AsmJsFunctionMemory::ModuleEnvRegister, dst + (int32)(mCompiler->GetIntOffset() / INT_SLOTS_SPACE + 0.5));
    }

    void AsmJSByteCodeGenerator::SetModuleFloat(Js::RegSlot dst, RegSlot src)
    {
        mWriter.AsmSlot(OpCodeAsmJs::StSlot_Flt, src, AsmJsFunctionMemory::ModuleEnvRegister, dst + (int32)(mCompiler->GetFloatOffset() / FLOAT_SLOTS_SPACE + 0.5));
    }

    void AsmJSByteCodeGenerator::SetModuleDouble( Js::RegSlot dst, RegSlot src )
    {
        mWriter.AsmSlot(OpCodeAsmJs::StSlot_Db, src, AsmJsFunctionMemory::ModuleEnvRegister, dst + mCompiler->GetDoubleOffset() / DOUBLE_SLOTS_SPACE);
    }


#ifdef SIMD_JS_ENABLED
    void AsmJSByteCodeGenerator::LoadModuleSimd(RegSlot dst, RegSlot index, AsmJsVarType type)
    {
        switch (type.which())
        {
            case AsmJsVarType::Int32x4:
                mWriter.AsmSlot(OpCodeAsmJs::Simd128_LdSlot_I4, dst, AsmJsFunctionMemory::ModuleEnvRegister, index + mCompiler->GetSimdOffset());
                break;
            case AsmJsVarType::Float32x4:
                mWriter.AsmSlot(OpCodeAsmJs::Simd128_LdSlot_F4, dst, AsmJsFunctionMemory::ModuleEnvRegister, index + mCompiler->GetSimdOffset());
                break;
            case AsmJsVarType::Float64x2:
                mWriter.AsmSlot(OpCodeAsmJs::Simd128_LdSlot_D2, dst, AsmJsFunctionMemory::ModuleEnvRegister, index + mCompiler->GetSimdOffset());
                break;
            default:
                Assert(UNREACHED);
        }
    }

    void AsmJSByteCodeGenerator::SetModuleSimd(RegSlot index, RegSlot src, AsmJsVarType type)
    {
        switch (type.which())
        {
        case AsmJsVarType::Int32x4:
            mWriter.AsmSlot(OpCodeAsmJs::Simd128_StSlot_I4, src, AsmJsFunctionMemory::ModuleEnvRegister, index + mCompiler->GetSimdOffset());
            break;
        case AsmJsVarType::Float32x4:
            mWriter.AsmSlot(OpCodeAsmJs::Simd128_StSlot_F4, src, AsmJsFunctionMemory::ModuleEnvRegister, index + mCompiler->GetSimdOffset());
            break;
        case AsmJsVarType::Float64x2:
            mWriter.AsmSlot(OpCodeAsmJs::Simd128_StSlot_D2, src, AsmJsFunctionMemory::ModuleEnvRegister, index + mCompiler->GetSimdOffset());
            break;
        default:
            Assert(UNREACHED);
        }
    }

    void AsmJSByteCodeGenerator::LoadSimd(RegSlot dst, RegSlot src, AsmJsVarType type)
    {
        switch (type.which())
        {
        case AsmJsVarType::Int32x4:
            
            mWriter.AsmReg2(OpCodeAsmJs::Simd128_Ld_I4, dst, src);
            break;
        case AsmJsVarType::Float32x4:
            mWriter.AsmReg2(OpCodeAsmJs::Simd128_Ld_F4, dst, src);
            break;
        case AsmJsVarType::Float64x2:
            mWriter.AsmReg2(OpCodeAsmJs::Simd128_Ld_D2, dst, src);
            break;
        default:
            Assert(UNREACHED);

        }

    }
#endif

    void AsmJsFunctionCompilation::CleanUp()
    {
        if( mGenerator && mGenerator->mInfo )
        {
            FunctionBody* body = mGenerator->mFunction->GetFuncBody();
            if( body )
            {
                body->ResetByteCodeGenState();
            }
            mGenerator->mWriter.Reset();
        }
    }

}
