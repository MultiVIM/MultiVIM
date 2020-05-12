#pragma once

#include "AST.hxx"

struct ExprNode : Node
{
    virtual void synthInScope (AbstractScope * scope) = 0;
    virtual void generateOn (CodeGen & gen) = 0;

    virtual bool isSuper ()
    {
        return false;
    }
};

struct PrimitiveExprNode : ExprNode
{
    int num;
    std::vector<ExprNode *> args;

    PrimitiveExprNode (int num, std::vector<ExprNode *> args)
        : num (num), args (args)
    {
    }

    virtual void print (int in);

    virtual void synthInScope (AbstractScope * scope);
    virtual void generateOn (CodeGen & gen);
};

struct IdentExprNode : ExprNode
{
    std::string id;
    VarNode * var;

    virtual bool isSuper ()
    {
        return id == "super";
    }

    IdentExprNode (std::string id) : id (id), var (NULL)
    {
    }

    virtual void synthInScope (AbstractScope * scope);
    virtual void generateOn (CodeGen & gen);
    virtual void generateAssignOn (CodeGen & gen, ExprNode * rValue);

    void print (int in);
};

struct AssignExprNode : ExprNode
{
    IdentExprNode * left;
    ExprNode * right;

    AssignExprNode (IdentExprNode * l, ExprNode * r) : left (l), right (r)
    {
    }

    virtual void synthInScope (AbstractScope * scope);
    virtual void generateOn (CodeGen & gen);

    void print (int in)
    {
        std::cout << blanks (in) << "<assign>\n";
        std::cout << blanks (in + 1) << "<left: " << left->id << ">\n";
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
    std::vector<ExprNode *> args;

    MessageExprNode (ExprNode * receiver, std::string selector,
                     std::vector<ExprNode *> args = {})
        : receiver (receiver), selector (selector), args (args)
    {
    }

    MessageExprNode (ExprNode * receiver,
                     std::vector<std::pair<std::string, ExprNode *>> list)
        : receiver (receiver)
    {
        for (auto & p : list)
        {
            selector += p.first;
            args.push_back (p.second);
        }
    }

    virtual void synthInScope (AbstractScope * scope);
    virtual void generateOn (CodeGen & gen)
    {
        generateOn (gen, false);
    }
    virtual void generateOn (CodeGen & gen, bool cascade);

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
    std::vector<MessageExprNode *> messages;

    CascadeExprNode (ExprNode * r) : receiver (r)
    {
        MessageExprNode * m = dynamic_cast<MessageExprNode *> (r);
        if (m)
        {
            receiver = m->receiver;
            messages.push_back (m);
        }
    }

    virtual void synthInScope (AbstractScope * scope);
    virtual void generateOn (CodeGen & gen);

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

struct BlockExprNode : ExprNode
{
    BlockScope * scope;
    std::vector<std::string> args;
    std::vector<StmtNode *> stmts;

    BlockExprNode (std::vector<std::string> a, std::vector<StmtNode *> s)
        : args (a), stmts (s)
    {
    }

    virtual void synthInScope (AbstractScope * parentScope);
    virtual void generateOn (CodeGen & gen);

    void print (int in);
};