#include "Compiler.hxx"
#include "AST/AST.hxx"
#include "OldVM/Bytecode.hxx"
#include "OldVM/VM.hxx"

GlobalVarNode * NameScope::addClass (ClassNode * aClass)
{
    auto cls = new GlobalVarNode (aClass);
    vars.insert ({aClass->name, cls});
    return cls;
}

void NameScope::addLocal (int idx, std::string name)
{
    vars[name] = new LocalVarNode (idx, name);
}
void NameScope::addArg (int idx, std::string name)
{
    vars[name] = new ArgumentVarNode (idx, name);
}

void NameScope::addIvar (int idx, std::string name)
{
    printf ("Namescope add ivar %d id %s\n", idx, name.c_str ());
    vars[name] = new InstanceVarNode (idx, name);
}

VarNode * NameScope::lookup (std::string aName)
{
    if (vars.find (aName) != vars.end ())
        return vars[aName];
    else
        return nullptr;
}

void NameScope::removeLocalsAbove (int i)
{
    std::list<std::map<std::string, VarNode *>::const_iterator> itrs;

    for (auto it = std::begin (vars); it != std::end (vars); it++)
    {
        LocalVarNode * local = dynamic_cast<LocalVarNode *> (it->second);
        if (local && local->getIndex () > i)
            itrs.push_back (it);
    }

    for (auto it : itrs)
    {
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

std::pair<std::vector<byte>, std::vector<objRef>>
GenerationContext::endMethod ()
{
    std::vector<byte> oldCode = meth.code;
    std::vector<objRef> lits = literalArrayStack.top ();

    printf ("OldCode: %d\n", oldCode.size ());
    meth.code.clear ();
    printf ("NowCode: %d\n", oldCode.size ());

    literalArrayStack.pop ();
    return {oldCode, lits};
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

    classVar->classObj = classObj;

    return classObj;
}

void GenerationContext::addMethodToClass (encPtr aClass, encPtr aMethod)
{
    /* now find or create a method table */
    encPtr methTable = orefOf (aClass, methodsInClass).ptr;
    encPtr selector;
    if (ptrEq ((objRef)methTable, (objRef)nilObj))
    { /* must make */
        methTable = newDictionary (MethodTableSize);
        orefOfPut (aClass, methodsInClass, (objRef)methTable);
    }

    orefOfPut (aMethod, methodClassInMethod, (objRef)aClass);
    selector = orefOf (aMethod, messageInMethod).ptr;
    nameTableInsert (methTable, oteIndexOf (selector), selector, aMethod);

    /*  printf ("Added method %s to class %s\n",
              vonNeumannSpaceOf (orefOf (aClass, nameInClass).ptr),
              vonNeumannSpaceOf (selector));*/
}

void GenerationContext::addMethodToMetaclassOf (encPtr aClass, encPtr aMethod)
{
    printf ("Add Method to Metaclass...\n");
    addMethodToClass (classOf (aClass), aMethod);
}

int GenerationContext::pushBlock (int argCount, int tempLoc)
{
    int fixLoc;
    encPtr newBlk = newBlock ();
    orefOfPut (newBlk, argumentCountInBlock, (objRef)encValueOf (argCount));
    orefOfPut (newBlk,
               argumentLocationInBlock,
               (objRef)encValueOf (tempLoc + 1 /* FIXME: + 1 */));
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
               (objRef)encValueOf (meth.code.size () + 1 /* FIXME: + 1*/));

    blockDepth++;
    return fixLoc;
}

void GenerationContext::popBlock (int fixLocation, int tempLoc)
{
    genInstruction (DoSpecial, StackReturn);
    meth.code[fixLocation] = meth.code.size ();
    restoreOldTop (tempLoc);
    blockDepth--;
}

void GenerationContext::beginLiteralArray ()
{
    literalArrayStack.emplace ();
}

int GenerationContext::endLiteralArray ()
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

    return numLit;
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
    /*printf ("(gen-literal newnum: %d objRef: %p\n",
            literalArrayStack.top ().size (),
            aLiteral);*/
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
                printf ("Generate unary send for message %s\n",
                        selector.c_str ());
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