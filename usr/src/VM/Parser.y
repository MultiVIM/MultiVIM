/*
 * Copyright 2020 David MacKay.  All rights reserved.
 * Use is subject to license terms.
 */

%include {
    #include <cstdlib>
	#include <iostream>
	#include <fstream>
	#include <list>

    #include "VM/NewCompiler.h"
	#include "VM/AST.hxx"
	#include "Parser.tab.h"
	#include "Lexer.l.h"

    #define LEMON_SUPER MVST_Parser
}

%token_type    {Token}
%token_prefix  TOK_

%code {

ProgramNode * MVST_Parser::parseFile (std::string fName)
{
    MVST_Parser *parser;
    std::string src;
    std::ifstream f;

    yyscan_t scanner;
    YY_BUFFER_STATE yyb;

    f.exceptions(std::ios::failbit | std::ios::badbit);

    try
    {
        f.open(fName);

        f.seekg(0, std::ios::end);
        src.reserve(f.tellg());
        f.seekg(0, std::ios::beg);

        src.assign((std::istreambuf_iterator<char>(f)),
                   std::istreambuf_iterator<char>());
    }

    catch (std::ios_base::failure &e)
    {
        std::cerr << "PDST: Error: File " + fName + " failed:\n\t" +
                         e.code().message() + "\n";
    }

    parser = MVST_Parser::create(fName, src);
    if (1)
        parser->trace(stdout, "<parser>: ");

    mvstlex_init_extra(parser, &scanner);
    /* Now we need to scan our string into the buffer. */
    yyb = mvst_scan_string(src.c_str(), scanner);

    // parser->parse(isCls ? TOK_CLASS : TOK_BAS);

    while (mvstlex(scanner))
        ;
    parser->parse(TOK_EOF);

	return parser->program;
}

MVST_Parser * MVST_Parser::create(std::string fName, std::string & fText)
{
	return new yypParser(fName, fText);
}

Position MVST_Parser::pos()
{
	yypParser *self = (yypParser *)this;
	return Position(self->m_oldLine, self->m_oldCol, self->m_oldPos, self->m_line, self->m_col, self->m_pos);
}

}

%syntax_error {
	const YYACTIONTYPE stateno = yytos->stateno;
	size_t eolPos = fText.find("\n", m_pos);
	std::string eLine = fText.substr(m_pos, eolPos - m_pos);
	size_t i;

	std::cerr << "PDST: " << fName << "(" << std::to_string(m_line) + "," 
			  << std::to_string(m_col) << "): "
			  << "Error V1001: Syntax error: unexpected " 
			  << yyTokenName[yymajor] << "\n";

	std::cerr << "+ " << eLine << "\n";
	std::cerr << "+ ";
	for (i = 0; i < m_oldCol; i++)
		std::cerr << " ";
	for (; i < m_col; i++)
		std::cerr << "^";

	std::cerr << "\n\texpected one of: \n";

	for (unsigned i = 0; i < YYNTOKEN; ++i)
	{
		int yyact = yy_find_shift_action(i, stateno);
		if (yyact != YY_ERROR_ACTION && yyact != YY_NO_ACTION)
			std::cerr << "\t" << yyTokenName[i] << "\n";
	}

}

file ::= lDecl(l) EOF.
	{
		for (auto d: l)
			printf("ADecl\n");
	}

%type lDecl { std::list<DeclNode *> }
%type decl { DeclNode * }

lDecl(L) ::= decl(d). { L = {d}; }
lDecl(L) ::= lDecl(l) decl(d).
	{
		L = l;
		L.push_back(d);
	}

decl(D) ::= identifier(super) SUBCLASSCOLON identifier(name)
	SQB_OPEN 
		oIvarDefs(iVars)
		olMethDecl(iMeths)
	SQB_CLOSE.
	{
		ClassNode * cls = new ClassNode(name, super, {}, iVars);
		cls->iMethods = iMeths;
		D = cls;
		cls->print(0);
		if (!program) program = new ProgramNode();
		program->addClass(cls);
	}

%type oIvarDefs { std::list<std::string> }
%type lIvar { std::list<std::string> }

oIvarDefs(L) ::= BAR lIvar(l) BAR. { L = l; }
oIvarDefs ::= .

lIvar(L) ::= IDENTIFIER(i). { L = {i}; }
lIvar(L) ::= lIvar(l) IDENTIFIER(i).
	{
		L = l;
		L.push_back(i); 
	}

%type olMethDecl { std::list<MethodNode *> }
%type lMethDecl { std::list<MethodNode *> }
%type methDecl { MethodNode * }

olMethDecl(L) ::= lMethDecl(l). { L = l; }
olMethDecl ::= .

lMethDecl(L) ::= methDecl(d). { L = {d}; }
lMethDecl(L) ::= lMethDecl(l) methDecl(d).
	{
		L = l;
		L.push_back(d); 
	}

