#pragma once

#include <iostream>
#include <list>
#include <map>

#include "../Compiler.hxx"
#include "Lowlevel/MVPrinting.hxx"

class CodeGen;
class CompilationContext;
class ExprNode;
struct VarNode;
struct AbstractScope;
struct BlockScope;
struct MethodScope;
struct ClassScope;

struct Node
{
    virtual void print (int in)
    {
        std::cout << blanks (in) << "<node: " << typeid (*this).name ()
                  << "/>\n";
    }
};

struct StmtNode : Node
{
    virtual void synthInScope (AbstractScope * scope) = 0;
    virtual void generateOn (CodeGen & gen) = 0;
};

struct ExprStmtNode : StmtNode
{
    ExprNode * expr;

    ExprStmtNode (ExprNode * e) : expr (e)
    {
    }

    virtual void synthInScope (AbstractScope * scope);
    virtual void generateOn (CodeGen & gen);

    virtual void print (int in);
};

struct ReturnStmtNode : StmtNode
{
    ExprNode * expr;

    ReturnStmtNode (ExprNode * e) : expr (e)
    {
    }

    virtual void synthInScope (AbstractScope * scope);
    virtual void generateOn (CodeGen & gen);

    virtual void print (int in);
};

struct DeclNode : Node
{
    virtual void synthInNamespace (DictionaryOop ns) = 0;
    virtual void generate () = 0;
};

struct MethodNode : public Node
{
    MethodScope * scope;

    bool isClassMethod;
    std::string sel;
    std::vector<std::string> args;
    std::vector<std::string> locals;
    std::vector<StmtNode *> stmts;

    MethodNode (bool isClassMethod, std::string sel,
                std::vector<std::string> args, std::vector<std::string> locals,
                std::vector<StmtNode *> stmts)
        : isClassMethod (isClassMethod), sel (sel), args (args),
          locals (locals), stmts (stmts)
    {
    }

    MethodNode * synthInClassScope (ClassScope * clsScope);
    MethodOop generate ();

    void print (int in);
};

struct ClassNode : public DeclNode
{
    ClassScope * scope;
    ClassOop cls;

    std::string name;
    std::string superName;
    std::vector<std::string> cVars;
    std::vector<std::string> iVars;
    std::vector<MethodNode *> cMethods;
    std::vector<MethodNode *> iMethods;

    /* Resolved later */
    // GlobalVarNode * superClass;

    ClassNode (std::string name, std::string superName,
               std::vector<std::string> cVars, std::vector<std::string> iVars);

    void addMethods (std::vector<MethodNode *> meths);

    void synthInNamespace (DictionaryOop ns);
    void generate ();

    void print (int in);
};

struct NamespaceNode : public DeclNode
{
    std::string name;
    // std::vector<std:
    std::vector<DeclNode *> decls;

    NamespaceNode (std::string name, std::vector<DeclNode *> decls)
        : name (name), decls (decls)
    {
    }

    void synthInNamespace (DictionaryOop ns);
    void generate ();
};

struct ProgramNode : public DeclNode
{
    std::vector<DeclNode *> decls;

    ProgramNode (std::vector<DeclNode *> decls) : decls (decls)
    {
    }

    void synthInNamespace (DictionaryOop ns);
    void synth ()
    {
        synthInNamespace (memMgr.objGlobals);
    }
    void generate ();

    void print (int in);
};