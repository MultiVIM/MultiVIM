#pragma once

#include <iostream>
#include <list>
#include <map>

#include "NewCompiler.h"
#include "VM.hxx"

class CompilationContext;

static inline std::string blanks (size_t n)
{
    return std::string (n, ' ');
}

struct Node
{
    virtual void generateInContext (GenerationContext * aContext)
    {
        std::cout << "CAN'T GENERATE\n";
    }

    virtual void print (int in)
    {
        std::cout << blanks (in) << "<undefinednode />";
    }
};

struct VarNode : Node
{
    enum Kind
    {
        kSpecial,
        kLocal,
        kArgument,
        kInstance,
        kGlobalConstant,
        kGlobal,
    } kind;

    std::string name;

    VarNode (Kind kind, std::string name) : kind (kind), name (name)
    {
    }
};

struct SpecialVarNode : VarNode
{
    SpecialVarNode (std::string name) : VarNode (kSpecial, name)
    {
    }
};

struct LocalVarNode : VarNode
{
    int index;
    LocalVarNode (int index, std::string name)
        : index (index), VarNode (kLocal, name)
    {
    }
};

struct ArgumentVarNode : VarNode
{
    int index;
    ArgumentVarNode (int index, std::string name)
        : index (index), VarNode (kLocal, name)
    {
    }
};

struct InstanceVarNode : VarNode
{
    int index;
    InstanceVarNode (int idx, std::string name)
        : VarNode (kInstance, name), index (idx)
    {
    }
};

struct GlobalVarNode : VarNode
{
    ClassNode * _class;
    encPtr classObj, metaClassObj;

    GlobalVarNode (ClassNode * _class);

    ClassNode * getClass ();
};

struct NameScope
{
    NameScope * parent;
    std::map<std::string, VarNode *> vars;

    GlobalVarNode * addClass (ClassNode * aClass);
    void addIvar (int idx, std::string name);
    void addLocal (int idx, std::string name);
    void addArg (int idx, std::string name);
    virtual VarNode * lookup (std::string aName);

    void removeLocalsAbove (int i);
};

struct ClassScope : NameScope
{
    GlobalVarNode * classVar;

    ClassScope (GlobalVarNode * klass) : classVar (klass)
    {
    }
};

struct MethodScope : NameScope
{
    MethodScope () : NameScope ()
    {
    }
};

struct BlockScope : NameScope
{
};

struct ExprNode : Node
{
};

struct IdentExprNode : ExprNode
{
    std::string id;

    IdentExprNode (std::string id) : id (id)
    {
    }

    void print (int in)
    {
        std::cout << blanks (in) << "<id:" << id << " />\n";
    }
};

struct AssignExprNode : ExprNode
{
    std::string left;
    ExprNode * right;

    AssignExprNode (std::string l, ExprNode * r) : left (l), right (r)
    {
    }

    void print (int in)
    {
        std::cout << blanks (in) << "<assign>\n";
        std::cout << blanks (in + 1) << "<left: " << left << ">\n";
        std::cout << blanks (in + 1) << "<right>\n";
        right->print (in + 2);
        std::cout << blanks (in + 1) << "</right>\n";
        std::cout << "</assign>";
    }
};

struct MessageExprNode : ExprNode
{
    ExprNode * receiver;
    std::string selector;
    std::list<ExprNode *> args;

    MessageExprNode (ExprNode * receiver, std::string selector,
                     std::list<ExprNode *> args = {})
        : receiver (receiver), selector (selector), args (args)
    {
    }

    MessageExprNode (ExprNode * receiver,
                     std::list<std::pair<std::string, ExprNode *>> list)
        : receiver (receiver)
    {
        for (auto & p : list)
        {
            selector += p.first;
            args.push_back (p.second);
        }
    }

    void print (int in)
    {
        std::cout << blanks (in) << "<message>\n";

        std::cout << blanks (in + 1) << "<receiver>\n";
        receiver->print (in + 2);
        std::cout << blanks (in + 1) << "</receiver>\n";

        std::cout << blanks (in + 1) << "<selector:#" << selector << "\n";

        for (auto e : args)
        {
            std::cout << blanks (in + 1) << "<arg>\n";
            e->print (in + 2);
            std::cout << blanks (in + 1) << "</arg>\n";
        }

        std::cout << blanks (in) << "</message>\n";
    }
};

