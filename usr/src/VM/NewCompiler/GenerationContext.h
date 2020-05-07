#pragma once

#include <string>

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

    int intValue = 0;
    std::string stringValue;
};

class MVST_Parser : public lemon_base<Token>
{
  protected:
    std::string fName;
    std::string & fText;
    // Unit * m_mod;
    int m_line = 0, m_col = 0, m_pos = 0;
    int m_oldLine = 0, m_oldCol = 0, m_oldPos = 0;

  public:
    int line () const;
    int col () const;
    int pos1 () const;
    using lemon_base::parse;

    static void parseFile (std::string fName);
    static MVST_Parser * create (std::string fName, std::string & fText);

    MVST_Parser (std::string f, std::string & ft) : fName (f), fText (ft)
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

    /* misc */
    /*Unit * mod ()
    {
        return m_mod;
    }*/

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

inline int MVST_Parser::line () const
{
    return m_line;
}

inline int MVST_Parser::col () const
{
    return m_col;
}

inline int MVST_Parser::pos1 () const
{
    return m_pos;
}