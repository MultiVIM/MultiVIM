#include "../NewCompiler.h"
#include "../AST.hxx"
#include "../Bytecode.hxx"
#include "../VM.hxx"

GlobalVarNode * NameScope::addClass (ClassNode * aClass)
{
    auto cls = new GlobalVarNode (aClass);
    vars[aClass->name] = cls;
    return cls;
}

void NameScope::addLocal (int idx, std::string name)
{
    printf ("Add local %s\n", name.c_str ());
    vars[name] = new LocalVarNode (idx, name);
}
void NameScope::addArg (int idx, std::string name)
{
    vars[name] = new ArgumentVarNode (idx, name);
}

void NameScope::addIvar (int idx, std::string name)
{
    vars[name] = new InstanceVarNode (idx, name);
}

VarNode * NameScope::lookup (std::string aName)
{
    return vars[aName];
}

void NameScope::removeLocalsAbove (int i)
{
    for (auto it = std::begin (vars); it != std::end (vars); it++)
    {
        LocalVarNode * local = dynamic_cast<LocalVarNode *> (it->second);
        if (local && local->getIndex () > i)
            vars.erase (it);
    }
}

GlobalVarNode * GenerationContext::addClass (ClassNode * aClass)
{
    return scopes.back ()->addClass (aClass);
}

void GenerationContext::addIvar (int index, std::string name)
{
    scopes.back ()->addIvar (index, name);
}

void GenerationContext::addArg (int index, std::string name)
{
    scopes.back ()->addArg (index, name);
}

void GenerationContext::addLocal (int index, std::string name)
{
    scopes.back ()->addLocal (index, name);
    if (index > highestLocal)
        highestLocal = index;
}

void GenerationContext::pushScope (NameScope * scope)
{
    if (!scopes.empty ())
        scope->parent = scopes.back ();
    scopes.push_back (scope);
}

void GenerationContext::popScope ()
{
    scopes.pop_back ();
}

GlobalVarNode * GenerationContext::lookupClass (std::string name)
{
    return dynamic_cast<GlobalVarNode *> (lookup (name));
}

VarNode * GenerationContext::lookup (std::string name)
{
    for (auto it = scopes.rbegin (); it != scopes.rend (); ++it)
    {
        VarNode * candidate = (*it)->lookup (name);
        if (candidate)
            return candidate;
    }
    return nullptr;
}

void GenerationContext::beginMethod ()
{
    literalArrayStack.emplace ();
}
void GenerationContext::endMethod ()
{
    literalArrayStack.pop ();
}

void GenerationContext::restoreOldTop (int oldTop)
{
    scopes.back ()->removeLocalsAbove (oldTop);
}

void GenerationContext::generateClasses ()
{
    for (auto c : scopes.back ()->vars)
    {
        GlobalVarNode * cls = dynamic_cast<GlobalVarNode *> (c.second);
        if (cls)
            cls->getClass ()->generateInContext (cls, this);
    }
}

encPtr GenerationContext::defClass (GlobalVarNode * classVar, std::string name,
                                    std::list<std::string> iVars, size_t size)
{
    encPtr classObj;

    classObj = findClass (name.c_str ());

    if (!classVar->getClass ()->superClass)
        size = 0;
    else
    {
        encPtr superObj;
        superObj = classVar->getClass ()->superClass->classObj;
        size = intValueOf (orefOf (superObj, sizeInClass).val);
        orefOfPut (classObj, superClassInClass, (objRef)superObj);
        {
            encPtr classMeta = classOf (classObj);
            encPtr superMeta = classOf (superObj);
            orefOfPut (classMeta, superClassInClass, (objRef)superMeta);
        }
    }

    if (!iVars.empty ())
    {
        encPtr instStr;
        int instTop;
        encPtr instVars[256];
        encPtr varVec;
        int i = 0;

        varVec = newArray (iVars.size ());
        for (auto s : iVars)
            orefOfPut (varVec, i + 1, newSymbol (s.c_str ()));
        orefOfPut (classObj, variablesInClass, (objRef)varVec);
    }
    orefOfPut (classObj, sizeInClass, (objRef)encValueOf (size));

    return classObj;
}

