#include "AST.hxx"

struct LiteralExprNode : ExprNode
{
};

/* Character literal */
struct CharExprNode : ExprNode
{
    std::string khar;

    CharExprNode (std::string aChar) : khar (aChar)
    {
    }

    virtual void generateInContext (GenerationContext * aContext);
};

/* Symbol literal */
struct SymbolExprNode : ExprNode
{
    std::string sym;

    SymbolExprNode (std::string aSymbol) : sym (aSymbol)
    {
    }

    virtual void generateInContext (GenerationContext * aContext);
};

/* Integer literal */
struct IntExprNode : ExprNode
{
    int num;

    IntExprNode (int aNum) : num (aNum)
    {
    }

    virtual void generateInContext (GenerationContext * aContext);
};

/* String literal */
struct StringExprNode : LiteralExprNode
{
    std::string str;

    StringExprNode (std::string aString) : str (aString)
    {
    }

    virtual void generateInContext (GenerationContext * aContext);
};

/* Integer literal */
struct FloatExprNode : ExprNode
{
    double num;

    FloatExprNode (double aNum) : num (aNum)
    {
    }

    virtual void generateInContext (GenerationContext * aContext);
};

struct ArrayExprNode : LiteralExprNode
{
    std::list<ExprNode *> elements;

    ArrayExprNode (std::list<ExprNode *> exprs) : elements (exprs)
    {
    }

    virtual void generateInContext (GenerationContext * aContext);
};