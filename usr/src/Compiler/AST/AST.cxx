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

void ProgramNode::addClass (ClassNode * aClass)
{
    classes.push_back (aClass);
}

void ProgramNode::mergeProgram (ProgramNode * aNode)
{
    for (auto c : aNode->classes)
        classes.push_back (c);
}
