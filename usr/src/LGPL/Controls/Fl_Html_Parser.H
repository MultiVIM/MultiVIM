#ifndef _ci_Html_Tokenizer_H_
#define _ci_Html_Tokenizer_H_

#include <algorithm>
#include <cstdio>
#include <stack>
#include <typeinfo>
#include <vector>

class Fl_Html_Pair_Table
{
  public:
    struct Entry
    {
        const char * name;
        const void * value;
    };

  protected:
    Entry * table;
    int n;
    int table_size_;
    bool static_table;
    void resize_buffer (unsigned size);

    Fl_Html_Pair_Table (Entry * const table, int filled,
                        int table_size); // constructor based on a static table
    Fl_Html_Pair_Table (const Fl_Html_Pair_Table & table); // copy constructor
    Fl_Html_Pair_Table (int size = 64);
    virtual ~Fl_Html_Pair_Table ();

    // Adds a new pair.
    // The name is a special character string WITNOUT '&' but with terminating
    // ';'. Both must be null-terminated strings, \a value corresponds to utf-8
    // representation of the special character sequence. During look-up the
    // last-added characters are searched first so you can override existing
    // table entry. To speed-up search add the most common special characters as
    // last. \a value (utf8 sequence) must not be longer than special character
    // sequence, that is (strlen(name) + 1 >= strlen(value))
    // - this is always true as all special characters "&xx;" have at least 4
    // characters and maximum 4 byte utf-8 representation. Both must live for
    // the whole duration of the table use as the strings are not copied.
    void add_ (const char * name, const void * value);

    // Replaces value for the \a name. If it does not exists, it adds the pair.
    // Returns previous value.
    const void * replace_ (const char * name, const void * value);

    // Finds value if the hey is a starting substring of \a src
    // src should point to position AFTER '&' character.
    // Returns number of characters translated (includong terminal ';') so that
    // src can be incremented by this value after the call. Returns 0 if the
    // search fails.
    int find_substring_ (const char * src, const void *& value,
                         int length = -1) const;

    // Similar to above but case-insensitive
    int find_case_substring_ (const char * src, const void *& value,
                              int length = -1) const;

    // find key if key  and src are equal including nul-termination or \a length
    // is equal to key length
    int find_ (const char * src, const void *& value, int length = -1) const;

    // similar to above but case insensitive
    int find_case_ (const char * src, const void *& value,
                    int length = -1) const;

    // int find_case_substring_(const char * src, const void * &value, int
    // length = -1) const;

    // Gives string where all special characters are translated (inclusing
    // "numeric" special characters). The fuction might return in \a value the
    // same pointer as src (no translation required) and might not be
    // null-terminated. The \a buffer, if provided (non-zero), must be created
    // with operator "new" with length at least of \a buffer_len. If longer
    // buffer is required for translation, buffer will be deleted and replaced
    // with a new one and buffer_len will be updated. After the use the buffer
    // can be safely destructed with operator delete[]. Note that because
    // translated string is always shorter than ahe original user can avoud
    // allocation of the buffer byst by supplying buffer_len >= src_len. User
    // can also perform "in-place" translation by suplying the same string
    // pointer as \a src and \a buffer - which will also be filled in the \a
    // value reference parameter. The function returns the length of the
    // translated string.
};

// This is fast, imple tokenizer of the html strings
// Its operator () should be called as long as its result is positive.
// After each call of the operator its return value indicates result type,
// (usually TAG_NAME, ATTRIBUTE_NAME, ATTRIVUTE_VALUE, END_TAG
// BODY, COMMENT or 0-FINISH) which value can be retreieved using value()
// and value_length() methods. For all available types see Result enumeration.

// The tokeniser is quite forgiving (eg unlike xml-parsers it accepts whitespace
// at various places like surrounding attribute "=") and attribute values can be
// unquoted if they do not contain whitespaces.

class Fl_Css_Tokenizer
{
    const char * value_;
    int value_length_;
    const char * selector_;
    const char * str_;
    int len_;
    int result_;

  public:
    enum Result
    {
        FINISH = 0,   // end of the string
        SELECTOR = 1, // value() returns the selector string
        PROPERTY = 2, // returns the property
        VALUE = 3,    // returns the value
        COMMA =
            4, // Returns property value sepparated from previous value by comma
        COMMA_SELECTOR = COMMA | SELECTOR,
        // COMMA_PROPERTY = COMMA|PROPERTY,
        COMMA_VALUE = COMMA | VALUE,
        INVALID_SEPARATOR = -1
    };
    const char * value () const
    {
        return value_;
    }
    int value_length () const
    {
        return value_length_;
    }
    void reset (const char * str, int len = -1, int in_field = SELECTOR)
    {
        str_ = str;
        len_ = len;
        result_ = in_field;
    }

