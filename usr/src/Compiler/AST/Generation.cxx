#include <cassert>

#include "../CodeGen.hxx"
#include "LiteralExpr.hxx"
#include "Scope.hxx"
#include "VM/Bytecode.hxx"
#include "tcc/libtcc_ext.h"

#pragma mark literals

void VarNode::generateOn (CodeGen & gen)
{
    switch (kind)
    {
    case kInstance:
        gen.genPushInstanceVar (getIndex ());
        return;
    case kArgument:
        if (!promoted)
            gen.genPushArgument (getIndex ());
        else
            gen.genPushMyHeapVar (promotedIndex);
        return;
    case kLocal:
        if (!promoted)
            gen.genPushLocal (getIndex ());
        else
            gen.genPushMyHeapVar (promotedIndex);
        return;
    case kParentsHeapVar:
    {
        std::map<int, int>::iterator it =
            gen.currentScope ()->parentHeapVarToMyHeapVar.find (getIndex ());
        if (promoted ||
            it != gen.currentScope ()->parentHeapVarToMyHeapVar.end ())
            gen.genPushMyHeapVar (it->second);
        else
            gen.genPushParentHeapVar (getIndex ());
        return;
    }
    case kHeapVar:
        gen.genPushMyHeapVar (getIndex ());
        return;
    }

    gen.genPushGlobal (name);
}

void VarNode::generateAssignOn (CodeGen & gen, ExprNode * expr)
{
    expr->generateOn (gen);
    switch (kind)
    {
    case kInstance:
        gen.genStoreInstanceVar (getIndex ());
        return;
    case kLocal:
        if (!promoted)
            gen.genStoreLocal (getIndex ());
        else
            gen.genStoreMyHeapVar (promotedIndex);
        return;
    case kParentsHeapVar:
    {
        std::map<int, int>::iterator it =
            gen.currentScope ()->parentHeapVarToMyHeapVar.find (getIndex ());
        if (promoted ||
            it != gen.currentScope ()->parentHeapVarToMyHeapVar.end ())
            gen.genStoreMyHeapVar (it->second);
        else
            gen.genStoreParentHeapVar (getIndex ());
        return;
    }
    case kHeapVar:
        gen.genStoreMyHeapVar (getIndex ());
        return;
    }

    gen.genStoreGlobal (name);
}

void VarNode::generatePromoteOn (CodeGen & gen)
{
    switch (kind)
    {
    case kParentsHeapVar:
        gen.genMoveParentHeapVarToMyHeapVars (getIndex (), promotedIndex);
        gen.currentScope ()->parentHeapVarToMyHeapVar[getIndex ()] =
            promotedIndex;
        return;

    case kLocal:
        gen.genMoveLocalToMyHeapVars (getIndex (), promotedIndex);
        return;

    case kArgument:
        gen.genMoveArgumentToMyHeapVars (getIndex (), promotedIndex);
        return;
    }

    assert (!"Unreached\n");
}

/**
 * Restores the myheapvar value to the parents' heapvars. Implements the ability
 * to mutate parents' data.
 */
void VarNode::generateRestoreOn (CodeGen & gen)
{
    switch (kind)
    {
    case kParentsHeapVar:
        gen.genMoveMyHeapVarToParentHeapVars (promotedIndex, getIndex ());
        return;
    }
}

void IntExprNode::generateOn (CodeGen & gen)
{
    gen.genPushInteger (num);
}

void CharExprNode::generateOn (CodeGen & gen)
{
    gen.genPushLiteralObject (CharOopDesc::newWith (khar[0]));
}

void SymbolExprNode::generateOn (CodeGen & gen)
{
    gen.genPushLiteralObject (SymbolOopDesc::fromString (sym));
}

void StringExprNode::generateOn (CodeGen & gen)
{
    gen.genPushLiteralObject (StringOopDesc::fromString (str));
}

void FloatExprNode::generateOn (CodeGen & gen)
{
    gen.genPushLiteralObject (Oop::nilObj ());
    /*int litNum = gen.genLiteral ((objRef)newFloat (num));
    if (!gen.inLiteralArray ())
        gen.genInstruction (PushLiteral, litNum);*/
}

