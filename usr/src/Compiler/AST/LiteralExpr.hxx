#include "Expr.hxx"

struct LiteralExprNode : ExprNode
{
    virtual void synthInScope (AbstractScope * parentScope)
    {
    }
};

/* Character literal */
struct CharExprNode : LiteralExprNode
{
    std::string khar;

    CharExprNode (std::string aChar) : khar (aChar)
    {
    }

    virtual void generateOn (CodeGen & gen);
};

/* Symbol literal */
struct SymbolExprNode : LiteralExprNode
{
    std::string sym;

    SymbolExprNode (std::string aSymbol) : sym (aSymbol)
    {
    }

    virtual void generateOn (CodeGen & gen);
};

/* Integer literal */
struct IntExprNode : LiteralExprNode
{
    int num;

    IntExprNode (int aNum) : num (aNum)
    {
    }

    virtual void generateOn (CodeGen & gen);
};

/* String literal */
struct StringExprNode : LiteralExprNode
{
    std::string str;

    StringExprNode (std::string aString) : str (aString)
    {
    }

    virtual void generateOn (CodeGen & gen);
};

/* Integer literal */
struct FloatExprNode : LiteralExprNode
{
    double num;

    FloatExprNode (double aNum) : num (aNum)
    {
    }

    virtual void generateOn (CodeGen & gen);
};

struct ArrayExprNode : LiteralExprNode
{
    std::vector<ExprNode *> elements;

    ArrayExprNode (std::vector<ExprNode *> exprs) : elements (exprs)
    {
    }

    virtual void generateOn (CodeGen & gen);
};