    Result operator() ();
    Fl_Css_Tokenizer (const char * str = 0, int len = -1,
                      int in_field = SELECTOR)
    {
        reset (str, len, in_field);
    }
    virtual ~Fl_Css_Tokenizer ()
    {
    }
};

class Fl_Css_Parser : public Fl_Css_Tokenizer
{

  public:
    class Property
    {
        const char * property_;
        int property_length_;
        int size_;
        const char ** values_;
        int * values_length_;
        bool * commas_;
        int no_values_;

      public:
        const char * property () const
        {
            return property_;
        }
        int property_length () const
        {
            return property_length_;
        }
        void property (const char * p, int length)
        {
            property_ = p;
            property_length_ = length;
        }
        const char ** values () const
        {
            return values_;
        }
        int * values_lengths () const
        {
            return values_length_;
        }
        bool * commas () const
        {
            return commas_;
        }
        int no_values () const
        {
            return no_values_;
        }
        void clear (const char * property = 0, int property_length = 0)
        {
            property_ = property;
            property_length_ = property_length;
            no_values_ = 0;
        }
        void add (const char * value, int value_length, bool comma);
        Property (const char * property = 0, int size = 256)
            : property_ (property), size_ (size), no_values_ (0)
        {
            values_ = new const char *[size];
            commas_ = new bool[size];
            values_length_ = new int[size];
        }
        virtual ~Property ()
        {
            delete[] values_;
            delete[] commas_;
        }
    };

    void parse_inline (const char * str, int str_len,
                       void (*callback) (Property & p, void *), Property & p,
                       void * data);
};

class Fl_Html_Tokenizer
{

  public:
    enum Result
    {
        FINISH = 0,
        TAG_NAME = 1,
        ATTRIBUTE_NAME = 2,
        ATTRIBUTE_VALUE = 3,
        END_TAG = 4,
        END_SELF_CLOSED_TAG = 5, // indicates end of self-closed tag < ....  />
        BODY = 6,
        COMMENT = 7,
        CDATA = 8,
        QUESTION_COMMENT = 9,
        UNKNOWN_COMMENT = 10, // this is usually <!DOCTYPE ... > but can be also
                              // other <!XXX  ... >

        // Errors:
        BAD_TAG_NAME = -1,
        BAD_ATTRIBUTE_NAME = -2,
        BAD_ATTRIBUTE_VALUE = -3,
        EOF_IN_TAG = -4,
        EOF_IN_COMMENT = -5
    };

  private:
    int in_tag;
    const char * val;
    const char * pos;
    int val_len;

    char skip_spaces ();
    Result read_quoted_value (char quote);
    Result read_unquoted_value (Result ret);

    // This is called only after opening '<' or when in_tag is not zero
    // to read value of next token.
    // Returns either TAG_NAME, ATTRIBUTE_NAME, ATTRIBUTE_VALUE or error.
    Result read_value ();

    // pos is at first character behind "<!", c is the character from that
    // position.
    Result read_unknown_comment (char c);
    // use only wnhen string at pos-1 is "<?"
    Result read_question_comment ();
    // Use only wnhen string at pos-1 is "<!", pos is at "!"
    Result read_comment ();

    // reads the body between tags
    Result read_body ();

  public:
    // This is a helper class to translate html string with special characters
    // to "normal" utf-8 encoded string
    class Special_Character_Table : public Fl_Html_Pair_Table
    {
        /*
      public:
        struct Special_Character_Entry {
          const char * name;
          const char * value;
        };
      private:
        Special_Character_Entry * table;
        int n;
        int table_size_;
        bool static_table;
        void resize_buffer(unsigned size);
      */
      public:
        Special_Character_Table (Fl_Html_Pair_Table::Entry * const table,
                                 int filled, int table_size)
            : Fl_Html_Pair_Table (table, filled, table_size)
        {
        }
        Special_Character_Table (const Special_Character_Table & table)
            : Fl_Html_Pair_Table (table){}; // copy constructor
        Special_Character_Table (int size = 64) : Fl_Html_Pair_Table (size){};
        //~Special_Character_Table();

        // Returns default (more-less complete) default special character table.
        static Special_Character_Table * default_table ();

