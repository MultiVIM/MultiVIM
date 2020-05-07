#include <cassert>
#include <iostream>

#include "../AST.hxx"
#include "../Bytecode.hxx"
#include "../VM.hxx"

void IdentExprNode::generateInContext (GenerationContext * aContext)
{
    VarNode * var = aContext->lookup (id);

    if ((var->name == "self") || (var->name == "super"))
    {
        aContext->genInstruction (PushArgument, 0);
        return;
    }

    switch (var->kind)
    {
    case VarNode::kLocal:
        aContext->genInstruction (PushTemporary, var->getIndex ());
        return;

    case VarNode::kArgument:
        aContext->genInstruction (PushArgument, var->getIndex ());
        return;

    default:
        for (int i = 0; glbsyms[i]; i++)
            if (var->name == glbsyms[i])
            {
                printf ("generateglobal constant because %s\n", glbsyms[i]);
                aContext->genInstruction (PushConstant, i + 4);
                return;
            }

        /* not anything else, it must be a global */
        /* must look it up at run time */
        aContext->genInstruction (
            PushLiteral,
            aContext->genLiteral ((objRef)newSymbol (var->name.c_str ())));
        aContext->genMessage (false, 0, "value");
    }
}

void MessageExprNode::generateInContext (GenerationContext * aContext,
                                         bool cascade)
{
    if (!cascade)
        receiver->generateInContext (aContext);

    for (auto a : args)
        a->generateInContext (aContext);

    aContext->genMessage (receiver->isSuper (), args.size (), selector);
}

void CascadeExprNode::generateInContext (GenerationContext * aContext)
{
    receiver->generateInContext (aContext);

    messages.front ()->generateInContext (aContext, true);
    printf ("First Message %s\n", (messages.front ())->selector.c_str ());

    aContext->genInstruction (DoSpecial, Duplicate);

    for (auto it = ++std::begin (messages); it != std::end (messages); it++)
    {
        printf ("Generate Message %s\n", (*it)->selector.c_str ());
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

    printf ("BLOCK\n\n\n");
    print (9);

    for (auto l : args)
    {
        printf ("add arg %d\n", i);
        aContext->addLocal (i++, l);
    }

    fixLoc = aContext->pushBlock (args.size (), oldTop);

    for (auto & s : stmts)
    {
        s->generateInContext (aContext);
        if (&s != &stmts.back ())
            aContext->genInstruction (DoSpecial, PopTop);
    }

    aContext->popBlock (fixLoc, oldTop);
}

void MethodNode::print (int in)
{
    std::cout << blanks (in) << "<method: " << sel << ">\n";

    for (auto a : args)
        std::cout << blanks (in + 1) << "<param: " << a << "/>\n";

    for (auto a : locals)
        std::cout << blanks (in + 1) << "<local: " << a << "/>\n";

    std::cout << blanks (in + 1) << "<statements>\n";
    for (auto s : stmts)
        s->print (in + 2);
    std::cout << blanks (in + 1) << "</ statements>\n";

    std::cout << blanks (in) << "</method>\n";
}

ClassNode::ClassNode (std::string name, std::string superName,
                      std::list<std::string> cVars,
                      std::list<std::string> iVars)
    : name (name), superName (superName), cVars (cVars), iVars (iVars),
      superClass (nullptr)
{
}

void MethodNode::generateInContext (bool isClassMethod,
                                    GenerationContext * aContext)
{
    int index = 1;
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

    aContext->endMethod ();
    aContext->popScope ();
}

void ClassNode::print (int in)
{
    std::cout << blanks (in) << "<class: " << name << " super: " << superName
              << ">\n";

    for (auto a : cVars)
        std::cout << blanks (in + 1) << "<cVar: " << a << "/>\n";
    for (auto a : iVars)
        std::cout << blanks (in + 1) << "<iVar: " << a << "/>\n";

    std::cout << blanks (in + 1) << "<cMethods>\n";
    for (auto a : cMethods)
    {
        a->print (in + 2);
    }
    std::cout << blanks (in + 1) << "</cMethods>\n";

    std::cout << blanks (in + 1) << "<iMethods>\n";
    for (auto a : iMethods)
    {
        a->print (in + 2);
    }
    std::cout << blanks (in + 1) << "</iMethods>\n";

    std::cout << blanks (in) << "</class>\n";
}

int ClassNode::addIVarsToContextStartingFrom (GenerationContext * aContext,
                                              int index)
{
    assert (superClass || (superName == "nil"));

    if (superClass)
        index = superClass->getClass ()->addIVarsToContextStartingFrom (
            aContext, index);

    for (auto i : iVars)
        aContext->addIvar (index++, i);

    return index;
}

void ClassNode::generateInContext (GlobalVarNode * myClassVar,
                                   GenerationContext * aContext)
{
    int size = 0;
    std::pair<encPtr, encPtr> classObjs;

    aContext->pushScope (new ClassScope (myClassVar));

    assert (superName != name);
    if (superName != "nil")
    {
        superClass = aContext->lookupClass (superName);
        assert (superClass && "Failed to find superclass!");
        size = addIVarsToContextStartingFrom (aContext, 0);
    }

    aContext->defClass (myClassVar, name, iVars, size);

    for (auto m : iMethods)
    {
        m->generateInContext (false, aContext);
    }

    //        aContext->popScope();
}

void ProgramNode::addClass (ClassNode * aClass)
{
    classes.push_back (aClass);
}

void ProgramNode::print (int in)
{
    std::cout << blanks (in) << "<program>\n";
    for (auto c : classes)
        c->print (in);
    std::cout << blanks (in) << "</program>\n";
}

void ProgramNode::generateInContext (GenerationContext * aContext)
{
    aContext->pushScope (new NameScope);

    for (auto & c : classes)
        aContext->addClass (c);

    aContext->generateClasses ();
}

GlobalVarNode::GlobalVarNode (ClassNode * _class)
    : VarNode (kGlobal, _class->name), _class (_class)
{
}

ClassNode * GlobalVarNode::getClass ()
{
    return _class;
}
