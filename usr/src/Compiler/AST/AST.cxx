#include <cassert>
#include <iostream>

#include "AST.hxx"
#include "Scope.hxx"

ClassNode::ClassNode (std::string name, std::string superName,
                      std::vector<std::string> cVars,
                      std::vector<std::string> iVars)
    : name (name), superName (superName), cVars (cVars), iVars (iVars)
{
}

void ClassNode::addMethods (std::vector<MethodNode *> meths)
{
    for (auto m : meths)
        if (m->isClassMethod)
            cMethods.push_back (m);
        else
            iMethods.push_back (m);
}