        // Adds a new pair.
        // The name is a special character string WITNOUT '&' but with
        // terminating ';'. Both must be null-terminated strings, \a value
        // corresponds to utf-8 representation of the special character
        // sequence. During look-up the last-added characters are searched first
        // so you can override existing table entry. To speed-up search add the
        // most common special characters as last. \a value (utf8 sequence) must
        // not be longer than special character sequence, that is (strlen(name)
        // + 1 >= strlen(value))
        // - this is always true as all special characters "&xx;" have at least
        // 4 characters and maximum 4 byte utf-8 representation. Both must live
        // for the whole duration of the table use as the strings are not
        // copied.
        void add (const char * name, const char * value)
        {
            add_ (name, value);
        };

        // Replaces value for the \a name special character. If it does not
        // exists, it adds the pair. Returns previous value.
        const char * replace (const char * name, const char * value)
        {
            return (const char *)replace_ (name, value);
        }

        // Finds utf-8 translated string  in "value" parameter.
        // src should point to position AFTER '&' character.
        // Returns number of characters translated (includong terminal ';') so
        // that src can be incremented by this value after the call. Returns 0
        // if the search fails.
        int find (const char * src, const char *& value, int length = -1) const
        {
            return find_substring_ (src, (const void *&)value, length);
        }

        // Gives string where all special characters are translated (inclusing
        // "numeric" special characters). The fuction might return in \a value
        // the same pointer as src (no translation required) and might not be
        // null-terminated. The \a buffer, if provided (non-zero), must be
        // created with operator "new" with length at least of \a buffer_len. If
        // longer buffer is required for translation, buffer will be deleted and
        // replaced with a new one and buffer_len will be updated. After the use
        // the buffer can be safely destructed with operator delete[]. Note that
        // because translated string is always shorter than ahe original user
        // can avoud allocation of the buffer byst by supplying buffer_len >=
        // src_len. User can also perform "in-place" translation by suplying the
        // same string pointer as \a src and \a buffer - which will also be
        // filled in the \a value reference parameter. The function returns the
        // length of the translated string.

        int translate (const char * src, unsigned src_len, const char *& value,
                       char *& buffer, unsigned & buffer_len);

        // Similar as above but it ALWAYS allocates new array in \a vaue where
        // translated string is copied. It should be later freed by the operator
        // delete[]. The function also assures that the string is double
        // null-terminated: this is because the html parser might want to add a
        // space at the end of the wors to separate it from following word. The
        // function rerurns the value equal to the length of the translated
        // string - strlen(value).
        int translate_copy (const char * src, unsigned src_len, char *& value);

        // Translates "in place" possibly modifying the \a src string.
        // This is possible because translated string is always shorter or equal
        // length than the original. Note that the translation might not be
        // null-terminated so always use returned value to get the length of the
        // translation.
        int translate_in_place (char * src, unsigned src_len)
        {
            const char * val;
            return translate (src, src_len, val, src, src_len);
        }
    };

    enum Int_Format
    {
        FORMAT_10 = 0, // always interpret as decacic (that is "010" is "ten"
                       // not "eight" as in octal interpretation)
        FORMAT_8 = 8,  // always interpret (force) as octal even with no leading
                       // characters ( "10" is "eight")
        FORMAT_16 = 16, // always interpret as hexadecimal even with no leading
                        // characters ("10" is " "sixteen")

        FORMAT_0 =
            0x100, // interpret as octal if there is a leading 0, otherwise
                   // treat as decimal (010 is eight but 10 is ten)
        FORMAT_0X = 0x400, // interpret as hexadicimal if there is leading  0x
                           // or 0X otherwise interpret as decimal
        FORMAT_X = 0x800,  // interpret as hexadecimal if there is leading x or
                           // X, otherwise as decimal. Used for encoded special
                           // characters "&#[x]yyyy;"
        FORMAT_HASH = 0x100, // interpretted as hxadecimal if there is leading
                             // '#', othervise as decima.
        FORMAT_ALL = FORMAT_0 | FORMAT_0X | FORMAT_X |
                     FORMAT_HASH // allow all formats. If no leading, characters
                                 // are present it defaults to decimal
    };

    // Gets value from decimal, octal or hexadecimal string.
    // On success, returns number of characters used so that src can be
    // incrementerd by that number. If it fails, \a value is unchanged and
    // function returns 0. \ a format forces function to assume that the string
    // is in particular (base) format - for instance FORMAT_16 asumes that it is
    // in hexadecimal format even without preceeding x or 0x. Formats FORMAT_0 ,
    // FORMAT_0X, FORMAT_X allow - apatr from decadic interpretation - interpret
    // the string as octal or hexadecimal respectively if there is a preceeding
    // "0"(octal), "0X"/"0x" (hexadecimal) or "X"/"x" (hexadecimal) and can be
    // OR-ed (see FORMAT_ALL) to achieve all (or subset) of possible
    // interpretations. The interpretation finishes at first whitespace unless
    // the length is limited  by \a src_len, default value -1 assumes that the
    // string length is unknown but null-terminated.
    static unsigned int_value (const char * src, long long & value,
                               int format = FORMAT_ALL, int src_len = -1);

