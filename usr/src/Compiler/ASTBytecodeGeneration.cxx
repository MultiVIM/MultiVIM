#include <cassert>

#include "AST/LiteralExpr.hxx"
#include "VM/Bytecode.hxx"

int ExprNode::generateOptimized (GenerationContext * aContext, int instruction,
                                 bool doPop)
{
    int location;

    aContext->genInstruction (DoSpecial, instruction);
    location = aContext->codeTop ();

    aContext->genCode (0);
    if (doPop)
        aContext->genInstruction (DoSpecial, PopTop);

    this->generateInContext (aContext);
    aContext->genMessage (false, 0, "value");

    aContext->setCodeTo (location, aContext->codeTop () + 1);
    // codeArray[location] = codeTop + 1;
    return (location);
}

/*
 * Literal expressions
 */

void CharExprNode::generateInContext (GenerationContext * aContext)
{
    int litNum = aContext->genLiteral ((objRef)newChar (khar[0]));
    if (!aContext->inLiteralArray ())
        aContext->genInstruction (PushLiteral, litNum);
}

void SymbolExprNode::generateInContext (GenerationContext * aContext)
{
    int litNum = aContext->genLiteral ((objRef)newSymbol (sym.c_str ()));
    if (!aContext->inLiteralArray ())
        aContext->genInstruction (PushLiteral, litNum);
}

void StringExprNode::generateInContext (GenerationContext * aContext)
{
    int litNum = aContext->genLiteral ((objRef)newString (str.c_str ()));
    if (!aContext->inLiteralArray ())
        aContext->genInstruction (PushLiteral, litNum);
}

void IntExprNode::generateInContext (GenerationContext * aContext)
{
    if (!aContext->inLiteralArray ())
        aContext->genInteger (num);
    else
        aContext->genLiteral ((objRef)encValueOf (num));
}

void FloatExprNode::generateInContext (GenerationContext * aContext)
{
    int litNum = aContext->genLiteral ((objRef)newFloat (num));
    if (!aContext->inLiteralArray ())
        aContext->genInstruction (PushLiteral, litNum);
}

void ArrayExprNode::generateInContext (GenerationContext * aContext)
{
    int litNum;

    aContext->beginLiteralArray ();
    for (auto el : elements)
        el->generateInContext (aContext);
    litNum = aContext->endLiteralArray ();

    if (!aContext->inLiteralArray ())
        aContext->genInstruction (PushLiteral, litNum);
}

/* Other expressions */
void PrimitiveExprNode::generateInContext (GenerationContext * aContext)
{
    printf ("generate prim %d<--\n", num);
    for (auto a : args)
        a->generateInContext (aContext);
    aContext->genInstruction (DoPrimitive, args.size ());
    aContext->genCode (num);
    printf ("-->generated prim %d\n", num);
}

void IdentExprNode::generateInContext (GenerationContext * aContext)
{
    VarNode * var = aContext->lookup (id);

    if ((id == "self") || (id == "super"))
    {
        aContext->genInstruction (PushArgument, 0);
        return;
    }

    if (!var)
        printf ("DID NOT RESOLVE %s!!\n", id.c_str ());

    if (var)
        switch (var->kind)
        {
        case VarNode::kLocal:
            printf ("Local %s is a LOCAL TEMPORARY\n", id.c_str ());
            aContext->genInstruction (PushTemporary, var->getIndex () - 1);
            return;

        case VarNode::kArgument:
            aContext->genInstruction (PushArgument, var->getIndex ());
            return;

        case VarNode::kInstance:
            aContext->genInstruction (PushInstance, var->getIndex ());
            return;

        case VarNode::kGlobalConstant:
        case VarNode::kGlobal:
            break;

        default:
            printf ("Unknown variable type %d\n\n!", var->kind);
            exit (0);
        }

    for (int i = 0; glbsyms[i]; i++)
        if (id == glbsyms[i])
        {
            aContext->genInstruction (PushConstant, i + 4);
            return;
        }

    /* not anything else, it must be a global */
    /* must look it up at run time */
    aContext->genInstruction (
        PushLiteral, aContext->genLiteral ((objRef)newSymbol (id.c_str ())));
    aContext->genMessage (false, 0, "value");
}