methDecl(D) ::= sel_decl(s)
	SQB_OPEN 
		oIvarDefs(locals)
		statementList(stmts)
	SQB_CLOSE.
	{
		D = new MethodNode(s.first, s.second, locals, stmts);
	}

%type statementList { std::list<StmtNode *> }
%type statement { StmtNode * }

statementList(L) ::= statement(s). { L = {s}; }
statementList(L) ::= statementList(l) DOT statement(s).
	{
		L = l;
		L.push_back(s); 
	}

statement(S) ::= UP sExpression(e). { S = new ReturnStmtNode(e); }
statement(S) ::= sExpression(e). { S = new ExprStmtNode(e); }

%type sExpression { ExprNode * }
%type cExpression { ExprNode * }
%type kContinuation { CascadeExprNode * }
%type bContinuation { CascadeExprNode * }
%type uContinuation { CascadeExprNode * }
%type expression { ExprNode * }

sExpression(E) ::= identifier(i) ASSIGN expression(e).
	{
		E = new AssignExprNode(i, e);
	}
sExpression(E) ::= cExpression(e). { E = e;}

cExpression(C) ::= expression(e). { C = e; }
cExpression(C) ::= kContinuation(e). { C = e; }

kContinuation(C) ::= bContinuation(c).
	{
		C = c;
	}
kContinuation(C) ::= bContinuation(c) keywordList(l).
	{
		C = c;
		C->messages.push_back(
			new MessageExprNode(C->receiver, l));
	}

bContinuation(C) ::= uContinuation(c). { C = c; }
bContinuation(C) ::= bContinuation(c) binOp(s) unary(e).
	{
		C = c;
		C->messages.push_back(new MessageExprNode(C->receiver, s, {e}));
	}


uContinuation(C) ::= cExpression(r) SEMICOLON.
	{
		/* FIXME: To avoid false positives, put bracketed expressions in their
		 * own thing. */
        CascadeExprNode * c = dynamic_cast<CascadeExprNode *> (r);
        C = c ? c : new CascadeExprNode (r);	
	}
uContinuation(C) ::= uContinuation(c) identifier(i).
	{
		C = c;
		C->messages.push_back(new MessageExprNode(C->receiver, i));
	}

expression(E) ::= binary(b). { E = b; }
expression(E) ::= binary(e) keywordList(k).
	{
		E = new MessageExprNode(e, k);
	}

%type keywordList { std::list<std::pair<std::string, ExprNode *>> }

keywordList(L) ::= keyword(k) binary(e).
	{
		L = { {k, e}};
	}
keywordList(L) ::= keywordList(l) keyword(k) binary(e).
	{
		(L = l).push_back( { k, e });
	}

%type binary { ExprNode * }
%type unary { ExprNode * }
%type primary { ExprNode * }

binary(E) ::= unary(e). { E = e; }
binary(E) ::= binary(r) binOp(s) unary(a).
	{
		E  = new MessageExprNode(r, s, { a }); 
	}

unary(E) ::= primary(e). { E = e; }
unary(U) ::= primary(p) identifier(i). { U  = new MessageExprNode(p, i); }

primary(S) ::= identifier(i). { S = new IdentExprNode (i); }
primary(S) ::= LBRACKET sExpression(s) RBRACKET. { S = s; }
primary ::= block.
primary ::= SYMBOL.
primary ::= INTEGER.
block ::= SQB_OPEN oBlockVarList statementList SQB_CLOSE.

%type oBlockVarList { std::list<std::string> }
%type colonVarList { std::list<std::string> }

oBlockVarList(L) ::= colonVarList(l) BAR. { L = l; }
oBlockVarList ::= .

colonVarList(L) ::= COLONVAR(v). { L = {v}; } 
colonVarList(L) ::= colonVarList(l) COLONVAR(v).
	{
		L = l;
		L.push_back(v);
	}

%type sel_decl { std::pair<std::string, std::list<std::string>> }

sel_decl(S) ::= identifier(i). { S = {i, {}};  }
sel_decl(S)
	::= binary_decl(b).
	{ 
		S = {b.first, {b.second}};
	}
sel_decl(S)
	::= keyw_decl_list(k).
	{
		S = k;
	}

%type keyw_decl_list { std::pair<std::string, std::list<std::string>> }

keyw_decl_list(L) ::= keyw_decl(k). {
    L = {k.first, {k.second}};
}
keyw_decl_list(L) ::= keyw_decl_list(l) keyw_decl(k). { 
    L = l;
	L.first += k.first;
	L.second.push_back(k.second);
}

%type keyw_decl { std::pair<std::string, std::string> }
%type binary_decl { std::pair<std::string, std::string> }

keyw_decl(K) ::= keyword(k) identifier(s). {
    K = {k, s};
}

binary_decl(B) ::= binOp(b) identifier(s). {
    B = {b, s};
}

identifier(I) ::= IDENTIFIER(i). { I = i; }
identifier(I) ::= NAMESPACENAME(i). { I = i; }
keyword(K) ::= KEYWORD(k). { K = k ;}
binOp(B) ::= BINARY(b). { B = b; }