    // Returns "abs" decadic value in \a value, if there is +/- returns  1/-1 in
    // sing (0 if not thesent
    static unsigned signed_int_value (const char * src, long long & value,
                                      long long & sign, int src_len = -1);

    // Reads floating point value (decadic). If no conversion can be made,
    // returns 0, otherwise returns length of converted part of the string. It
    // is similar to strtod() but it is not locale dependent (that is '.' is
    // ALWAYS assumed as decimal point).
    static unsigned float_value (const char * src, double & value,
                                 int src_len = -1);

    //  Returns strlen(to) if \a to is a substring at the beginning of src,
    //  otherwise returns 0. \a src_len is maximum length of the source string,
    //  -1 indicates that it is unknown but null-terminated.
    static int substring (const char * src, const char * to, int src_len = -1);

    // Similar to above but case insensitive.
    static int case_substring (const char * src, const char * to,
                               int src_len = -1);

    // Similar to above but returns non-zero ony if the substring is followed by
    // an allowed termination character for a tag or attribute name like a
    // whitespace, '>', '=' and similar. Note that if src_len is equal to
    // strlen(to) the function could return non-zero even if the name is not yet
    // finished as the function is not allowed to read above src_len to avoid
    // buffer overrun. If \a src_len is shorter than strlen(to) (but positive),
    // the function always returns 0. Negative strlen indicates unknown length
    // but the function never reads after null terminating character so if you
    // know src is null terminated, it is always safe to use negative src_len.
    // Case sensitive.
    static int name_substring (const char * src, const char * to,
                               int src_len = -1);

    // Similar to above but case insensitive.
    static int case_name_substring (const char * src, const char * to,
                                    int src_len = -1);

    // Simple, case-sensitive comparison of two null-termonated strings
    static bool str_equal (const char * src, const char * to);

    // Returns number of consumed characters if the \a word is first word.
    // It might be preceeded by white space(s) and must be terminated by a
    // "not-a-character" or null. Case sensitive.
    static int first_word_equal (const char * str, const char * word,
                                 int str_len);

    // similsr to above but case insensitive
    static int case_first_word_equal (const char * str, const char * word,
                                      int str_len);

    // Gets the the first word from the string by skipping the whitespaces (thus
    // possibly modifying the poiter in \a src) and returns the word length.
    // Note that user can detect if trere are preceeding and/or following
    // whitespaces by:
    // - preceeding if \a src was changed by the call
    // - followinf if the return value and src_len after the call are different
    // (another word might follow)
    static int get_word (const char *& src, int & src_len);

    //
    static int spaces (const char * str, int len);

    // This operator reads next token of the html string.
    // It shud be called repetively as long as it returns a value bigger than 0.

    // It returns either:
    // BODY: value returns everything after previous closing braclet '>' (or
    // beginning of the file string) and next openning bracket '<' (or end of
    // file string) COMMENT: value() returns everything between <!-- and -->
    // QUESTION_COMMENT: value() returns everythineg between <? and ?>
    // UNKNOWN_COMMENT: a value between <! and >, such as <!DOCTYPE ... >
    // CDATA: value returns everything between  <[CDATA[ and ]]>
    // TAG_NAME: name of new tag. Note that closing tags (like </div>) are not
    // distinguished - it can be inspected by querying first chatacter of
    // value() string equal to '/' ATTRIBUTE_NAME: value() returns the (next)
    // name of the attribute for last read tag ATTRIBUTE_VALUE: value() returns
    // the value of the last read attribute. Note tat in a case that the value
    // is quoted, value() and value_length() INCLUDES
    //                  the quotes. So if you want to get unquoted value, you
    //                  can do something like: const char * val =
    //                  parser->value(); int val_len = parser->value_length();
    //                  if(*val == '"' || *val == '\''){
    //                     val++;
    //                     val_len -= 2;
    //                  }
    // END_TAG: indicates ">", end of opened tag
    // END_SELF_CLOSING_TAG: indicates "/>", end of self-closing tag
    // FINISH (0): Correct fininish of the file
    // Any negative value indicates html syntax error, see Result enumerations.
    // value() returns near-by position of the error.
    Result operator() ();