void ArrayExprNode::generateOn (CodeGen & gen)
{
    gen.genPushLiteralObject (Oop::nilObj ());
    /*int litNum;

    gen.beginLiteralArray ();
    for (auto el : elements)
        el->generateOn (gen);
    litNum = gen.endLiteralArray ();

    if (!gen.inLiteralArray ())
        gen.genInstruction (PushLiteral, litNum);*/
}

#pragma mark expressions

static void tccErr (void * opaqueError, const char * msg)
{
    fprintf (stderr, "%s\n", msg);
    fflush (NULL);
};

static const char jitPrelude[] = {
#include "JITPrelude.rh"
    0};

void PrimitiveExprNode::generateOn (CodeGen & gen)
{
    std::string name;

    if (num != 0)
    {
        for (auto arg : args)
            arg->generateOn (gen);
        gen.genPrimitive (num, args.size ());
        return;
    }

    name = dynamic_cast<IdentExprNode *> (args[0])->id;

    if (name == "C")
    {
        std::string code = std::string (jitPrelude) +
                           dynamic_cast<StringExprNode *> (args[1])->str;
        TCCState * tcc = atcc_new ();
        size_t codeSize;
        NativeCodeOop nativeCode;
        NativeCodeOopDesc::Fun fun;

        tcc_set_error_func (tcc, NULL, tccErr);
        tcc_set_output_type (tcc, TCC_OUTPUT_MEMORY);
        tcc_define_symbol (tcc, "__MultiVIM_JIT", "true");
        if (tcc_compile_string (tcc, code.c_str ()) == -1)
            abort ();
        codeSize = tcc_relocate (tcc, NULL);
        printf ("Code size: %d\n", codeSize);
        assert (codeSize != -1);

        nativeCode = NativeCodeOopDesc::newWithSize (codeSize);
        tcc_relocate (tcc, nativeCode->funCode ());
        fun = (NativeCodeOopDesc::Fun)tcc_get_symbol (tcc, "main");
        nativeCode->setFun (fun);

        tcc_delete (tcc);

        gen.genPushLiteralObject (nativeCode);
    }
}

void IdentExprNode::generateOn (CodeGen & gen)
{
    if (isSuper () || id == "self")
        gen.genPushSelf ();
    else if (id == "nil")
        gen.genPushNil ();
    else if (id == "true")
        gen.genPushTrue ();
    else if (id == "false")
        gen.genPushFalse ();
    else if (id == "Smalltalk")
        gen.genPushSmalltalk ();
    else
        var->generateOn (gen);
}

void IdentExprNode::generateAssignOn (CodeGen & gen, ExprNode * rValue)
{
    var->generateAssignOn (gen, rValue);
}

void AssignExprNode::generateOn (CodeGen & gen)
{
    left->generateAssignOn (gen, right);
}

void MessageExprNode::generateOn (CodeGen & gen, bool cascade)
{
    if (!cascade)
        receiver->generateOn (gen);

    for (auto a : args)
        a->generateOn (gen);

    if (selector == std::string ("ifTrue:ifFalse:"))
    {
        printf ("GEnerate optimised\n");
        gen.genIfTrueIfFalse ();
    }
    else
    {
        printf ("generate unoptimised %s\n", selector.c_str ());
        gen.genMessage (receiver->isSuper (), args.size (), selector);
    }
}

void CascadeExprNode::generateOn (CodeGen & gen)
{
    receiver->generateOn (gen);
    messages.front ()->generateOn (gen, true);
    /* duplicate result of first send so it remains after message send */
    gen.genDup ();

    for (auto it = ++std::begin (messages); it != std::end (messages); it++)
    {
        if (*it == messages.back ())
            /* as we are the last one, get rid of the original receiver
             * duplicate; we are consuming it. */
            gen.genPop ();
        (*it)->generateOn (gen, true);
        if (*it != messages.back ())
        {
            /* pop result of this send */
            gen.genPop ();
            /* duplicate receiver so it remains after next send*/
            gen.genDup ();
        }
    }
}

