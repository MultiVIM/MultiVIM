#include <cassert>

#include "../CodeGen.hxx"
#include "LiteralExpr.hxx"
#include "OldVM/Bytecode.hxx"
#include "Scope.hxx"
#include "VM/Bytecode.hxx"

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
        gen.genPushParentHeapVar (getIndex ());
        return;
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
            gen.genPushLocal (getIndex ());
        else
            gen.genStoreMyHeapVar (promotedIndex);
        return;
    case kParentsHeapVar:
        gen.genStoreParentHeapVar (getIndex ());
        return;
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

void IntExprNode::generateOn (CodeGen & gen)
{
    gen.genPushInteger (num);
}

void CharExprNode::generateOn (CodeGen & gen)
{
    gen.genPushLiteralObject (memMgr.objNil ());
    /*int litNum = gen.genLiteral ((objRef)newChar (khar[0]));
    if (!gen.inLiteralArray ())
        gen.genInstruction (PushLiteral, litNum);*/
}

void SymbolExprNode::generateOn (CodeGen & gen)
{
    gen.genPushLiteralObject (SymbolOop::fromString (sym));
    /*int litNum = gen.genLiteral ((objRef)newSymbol (sym.c_str ()));
    if (!gen.inLiteralArray ())
        gen.genInstruction (PushLiteral, litNum);*/
}

void StringExprNode::generateOn (CodeGen & gen)
{
    gen.genPushLiteralObject (StringOop::fromString (str));
    /*int litNum = gen.genLiteral ((objRef)newString (str.c_str ()));
    if (!gen.inLiteralArray ())
        gen.genInstruction (PushLiteral, litNum);*/
}

void FloatExprNode::generateOn (CodeGen & gen)
{
    gen.genPushLiteralObject (memMgr.objNil ());
    /*int litNum = gen.genLiteral ((objRef)newFloat (num));
    if (!gen.inLiteralArray ())
        gen.genInstruction (PushLiteral, litNum);*/
}

void ArrayExprNode::generateOn (CodeGen & gen)
{
    gen.genPushLiteralObject (memMgr.objNil ());
    /*int litNum;

    gen.beginLiteralArray ();
    for (auto el : elements)
        el->generateOn (gen);
    litNum = gen.endLiteralArray ();

    if (!gen.inLiteralArray ())
        gen.genInstruction (PushLiteral, litNum);*/
}

#pragma mark expressions

void PrimitiveExprNode::generateOn (CodeGen & gen)
{
    for (auto arg : args)
        arg->generateOn (gen);
    gen.genPrimitive (num, args.size ());
}

void IdentExprNode::generateOn (CodeGen & gen)
{
    if (isSuper ())
        gen.genPushSelf ();
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

    gen.genMessage (receiver->isSuper (), args.size (), selector);
}