    // Returns the value of the token after calling operator (). Note that the
    // token is NOT null-terminated so use value_length() to determine the sring
    // length.
    const char * value () const
    {
        return val;
    }

    // Returns the length of the token value.
    int value_length () const
    {
        return val_len;
    }

    // The constructor takes the parsed string as a parameter. The string is NOT
    // copied (and not modified during parsing) so it should be available
    // unchenged until parsing is finished.
    Fl_Html_Tokenizer (const char * str)
    {
        reset (str);
    }
    void reset (const char * str)
    {
        in_tag = 0;
        val = 0;
        pos = str;
        val_len = 0;
    }
    virtual ~Fl_Html_Tokenizer ()
    {
    }
};

class Fl_Xml_Parser;
class Fl_Xml_Object;

void destroy_object_tree (Fl_Xml_Object * t);
// This is a base class for all xml/html elements

class Fl_Xml_Object
{
  protected:
    Fl_Xml_Object * next_; // next brother
    Fl_Xml_Object * prev_; // previous brother
    Fl_Xml_Object * aux_;  // first child OR next word for "word"object

  public:
    Fl_Xml_Object * parent;
    std::vector<Fl_Xml_Object *> childTable;

    // Assure virtual destructor.
    // Note that it DOES NOT destroy the children nor the brothers.
    // For that  use destroy_children and destroy_object_list()
    virtual ~Fl_Xml_Object (){};

    void destroy_children ()
    {
        destroy_object_list (children ());
        aux_ = 0;
    }

    static void destroy_object_list (Fl_Xml_Object * t)
    {
        while (t)
        {
            Fl_Xml_Object * n = t->next_;
            t->destroy_children ();
            delete t;
            t = n;
        }
    }

    typedef void (*Iterate_Callback_Function) (Fl_Xml_Object * ob, int level);

    static void iterate_list (Fl_Xml_Object * t, Iterate_Callback_Function fn,
                              int level = 0)
    {
        while (t)
        {
            (*fn) (t, level);
            Fl_Xml_Object * c = t->children ();
            if (c)
                iterate_list (c, fn, level + 1);
            t = t->next ();
        }
    }

    void next (Fl_Xml_Object * n)
    {
        n->prev (this);
        next_ = n;
    }
    void prev (Fl_Xml_Object * p)
    {
        prev_ = p;
    }

    Fl_Xml_Object ** next_address ()
    {
        return &next_;
    } // returns reference (placeholder) of next brother;
    Fl_Xml_Object ** aux_address ()
    {
        return &aux_;
    } // returns reference (placeholder) of children (or next word);

    /**
     * If this object has children, returns a pointer to its first child. Access
     * to other children done by calling next().
     * This is virtual because aux_ is used for some other purposes. For
     * example, Fl_Html_Object uses it for "next word" to be able to concatenate
     * a word if there is no space but only an inline tag separating the two.
     */
    virtual Fl_Xml_Object * children () const
    {
        return aux_;
    }
    Fl_Xml_Object * aux () const
    {
        return aux_;
    } // fast access to children or next word
    Fl_Xml_Object * next () const
    {
        return next_;
    } // next object()
    Fl_Xml_Object * prev () const
    {
        return prev_;
    }

    Fl_Xml_Object * nextAfterChild (Fl_Xml_Object * o)
    {
        auto it = std::find (childTable.begin (), childTable.end (), o);
        if (it != childTable.end () && ++it != childTable.end ())
            return *it;
        else
            return parent ? parent->nextAfterChild (this) : NULL;
    }

    Fl_Xml_Object * nextObject ()
    {
        return childTable.size ()
                   ? childTable[0]
                   : parent ? parent->nextAfterChild (this) : NULL;
    }

    Fl_Xml_Object * prevBeforeChild (Fl_Xml_Object * o)
    {
        auto it = std::find (childTable.begin (), childTable.end (), o);
        if (it != childTable.begin ())
            return *--it;
        else
            return parent ? parent->prevBeforeChild (this) : NULL;
    }

    Fl_Xml_Object * prevObject ()
    {
        return childTable.size ()
                   ? childTable.back ()
                   : parent ? parent->prevBeforeChild (this) : NULL;
    }

    Fl_Xml_Object () : next_ (0), prev_ (0), aux_ (0), parent (0)
    {
    }

    // This should return the null-terminated tag name or 0 for word object
    virtual const char * name () const = 0;

