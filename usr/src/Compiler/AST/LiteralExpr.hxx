#include "VM/AST.hxx"

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
};

/* Symbol literal */
struct SymbolExprNode : ExprNode
{
    std::string sym;

    SymbolExprNode (std::string aSymbol) : sym (aSymbol)
    {
    }
};

/* Integer literal */
struct IntExprNode : ExprNode
{
    int num;

    IntExprNode (int aNum) : num (aNum)
    {
    }
};

/* String literal */
struct StringExprNode : LiteralExprNode
{
    std::string str;

    StringExprNode (std::string aString) : str (aString)
    {
    }
};

struct ArrayExprNode : LiteralExprNode
{
    std::list<ExprNode *> elements;

    ArrayExprNode (std::list<ExprNode *> exprs) : elements (exprs)
    {
    }
};