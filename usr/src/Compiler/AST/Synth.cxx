#include "AST.hxx"
#include "Expr.hxx"
#include "LiteralExpr.hxx"
#include "Scope.hxx"

VarNode * AbstractScope::lookup (std::string name)
{
    if (parent ())
        return parent ()->lookup (name);
    else
    {
        /*printf ("In scope %s: Failed to find %s, assuming GLOBAL.\n",
                typeid (*this).name (),
                name.c_str ());*/
        return new GlobalVarNode (name);
    }
}

VarNode * ClassScope::lookup (std::string name)
{
    for (auto v : iVars)
        if (v->name == name)
            return v;
    return AbstractScope::lookup (name);
}

void ClassScope::addIvar (std::string name)
{
    iVars.push_back (new InstanceVarNode (iVars.size () + 1, name));
}

void AbstractCodeScope::addArg (std::string name)
{
    args.push_back (new ArgumentVarNode (args.size () + 1, name));
}

void AbstractCodeScope::addLocal (std::string name)
{
    locals.push_back (new LocalVarNode (locals.size () + 1, name));
}

ParentsHeapVarNode * AbstractCodeScope::promote (VarNode * aNode)
{
    HeapVarNode * newVar =
        new HeapVarNode (myHeapVars.size () + 1, aNode->name);
    // printf ("Promoting\n");
    // aNode->print (5);
    aNode->promoted = true;
    aNode->promotedIndex = newVar->index;
    myHeapVars.push_back ({newVar, aNode});
    return new ParentsHeapVarNode (myHeapVars.size (), aNode->name);
}

VarNode * MethodScope::lookup (std::string aName)
{
    for (auto v : locals)
        if (v->name == aName)
            return v;
    for (auto v : args)
        if (v->name == aName)
            return v;
    return AbstractCodeScope::lookup (aName);
}

VarNode * MethodScope::lookupFromBlock (std::string aName)
{
    for (auto v : myHeapVars)
        /* already promoted */
        if (v.first->name == aName)
            return new ParentsHeapVarNode (v.first->index, aName);
    for (auto v : locals)
        if (v->name == aName)
            return promote (v);
    for (auto v : args)
        if (v->name == aName)
            return promote (v);
    return AbstractCodeScope::lookup (aName);
}

VarNode * BlockScope::lookup (std::string aName)
{
    VarNode * candidate;
    for (auto v : myHeapVars)
        if (v.first->name == aName)
            return v.first;
    for (auto v : args)
        if (v->name == aName)
            return v;
    return parent ()->lookupFromBlock (aName);
}

VarNode * BlockScope::lookupFromBlock (std::string aName)
{
    VarNode * par;
    ParentsHeapVarNode * parCand;
    for (auto v : myHeapVars)
        /* already promoted */
        if (v.first->name == aName)
            return new ParentsHeapVarNode (v.first->index, aName);
    for (auto v : locals)
        if (v->name == aName)
            return promote (v);
    for (auto v : args)
        if (v->name == aName)
            return promote (v);

    par = parent ()->lookupFromBlock (aName);

    if ((parCand = dynamic_cast<ParentsHeapVarNode *> (par)))
        /*
         * Parent has a heapvar for it, but we don't have a local reference!
         * Therefore let us create a local copy in our own heapvars.
         */
        return promote (par);
    return par;
}

#pragma expressions

void PrimitiveExprNode::synthInScope (AbstractScope * scope)
{
    for (auto arg : args)
        arg->synthInScope (scope);
}

void IdentExprNode::synthInScope (AbstractScope * scope)
{
    var = scope->lookup (id);
}

void AssignExprNode::synthInScope (AbstractScope * scope)
{
    left->synthInScope (scope);
    right->synthInScope (scope);
}

void MessageExprNode::synthInScope (AbstractScope * scope)
{
    receiver->synthInScope (scope);
    for (auto arg : args)
        arg->synthInScope (scope);
}

void CascadeExprNode::synthInScope (AbstractScope * scope)
{
    receiver->synthInScope (scope);
    for (auto message : messages)
        message->synthInScope (scope);
}

void BlockExprNode::synthInScope (AbstractScope * parentScope)
{
    scope = new BlockScope (dynamic_cast<AbstractCodeScope *> (parentScope));

    for (auto arg : args)
        scope->addArg (arg);

    for (auto stmt : stmts)
        stmt->synthInScope (scope);
}

#pragma statements

void ExprStmtNode::synthInScope (AbstractScope * scope)
{
    expr->synthInScope (scope);
}

void ReturnStmtNode::synthInScope (AbstractScope * scope)
{
    expr->synthInScope (scope);
}

#pragma decls

MethodNode * MethodNode::synthInClassScope (ClassScope * clsScope)
{
    /* We expect to get a class node already set up with its ivars etc. If not,
     * no problem. */
    scope = new MethodScope (clsScope);

    for (auto arg : args)
        scope->addArg (arg);
    for (auto local : locals)
        scope->addLocal (local);
    for (auto stmt : stmts)
        stmt->synthInScope (scope);

    return this;
}

static void classOopAddIvarsToScopeStartingFrom (ClassOop aClass,
                                                 ClassScope * scope)
{
    ClassOop superClass = aClass.superClass ();

    if (!superClass.isNil ())
        classOopAddIvarsToScopeStartingFrom (superClass, scope);

    for (int i = 1; i <= aClass.nstVars ().size (); i++)
        scope->addIvar (
            aClass.nstVars ().basicAt (i).asSymbolOop ().asString ());
}

void ClassNode::synth ()
{
    int index = 0;
    scope = new ClassScope;
    ClassOop superCls = memMgr.objNil ().asClassOop ();

    if (superName != "nil")
    {
        superCls = memMgr.lookupClass (superName);
        assert (!superCls.isNil ());
    }

    cls = memMgr.findOrCreateClass (superCls, name);
    cls.setNstVars (ArrayOop::symbolArrayFromStringVector (iVars));

    classOopAddIvarsToScopeStartingFrom (cls, scope);
    cls.setNstSize (SmiOop (scope->iVars.size ()));
    memMgr.objGlobals ().symbolInsert (cls.name (), cls);

    for (auto meth : cMethods)
        meth->synthInClassScope (scope);

    for (auto meth : iMethods)
    {
        meth->synthInClassScope (scope);
    }
}

void ProgramNode::synth ()
{
    for (auto & c : classes)
        c->synth ();
}