void CascadeExprNode::generateOn (CodeGen & gen)
{
    receiver->generateOn (gen);
    messages.front ()->generateOn (gen, true);
    /* duplicate result of first send so it remains after message send */
    gen.genDup ();

    for (auto it = ++std::begin (messages); it != std::end (messages); it++)
    {
        // REMOVE: printf ("begin generate iteration\n\n");
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

void BlockExprNode::generateOn (CodeGen & gen)
{
    BlockOop block = BlockOop::allocate ();
    CodeGen blockGen (true);

    // REMOVE: printf ("\nBegin Block\n\n");

    for (auto & v : scope->myHeapVars)
        v.second->generatePromoteOn (blockGen);

    for (auto & s : stmts)
    {
        s->generateOn (blockGen);
        if (s != stmts.back ())
            blockGen.genPop ();
        else if (!dynamic_cast<ReturnStmtNode *> (s))
        {
            // REMOVE printf ("gen return as at end of line\n");
            // REMOVE s->print (12);
            blockGen.genReturn ();
        }
    }

    if (stmts.empty ())
    {
        blockGen.genPushNil ();
        blockGen.genReturn ();
    }

    block.setBytecode (ByteArrayOop::fromVector (blockGen.bytecode ()));
    block.setLiterals (ArrayOop::fromVector (blockGen.literals ()));
    block.setArgumentCount (SmiOop (args.size ()));
    block.setTemporarySize (SmiOop (0));
    block.setHeapVarsSize (SmiOop (scope->myHeapVars.size ()));

    gen.popCurrentScope ();
    // REMOVE printf ("\nEnd Block\n\n");

    gen.genPushLiteralObject (block);
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
    MethodOop meth = MethodOop::allocate ();
    CodeGen gen;

    printf (" -> %s\n", sel.c_str ());

    // REMOVE printf ("\n\n\n\n\n\n\nBegin a Method %s of %s\n\n", sel.c_str
    // ());
    gen.pushCurrentScope (scope);

    for (auto & v : scope->myHeapVars)
        v.second->generatePromoteOn (gen);

    for (auto s : stmts)
    {
        s->generateOn (gen);
        if (s != stmts.back () || !dynamic_cast<ReturnStmtNode *> (s))
        {
            // REMOVE printf ("generate pop as not at end\n");
            gen.genPop ();
        }
    }

    if (!1)
    {
        // gen.genPushSelf ();
        // gen.genReturn ();
    }

    meth.setSelector (SymbolOop::fromString (sel));
    meth.setBytecode (ByteArrayOop::fromVector (gen.bytecode ()));
    meth.setLiterals (ArrayOop::fromVector (gen.literals ()));
    meth.setTemporarySize (locals.size ());
    meth.setHeapVarsSize (scope->myHeapVars.size ());

    gen.popCurrentScope ();

    // REMOVE printf ("\nEnd a method\n\n\n\n\n");

    return meth;
}

void ClassNode::generate ()
{
    printf ("CLASS %s\n", name.c_str ());
    for (auto m : cMethods)
        cls.addClassMethod (m->generate ());

    for (auto m : iMethods)
        cls.addMethod (m->generate ());
}

void ProgramNode::generate ()
{
    for (auto cls : classes)
        cls->generate ();
}

#pragma mark OLD

/*int ExprNode::generateOptimized (CodeGen & gen, int
instruction, bool doPop)
{
int location;

gen.genInstruction (DoSpecial, instruction);
location = gen.codeTop ();

gen.genCode (0);
if (doPop)
gen.genInstruction (DoSpecial, PopTop);

this->generateOn (gen);
gen.genMessage (false, 0, "value");

gen.setCodeTo (location, gen.codeTop () + 1);
// codeArray[location] = codeTop + 1;
return (location);
}


Other expressions
void PrimitiveExprNode::generateOn (CodeGen & gen)
{
printf ("generate prim %d<--\n", num);
for (auto a : args)
a->generateOn (gen);
gen.genInstruction (DoPrimitive, args.size ());
gen.genCode (num);
printf ("-->generated prim %d\n", num);
}

void IdentExprNode::generateOn (CodeGen & gen)
{
VarNode * var = gen.lookup (id);

if ((id == "self") || (id == "super"))
{
gen.genInstruction (PushArgument, 0);
return;
}

if (!var)
printf ("DID NOT RESOLVE %s!!\n", id.c_str ());

if (var)
switch (var->kind)
{
case VarNode::kLocal:
printf ("Local %s is a LOCAL TEMPORARY\n", id.c_str ());
gen.genInstruction (PushTemporary, var->getIndex () - 1);
return;

case VarNode::kArgument:
gen.genInstruction (PushArgument, var->getIndex ());
return;

case VarNode::kInstance:
gen.genInstruction (PushInstance, var->getIndex ());
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
gen.genInstruction (PushConstant, i + 4);
return;
}

// not anything else, it must be a global
// must look it up at run time
gen.genInstruction (
PushLiteral, gen.genLiteral ((objRef)newSymbol (id.c_str ())));
gen.genMessage (false, 0, "value");
}

void IdentExprNode::generateAssignInContext (CodeGen & gen,
                                 ExprNode * rValue)
{
VarNode * var = gen.lookup (id);

if (var)
{
switch (var->kind)
{
case VarNode::kLocal:
rValue->generateOn (gen);
gen.genInstruction (AssignTemporary, var->getIndex () - 1);
return;

case VarNode::kInstance:
rValue->generateOn (gen);
gen.genInstruction (AssignInstance, var->getIndex ());
return;
}
}

gen.genInstruction (PushArgument, 0);
gen.genInstruction (
PushLiteral, gen.genLiteral ((objRef)newSymbol (id.c_str ())));
rValue->generateOn (gen);
gen.genMessage (false, 2, "assign:value:");
}




void ReturnStmtNode::generateOn (CodeGen & gen)
{
expr->generateOn (gen);
if (gen.inBlock ())
{
// change return point before returning
gen.genInstruction (PushConstant, contextConst);
gen.genMessage (false, 0, "blockReturn");
gen.genInstruction (DoSpecial, PopTop);
}
gen.genInstruction (DoSpecial, StackReturn);
}

encPtr MethodNode::generateOn (bool isClassMethod,
                          CodeGen & gen)
{
int index = 1;
encPtr bytecodes, theLiterals;
encPtr method = newMethod ();
byte * bp;
std::pair<std::vector<byte>, std::vector<objRef>> result;
gen.pushScope (new MethodScope);
gen.beginMethod ();

printf ("METHOD GENERATING IN CONTEXT...\n\n");

for (auto i : args)
gen.addArg (index++, i);

index = 1;
for (auto i : locals)
gen.addLocal (index++, i);

for (auto & s : stmts)
{
s->generateOn (gen);
if (&s != &stmts.back ())
 gen.genInstruction (DoSpecial, PopTop);
}

gen.genInstruction (DoSpecial, PopTop);
gen.genInstruction (DoSpecial, SelfReturn);

result = gen.endMethod ();

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
    (objRef)encValueOf (1 + gen.localTop ()));
if (1) // saveText)
{
orefOfPut (method, textInMethod, (objRef)newString ("codum"));
}

gen.popScope ();

return method;
}

int ClassNode::addIVarsToContextStartingFrom (CodeGen & gen,
                                  int index)
{
assert (superClass || (superName == "nil"));

if (superClass)
index = superClass->getClass ()->addIVarsToContextStartingFrom (
gen, index);

for (auto i : iVars)
{
gen.addIvar (index++, i);
}

return index;
}

void ClassNode::generateOn (GlobalVarNode * myClassVar,
                       CodeGen & gen)
{
int size;
std::pair<encPtr, encPtr> classObjs;
encPtr thisClass;

gen.pushScope (new ClassScope (myClassVar));

printf ("Generate %s\n", name.c_str ());

assert (superName != name);
if (superName != "nil")
{
superClass = gen.lookupClass (superName);
assert (superClass && "Failed to find superclass!");
}

size = addIVarsToContextStartingFrom (gen, 0);

thisClass = gen.defClass (myClassVar, name, iVars, size);

for (auto m : iMethods)
{
encPtr newMeth = m->generateOn (m->isClassMethod, gen);
if (m->isClassMethod)
 gen.addMethodToMetaclassOf (thisClass, newMeth);
else
 gen.addMethodToClass (thisClass, newMeth);
}

gen.popScope ();
}

void ProgramNode::generateOn (CodeGen & gen)
{
gen.pushScope (new NameScope);

for (auto & c : classes)
gen.addClass (c);

for (auto & c : classes)
c->generateOn (gen.lookupClass (c->name), gen);

//    gen.generateClasses ();
}
*/