    // This returns true if the \a src compares within its lenght to the tag
    // name. Note that this version is case-sensitive but a subclass can
    // reimplement the function as case insensitive (eg. for html) using
    // Fl_Html_Tokenizer::case_substring() instesd of
    // Fl_Html_Tokenizer::substring().
    virtual bool name_compares (const char * str, int len)
    {
        return (len == Fl_Html_Tokenizer::substring (str, name (), len));
    }

    // This updates the object based on the attribute name/value pair.
    // The parser parameter \p, can be used to inspect  (modify) the parser
    // state but it is usually not required.
    virtual void process_attribute (Fl_Xml_Parser * p, const char * name,
                                    int name_len, const char * value,
                                    int value_len) = 0;

    // This should terurn the string for word objects
    virtual const char * value () const
    {
        return 0;
    };

    // This is for special tags which never have a bodu
    virtual bool no_body () const
    {
        return false;
    }
};

typedef Fl_Xml_Object * (*Html_Object_Cteator) (const char * name, int name_len,
                                                unsigned flags, bool translate);

static const char * error_strings[] = {"No error",
                                       "Bad closing tag",
                                       "Bad closing tag name",
                                       "Unclosed tag",
                                       "Unfinished tag"};

class Fl_Xml_Parser : public Fl_Html_Tokenizer
{
    std::stack<Fl_Xml_Object *> currentParent;
    Fl_Xml_Object ** parent_table_;
    Fl_Xml_Object ***
        parent_table_next_; // address table where next brother should be placed
                            // within the parent tree
    int parent_table_size_;
    int current_parent_;
    Result last_result_;
    Fl_Html_Tokenizer::Special_Character_Table * scht_;

  protected:
    // This pushes current tag as parent for all newly created objects.
    // Note that this function is virtual: a subclass might not allow
    // pushing certain tags as parents (for instance html parser does not allow
    // body in tags like <br> or <hr>)
    virtual void push_parent (Fl_Xml_Object * o);
    // Pops parent stack
    void pop_parent ()
    {
        currentParent.pop ();
        current_parent_--;
    }

    virtual void add_object (Fl_Xml_Object * o)
    {
        if (!o)
            return;
        parent_table_[current_parent_] = o;
        *(parent_table_next_[current_parent_]) = o;
        printf (
            "current parent: %d (%s)\n", current_parent_, typeid (*o).name ());
        if (current_parent_)
            o->prev (parent_table_[current_parent_ - 1]);
        parent_table_next_[current_parent_] = o->next_address ();
        if (currentParent.size ())
        {
            o->parent = currentParent.top ();
            printf ("Childtable pushback o: %p\n", o);
            currentParent.top ()->childTable.push_back (o);
        }
    }

    // This should cteate the object as there woud be no attributes present in
    // the tag.
    virtual Fl_Xml_Object * create_tag (const char * name,
                                        unsigned name_len) = 0;

    // This should cteate a "word" object.
    virtual Fl_Xml_Object * create_word (const char * word, unsigned word_len,
                                         int type = 0) = 0;

    // This should create (and optionally process) a comment string.
    // It can also return null to ignore comment. \a type is a comment type
    // (COMMENT, QUESTION_COMMENT or ONKNOWN_COMMENT), the \a comment string is
    // continouos string between comment boundaries of length \a comment_len. By
    // default comments are ignored.
    virtual Fl_Xml_Object * create_comment (int type, const char * comment,
                                            unsigned comment_len)
    {
        return 0;
    };

    // This is called whenever closing tag does not match current parent() name
    // and should return true if the mismatch was handledand is OK, otherwise it
    // should return false. For instance Fl_Html_Parser overrides this function
    // to automatically close "broken" tags such as <p>
    virtual bool handle_bad_closing_tag ()
    {
        return 0;
    }

    // This is called just before tag close upon </tag> or at automatic close of
    // unmatched "open" tag. The tag closed can be inspected using parent(),
    // last_object() indicates last created object.
    virtual void finish_tag ()
    {
    }

    // This is for exceptional processing of body of special "extraordinary"
    // tags like <pre>
    virtual bool process_body ()
    {
        return false;
    }

  public:
    Fl_Html_Tokenizer::Special_Character_Table *
    special_character_table () const
    {
        return scht_;
    }
    void
    special_character_table (Fl_Html_Tokenizer::Special_Character_Table * t)
    {
        scht_ = t;
    }

