#include <cassert>
#include <iostream>

#include "AST.hxx"
#include "OldVM/Bytecode.hxx"
#include "OldVM/VM.hxx"

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

void ProgramNode::addClass (ClassNode * aClass)
{
    classes.push_back (aClass);
}

void ProgramNode::mergeProgram (ProgramNode * aNode)
{
    std::cout << " My classes before\n";
    for (auto c : classes)
        std::cout << "My class: " << c->name << "\n";
    for (auto c : aNode->classes)
        classes.push_back (c);

    for (auto c : classes)
        std::cout << "My now class: " << c->name << "\n";
}

void ProgramNode::print (int in)
{
    std::cout << blanks (in) << "<program>\n";
    for (auto c : classes)
        c->print (in);
    std::cout << blanks (in) << "</program>\n";
}

GlobalVarNode::GlobalVarNode (ClassNode * _class)
    : VarNode (kGlobal, _class->name), _class (_class)
{
}

ClassNode * GlobalVarNode::getClass ()
{
    return _class;
}