void IdentExprNode::generateAssignInContext (GenerationContext * aContext,
                                             ExprNode * rValue)
{
    VarNode * var = aContext->lookup (id);

    if (var)
    {
        switch (var->kind)
        {
        case VarNode::kLocal:
            rValue->generateInContext (aContext);
            aContext->genInstruction (AssignTemporary, var->getIndex () - 1);
            return;

        case VarNode::kInstance:
            rValue->generateInContext (aContext);
            aContext->genInstruction (AssignInstance, var->getIndex ());
            return;
        }
    }

    aContext->genInstruction (PushArgument, 0);
    aContext->genInstruction (
        PushLiteral, aContext->genLiteral ((objRef)newSymbol (id.c_str ())));
    rValue->generateInContext (aContext);
    aContext->genMessage (false, 2, "assign:value:");
}

void MessageExprNode::generateInContext (GenerationContext * aContext,
                                         bool cascade)
{
    if (!cascade)
        receiver->generateInContext (aContext);

    if (selector == "ifTrue:ifFalse:")
    {
        BlockExprNode * trueBlock = dynamic_cast<BlockExprNode *> (args[0]);
        BlockExprNode * falseBlock = dynamic_cast<BlockExprNode *> (args[1]);
    }

    /*       if (streq (tokenString, "ifTrue:"))
           {
               i = optimizeBlock (BranchIfFalse, false);
               if (streq (tokenString, "ifFalse:"))
               {
                   codeArray[i] = codeTop + 3;
                   (void)optimizeBlock (Branch, true);
               }
           }
           else if (streq (tokenString, "ifFalse:"))
           {
               i = optimizeBlock (BranchIfTrue, false);
               if (streq (tokenString, "ifTrue:"))
               {
                   codeArray[i] = codeTop + 3;
                   (void)optimizeBlock (Branch, true);
               }
           }
           else if (streq (tokenString, "whileTrue:"))
           {
               j = codeTop;
               genInstruction (DoSpecial, Duplicate);
               genMessage (false, 0, newSymbol ("value"));
               i = optimizeBlock (BranchIfFalse, false);
               genInstruction (DoSpecial, PopTop);
               genInstruction (DoSpecial, Branch);
               genCode (j + 1);
               codeArray[i] = codeTop + 1;
               genInstruction (DoSpecial, PopTop);
           }
           else if (streq (tokenString, "and:"))
               (void)optimizeBlock (AndBranch, false);
           else if (streq (tokenString, "or:"))
               (void)optimizeBlock (OrBranch, false);*/

    for (auto a : args)
        a->generateInContext (aContext);

    aContext->genMessage (receiver->isSuper (), args.size (), selector);
}

void AssignExprNode::generateInContext (GenerationContext * aContext)
{
    left->generateAssignInContext (aContext, right);
}

void CascadeExprNode::generateInContext (GenerationContext * aContext)
{
    receiver->generateInContext (aContext);

    messages.front ()->generateInContext (aContext, true);

    aContext->genInstruction (DoSpecial, Duplicate);

    for (auto it = ++std::begin (messages); it != std::end (messages); it++)
    {
        (*it)->generateInContext (aContext, true);
        aContext->genInstruction (DoSpecial, PopTop);
        if (*it != messages.back ())
            aContext->genInstruction (DoSpecial, Duplicate);
    }
}

void BlockExprNode::generateInContext (GenerationContext * aContext)
{
    int oldTop = aContext->localTop ();
    int i = oldTop;
    int fixLoc;

    for (auto l : args)
    {
        aContext->addLocal (++i, l);
    }

    fixLoc = aContext->pushBlock (args.size (), oldTop);

    for (auto & s : stmts)
    {
        s->generateInContext (aContext);
        if (&s != &stmts.back ())
            aContext->genInstruction (DoSpecial, PopTop);
    }

    if (stmts.empty ())
        aContext->genInstruction (PushConstant, nilConst);

    aContext->popBlock (fixLoc, oldTop);
}

