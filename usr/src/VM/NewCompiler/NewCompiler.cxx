#include "../NewCompiler.h"
#include "../AST.hxx"

GlobalVarNode * NameScope::addClass (ClassNode * aClass)
{
    auto cls = new GlobalVarNode (aClass);
    vars[aClass->name] = cls;
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
    vars[name] = new InstanceVarNode (idx, name);
}

VarNode * NameScope::lookup (std::string aName)
{
    return vars[aName];
}

void NameScope::removeLocalsAbove (int i)
{
    LocalVarNode * local;
    for (auto c : vars)
    {
        local = dynamic_cast<LocalVarNode *> (c.second);
        if (local)
            vars.erase (c.first);
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