    enum Parse_Error
    {
        BAD_CLOSING_TAG = 1, // closing </tag> without openning <tag>
        BAD_CLOSING_TAG_NAME =
            2,              // the names of pair of <tag> and </tag differ>
        UNCLOSED_TAG = 3,   // there is no closong </tag> if it is required
        UNFINISHED_TAG = 4, // tag has no finishing ">" or "/>"
        LAST_ERROR,
        UNKNOWN_ERROR = 255 // something realy terrible happened...
    };

    Fl_Xml_Parser (int table_size = 32)
        : Fl_Html_Tokenizer (0), parent_table_size_ (table_size)
    {
        parent_table_ = new Fl_Xml_Object *[table_size];
        parent_table_next_ = new Fl_Xml_Object **[table_size];
        scht_ = Fl_Html_Tokenizer::Special_Character_Table::default_table ();
    }

    Fl_Xml_Object * last_object () const
    {
        return parent_table_[current_parent_];
    }

    Fl_Xml_Object * parent () const
    {
        if (!current_parent_)
            return 0;
        return parent_table_[current_parent_ - 1];
    }

    Result last_result () const
    {
        return last_result_;
    }

    // This is main function which parses the source \a src.
    // Returns:
    // - 0 on success
    // - negative value on syntax error (see Html_Tokeniser::Error enumerations)
    // - positive error on logical/structure fault (see Parse_Error
    // enumerations) Near-by position of the error can be inspected by the
    // vallue() function. The resulting list/tree structure is filled in \a
    // parse_result and when not needed any more it MUST be destroyed with
    // function Fl_Xml_Object::destroy_object_list() even in a case of an error
    // as an incomplete structure still might be built.
    int parse (const char * src, Fl_Xml_Object ** parse_result);

    static const char * friendly_error (int error)
    {
        if (error >= LAST_ERROR)
            return "Unknown error";
        return error_strings[error];
    }

    virtual ~Fl_Xml_Parser ()
    {
        delete[] parent_table_;
        delete[] parent_table_next_;
    }
};

class Fl_Html_Parser;

////////////////////////////////////////   Fl_Html_Object_ class
///////////////////////////////////////////////////////////
// This class  is a base class for Html_ tags and words but does not have any
// internal data or methods  yet to deal with it.
class Fl_Html_Object_ : public Fl_Xml_Object
{
    // making name comparison case-insensitive
    int index_;

  public:
    typedef Fl_Html_Object_ * (*Create_Tag_Function) ();

    // This is a class which contains all information required for creation
    // of the object-tree for all tags and words.
    class Create_Object_Table : public Fl_Html_Pair_Table
    {
      public:
        Create_Object_Table (Fl_Html_Pair_Table::Entry * const table,
                             int filled, int table_size)
            : Fl_Html_Pair_Table (table, filled, table_size)
        {
        }
        Create_Object_Table (const Create_Object_Table & table)
            : Fl_Html_Pair_Table (table){}; // copy constructor
        Create_Object_Table (int size = 64) : Fl_Html_Pair_Table (size){};

        // This function should define creation of orinary word object with
        // translation basen on special character table \a t. "Special" words
        // (like lines in <pre> tag)could be created if type !=0
        virtual Fl_Html_Object_ *
        create_word (Fl_Html_Tokenizer::Special_Character_Table * t,
                     const char * src, int src_len, int type = 0) const = 0;
        // This function should define creation of a word from CDATA string
        // without translation
        virtual Fl_Html_Object_ * create_word (const char * word,
                                               unsigned word_len) const = 0;

        // This should create unknown tag when the name is not present in the
        // table.
        virtual Fl_Html_Object_ *
        create_unknown_tag (const char * name, unsigned name_len) const = 0;

        void add (const char * name, Create_Tag_Function value)
        {
            add_ (name, (const void *)value);
        };
        Create_Tag_Function replace (const char * name,
                                     Create_Tag_Function * value)
        {
            return (Create_Tag_Function)replace_ (name, value);
        }
        int find (const char * src, Create_Tag_Function & value,
                  int length = -1) const
        {
            return find_case_ (src, (const void *&)value, length);
        }
    };

    // Useful for comparison of attribute name
    static bool is_word (const char * str, const char * to, int str_len)
    {
        return (str_len ==
                Fl_Html_Tokenizer::case_name_substring (str, to, str_len));
    }

    // skips spaces from the beginning of the strung
    static int skip_spaces (const char *& str, int & len)
    {
        int a = Fl_Html_Tokenizer::spaces (str, len);
        str += a;
        len -= a;
        return a;
    }
    // Finds if \a word is first word in the string, on success returns number
    // of consumed characters.
    static int is_first_word (const char * str, const char * word,
                              int str_len = -1)
    {
        return Fl_Html_Tokenizer::case_first_word_equal (str, word, str_len);
    }