void BlockExprNode::generateReturnPreludeOn (CodeGen & gen)
{
    for (auto & v : scope->myHeapVars)
        v.second->generateRestoreOn (gen);
}

void BlockExprNode::generateOn (CodeGen & gen)
{
    BlockOop block = BlockOopDesc::allocate ();
    CodeGen blockGen (true);

    blockGen.pushCurrentScope (scope);

    for (auto & v : scope->myHeapVars)
        v.second->generatePromoteOn (blockGen);

    for (auto & s : stmts)
    {
        s->generateOn (blockGen);
        if (s != stmts.back ())
            blockGen.genPop ();
        else if (!dynamic_cast<ReturnStmtNode *> (s))
        {
            generateReturnPreludeOn (blockGen);
            blockGen.genReturn ();
        }
    }

    if (stmts.empty ())
    {
        generateReturnPreludeOn (blockGen);
        blockGen.genPushNil ();
        blockGen.genReturn ();
    }

    block->setBytecode (ByteArrayOopDesc::fromVector (blockGen.bytecode ()));
    block->setLiterals (ArrayOopDesc::fromVector (blockGen.literals ()));
    block->setArgumentCount (SmiOop (args.size ()));
    block->setTemporarySize (SmiOop ((intptr_t)0));
    block->setHeapVarsSize (SmiOop (scope->myHeapVars.size ()));
    block->setStackSize (SmiOop (blockGen.maxStackSize ()));

    // gen.popCurrentScope ();
    // if (!blockGen._blockHasBlockReturn)
    //    gen.genPushLiteralObject (block);
    // else
    gen.genPushBlockCopy (block);
}

#pragma mark statements

void ExprStmtNode::generateOn (CodeGen & gen)
{
    expr->generateOn (gen);
}

void ReturnStmtNode::generateOn (CodeGen & gen)
{
    expr->generateOn (gen);
    //  FIXME: Do we need to do anything with parents' heapvars? Set them back
    //  or something like that?
    if (gen.isBlock ())
        gen.genBlockReturn ();
    else
        gen.genReturn ();
}

#pragma mark decls

MethodOop MethodNode::generate ()
{
    bool finalIsReturn;
    MethodOop meth = MethodOopDesc::allocate ();
    CodeGen gen;

    gen.pushCurrentScope (scope);

    for (auto & v : scope->myHeapVars)
        v.second->generatePromoteOn (gen);

    for (auto s : stmts)
    {
        s->generateOn (gen);
        if (s != stmts.back () ||
            !(finalIsReturn = dynamic_cast<ReturnStmtNode *> (s)))
            gen.genPop ();
    }

    if (!finalIsReturn)
    {
        gen.genPushSelf ();
        gen.genReturn ();
    }

    meth->setSelector (SymbolOopDesc::fromString (sel));
    meth->setBytecode (ByteArrayOopDesc::fromVector (gen.bytecode ()));
    meth->setLiterals (ArrayOopDesc::fromVector (gen.literals ()));
    meth->setArgumentCount (args.size ());
    meth->setTemporarySize (locals.size ());
    meth->setHeapVarsSize (scope->myHeapVars.size ());
    meth->setStackSize (gen.maxStackSize ());

    // meth->print (2);

    gen.popCurrentScope ();

    // REMOVE printf ("\nEnd a method\n\n\n\n\n");

    return meth;
}

void ClassNode::generate ()
{
    printf ("CLASS %s\n", name.c_str ());
    for (auto m : cMethods)
        cls->addClassMethod (m->generate ());

    for (auto m : cMethods)
        cls->addClassMethod (m->generate ());
    for (auto m : iMethods)
        cls->addMethod (m->generate ());
}

void NamespaceNode::generate ()
{
    printf ("NAMESPACE %s\n", name.c_str ());
    for (auto cls : decls)
        cls->generate ();
}

void ProgramNode::generate ()
{
    for (auto cls : decls)
        cls->generate ();
}