#include "AST.hxx"
#include "Expr.hxx"
#include "LiteralExpr.hxx"

void PrimitiveExprNode::print (int in)
{
    std::cout << blanks (in) << "<prim:" << num << ">\n";
    for (auto a : args)
    {
        std::cout << blanks (in + 1) << "<arg>\n";
        a->print (in + 2);
        std::cout << blanks (in + 1) << "</arg>\n";
    }
    std::cout << blanks (in) << "</prim>\n";
}

void IdentExprNode::print (int in)
{
    std::cout << blanks (in) << "<id:" << id << " />\n";
}

void BlockExprNode::print (int in)
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

void ExprStmtNode::print (int in)
{
    std::cout << blanks (in) << "<exprstmt>\n";
    expr->print (in + 1);
    std::cout << blanks (in) << "</exprstmt>\n";
}

void ReturnStmtNode::print (int in)
{
    std::cout << blanks (in) << "<returnstmt> ";
    expr->print (in + 1);
    std::cout << blanks (in) << "</returnstmt>";
}

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

void ProgramNode::print (int in)
{
    std::cout << blanks (in) << "<program>\n";
    for (auto c : classes)
        c->print (in);
    std::cout << blanks (in) << "</program>\n";
}