int GenerationContext::pushBlock (int argCount, int tempLoc)
{
    int fixLoc;
    encPtr newBlk = newBlock ();
    orefOfPut (newBlk, argumentCountInBlock, (objRef)encValueOf (argCount));
    orefOfPut (newBlk,
               argumentLocationInBlock,
               (objRef)encValueOf (tempLoc /* FIXME: + 1 */));
    genInstruction (PushLiteral, genLiteral ((objRef)newBlk));
    genInstruction (PushConstant, contextConst);
    genInstruction (DoPrimitive, 2);
    genCode (29);
    genInstruction (DoSpecial, Branch);
    fixLoc = meth.code.size ();
    genCode (0);
    /*genInstruction(DoSpecial, PopTop); */
    orefOfPut (newBlk,
               bytecountPositionInBlock,
               (objRef)encValueOf (meth.code.size () /* FIXME: + 1*/));

    inBlock++;
    return fixLoc;
}

void GenerationContext::popBlock (int fixLocation, int tempLoc)
{
    genInstruction (DoSpecial, StackReturn);
    meth.code[fixLocation] = meth.code.size ();
    restoreOldTop (tempLoc);
    inBlock--;
}

void GenerationContext::beginLiteralArray ()
{
    literalArrayStack.emplace ();
}

void GenerationContext::endLiteralArray ()
{
    int numLit, size;
    std::vector<objRef> literalArray = literalArrayStack.top ();
    encPtr obj;
    encPtr newLit;

    literalArrayStack.pop ();
    size = literalArray.size ();
    newLit = newArray (size);
    for (int i = 0; i < size; i++)
    {
        orefOfPut (newLit, i + 1, literalArray[i]);
    }

    numLit = genLiteral ((objRef)newLit);

    if (literalArrayStack.size () > 1)
        genInstruction (PushLiteral, numLit);
}

void GenerationContext::genCode (int value)
{
    // if (codeTop >= codeLimit)
    //    compilError (selector, "too many bytecode instructions in method",
    //    "");
    // else
    // codeArray[codeTop++] = value;
    meth.code.push_back (value);
}

void GenerationContext::genInstruction (int high, int low)
{
    printf ("(gen-instr (%d %d))\n", high, low);
    if (low >= 16)
    {
        genInstruction (Extended, high);
        genCode (low);
    }
    else
        genCode (high * 16 + low);
}

int GenerationContext::genLiteral (objRef aLiteral)
{
    //   if (literalTop >= literalLimit)
    //   compilError (selector, "too many literals in method", "");
    // else
    {
        // literalArray[++literalTop] = aLiteral;
        // return (literalTop - 1);
        literalArrayStack.top ().push_back (aLiteral);
        return literalArrayStack.top ().size () - 1;
    }
}

void GenerationContext::genInteger (int val)
{
    if (val == -1)
        genInstruction (PushConstant, minusOne);
    else if ((val >= 0) && (val <= 2))
        genInstruction (PushConstant, val);
    else
        genInstruction (PushLiteral, genLiteral ((objRef)encValueOf (val)));
}

void GenerationContext::genMessage (bool toSuper, int argumentCount,
                                    std::string selector)
{
    encPtr messagesym = newSymbol (selector.c_str ());

    bool sent = false;
    int i;

    if ((!toSuper) && (argumentCount == 0))
        for (i = 0; (!sent) && ptrNe ((objRef)unSyms[i], (objRef)nilObj); i++)
            if (ptrEq ((objRef)messagesym, (objRef)unSyms[i]))
            {
                genInstruction (SendUnary, i);
                sent = true;
            }
    if ((!toSuper) && (argumentCount == 1))
        for (i = 0; (!sent) && ptrNe ((objRef)binSyms[i], (objRef)nilObj); i++)
            if (ptrEq ((objRef)messagesym, (objRef)binSyms[i]))
            {
                genInstruction (SendBinary, i);
                sent = true;
            }
    if (!sent)
    {
        genInstruction (MarkArguments, 1 + argumentCount);
        if (toSuper)
        {
            genInstruction (DoSpecial, SendToSuper);
            genCode (genLiteral ((objRef)messagesym));
        }
        else
            genInstruction (SendMessage, genLiteral ((objRef)messagesym));
    }
}