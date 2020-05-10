#pragma once

#include <list>
#include <stack>
#include <string>
#include <vector>

#include "OldVM/VM.hxx"
#include "lemon/lemon_base.h"

/* Details of the position of some source code. */
class Position
{
    size_t m_oldLine, m_oldCol, m_oldPos;
    size_t m_line, m_col, m_pos;

  public:
    Position (size_t oldLine, size_t oldCol, size_t oldPos, size_t line,
              size_t col, size_t pos)
        : m_oldLine (oldLine), m_oldCol (oldCol), m_oldPos (oldPos),
          m_line (line), m_col (col), m_pos (pos)
    {
    }

    /* Get line number */
    size_t line () const;
    /* Get column number*/
    size_t col () const;
    /* Get absolute position in source-file */
    size_t pos () const;
};

inline size_t Position::line () const
{
    return m_line;
}

inline size_t Position::col () const
{
    return m_col;
}

inline size_t Position::pos () const
{
    return m_pos;
}

struct Token
{
    Token () = default;
    Token (const Token &) = default;
    Token (Token &&) = default;

    Token (double f) : floatValue (f)
    {
    }
    Token (int i) : intValue (i)
    {
    }
    Token (const std::string & s) : stringValue (s)
    {
    }
    Token (std::string && s) : stringValue (std::move (s))
    {
    }

    Token & operator= (const Token &) = default;
    Token & operator= (Token &&) = default;

    operator std::string () const
    {
        return stringValue;
    }
    operator const char * () const
    {
        return stringValue.c_str ();
    }
    operator double () const
    {
        return floatValue;
    }

    double floatValue = 0.0;
    int intValue = 0;
    std::string stringValue;
};

class ClassNode;
struct NameScope;
class VarNode;
class GlobalVarNode;

struct CompiledMethod
{
    std::vector<byte> code;
};

class GenerationContext
{
    std::vector<NameScope *> scopes;
    /* On beginning array generation, the size of the literals vector is pushed
     * here. After completing array generation the new literals added are
     * converted into an array. */
    std::stack<std::vector<objRef>> literalArrayStack;
    int blockDepth;
    int highestLocal;
    CompiledMethod meth;

  public:
    GenerationContext () : highestLocal (0), blockDepth (0)
    {
    }

    GlobalVarNode * addClass (ClassNode * aClass);
    void addIvar (int index, std::string name);
    void addArg (int index, std::string name);
    void addLocal (int index, std::string name);
    void pushScope (NameScope * scope);
    void popScope ();
    GlobalVarNode * lookupClass (std::string name);
    VarNode * lookup (std::string name);

    void beginMethod ();
    /* code, literals */
    std::pair<std::vector<byte>, std::vector<objRef>> endMethod ();

    int localTop ()
    {
        return highestLocal;
    }

    int codeTop ()
    {
        return meth.code.size ();
    }

    void setCodeTo (int index, int val)
    {
        meth.code[index] = val;
    }

    void restoreOldTop (int oldTop);

    void generateClasses ();

    /* What are we doing here? Do we want to keep this state in this big class
     * or make new ClassBuilders and whatever else we need instead? */
    encPtr defClass (GlobalVarNode * classVar, std::string name,
                     std::list<std::string> iVars, size_t size);

    void addMethodToClass (encPtr aClass, encPtr aMethod);
    void addMethodToMetaclassOf (encPtr aClass, encPtr aMethod);

    /* returns fixLocation */
    int pushBlock (int argCount, int tempLoc);
    bool inBlock ()
    {
        return blockDepth;
    }
    void popBlock (int fixLocation, int tempLoc);

    void beginLiteralArray ();
    bool inLiteralArray ()
    {
        return literalArrayStack.size () > 1;
    }
    int endLiteralArray ();

    void genCode (int value);
    void genInstruction (int high, int low);
    int genLiteral (objRef aLiteral);
    void genInteger (int val);
    void genMessage (bool isSuper, int argumentCount, std::string selector);
};

class ProgramNode;

class MVST_Parser : public lemon_base<Token>
{
  protected:
    std::string fName;
    std::string path;
    std::string & fText;
    ProgramNode * program;
    int m_line = 0, m_col = 0, m_pos = 0;
    int m_oldLine = 0, m_oldCol = 0, m_oldPos = 0;

  public:
    using lemon_base::parse;

    int line () const
    {
        return m_line;
    }
    int col () const
    {
        return m_col;
    }
    int pos1 () const
    {
        return m_pos;
    }

    static ProgramNode * parseFile (std::string fName);
    static MVST_Parser * create (std::string fName, std::string & fText);

    MVST_Parser (std::string f, std::string & ft)
        : fName (f), fText (ft), program (nullptr)
    {
    }

    /* parsing */
    void parse (int major)
    {
        parse (major, Token{});
    }

    template <class T> void parse (int major, T && t)
    {
        parse (major, Token (std::forward<T> (t)));
    }

    virtual void trace (FILE *, const char *) = 0;

    /* line tracking */
    Position pos ();

    void recOldPos ()
    {
        m_oldPos = m_pos;
        m_oldLine = m_line;
        m_oldCol = m_col;
    }

    void cr ()
    {
        m_pos += m_col + 1;
        m_line++;
        m_col = 0;
    }
    void incCol ()
    {
        m_col++;
    }
};