    // This is reimplemented to be case-insensitive. Note that str is never
    // null.
    bool name_compares (const char * str, int len)
    {
        const char * n = name ();
        if (!n)
            return false;
        return (len == Fl_Html_Tokenizer::case_substring (str, n, len));
    }

    enum Type
    {
        BLOCK = 1,
        TD = 2,
        TR = 4,
        TABLE = 8,
        LI = 0x10,
        DT = 0x20,
        DD = 0x30
    };

    virtual int type () const = 0;
    virtual int closed_by () const = 0;
    int index () const
    {
        return index_;
    }
    void index (int i)
    {
        index_ = i;
    }

    // Indicates that it can be "open" and aotomatically closed closed by either
    // another immediate starting "similar" (that is which itself is
    // closed_by_similar()) tag of by any unmatched closing tag. Exlamples are
    // <tr>, <td>, <th>. Note that such a tag must be block-level and return
    // trure with block_level().
    //  virtual bool closed_by_similar() const  = 0;

    // This should indicate that the tag (or its end) splits the text by
    // inserting a "virtual" space. if (breaks_word() & 1) is true, it is always
    // block level tag if (breaks_word() & 2) is true (but not &1), it is an
    // alien close-by object (such as inline-table)
    //
    virtual int breaks_word () const
    {
        return BLOCK & type ();
    }

    // Possible task for a tag when it is closed
    virtual void finish (Fl_Xml_Parser *)
    {
    }

    virtual int process_body () const
    {
        return 0;
    }
};

class Fl_Html_Image_;

class Fl_Html_Parser : public Fl_Xml_Parser
{
  public:
    typedef Fl_Html_Image_ * (*Image_Creator) (const char * url, int url_len,
                                               int type);

  protected:
    char * last_word_finish;
    Fl_Html_Object_::Create_Object_Table * create_object_table_;
    Fl_Xml_Object ** last_word_concatenation;
    Fl_Xml_Object * dummy_word_concatenation;
    char * buffer_;
    int buffer_size_;
    char * directory_;
    char * filename_;
    int filename_size_;
    Image_Creator image_creator_;
    int index;

    void add_object (Fl_Xml_Object * o);

    bool process_body ();

  public:
    void last_word_space (char * c)
    {
        last_word_finish = c;
    }
    char * get_buffer (int size)
    {
        if (size > buffer_size_)
        {
            delete[] buffer_;
            buffer_ = new char[size];
            buffer_size_ = size;
        }
        return buffer_;
    }

    void image_creator (Image_Creator ic)
    {
        image_creator_ = ic;
    }
    Image_Creator image_creator () const
    {
        return image_creator_;
    }
    void filename (const char * s);
    const char * filename () const
    {
        return filename_;
    }
    const char * directory () const
    {
        return directory_;
    }
    static Fl_Html_Object_::Create_Object_Table *
    default_create_object_table ();

    // This should create a word object and is called either within tag body or
    // CDATA (this can be inspected using last_result(). Surrounding spaces can
    // be ispected by the position within the body (value(), value_length()).
    Fl_Xml_Object * create_word (const char * word, unsigned word_len,
                                 int type = 0);
    Fl_Xml_Object * create_tag (const char * name, unsigned name_len);
    bool handle_bad_closing_tag ()
    {
        Fl_Html_Object_ * p = (Fl_Html_Object_ *)parent ();
        if (p && p->closed_by ())
            return true;
        if (Fl_Html_Object_::is_word (value (), "/p", value_length ()))
            return true;
        return false;
    }

    int parse (const char * src, Fl_Xml_Object ** parse_result);
    void finish_tag ()
    {
        Fl_Html_Object_ * p = (Fl_Html_Object_ *)parent ();
        int br = p->breaks_word ();
        if (br)
        {
            if (br & 1)
                *last_word_finish = ' ';
            last_word_concatenation = &dummy_word_concatenation;
        }
        p->finish (this);

        // last_word_concatenation = &dummy_word_concatenation;
    }

    // Creates the parser with object creation rules based on the \a table
    Fl_Html_Parser (Fl_Html_Object_::Create_Object_Table * table)
        : create_object_table_ (table), buffer_ (0), buffer_size_ (0),
          directory_ (0), filename_ (0), filename_size_ (0), image_creator_ (0)
    {
    }
    ~Fl_Html_Parser ()
    {
        delete[] buffer_;
        delete[] directory_;
        delete[] filename_;
    }
};

#endif // _ci_Html_Tokenizer_H_