void ReturnStmtNode::generateInContext (GenerationContext * aContext)
{
    expr->generateInContext (aContext);
    if (aContext->inBlock ())
    {
        /* change return point before returning */
        aContext->genInstruction (PushConstant, contextConst);
        aContext->genMessage (false, 0, "blockReturn");
        aContext->genInstruction (DoSpecial, PopTop);
    }
    aContext->genInstruction (DoSpecial, StackReturn);
}

encPtr MethodNode::generateInContext (bool isClassMethod,
                                      GenerationContext * aContext)
{
    int index = 1;
    encPtr bytecodes, theLiterals;
    encPtr method = newMethod ();
    byte * bp;
    std::pair<std::vector<byte>, std::vector<objRef>> result;
    aContext->pushScope (new MethodScope);
    aContext->beginMethod ();

    printf ("METHOD GENERATING IN CONTEXT...\n\n");

    for (auto i : args)
        aContext->addArg (index++, i);

    index = 1;
    for (auto i : locals)
        aContext->addLocal (index++, i);

    for (auto & s : stmts)
    {
        s->generateInContext (aContext);
        if (&s != &stmts.back ())
            aContext->genInstruction (DoSpecial, PopTop);
    }

    aContext->genInstruction (DoSpecial, PopTop);
    aContext->genInstruction (DoSpecial, SelfReturn);

    result = aContext->endMethod ();

    bytecodes = newByteArray (result.first.size ());
    bp = (byte *)vonNeumannSpaceOf (bytecodes);
    for (int i = 0; i < result.first.size (); i++)
    {
        bp[i] = result.first[i];
    }
    orefOfPut (method, messageInMethod, (objRef)newSymbol (sel.c_str ()));
    orefOfPut (method, bytecodesInMethod, (objRef)bytecodes);
    if (result.second.size () > 0)
    {
        theLiterals = newArray (result.second.size ());
        for (int i = 1; i <= result.second.size (); i++)
        {
            orefOfPut (theLiterals, i, result.second[i - 1]);
        }
        orefOfPut (method, literalsInMethod, (objRef)theLiterals);
    }
    else
    {
        orefOfPut (method, literalsInMethod, (objRef)nilObj);
    }
    orefOfPut (method, stackSizeInMethod, (objRef)encValueOf (6));
    orefOfPut (method,
               temporarySizeInMethod,
               (objRef)encValueOf (1 + aContext->localTop ()));
    if (1) // saveText)
    {
        orefOfPut (method, textInMethod, (objRef)newString ("codum"));
    }

    aContext->popScope ();

    return method;
}

int ClassNode::addIVarsToContextStartingFrom (GenerationContext * aContext,
                                              int index)
{
    assert (superClass || (superName == "nil"));

    if (superClass)
        index = superClass->getClass ()->addIVarsToContextStartingFrom (
            aContext, index);

    for (auto i : iVars)
    {
        aContext->addIvar (index++, i);
    }

    return index;
}

void ClassNode::generateInContext (GlobalVarNode * myClassVar,
                                   GenerationContext * aContext)
{
    int size;
    std::pair<encPtr, encPtr> classObjs;
    encPtr thisClass;

    aContext->pushScope (new ClassScope (myClassVar));

    printf ("Generate %s\n", name.c_str ());

    assert (superName != name);
    if (superName != "nil")
    {
        superClass = aContext->lookupClass (superName);
        assert (superClass && "Failed to find superclass!");
    }

    size = addIVarsToContextStartingFrom (aContext, 0);

    thisClass = aContext->defClass (myClassVar, name, iVars, size);

    for (auto m : iMethods)
    {
        encPtr newMeth = m->generateInContext (m->isClassMethod, aContext);
        if (m->isClassMethod)
            aContext->addMethodToMetaclassOf (thisClass, newMeth);
        else
            aContext->addMethodToClass (thisClass, newMeth);
    }

    aContext->popScope ();
}

void ProgramNode::generateInContext (GenerationContext * aContext)
{
    aContext->pushScope (new NameScope);

    for (auto & c : classes)
        aContext->addClass (c);

    for (auto & c : classes)
        c->generateInContext (aContext->lookupClass (c->name), aContext);

    //    aContext->generateClasses ();
}