struct CascadeExprNode : ExprNode
{
    ExprNode * receiver;
    std::list<MessageExprNode *> messages;

    CascadeExprNode (ExprNode * r) : receiver (r)
    {
        MessageExprNode * m = dynamic_cast<MessageExprNode *> (r);
        if (m)
        {
            receiver = m->receiver;
            messages.push_back (m);
        }
    }

    void print (int in)
    {
        std::cout << blanks (in) << "<cascade>\n";

        std::cout << blanks (in + 1) << "<receiver>\n";
        receiver->print (in + 2);
        std::cout << blanks (in + 1) << "</receiver>\n";

        for (auto e : messages)
        {
            e->print (in + 1);
        }

        std::cout << blanks (in) << "</cascade>\n";
    }
};

struct StmtNode : Node
{
};

struct BlockExprNode : ExprNode
{
    std::list<std::string> args;
    std::list<StmtNode *> stmts;

    BlockExprNode (std::list<std::string> a, std::list<StmtNode *> s)
        : args (a), stmts (s)
    {
    }

    virtual void generateInContext (GenerationContext * aContext);

    virtual void print (int in)
    {
        std::cout << "<block>\n";

        std::cout << blanks (in + 1) << "<params>\n";
        for (auto e : args)
            std::cout << blanks (in + 2) << "<param:" << e << " />\n";
        std::cout << blanks (in + 1) << "</params>\n";

        std::cout << blanks (in + 1) << "<statements>\n";
        for (auto e : stmts)
        {
            std::cout << blanks (in + 1) << "<statement>\n";
            e->print (in + 2);
            std::cout << blanks (in + 1) << "</statement>\n";
        }
        std::cout << blanks (in + 1) << "</statements>\n";

        std::cout << "</block>\n";
    }
};

struct ExprStmtNode : StmtNode
{
    ExprNode * expr;

    ExprStmtNode (ExprNode * e) : expr (e)
    {
    }

    virtual void print (int in)
    {
        std::cout << blanks (in) << "<exprstmt>\n";
        expr->print (in + 1);
        std::cout << blanks (in) << "</exprstmt>\n";
    }
};

struct ReturnStmtNode : StmtNode
{
    ExprNode * expr;

    ReturnStmtNode (ExprNode * e) : expr (e)
    {
    }

    virtual void print (int in)
    {
        std::cout << blanks (in) << "<returnstmt> ";
        expr->print (in + 1);
        std::cout << blanks (in) << "</returnstmt>";
    }
};

struct DeclNode : Node
{
};

struct MethodNode : DeclNode
{
    std::string sel;
    std::list<std::string> args;
    std::list<std::string> locals;
    std::list<StmtNode *> stmts;

    MethodNode (std::string sel, std::list<std::string> args,
                std::list<std::string> locals, std::list<StmtNode *> stmts)
        : sel (sel), args (args), locals (locals), stmts (stmts)
    {
    }

    virtual void generateInContext (bool isClassMethod,
                                    GenerationContext * aContext);

    void print (int in);
};

struct ClassNode : DeclNode
{
    std::string name;
    std::string superName;
    std::list<std::string> cVars;
    std::list<std::string> iVars;
    std::list<MethodNode *> cMethods;
    std::list<MethodNode *> iMethods;

    /* Resolved later */
    GlobalVarNode * superClass;

    ClassNode (std::string name, std::string superName,
               std::list<std::string> cVars, std::list<std::string> iVars);

    int addIVarsToContextStartingFrom (GenerationContext * aContext, int index);
    virtual void generateInContext (GlobalVarNode * myClassVar,
                                    GenerationContext * aContext);

    void print (int in);
};

struct ProgramNode : DeclNode
{
    std::list<ClassNode *> classes;

    void addClass (ClassNode * aClass);

    virtual void generateInContext (GenerationContext * aContext);

    void print (int in);
};